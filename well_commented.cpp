#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <optional>
#include <queue>
#include <iomanip>
#include <bitset>
#include <cmath>


enum class GateType { 
    // Basic inputs and outputs
    INPUT, CLOCK, OUTPUT,
    
    // Combinational gates
    AND, OR, XOR, XNOR, NAND, NOR, NOT, 
    
    // Sequential elements
    DFF,     // D Flip-Flop (positive edge triggered)
    DFFN,    // D Flip-Flop (negative edge triggered)
    DLATCH,  // D Latch (level sensitive)
    TFF,     // T Flip-Flop (toggle)
    JKFF     // JK Flip-Flop
};

// Convert GateType to string for display
std::string gateTypeToString(GateType type) {
    switch (type) {
        case GateType::INPUT: return "INPUT";
        case GateType::CLOCK: return "CLOCK";
        case GateType::OUTPUT: return "OUTPUT";
        case GateType::AND: return "AND";
        case GateType::OR: return "OR";
        case GateType::XOR: return "XOR";
        case GateType::XNOR: return "XNOR";
        case GateType::NAND: return "NAND";
        case GateType::NOR: return "NOR";
        case GateType::NOT: return "NOT";
        case GateType::DFF: return "DFF";
        case GateType::DFFN: return "DFFN";
        case GateType::DLATCH: return "DLATCH";
        case GateType::TFF: return "TFF";
        case GateType::JKFF: return "JKFF";
        default: return "UNKNOWN";
    }
}

// String ID for each gate
using GateID = std::string;

// Gate representation
struct Gate {
    GateID id;                  // Unique identifier
    GateType type;              // Gate type
    int num_inputs;             // Number of inputs
    std::vector<GateID> inputs; // IDs of input gates
    bool output = false;        // Current output state
    bool next_state = false;    // Next state (for sequential elements)
    bool prev_clk = false;      // Previous clock value for edge detection
};

/**
 * Circuit class containing all gates and simulation logic
 */
class Circuit {
public:
    // Add a gate to the circuit
    void addGate(const Gate& g) {
        indexMap[g.id] = gates.size();
        gates.push_back(g);
    }

    // Get gate by ID
    Gate& getGate(const GateID& id) {
        return gates[indexMap.at(id)];
    }

    // Build topological ordering for combinational evaluation
    void buildTopo() {
        int n = gates.size();
        std::vector<int> indegree(n, 0);
        adjList.assign(n, {});
        
        // Build adjacency list and compute in-degrees
        for (int i = 0; i < n; ++i) {
            for (auto& in : gates[i].inputs) {
                int j = indexMap.at(in);
                adjList[j].push_back(i);
                indegree[i]++;
            }
        }
        
        // Perform topological sort
        std::queue<int> q;
        for (int i = 0; i < n; ++i)
            if (indegree[i] == 0)
                q.push(i);
                
        topoOrder.clear();
        while (!q.empty()) {
            int u = q.front(); q.pop();
            topoOrder.push_back(u);
            for (int v : adjList[u]) {
                if (--indegree[v] == 0)
                    q.push(v);
            }
        }
    }

    // Classify gates by type for simulation
    void classifyGates() {
        inputGates.clear();
        outputGates.clear();
        clockGate = "";
        sequentialGates.clear();
        
        for (auto& gate : gates) {
            if (gate.type == GateType::INPUT)
                inputGates.push_back(gate.id);
            else if (gate.type == GateType::OUTPUT)
                outputGates.push_back(gate.id);
            else if (gate.type == GateType::CLOCK)
                clockGate = gate.id;
            else if (gate.type == GateType::DFF || gate.type == GateType::DFFN || 
                     gate.type == GateType::DLATCH || gate.type == GateType::TFF || 
                     gate.type == GateType::JKFF)
                sequentialGates.push_back(gate.id);
        }
    }

