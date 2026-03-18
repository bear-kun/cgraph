#ifndef GRAPH_ITER_H
#define GRAPH_ITER_H

#include "types.h"

CGraphIter *cgraphGetIter(const CGraph *graph);
void cgraphIterRelease(CGraphIter *iter);

void cgraphIterResetVert(CGraphIter *iter);
void cgraphIterResetEdge(CGraphIter *iter, CGraphId from);// INVALID_ID -> reset all

void cgraphIterCurr(const CGraphIter *iter, CGraphId *from, CGraphId *eid, CGraphId *to);
CGraphBool cgraphIterNextVert(CGraphIter *iter, CGraphId *vid);
CGraphBool cgraphIterNextEdge(CGraphIter *iter, CGraphId from, CGraphId *eid, CGraphId *to);


CGraphIterLite cgraphGetVertIter(const CGraph *graph);
CGraphIterLite cgraphGetEdgeIter(const CGraph *graph, CGraphId from);
CGraphBool cgraphIterLiteNextVert(CGraphIterLite *iter, CGraphId *vid);
CGraphBool cgraphIterLiteNextEdge(CGraphIterLite *iter, CGraphId *eid, CGraphId *to);

#endif // GRAPH_ITER_H
