#ifndef GRAPH_GRAPH_H
#define GRAPH_GRAPH_H

#include "types.h"

void cgraphInit(CGraph *graph, CGraphBool directed, CGraphSize vertCap, CGraphSize edgeCap);
void cgraphRelease(const CGraph *graph);
void cgraphCopy(CGraph *dst, const CGraph *src);
void cgraphCopyVertices(CGraph *dst, const CGraph *src);
void cgraphClearEdges(CGraph *graph);
void cgraphClear(CGraph *graph);

CGraphId cgraphAddVert(CGraph *graph);
void cgraphAddVertices(CGraph *graph, CGraphSize count);
void cgraphDeleteVert(CGraph *graph, CGraphId vid);

CGraphId cgraphAddEdge(CGraph *graph, CGraphId from, CGraphId to);
void cgraphAddEdges(CGraph *graph, CGraphSize count, const CGraphId endpoints[][2]);
// do not check validity
void cgraphDeleteEdge(CGraph *graph, CGraphId eid);
// do not check validity
void cgraphReverseEdge(const CGraph *graph, CGraphId eid);
// do not check validity
CGraphId cgraphFindEdge(const CGraph *graph, CGraphId from, CGraphId to);
// do not check validity
void cgraphWhereEdgeFromTo(const CGraph *graph, CGraphId eid, CGraphId *from, CGraphId *to);

void cgraphSetVertResizeCallback(CGraph *graph, CGraphResizeCallback callback);
void cgraphSetEdgeResizeCallback(CGraph *graph, CGraphResizeCallback callback);

void cgraphSaveBinary(const CGraph *graph, const char *path);
void cgraphLoadBinary(CGraph *graph, const char *path); // Don't init graph !

#endif // GRAPH_GRAPH_H