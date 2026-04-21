#ifndef GRAPH_GRAPH_H
#define GRAPH_GRAPH_H

#include "types.h"

void cgraphInit(CGraph *graph, CGraphBool directed, CGraphSize vertCap, CGraphSize edgeCap);
void cgraphRelease(const CGraph *graph);
void cgraphCopy(CGraph *dst, const CGraph *src);
void cgraphCopyVert(CGraph *dst, const CGraph *src);
void cgraphClearEdges(CGraph *graph);
void cgraphClear(CGraph *graph);

CGraphId cgraphAddVert(CGraph *graph);
void cgraphReserveVert(CGraph *graph, CGraphSize num);
void cgraphDeleteVert(CGraph *graph, CGraphId vid);

CGraphId cgraphAddEdge(CGraph *graph, CGraphId from, CGraphId to);
CGraphId cgraphPushEdgeBack(CGraph *graph, CGraphId from, CGraphId to);
void cgraphReverseEdge(const CGraph *graph, CGraphId eid);
void cgraphDeleteEdge(CGraph *graph, CGraphId eid);
CGraphId cgraphFindEdge(const CGraph *graph, CGraphId from, CGraphId to);
CGraphId cgraphWhereEdgeFrom(const CGraph *graph, CGraphId eid);
CGraphId cgraphWhereEdgeTo(const CGraph *graph, CGraphId eid);

void cgraphSetVertResizeCallback(CGraph *graph, CGraphResizeCallback callback);
void cgraphSetEdgeResizeCallback(CGraph *graph, CGraphResizeCallback callback);

#endif // GRAPH_GRAPH_H