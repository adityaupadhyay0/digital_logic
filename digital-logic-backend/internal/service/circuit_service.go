package service

import (
	"digital-logic-backend/internal/model"
	"digital-logic-backend/internal/repo"
	"errors"

	"go.mongodb.org/mongo-driver/bson/primitive"
)

type CircuitService struct {
	circuitRepo *repo.CircuitRepo
}

func NewCircuitService(circuitRepo *repo.CircuitRepo) *CircuitService {
	return &CircuitService{
		circuitRepo: circuitRepo,
	}
}

type Circuit struct {
	model.Circuit
	indexMap      map[model.GateID]int
	adjList       [][]int
	topoOrder     []int
	cyclic        bool
	inputGates    []model.GateID
	outputGates   []model.GateID
	sequentialGates []model.GateID
	clockGate     model.GateID
}

func NewCircuit(dbCircuit *model.Circuit) *Circuit {
	return &Circuit{
		Circuit: *dbCircuit,
	}
}

func (c *Circuit) getGate(id model.GateID) *model.Gate {
	idx, ok := c.indexMap[id]
	if !ok {
		return nil
	}
	return &c.Gates[idx]
}

func (c *Circuit) buildAdjacencyList() {
	n := len(c.Gates)
	c.adjList = make([][]int, n)
	c.indexMap = make(map[model.GateID]int, n)
	for i, g := range c.Gates {
		c.indexMap[g.ID] = i
	}

	for i := 0; i < n; i++ {
		for _, inp := range c.Gates[i].Inputs {
			j, ok := c.indexMap[inp]
			if ok {
				c.adjList[j] = append(c.adjList[j], i)
			}
		}
	}
}

func (c *Circuit) hasCycle() bool {
	c.buildAdjacencyList()
	n := len(c.Gates)
	vis := make([]bool, n)
	stk := make([]bool, n)

	var dfs func(int) bool
	dfs = func(v int) bool {
		vis[v] = true
		stk[v] = true
		for _, u := range c.adjList[v] {
			if !vis[u] {
				if dfs(u) {
					return true
				}
			} else if stk[u] {
				return true
			}
		}
		stk[v] = false
		return false
	}

	for i := 0; i < n; i++ {
		if !vis[i] {
			if dfs(i) {
				return true
			}
		}
	}
	return false
}

func (c *Circuit) buildTopo() {
	n := len(c.Gates)
	c.buildAdjacencyList()
	inDegree := make([]int, n)
	for u := 0; u < n; u++ {
		for _, v := range c.adjList[u] {
			inDegree[v]++
		}
	}

	q := []int{}
	for i := 0; i < n; i++ {
		if inDegree[i] == 0 {
			q = append(q, i)
		}
	}

	c.topoOrder = []int{}
	for len(q) > 0 {
		u := q[0]
		q = q[1:]
		c.topoOrder = append(c.topoOrder, u)
		for _, v := range c.adjList[u] {
			inDegree[v]--
			if inDegree[v] == 0 {
				q = append(q, v)
			}
		}
	}

	c.cyclic = len(c.topoOrder) < n
	if c.cyclic {
		ordered := make(map[int]bool)
		for _, idx := range c.topoOrder {
			ordered[idx] = true
		}
		for i := 0; i < n; i++ {
			if !ordered[i] {
				c.topoOrder = append(c.topoOrder, i)
			}
		}
	}
}

func (c *Circuit) classifyGates() {
	c.inputGates = []model.GateID{}
	c.outputGates = []model.GateID{}
	c.sequentialGates = []model.GateID{}
	c.clockGate = ""
	for _, g := range c.Gates {
		switch g.Type {
		case model.Input:
			c.inputGates = append(c.inputGates, g.ID)
		case model.Output:
			c.outputGates = append(c.outputGates, g.ID)
		case model.Clock:
			c.clockGate = g.ID
		case model.Dff, model.Dffn, model.Dlatch, model.Tff, model.Jkff:
			c.sequentialGates = append(c.sequentialGates, g.ID)
		}
	}
}

func (c *Circuit) evaluateCyclic() {
	const maxIter = 100
	for iter := 0; iter < maxIter; iter++ {
		changed := false
		for _, idx := range c.topoOrder {
			g := &c.Gates[idx]
			switch g.Type {
			case model.Input, model.Clock, model.Dff, model.Dffn, model.Tff, model.Jkff:
				continue
			case model.Dlatch:
				en := false
				if g.NumInputs > 1 {
					en = c.getGate(g.Inputs[1]).Output
				}
				if en {
					d := c.getGate(g.Inputs[0]).Output
					if d != g.Output {
						g.Output = d
						changed = true
					}
				}
				continue
			}
			a := false
			if g.NumInputs > 0 {
				a = c.getGate(g.Inputs[0]).Output
			}
			b := false
			if g.NumInputs > 1 {
				b = c.getGate(g.Inputs[1]).Output
			}
			out := false
			if g.Type == model.Not {
				out = !a
			} else if g.Type == model.Output {
				out = a
			} else {
				out = c.eval2(g.Type, a, b)
			}
			if out != g.Output {
				g.Output = out
				changed = true
			}
		}
		if !changed {
			break
		}
	}
}

