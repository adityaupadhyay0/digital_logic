import React from 'react';
import { useStore } from '../lib/store';
import { Undo2, Redo2 } from 'lucide-react';

export function History() {
  const { history, historyIndex, undo, redo } = useStore();

  return (
    <div className="bg-white dark:bg-gray-800 p-4 rounded-lg shadow-lg w-64">
      <div className="flex justify-between items-center mb-4">
        <h3 className="font-semibold dark:text-white">History</h3>
        <div className="flex space-x-2">
          <button
        
    
      
      
  
      
      
      

      

        
            onClick={undo}
            disabled={historyIndex < 0}
            className="p-1 rounded hover:bg-gray-100 dark:hover:bg-gray-700 disabled:opacity-50"
          >
            <Undo2 size={16} />
          </button>
          <button
            onClick={redo}
            disabled={historyIndex >= history.length - 1}
            className="p-1 rounded hover:bg-gray-100 dark:hover:bg-gray-700 disabled:opacity-50"
          >
            <Redo2 size={16} />
          </button>
        </div>
      </div>

      <div className="space-y-2 max-h-48 overflow-y-auto">
        {history.map((action, index) => (
          <div
            key={action.timestamp}
            className={`
              p-2 rounded text-sm
              ${index === historyIndex ? 'bg-blue-100 dark:bg-blue-900' : ''}
              dark:text-white
            `}
          >
            {action.description}
          </div>
        ))}
      </div>
    </div>
  );
}