import { useCallback } from 'react';
import ReactFlow, {
  Background,
  Controls,
  MiniMap,
  Panel,
  useReactFlow,
  applyNodeChanges,
} from 'reactflow';
import { useStore } from '../lib/store';
import { LogicGate } from './LogicGate';
import { History } from './History';
import { Toolbar } from './Toolbar';

const nodeTypes = {
  logicGate: LogicGate,
};

export function Canvas() {
  const {
    nodes,
    edges,
    isDarkMode,
    zoom,
    position,
    addNode,
    removeNode,
    addEdge,
    removeEdge,
    updateZoom,
    updatePosition,
  } = useStore();

  const { fitView } = useReactFlow();

  const onNodesChange = useCallback((changes: any) => {
    const updatedNodes = applyNodeChanges(changes, nodes);
    changes.forEach((change: any) => {
      if (change.type === 'remove') {
        removeNode(change.id);
      }
    });
    // Update nodes with new positions
    updatedNodes.forEach(node => {
      if (node.position) {
        const existingNode = nodes.find(n => n.id === node.id);
        if (existingNode && (
          existingNode.position.x !== node.position.x ||
          existingNode.position.y !== node.position.y
        )) {
          addNode({
            ...node,
            position: node.position
          });
        }
      }
    });
  }, [nodes, removeNode, addNode]);

  const onEdgesChange = useCallback((changes: any) => {
    changes.forEach((change: any) => {
      if (change.type === 'remove') {
        removeEdge(change.id);
      }
    });
  }, [removeEdge]);

  const onConnect = useCallback((params: any) => {
    addEdge({
      id: `e${params.source}-${params.target}`,
      source: params.source,
      target: params.target,
      type: 'smoothstep',
    });
  }, [addEdge]);

  return (
    <div className={`w-full h-screen ${isDarkMode ? 'dark' : ''}`}>
      <ReactFlow
        nodes={nodes}
        edges={edges}
        onNodesChange={onNodesChange}
        onEdgesChange={onEdgesChange}
        onConnect={onConnect}
        nodeTypes={nodeTypes}
        onMoveEnd={(_, zoom) => {
          updateZoom(zoom);
          updatePosition(position.x, position.y);
        }}
        fitView
      >
        <Background />
        <Controls />
        <MiniMap />
        <Panel position="top-left">
          <Toolbar />
        </Panel>
        <Panel position="top-right">
          <History />
        </Panel>
      </ReactFlow>
    </div>
  );
}