#ifndef GRAPH_ITER_H
#define GRAPH_ITER_H

#include "types.h"

CGraphIter *cgraphGetIter(const CGraph *graph);
void cgraphIterRelease(CGraphIter *iter);

void cgraphIterResetVert(CGraphIter *iter);
void cgraphIterResetEdge(CGraphIter *iter, CGraphId from);// INVALID_ID -> reset all

CGraphBool cgraphIterNextVert(CGraphIter *iter, CGraphId *vid);
CGraphBool cgraphIterNextEdge(CGraphIter *iter, CGraphId from, CGraphId *eid, CGraphId *to);

// lite
CGraphIterLite cgraphGetVertIter(const CGraph *graph);
CGraphIterLite cgraphGetEdgeIter(const CGraph *graph, CGraphId from);
CGraphIterLite cgraphGetEdgeIterRev(const CGraph *graph, CGraphId to);

CGraphBool cgraphIterLiteNextVert(CGraphIterLite *iter, CGraphId *vid);
CGraphBool cgraphIterLiteNextEdge(CGraphIterLite *iter, CGraphId *eid, CGraphId *to);
CGraphBool cgraphIterLiteNextEdgeRev(CGraphIterLite *iter, CGraphId *eid, CGraphId *from);

void cgraphTraverseEdges(const CGraph *graph, void *userData,
                         void (*callback)(CGraphId from, CGraphId eid, CGraphId to, void *userData));

#endif // GRAPH_ITER_H
