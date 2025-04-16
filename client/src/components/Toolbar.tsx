import { useStore } from '../lib/store';
import { Sun, Moon, ZoomIn, ZoomOut, Maximize2, Plus } from 'lucide-react';
import { useReactFlow } from 'reactflow';

const LOGIC_GATES = [
  { type: 'AND', label: 'AND' },
  { type: 'OR', label: 'OR' },
  { type: 'NOT', label: 'NOT' },
  { type: 'XOR', label: 'XOR' },
  { type: 'NAND', label: 'NAND' },
  { type: 'NOR', label: 'NOR' },
] as const;

export function Toolbar() {
  const { isDarkMode, toggleTheme, addNode } = useStore();
  const { zoomIn, zoomOut, fitView, project } = useReactFlow();

  const onAddGate = (type: typeof LOGIC_GATES[number]['type']) => {
    const position = project({ x: window.innerWidth / 2, y: window.innerHeight / 2 });
    addNode({
      id: `${type}-${Date.now()}`,
      type: 'logicGate',
      position,
      data: { type, label: type },
      draggable: true // This enables dragging

    });
  };

  return (
    <div className="flex flex-col gap-4">
      <div className="flex items-center space-x-2 bg-white dark:bg-gray-800 p-2 rounded-lg shadow-lg">
        <button
          onClick={toggleTheme}
          className="p-2 rounded hover:bg-gray-100 dark:hover:bg-gray-700"
          title="Toggle theme"
        >
          {isDarkMode ? <Sun size={20} /> : <Moon size={20} />}
        </button>
        
        <div className="w-px h-6 bg-gray-300 dark:bg-gray-600" />
        
        <button
          onClick={() => zoomIn()}
          className="p-2 rounded hover:bg-gray-100 dark:hover:bg-gray-700"
          title="Zoom in"
        >
          <ZoomIn size={20} />
        </button>
        
        <button
          onClick={() => zoomOut()}
          className="p-2 rounded hover:bg-gray-100 dark:hover:bg-gray-700"
          title="Zoom out"
        >
          <ZoomOut size={20} />
        </button>
        
        <button
          onClick={() => fitView()}
          className="p-2 rounded hover:bg-gray-100 dark:hover:bg-gray-700"
          title="Fit to view"
        >
          <Maximize2 size={20} />
        </button>
      </div>

      <div className="bg-white dark:bg-gray-800 p-2 rounded-lg shadow-lg">
        <h3 className="text-sm font-semibold mb-2 px-2 dark:text-white">Logic Gates</h3>
        <div className="space-y-1">
          {LOGIC_GATES.map((gate) => (
            <button
              key={gate.type}
              onClick={() => onAddGate(gate.type)}
              className="w-full flex items-center gap-2 px-3 py-2 text-sm rounded hover:bg-gray-100 dark:hover:bg-gray-700 dark:text-white"
            >
              <Plus size={16} />
              {gate.label}
            </button>
          ))}
        </div>
      </div>
    </div>
  );
}