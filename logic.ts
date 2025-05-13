export enum GateType {
    INPUT = "INPUT",
    CLOCK = "CLOCK",
    OUTPUT = "OUTPUT",
    AND = "AND",
    OR = "OR",
    XOR = "XOR",
    XNOR = "XNOR",
    NAND = "NAND",
    NOR = "NOR",
    NOT = "NOT",
    DFF = "DFF",
    DFFN = "DFFN",
    DLATCH = "DLATCH",
    TFF = "TFF",
    JKFF = "JKFF",
}

export type GateID = string;

export interface Gate {
    id: GateID;
    type: GateType;
    numInputs: number;
    inputs: GateID[];
    output: boolean;
    nextState: boolean;
    prevClk: boolean;
}

export class Circuit {
    private gates: Gate[] = [];
    private indexMap: Map<GateID, number> = new Map();
    private adjList: number[][] = [];
    private topoOrder: number[] = [];
    private cyclic = false;
    private inputGates: GateID[] = [];
    private outputGates: GateID[] = [];
    private sequentialGates: GateID[] = [];
    private clockGate: GateID | "" = "";

    public addGate(g: Gate): void {
        this.indexMap.set(g.id, this.gates.length);
        this.gates.push({ ...g, output: false, nextState: false, prevClk: false });
    }

    public getGate(id: GateID): Gate {
        const idx = this.indexMap.get(id);
        if (idx === undefined) throw new Error(`Gate ${id} not found`);
        return this.gates[idx];
    }

    public hasCycle(): boolean {
        this.buildAdjacencyList();
        const n = this.gates.length;
        const vis = Array(n).fill(false);
        const stk = Array(n).fill(false);
        const dfs = (v: number): boolean => {
            vis[v] = true;
            stk[v] = true;
            for (const u of this.adjList[v]) {
                if (!vis[u] && dfs(u)) return true;
                if (stk[u]) return true;
            }
            stk[v] = false;
            return false;
        };
        for (let i = 0; i < n; i++) if (!vis[i] && dfs(i)) return true;
        return false;
    }

    public buildTopo(): void {
        const n = this.gates.length;
        this.buildAdjacencyList();
        const indegree = Array(n).fill(0);
        for (let u = 0; u < n; u++) {
            for (const v of this.adjList[u]) indegree[v]++;
        }
        const q: number[] = [];
        for (let i = 0; i < n; i++) if (indegree[i] === 0) q.push(i);
        this.topoOrder = [];
        while (q.length) {
            const u = q.shift()!;
            this.topoOrder.push(u);
            for (const v of this.adjList[u]) {
                if (--indegree[v] === 0) q.push(v);
            }
        }
        this.cyclic = this.topoOrder.length < n;
        if (this.cyclic) {
            const ordered = new Set(this.topoOrder);
            for (let i = 0; i < n; i++) if (!ordered.has(i)) this.topoOrder.push(i);
        }
    }

    public classifyGates(): void {
        this.inputGates = [];
        this.outputGates = [];
        this.sequentialGates = [];
        this.clockGate = "";
        for (const g of this.gates) {
            switch (g.type) {
                case GateType.INPUT: this.inputGates.push(g.id); break;
                case GateType.OUTPUT: this.outputGates.push(g.id); break;
                case GateType.CLOCK: this.clockGate = g.id; break;
                case GateType.DFF:
                case GateType.DFFN:
                case GateType.DLATCH:
                case GateType.TFF:
                case GateType.JKFF:
                    this.sequentialGates.push(g.id);
                    break;
                default: break;
            }
        }
    }

    public evaluateCombinational(): void {
        if (this.cyclic) return this.evaluateCyclic();
        for (const idx of this.topoOrder) {
            const g = this.gates[idx];
            if ([GateType.INPUT, GateType.CLOCK, GateType.DFF, GateType.DFFN, GateType.TFF, GateType.JKFF].includes(g.type))
                continue;
            if (g.type === GateType.DLATCH) {
                const en = g.numInputs > 1 ? this.getGate(g.inputs[1]).output : false;
                if (en) g.output = this.getGate(g.inputs[0]).output;
                continue;
            }
            const a = g.numInputs > 0 ? this.getGate(g.inputs[0]).output : false;
            const b = g.numInputs > 1 ? this.getGate(g.inputs[1]).output : false;
            if (g.type === GateType.NOT) g.output = !a;
            else if (g.type === GateType.OUTPUT) g.output = a;
            else g.output = this.eval2(g.type, a, b);
        }
    }

    public setInputs(pattern: number): void {
        for (let i = 0; i < this.inputGates.length && i < 32; i++) {
            const g = this.getGate(this.inputGates[i]);
            g.output = Boolean((pattern >> i) & 1);
        }
    }

    public isPurelyCombinational(): boolean {
        return this.sequentialGates.length === 0;
    }