func (c *Circuit) evaluateCombinational() {
	if c.cyclic {
		c.evaluateCyclic()
		return
	}
	for _, idx := range c.topoOrder {
		g := &c.Gates[idx]
		switch g.Type {
		case model.Input, model.Clock, model.Dff, model.Dffn, model.Tff, model.Jkff:
			continue
		case model.Dlatch:
			en := false
			if g.NumInputs > 1 {
				en = c.getGate(g.Inputs[1]).Output
			}
			if en {
				g.Output = c.getGate(g.Inputs[0]).Output
			}
			continue
		}
		a := false
		if g.NumInputs > 0 {
			a = c.getGate(g.Inputs[0]).Output
		}
		b := false
		if g.NumInputs > 1 {
			b = c.getGate(g.Inputs[1]).Output
		}
		if g.Type == model.Not {
			g.Output = !a
		} else if g.Type == model.Output {
			g.Output = a
		} else {
			g.Output = c.eval2(g.Type, a, b)
		}
	}
}

func (c *Circuit) eval2(t model.GateType, a, b bool) bool {
	switch t {
	case model.And:
		return a && b
	case model.Or:
		return a || b
	case model.Xor:
		return a != b
	case model.Xnor:
		return a == b
	case model.Nand:
		return !(a && b)
	case model.Nor:
		return !(a || b)
	}
	return false
}

func (s *CircuitService) CreateCircuit(name, description string, userID primitive.ObjectID, gates []model.Gate) error {
	circuit := &model.Circuit{
		Name:        name,
		Description: description,
		UserID:      userID,
		Gates:       gates,
	}
	return s.circuitRepo.CreateCircuit(circuit)
}

func (s *CircuitService) GetCircuitsByUserID(userID primitive.ObjectID) ([]*model.Circuit, error) {
	return s.circuitRepo.GetCircuitsByUserID(userID)
}

func (s *CircuitService) GetCircuitByID(id primitive.ObjectID) (*model.Circuit, error) {
	return s.circuitRepo.GetCircuitByID(id)
}

func (s *CircuitService) UpdateCircuit(circuit *model.Circuit) error {
	return s.circuitRepo.UpdateCircuit(circuit)
}

func (s *CircuitService) DeleteCircuit(id primitive.ObjectID) error {
	return s.circuitRepo.DeleteCircuit(id)
}

func (s *CircuitService) Simulate(id primitive.ObjectID, inputs map[model.GateID]bool, cycles int) (*Circuit, error) {
	dbCircuit, err := s.circuitRepo.GetCircuitByID(id)
	if err != nil {
		return nil, err
	}

	circuit := NewCircuit(dbCircuit)
	circuit.classifyGates()
	circuit.buildTopo()

	if len(circuit.sequentialGates) == 0 { // Combinational circuit
		if len(inputs) != len(circuit.inputGates) {
			return nil, errors.New("incorrect number of inputs")
		}
		for id, val := range inputs {
			gate := circuit.getGate(id)
			if gate == nil || gate.Type != model.Input {
				return nil, errors.New("invalid input gate")
			}
			gate.Output = val
		}
		circuit.evaluateCombinational()
	} else { // Sequential circuit
		clk := false
		for cyc := 0; cyc < cycles; cyc++ {
			clk = !clk
			if circuit.clockGate != "" {
				circuit.getGate(circuit.clockGate).Output = clk
			}
			circuit.evaluateCombinational()

			for _, id := range circuit.sequentialGates {
				g := circuit.getGate(id)
				currClk := circuit.getGate(circuit.clockGate).Output
				rising := currClk && !g.PrevClk
				falling := !currClk && g.PrevClk
				g.PrevClk = currClk

				switch g.Type {
				case model.Dff:
					if rising {
						g.NextState = circuit.getGate(g.Inputs[0]).Output
					}
				case model.Dffn:
					if falling {
						g.NextState = circuit.getGate(g.Inputs[0]).Output
					}
				case model.Tff:
					if rising {
						t := circuit.getGate(g.Inputs[0]).Output
						if t {
							g.NextState = !g.Output
						} else {
							g.NextState = g.Output
						}
					}
				case model.Jkff:
					if rising {
						j := circuit.getGate(g.Inputs[0]).Output
						k := false
						if g.NumInputs > 1 {
							k = circuit.getGate(g.Inputs[1]).Output
						}
						if !j && !k {
							g.NextState = g.Output
						} else if j && !k {
							g.NextState = true
						} else if !j && k {
							g.NextState = false
						} else {
							g.NextState = !g.Output
						}
					}
				}
			}

			for _, id := range circuit.sequentialGates {
				g := circuit.getGate(id)
				if g.Type != model.Dlatch {
					g.Output = g.NextState
				}
			}
		}
	}

	return circuit, nil
}
