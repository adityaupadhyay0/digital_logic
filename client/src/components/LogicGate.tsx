// LogicGate.tsx
import { memo } from 'react';

import { Handle, Position } from 'reactflow';

interface LogicGateProps {
  data: {
    label: string;
    type: 'AND' | 'OR' | 'NOT' | 'XOR' | 'NAND' | 'NOR';
  };
  selected: boolean;
}

export const LogicGate = memo(({ data, selected }: LogicGateProps) => {
  const isInverter = data.type === 'NOT';

  return (
    <div
      className={`
        px-4 py-2 rounded-lg shadow-lg border-2
        ${selected ? 'border-blue-500' : 'border-gray-300'}
        dark:bg-gray-800 bg-white
        transition-colors duration-200
      `}
    >
      <Handle
        type="target"
        position={Position.Left}
        className="w-2 h-2 !bg-blue-500"
      />
      
      {!isInverter && (
        <Handle
          type="target"
          position={Position.Left}
          className="w-2 h-2 !bg-blue-500"
          style={{ top: '70%' }}
        />
      )}

      <div className="font-mono font-bold text-sm dark:text-white">
        {data.label}
      </div>

      <Handle
        type="source"
        position={Position.Right}
        className="w-2 h-2 !bg-green-500"
      />
    </div>
  );
});