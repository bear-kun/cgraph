#ifndef GRAPH_ITER_H
#define GRAPH_ITER_H

#include "types.h"

// explorer
CGraphExplorer *cgraphGetExplorer(const CGraph *graph);
CGraphExplorer *cgraphGetExplorerRev(const CGraph *graph);
void cgraphExplorerRelease(CGraphExplorer *iter);

void cgraphExplorerResetVert(CGraphExplorer *iter);
void cgraphExplorerResetEdge(CGraphExplorer *iter, CGraphId from);
void cgraphExplorerResetAllEdges(CGraphExplorer *iter);
void cgraphExplorerResetEdgeRev(CGraphExplorer *iter, CGraphId to);
void cgraphExplorerResetAllEdgesRev(CGraphExplorer *iter);

// reverse of the insertion order
CGraphBool cgraphExplorerNextVert(CGraphExplorer *iter, CGraphId *vid);
// reverse of the insertion order
CGraphBool cgraphExplorerNextEdge(CGraphExplorer *iter, CGraphId from, CGraphId *eid, CGraphId *to);
// reverse of the insertion order
CGraphBool cgraphExplorerNextEdgeRev(CGraphExplorer *iter, CGraphId to, CGraphId *eid,
                                     CGraphId *from);
// iter
CGraphIter cgraphGetVertIter(const CGraph *graph);
CGraphIter cgraphGetEdgeIter(const CGraph *graph, CGraphId from);
CGraphIter cgraphGetEdgeIterRev(const CGraph *graph, CGraphId to);

// reverse of the insertion order
CGraphBool cgraphIterNextVert(CGraphIter *iter, CGraphId *vid);
// reverse of the insertion order
CGraphBool cgraphIterNextEdge(CGraphIter *iter, CGraphId *eid, CGraphId *to);
// reverse of the insertion order
CGraphBool cgraphIterNextEdgeRev(CGraphIter *iter, CGraphId *eid, CGraphId *from);

void cgraphTraverseEdges(const CGraph *graph, void *data,
                         void (*callback)(CGraphId from, CGraphId eid, CGraphId to, void *data));

#endif // GRAPH_ITER_H