    public generateTruthTable(): void {
        this.classifyGates();
        this.buildTopo();
        const n = this.inputGates.length;
        if (n > 10) throw new Error(`Too many inputs (${n}) for truth table.`);
        console.log(`\nTruth Table:`);
        console.log([...this.inputGates.map(id => id.padStart(5))].join(""), " | ", [...this.outputGates.map(id => id.padStart(6))].join(""));
        for (let pat = 0; pat < (1 << n); pat++) {
            this.setInputs(pat);
            this.evaluateCombinational();
            console.log(
                [...this.inputGates.map(id => String(Number(this.getGate(id).output)).padStart(5))].join(""),
                " | ",
                [...this.outputGates.map(id => String(Number(this.getGate(id).output)).padStart(6))].join("")
            );
        }
    }

    private buildAdjacencyList(): void {
        const n = this.gates.length;
        this.adjList = Array.from({ length: n }, () => []);
        for (let i = 0; i < n; i++) {
            for (const inp of this.gates[i].inputs) {
                const j = this.indexMap.get(inp);
                if (j !== undefined) this.adjList[j].push(i);
            }
        }
    }

    private eval2(t: GateType, a: boolean, b: boolean): boolean {
        switch (t) {
            case GateType.AND: return a && b;
            case GateType.OR: return a || b;
            case GateType.XOR: return a !== b;
            case GateType.XNOR: return a === b;
            case GateType.NAND: return !(a && b);
            case GateType.NOR: return !(a || b);
            default: return false;
        }
    }

    private evaluateCyclic(): void {
        const MAX_ITER = 100;
        for (let iter = 0; iter < MAX_ITER; iter++) {
            let changed = false;
            for (const idx of this.topoOrder) {
                const g = this.gates[idx];
                if ([GateType.INPUT, GateType.CLOCK, GateType.DFF, GateType.DFFN, GateType.TFF, GateType.JKFF].includes(g.type))
                    continue;
                if (g.type === GateType.DLATCH) {
                    const en = g.numInputs > 1 ? this.getGate(g.inputs[1]).output : false;
                    if (en) {
                        const d = this.getGate(g.inputs[0]).output;
                        if (d !== g.output) { g.output = d; changed = true; }
                    }
                    continue;
                }
                const a = g.numInputs > 0 ? this.getGate(g.inputs[0]).output : false;
                const b = g.numInputs > 1 ? this.getGate(g.inputs[1]).output : false;
                const out = g.type === GateType.NOT ? !a : g.type === GateType.OUTPUT ? a : this.eval2(g.type, a, b);
                if (out !== g.output) { g.output = out; changed = true; }
            }
            if (!changed) break;
            if (iter === MAX_ITER - 1) console.warn(`Circuit did not converge after ${MAX_ITER} iterations.`);
        }
    }
}

// Simulation functions
export function runSimulation(c: Circuit, cycles: number = 10): void {
    c.classifyGates();
    c.buildTopo();
    console.log(`\nStarting simulation for ${cycles} cycles...`);
    let clk = false;
    for (let cyc = 0; cyc < cycles; cyc++) {
        clk = !clk;
        if (c.clockGate) c.getGate(c.clockGate).output = clk;
        c.evaluateCombinational();
        // Update sequential
        for (const id of c.sequentialGates) {
            const g = c.getGate(id);
            const currClk = c.getGate(c.clockGate).output;
            const rising = currClk && !g.prevClk;
            const falling = !currClk && g.prevClk;
            g.prevClk = currClk;
            switch (g.type) {
                case GateType.DFF:
                    if (rising) g.nextState = c.getGate(g.inputs[0]).output;
                    break;
                case GateType.DFFN:
                    if (falling) g.nextState = c.getGate(g.inputs[0]).output;
                    break;
                case GateType.TFF:
                    if (rising) {
                        const t = c.getGate(g.inputs[0]).output;
                        g.nextState = t ? !g.output : g.output;
                    }
                    break;
                case GateType.JKFF:
                    if (rising) {
                        const j = c.getGate(g.inputs[0]).output;
                        const k = g.numInputs > 1 ? c.getGate(g.inputs[1]).output : false;
                        if (!j && !k) g.nextState = g.output;
                        else if (j && !k) g.nextState = true;
                        else if (!j && k) g.nextState = false;
                        else g.nextState = !g.output;
                    }
                    break;
            }
        }
        // Apply new states
        for (const id of c.sequentialGates) {
            const g = c.getGate(id);
            if (g.type !== GateType.DLATCH) g.output = g.nextState;
        }
        // Print status
        const state = [`Cycle ${cyc.toString().padStart(3)} CLK=${clk}`];
        for (const id of c.inputGates) state.push(`${id}=${c.getGate(id).output}`);
        for (const id of c.sequentialGates) state.push(`${id}=${c.getGate(id).output}`);
        for (const id of c.outputGates) state.push(`${id}=${c.getGate(id).output}`);
        console.log(state.join(" "));
    }
}

export function runCombinationalCircuit(c: Circuit): void {
    c.classifyGates();
    c.buildTopo();
    if (c.sequentialGates.length)
        console.warn("Warning: Sequential elements present, behaviour may not be purely combinational.");
    c.generateTruthTable();
}