    // Evaluate combinational logic until stable
    void evaluateCombinational() {
        bool changed = false;
        do {
            changed = false;
            // Evaluate gates in topological order
            for (int idx : topoOrder) {
                Gate& g = gates[idx];
                
                // Skip input, clock, and sequential elements in combinational evaluation
                if (g.type == GateType::INPUT || g.type == GateType::CLOCK || 
                    g.type == GateType::DFF || g.type == GateType::DFFN || 
                    g.type == GateType::TFF || g.type == GateType::JKFF)
                    continue;
                    
                // Handle D-Latch separately as it's level sensitive
                if (g.type == GateType::DLATCH) {
                    bool enable = g.num_inputs >= 2 ? getGate(g.inputs[1]).output : false;
                    if (enable) {
                        // When enabled, D-Latch passes the input to output
                        bool d = getGate(g.inputs[0]).output;
                        if (d != g.output) {
                            g.output = d;
                            changed = true;
                        }
                    }
                    continue;
                }

                // Get input values
                bool a = g.num_inputs >= 1 ? getGate(g.inputs[0]).output : false;
                bool b = g.num_inputs >= 2 ? getGate(g.inputs[1]).output : false;
                
                // Compute new output
                bool new_out;
                if (g.type == GateType::NOT)
                    new_out = !a;
                else if (g.type == GateType::OUTPUT)
                    new_out = a;
                else
                    new_out = eval2(g.type, a, b);
                    
                // Update if changed
                if (new_out != g.output) {
                    g.output = new_out;
                    changed = true;
                }
            }
        } while (changed);
    }
    
    // Set input values based on a binary pattern
    void setInputs(int pattern) {
        for (int i = 0; i < inputGates.size(); i++) {
            if (i < 32) { // Avoid overflow
                bool bitValue = (pattern >> i) & 1;
                getGate(inputGates[i]).output = bitValue;
            }
        }
    }
    
    // Check if the circuit is purely combinational (no sequential elements)
    bool isPurelyCombinational() const {
        return sequentialGates.empty();
    }
    
