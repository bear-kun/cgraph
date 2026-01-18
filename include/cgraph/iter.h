#ifndef GRAPH_ITER_H
#define GRAPH_ITER_H

#include "type.h"

CGraphIter *cgraphGetIter(const CGraph *graph);
void cgraphIterRelease(CGraphIter *iter);
void cgraphIterResetVert(CGraphIter *iter);
// 重置迭代边，INVALID_ID -> 重置全部
void cgraphIterResetEdge(CGraphIter *iter, CGraphId from);
void cgraphIterCurr(const CGraphIter *iter, CGraphId *from, CGraphId *eid,
                    CGraphId *to);
CGraphBool cgraphIterNextVert(CGraphIter *iter, CGraphId *vid);
CGraphBool cgraphIterNextEdge(CGraphIter *iter, CGraphId from, CGraphId *eid,
                              CGraphId *to);

#endif // GRAPH_ITER_H
