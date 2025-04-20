#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <queue>
#include <iomanip>
#include <set>
#include <stack>

//======================
// Gate and Circuit Definitions
//======================

enum class GateType { 
    INPUT, CLOCK, OUTPUT,
    AND, OR, XOR, XNOR, NAND, NOR, NOT,
    DFF, DFFN, DLATCH, TFF, JKFF
};

std::string gateTypeToString(GateType type) {
    switch (type) {
        case GateType::INPUT:  return "INPUT";
        case GateType::CLOCK:  return "CLOCK";
        case GateType::OUTPUT: return "OUTPUT";
        case GateType::AND:    return "AND";
        case GateType::OR:     return "OR";
        case GateType::XOR:    return "XOR";
        case GateType::XNOR:   return "XNOR";
        case GateType::NAND:   return "NAND";
        case GateType::NOR:    return "NOR";
        case GateType::NOT:    return "NOT";
        case GateType::DFF:    return "DFF";
        case GateType::DFFN:   return "DFFN";
        case GateType::DLATCH: return "DLATCH";
        case GateType::TFF:    return "TFF";
        case GateType::JKFF:   return "JKFF";
        default:               return "UNKNOWN";
    }
}

using GateID = std::string;

struct Gate {
    GateID id;
    GateType type;
    int num_inputs;
    std::vector<GateID> inputs;
    bool output = false;
    bool next_state = false;
    bool prev_clk = false;
};

class Circuit {
public:
    void addGate(const Gate& g) {
        indexMap[g.id] = gates.size();
        gates.push_back(g);
    }

    Gate& getGate(const GateID& id) {
        return gates.at(indexMap.at(id));
    }

    bool hasCycle() {
        buildAdjacencyList();
        int n = gates.size();
        std::vector<bool> vis(n), stk(n);
        for (int i = 0; i < n; ++i)
            if (!vis[i] && dfsCycle(i, vis, stk))
                return true;
        return false;
    }

    void buildTopo() {
        int n = gates.size();
        buildAdjacencyList();
        std::vector<int> indegree(n);
        for (int u = 0; u < n; ++u)
            for (int v : adjList[u])
                indegree[v]++;
        std::queue<int> q;
        for (int i = 0; i < n; ++i)
            if (indegree[i] == 0)
                q.push(i);
        topoOrder.clear();
        while (!q.empty()) {
            int u = q.front(); q.pop();
            topoOrder.push_back(u);
            for (int v : adjList[u])
                if (--indegree[v] == 0)
                    q.push(v);
        }
        cyclic = (topoOrder.size() < n);
        if (cyclic) {
            std::set<int> ordered(topoOrder.begin(), topoOrder.end());
            for (int i = 0; i < n; ++i)
                if (!ordered.count(i))
                    topoOrder.push_back(i);
        }
    }

    void classifyGates() {
        inputGates.clear(); outputGates.clear(); sequentialGates.clear(); clockGate.clear();
        for (auto& g : gates) {
            if (g.type == GateType::INPUT)
                inputGates.push_back(g.id);
            else if (g.type == GateType::OUTPUT)
                outputGates.push_back(g.id);
            else if (g.type == GateType::CLOCK)
                clockGate = g.id;
            else if (g.type == GateType::DFF || g.type == GateType::DFFN ||
                     g.type == GateType::DLATCH || g.type == GateType::TFF ||
                     g.type == GateType::JKFF)
                sequentialGates.push_back(g.id);
        }
    }

    void evaluateCombinational() {
        if (cyclic) { evaluateCyclic(); return; }
        for (int idx : topoOrder) {
            auto& g = gates[idx];
            if (g.type==GateType::INPUT || g.type==GateType::CLOCK ||
                g.type==GateType::DFF   || g.type==GateType::DFFN  ||
                g.type==GateType::TFF   || g.type==GateType::JKFF)
                continue;
            if (g.type == GateType::DLATCH) {
                bool en = (g.num_inputs>1 ? getGate(g.inputs[1]).output : false);
                if (en) g.output = getGate(g.inputs[0]).output;
                continue;
            }
            bool a = (g.num_inputs>0 ? getGate(g.inputs[0]).output : false);
            bool b = (g.num_inputs>1 ? getGate(g.inputs[1]).output : false);
            if (g.type == GateType::NOT)      g.output = !a;
            else if (g.type == GateType::OUTPUT) g.output = a;
            else                                g.output = eval2(g.type, a, b);
        }
    }

    void setInputs(int pattern) {
        for (int i = 0; i < (int)inputGates.size() && i < 32; ++i)
            getGate(inputGates[i]).output = ((pattern >> i) & 1);
    }

    bool isPurelyCombinational() const {
        return sequentialGates.empty();
    }

    void generateTruthTable() {
        classifyGates(); buildTopo();
        int n = inputGates.size();
        if (n > 10) {
            std::cerr << "Error: Too many inputs (" << n << ") for truth table.";
            return;
        }
        std::cout << "\nTruth Table:\n";
        for (auto &id: inputGates)  std::cout << std::setw(5) << id;
        std::cout << " | ";
        for (auto &id: outputGates) std::cout << std::setw(6) << id;
        std::cout << "\n";
        for (int pat = 0; pat < (1 << n); ++pat) {
            setInputs(pat);
            evaluateCombinational();
            for (auto &id: inputGates)  std::cout << std::setw(5) << getGate(id).output;
            std::cout << " | ";
            for (auto &id: outputGates) std::cout << std::setw(6) << getGate(id).output;
            std::cout << "\n";
        }
    }

private:
    std::vector<Gate> gates;
    std::unordered_map<GateID,int> indexMap;
    std::vector<std::vector<int>> adjList;
    std::vector<int> topoOrder;
    bool cyclic = false;
    std::vector<GateID> inputGates, outputGates, sequentialGates;
    GateID clockGate;

