import { create } from 'zustand';
import { Node, Edge } from 'reactflow';

interface HistoryAction {
  type: 'add-node' | 'remove-node' | 'add-edge' | 'remove-edge' | 'move-node';
  description: string;
  data: any;
  timestamp: number;
}

interface SimulatorState {
  nodes: Node[];
  edges: Edge[];
  isDarkMode: boolean;
  history: HistoryAction[];
  historyIndex: number;
  zoom: number;
  position: { x: number; y: number };
  selectedNodes: string[];
  
  // Actions
  addNode: (node: Node) => void;
  removeNode: (nodeId: string) => void;
  addEdge: (edge: Edge) => void;
  removeEdge: (edgeId: string) => void;
  toggleTheme: () => void;
  undo: () => void;
  redo: () => void;
  updateZoom: (zoom: number) => void;
  updatePosition: (x: number, y: number) => void;
  selectNode: (nodeId: string) => void;
}

export const useStore = create<SimulatorState>((set, get) => ({
  nodes: [],
  edges: [],
  isDarkMode: false,
  history: [],
  historyIndex: -1,
  zoom: 1,
  position: { x: 0, y: 0 },
  selectedNodes: [],

  addNode: (node) => {
    set((state) => {
      const newHistory = state.history.slice(0, state.historyIndex + 1);
      return {
        nodes: [...state.nodes, node],
        history: [...newHistory, {
          type: 'add-node',
          description: `Added ${node.type} gate`,
          data: node,
          timestamp: Date.now()
        }],
        historyIndex: state.historyIndex + 1
      };
    });
  },

  removeNode: (nodeId) => {
    set((state) => {
      const node = state.nodes.find(n => n.id === nodeId);
      if (!node) return state;
      
      const newHistory = state.history.slice(0, state.historyIndex + 1);
      return {
        nodes: state.nodes.filter(n => n.id !== nodeId),
        edges: state.edges.filter(e => e.source !== nodeId && e.target !== nodeId),
        history: [...newHistory, {
          type: 'remove-node',
          description: `Removed ${node.type} gate`,
          data: node,
          timestamp: Date.now()
        }],
        historyIndex: state.historyIndex + 1
      };
    });
  },

  addEdge: (edge) => {
    set((state) => {
      const newHistory = state.history.slice(0, state.historyIndex + 1);
      return {
        edges: [...state.edges, edge],
        history: [...newHistory, {
          type: 'add-edge',
          description: 'Added connection',
          data: edge,
          timestamp: Date.now()
        }],
        historyIndex: state.historyIndex + 1
      };
    });
  },

  removeEdge: (edgeId) => {
    set((state) => {
      const edge = state.edges.find(e => e.id === edgeId);
      if (!edge) return state;

      const newHistory = state.history.slice(0, state.historyIndex + 1);
      return {
        edges: state.edges.filter(e => e.id !== edgeId),
        history: [...newHistory, {
          type: 'remove-edge',
          description: 'Removed connection',
          data: edge,
          timestamp: Date.now()
        }],
        historyIndex: state.historyIndex + 1
      };
    });
  },

  toggleTheme: () => set((state) => ({ isDarkMode: !state.isDarkMode })),

  undo: () => {
    const state = get();
    if (state.historyIndex < 0) return;

    const action = state.history[state.historyIndex];
    set((state) => {
      let newNodes = [...state.nodes];
      let newEdges = [...state.edges];

      switch (action.type) {
        case 'add-node':
          newNodes = newNodes.filter(n => n.id !== action.data.id);
          break;
        case 'remove-node':
          newNodes = [...newNodes, action.data];
          break;
        case 'add-edge':
          newEdges = newEdges.filter(e => e.id !== action.data.id);
          break;
        case 'remove-edge':
          newEdges = [...newEdges, action.data];
          break;
      }

      return {
        nodes: newNodes,
        edges: newEdges,
        historyIndex: state.historyIndex - 1
      };
    });
  },

  redo: () => {
    const state = get();
    if (state.historyIndex >= state.history.length - 1) return;

    const action = state.history[state.historyIndex + 1];
    set((state) => {
      let newNodes = [...state.nodes];
      let newEdges = [...state.edges];

      switch (action.type) {
        case 'add-node':
          newNodes = [...newNodes, action.data];
          break;
        case 'remove-node':
          newNodes = newNodes.filter(n => n.id !== action.data.id);
          break;
        case 'add-edge':
          newEdges = [...newEdges, action.data];
          break;
        case 'remove-edge':
          newEdges = newEdges.filter(e => e.id !== action.data.id);
          break;
      }

      return {
        nodes: newNodes,
        edges: newEdges,
        historyIndex: state.historyIndex + 1
      };
    });
  },

  updateZoom: (zoom) => set({ zoom }),
  updatePosition: (x, y) => set({ position: { x, y } }),
  selectNode: (nodeId) => set((state) => ({
    selectedNodes: state.selectedNodes.includes(nodeId)
      ? state.selectedNodes.filter(id => id !== nodeId)
      : [...state.selectedNodes, nodeId]
  }))
}));