#ifndef GRAPH_GRAPH_H
#define GRAPH_GRAPH_H

#include "type.h"

void cgraphInit(CGraph *graph, CGraphBool directed, CGraphSize vertCap, CGraphSize edgeCap);
void cgraphRelease(const CGraph *graph);

CGraphId cgraphAddVert(CGraph *graph);
CGraphId cgraphAddEdge(CGraph *graph, CGraphId from, CGraphId to, CGraphBool directed);
void cgraphReserveVert(CGraph *graph, CGraphSize num);
void cgraphDeleteVert(CGraph *graph, CGraphId vid);
void cgraphDeleteEdge(CGraph *graph, CGraphId eid);

void cgraphEdgeTraverse(const CGraph *graph, void *userData,
                        void (*callback)(CGraphId from, CGraphId eid, CGraphId to, void *userData));

#endif // GRAPH_GRAPH_H