    bool dfsCycle(int v, std::vector<bool>& vis, std::vector<bool>& stk) {
        vis[v] = stk[v] = true;
        for (int u : adjList[v])
            if (!vis[u] ? dfsCycle(u, vis, stk) : stk[u])
                return true;
        stk[v] = false;
        return false;
    }

    void buildAdjacencyList() {
        int n = gates.size();
        adjList.assign(n, {});
        for (int i = 0; i < n; ++i)
            for (auto& in : gates[i].inputs)
                adjList[indexMap[in]].push_back(i);
    }

    void evaluateCyclic() {
        const int MAX_ITER = 100;
        for (int iter = 0; iter < MAX_ITER; ++iter) {
            bool changed = false;
            for (int idx : topoOrder) {
                auto& g = gates[idx];
                if (g.type==GateType::INPUT || g.type==GateType::CLOCK ||
                    g.type==GateType::DFF   || g.type==GateType::DFFN  ||
                    g.type==GateType::TFF   || g.type==GateType::JKFF)
                    continue;
                if (g.type == GateType::DLATCH) {
                    bool en = (g.num_inputs>1 ? getGate(g.inputs[1]).output : false);
                    if (en) {
                        bool d = getGate(g.inputs[0]).output;
                        if (d != g.output) { g.output = d; changed = true; }
                    }
                    continue;
                }
                bool a = (g.num_inputs>0 ? getGate(g.inputs[0]).output : false);
                bool b = (g.num_inputs>1 ? getGate(g.inputs[1]).output : false);
                bool out = (g.type==GateType::NOT ? !a :
                            g.type==GateType::OUTPUT ? a : eval2(g.type, a, b));
                if (out != g.output) { g.output = out; changed = true; }
            }
            if (!changed) break;
            if (iter == MAX_ITER - 1)
                std::cerr << "Warning: Circuit did not converge after fixed-point iterations.\n";
        }
    }

    bool eval2(GateType t, bool a, bool b) {
        switch (t) {
            case GateType::AND:  return a && b;
            case GateType::OR:   return a || b;
            case GateType::XOR:  return a ^ b;
            case GateType::XNOR: return !(a ^ b);
            case GateType::NAND: return !(a && b);
            case GateType::NOR:  return !(a || b);
            default:             return false;
        }
    }
};

//======================
// Simulation Routines
//======================

void runSimulation(Circuit& c, int cycles) {
    c.classifyGates();
    c.buildTopo();
    std::cout << "\nStarting simulation for " << cycles << " cycles...\n";
    bool clk = false;
    for (int cyc = 0; cyc < cycles; ++cyc) {
        clk = !clk;
        if (!c.clockGate.empty())
            c.getGate(c.clockGate).output = clk;
        c.evaluateCombinational();
        // Update sequential elements
        for (auto& id : c.sequentialGates) {
            Gate& g = c.getGate(id);
            bool curr_clk = c.getGate(c.clockGate).output;
            bool rising  = curr_clk && !g.prev_clk;
            bool falling = !curr_clk && g.prev_clk;
            g.prev_clk = curr_clk;
            switch (g.type) {
                case GateType::DFF:
                    if (rising) g.next_state = c.getGate(g.inputs[0]).output;
                    break;
                case GateType::DFFN:
                    if (falling) g.next_state = c.getGate(g.inputs[0]).output;
                    break;
                case GateType::TFF:
                    if (rising) {
                        bool t = c.getGate(g.inputs[0]).output;
                        g.next_state = t ? !g.output : g.output;
                    }
                    break;
                case GateType::JKFF: {
                    if (rising) {
                        bool j = c.getGate(g.inputs[0]).output;
                        bool k = (g.num_inputs>1 ? c.getGate(g.inputs[1]).output : false);
                        if (!j && !k)       g.next_state = g.output;
                        else if (j && !k)  g.next_state = true;
                        else if (!j && k)  g.next_state = false;
                        else                g.next_state = !g.output;
                    }
                    break;
                }
                default:
                    break;
            }
        }
        // Apply new states
        for (auto& id : c.sequentialGates) {
            if (c.getGate(id).type != GateType::DLATCH)
                c.getGate(id).output = c.getGate(id).next_state;
        }
        // Print status
        std::cout << "Cycle " << std::setw(3) << cyc << " CLK=" << clk;
        for (auto& id : c.inputGates)      std::cout << " " << id << "=" << c.getGate(id).output;
        for (auto& id : c.sequentialGates) std::cout << " " << id << "=" << c.getGate(id).output;
        for (auto& id : c.outputGates)     std::cout << " " << id << "=" << c.getGate(id).output;
        std::cout << "\n";
    }
}

void runCombinationalCircuit(Circuit& c) {
    c.classifyGates();
    c.buildTopo();
    if (!c.sequentialGates.empty())
        std::cerr << "Warning: Sequential elements present, behaviour may not be purely combinational.\n";
    c.generateTruthTable();
}

//======================
// Main Entry Point
//======================

int main(int argc, char* argv[]) {
    Circuit circuit;
    // TODO: add gates here or load netlist from file

    circuit.buildTopo();
    if (circuit.isPurelyCombinational()) {
        runCombinationalCircuit(circuit);
    } else {
        int cycles = 10;
        if (argc > 1) cycles = std::stoi(argv[1]);
        runSimulation(circuit, cycles);
    }
    return 0;
}