    // Generate and print a truth table for a combinational circuit
    void generateTruthTable() {
        // Make sure gates are classified
        classifyGates();
        
        // Verify this is a suitable circuit for truth table generation
        if (!isPurelyCombinational() && !sequentialGates.empty()) {
            std::cout << "Warning: Truth table generation works best for purely combinational circuits.\n";
            std::cout << "Sequential elements will use their current state values.\n\n";
        }
        
        int inputCount = inputGates.size();
        if (inputCount > 10) {
            std::cout << "Error: Truth table generation limited to circuits with 10 or fewer inputs.\n";
            std::cout << "This circuit has " << inputCount << " inputs, which would generate " 
                      << (1ULL << inputCount) << " rows.\n";
            return;
        }
        
        // Print truth table header
        std::cout << "\nTruth Table for Circuit\n";
        std::cout << "=====================\n";
        
        // Print header row
        for (const auto& input : inputGates) {
            std::cout << std::setw(5) << input;
        }
        std::cout << " | ";
        for (const auto& output : outputGates) {
            std::cout << std::setw(6) << output;
        }
        std::cout << std::endl;
        
        // Print separator
        for (size_t i = 0; i < inputGates.size(); i++) {
            std::cout << "-----";
        }
        std::cout << " | ";
        for (size_t i = 0; i < outputGates.size(); i++) {
            std::cout << "------";
        }
        std::cout << std::endl;
        
        // Compute all possible input combinations
        int combinations = 1 << inputCount;
        for (int i = 0; i < combinations; i++) {
            // Set the inputs according to the binary representation of i
            setInputs(i);
            
            // Evaluate the circuit
            evaluateCombinational();
            
            // Print the input values
            for (const auto& input : inputGates) {
                std::cout << std::setw(5) << getGate(input).output;
            }
            std::cout << " | ";
            
            // Print the output values
            for (const auto& output : outputGates) {
                std::cout << std::setw(6) << getGate(output).output;
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    // Data members
    std::vector<Gate> gates;                   // All gates in circuit
    std::unordered_map<GateID,int> indexMap;   // Map gate ID to array index
    std::vector<int> topoOrder;                // Topological order for evaluation
    
    // Organized references to special gate types
    std::vector<GateID> inputGates;
    std::vector<GateID> outputGates;
    std::vector<GateID> sequentialGates;
    GateID clockGate;

private:
    std::vector<std::vector<int>> adjList;     // Adjacency list for topological sort
    
    /**
     * Evaluate a 2-input gate with the given operation
     */
    bool eval2(GateType t, bool a, bool b) {
        switch (t) {
            case GateType::AND:  return a && b;
            case GateType::OR:   return a || b;
            case GateType::XOR:  return a ^ b;
            case GateType::XNOR: return !(a ^ b);
            case GateType::NAND: return !(a && b);
            case GateType::NOR:  return !(a || b);
            default: return false;
        }
    }
};

/**
 * Run circuit simulation for specified number of cycles
 */
void runSimulation(Circuit& c, int cycles) {
    c.classifyGates();
    
    bool clock = false;
    
    // For each cycle
    for (int cycle = 0; cycle < cycles; ++cycle) {
        clock = !clock;
        
        // 1. Update clock gate
        if (!c.clockGate.empty()) {
            c.getGate(c.clockGate).output = clock;
        }
        
        // 2. Evaluate combinational logic until stable
        c.evaluateCombinational();

        // 3. Update sequential elements
        for (auto& seq_id : c.sequentialGates) {
            Gate& g = c.getGate(seq_id);
            bool clk_val = !c.clockGate.empty() ? c.getGate(c.clockGate).output : clock;
            bool rising_edge = clk_val && !g.prev_clk;
            bool falling_edge = !clk_val && g.prev_clk;
            
            // Save previous clock for edge detection
            g.prev_clk = clk_val;
            
            // Handle different types of sequential elements
            switch (g.type) {
                case GateType::DFF:
                    // D Flip-Flop: Update on rising edge
                    if (rising_edge) {
                        g.next_state = c.getGate(g.inputs[0]).output;
                    }
                    break;
                    
                case GateType::DFFN: 
                    // Negative edge D Flip-Flop: Update on falling edge
                    if (falling_edge) {
                        g.next_state = c.getGate(g.inputs[0]).output;
                    }
                    break;
                    
                case GateType::TFF:
                    // T Flip-Flop: Toggle on rising edge if T=1
                    if (rising_edge) {
                        bool t = c.getGate(g.inputs[0]).output;
                        g.next_state = t ? !g.output : g.output;
                    }
                    break;
                    
                case GateType::JKFF: {
                    // JK Flip-Flop: J=Set, K=Reset
                    if (rising_edge) {
                        bool j = c.getGate(g.inputs[0]).output;
                        bool k = g.num_inputs >= 2 ? c.getGate(g.inputs[1]).output : false;
                        
                        if (!j && !k) {
                            // No change
                            g.next_state = g.output;
                        } else if (j && !k) {
                            // Set
                            g.next_state = true;
                        } else if (!j && k) {
                            // Reset
                            g.next_state = false;
                        } else {
                            // Toggle
                            g.next_state = !g.output;
                        }
                    }
                    break;
                }
                
                default:
                    break;
            }
        }
        
        // Apply all next states at once
        for (auto& seq_id : c.sequentialGates) {
            Gate& g = c.getGate(seq_id);
            if (g.type != GateType::DLATCH) { // D-Latch is already handled above
                g.output = g.next_state;
            }
        }

        // Print cycle state
        std::cout << "Cycle " << std::setw(3) << cycle << " CLK=" << clock;
        
        // Print inputs
        for (auto& in_id : c.inputGates) {
            std::cout << " " << in_id << "=" << c.getGate(in_id).output;
        }
        
        // Print sequential elements
        for (auto& seq_id : c.sequentialGates) {
            std::cout << " " << seq_id << "=" << c.getGate(seq_id).output;
        }
        
        // Print outputs
        for (auto& out_id : c.outputGates) {
            std::cout << " " << out_id << "=" << c.getGate(out_id).output;
        }
        
        std::cout << std::endl;
    }
}

/**
 * Run a purely combinational circuit without clock
 * This is optimized for truth table generation
 */
void runCombinationalCircuit(Circuit& c) {
    c.classifyGates();
    
    // Check if suitable for combinational simulation
    if (!c.sequentialGates.empty()) {
        std::cout << "Warning: Circuit contains sequential elements, ";
        std::cout << "which may not behave as expected without a clock.\n";
    }
    
    std::cout << "Combinational Circuit Analysis\n";
    std::cout << "=============================\n";
    
    // Generate truth table
    c.generateTruthTable();
}

/**
 * Create a simple combinational circuit for demonstration
 */
Circuit createCombinationalDemo() {
    Circuit c;
    
    // Define inputs
    c.addGate({"A", GateType::INPUT, 0, {}, false});
    c.addGate({"B", GateType::INPUT, 0, {}, false});
    c.addGate({"C", GateType::INPUT, 0, {}, false});
    
    // Combinational gates
    c.addGate({"AND1", GateType::AND, 2, {"A","B"}});
    c.addGate({"OR1", GateType::OR, 2, {"B","C"}});
    c.addGate({"XOR1", GateType::XOR, 2, {"AND1","OR1"}});
    c.addGate({"NOT1", GateType::NOT, 1, {"C"}});
    c.addGate({"AND2", GateType::AND, 2, {"XOR1","NOT1"}});
    
    // Define outputs
    c.addGate({"OUT1", GateType::OUTPUT, 1, {"XOR1"}});
    c.addGate({"OUT2", GateType::OUTPUT, 1, {"AND2"}});
    
    // Build topological ordering
    c.buildTopo();
    
    return c;
}

/**
 * Create a mixed circuit with both combinational and sequential elements
 */
Circuit createMixedCircuit() {
    Circuit c;
    
    // Define inputs
    c.addGate({"A", GateType::INPUT, 0, {}, true});   // Input A (initially true)
    c.addGate({"B", GateType::INPUT, 0, {}, false});  // Input B (initially false)
    
    // Clock
    c.addGate({"CLK", GateType::CLOCK, 0, {}, false});
    
    // Sequential elements
    c.addGate({"DFF1", GateType::DFF, 1, {"AND1"}, false});       // D Flip-Flop (pos edge)
    c.addGate({"DFFN1", GateType::DFFN, 1, {"OR1"}, false});      // D Flip-Flop (neg edge)
    c.addGate({"DLATCH1", GateType::DLATCH, 2, {"XOR1", "B"}, false}); // D Latch with B as enable
    c.addGate({"TFF1", GateType::TFF, 1, {"B"}, false});          // T Flip-Flop controlled by B
    c.addGate({"JKFF1", GateType::JKFF, 2, {"A", "B"}, false});   // JK Flip-Flop with J=A, K=B
    
    // Combinational gates
    c.addGate({"AND1", GateType::AND, 2, {"A","B"}});
    c.addGate({"OR1", GateType::OR, 2, {"A","B"}});
    c.addGate({"XOR1", GateType::XOR, 2, {"A","B"}});
    c.addGate({"NOT1", GateType::NOT, 1, {"AND1"}});
    
    // Define outputs
    c.addGate({"OUT_AND", GateType::OUTPUT, 1, {"AND1"}});
    c.addGate({"OUT_OR", GateType::OUTPUT, 1, {"OR1"}});
    c.addGate({"OUT_DFF1", GateType::OUTPUT, 1, {"DFF1"}});
    c.addGate({"OUT_DFFN1", GateType::OUTPUT, 1, {"DFFN1"}});
    c.addGate({"OUT_TFF1", GateType::OUTPUT, 1, {"TFF1"}});
    c.addGate({"OUT_JKFF1", GateType::OUTPUT, 1, {"JKFF1"}});

    // Build topological ordering
    c.buildTopo();
    
    return c;
}

/**
 * Main function demonstrating circuit construction and simulation
 */
int main(int argc, char* argv[]) {
    bool runComboDemo = (argc > 1 && std::string(argv[1]) == "--combo");
    
    if (runComboDemo) {
        // Create and run a purely combinational circuit with truth table
        Circuit comboCircuit = createCombinationalDemo();
        runCombinationalCircuit(comboCircuit);
    } else {
        // Create and run the mixed circuit with sequential elements
        Circuit mixedCircuit = createMixedCircuit();
        
        std::cout << "Digital Circuit Simulation\n";
        std::cout << "=========================\n";
        runSimulation(mixedCircuit, 10);
        
        // After running the simulation, generate a truth table
        // (Note: This will use current state of sequential elements)
        std::cout << "\nGenerating truth table for outputs based on current state:\n";
        mixedCircuit.generateTruthTable();
    }

    return 0;
}
