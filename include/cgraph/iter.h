#ifndef GRAPH_ITER_H
#define GRAPH_ITER_H

#include "types.h"

// explorer
CGraphExplorer *cgraphGetExplorer(const CGraph *graph, CGraphBool dir);
void cgraphExplorerRelease(CGraphExplorer *iter);

void cgraphExplorerResetVert(CGraphExplorer *iter);
void cgraphExplorerResetEdge(CGraphExplorer *iter, CGraphId vid);
void cgraphExplorerResetAllEdges(CGraphExplorer *iter, CGraphBool dir);

CGraphBool cgraphExplorerNextVert(CGraphExplorer *iter, CGraphId *vid);
// reverse of the insertion order
CGraphBool cgraphExplorerNextEdge(CGraphExplorer *iter, CGraphId vid, CGraphId *eid,
                                  CGraphId *other);

// iter
CGraphIter cgraphGetVertIter(const CGraph *graph);
CGraphIter cgraphGetEdgeIter(const CGraph *graph, CGraphId vid, CGraphBool dir);

CGraphBool cgraphIterNextVert(CGraphIter *iter, CGraphId *vid);
// reverse of the insertion order
CGraphBool cgraphIterNextEdge(CGraphIter *iter, CGraphId *eid, CGraphId *other);

void cgraphTraverseEdges(const CGraph *graph, void *data,
                         void (*callback)(CGraphId from, CGraphId eid, CGraphId to, void *data));

#endif // GRAPH_ITER_H