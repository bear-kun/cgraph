#include "internal/developer.h"
#include <stdlib.h>
#include <string.h>

CGraphSize cgraphGetGraphSize() { return sizeof(CGraph); }

void cgraphInit(CGraph *const graph, const CGraphBool directed,
                const CGraphSize vertCap, const CGraphSize edgeCap) {
  graph->vertCap = vertCap;
  graph->edgeCap = edgeCap;
  graph->edgeNum = graph->vertNum = 0;
  graph->vertFree = graph->edgeFree = 0;

  CGraphView *const view = &graph->view;
  view->vertRange = 0;
  view->vertHead = INVALID_ID;
  view->vertNext = malloc(vertCap * sizeof(CGraphId));
  view->vertNext[vertCap - 1] = INVALID_ID;
  for (CGraphId i = (CGraphId)vertCap - 1; i; --i) view->vertNext[i - 1] = i;

  view->directed = directed;
  view->edgeRange = 0;
  view->edgeHead = malloc(vertCap * sizeof(CGraphId));
  memset(view->edgeHead, INVALID_ID, vertCap * sizeof(CGraphId));
  view->endpoints = malloc(edgeCap * sizeof(CGraphEndpoint));

  if (directed) {
    view->edgeNext = malloc(edgeCap * sizeof(CGraphId));
    view->edgeNext[edgeCap - 1] = INVALID_ID;
    for (CGraphId i = (CGraphId)edgeCap - 1; i; --i) view->edgeNext[i - 1] = i;
  } else {
    view->edgeNext = malloc(2 * edgeCap * sizeof(CGraphId));
    view->edgeNext[2 * (edgeCap - 1)] = INVALID_ID;
    for (CGraphId i = 2 * ((CGraphId)edgeCap - 1); i; i -= 2)
      view->edgeNext[i - 2] = i;
  }
}

void cgraphRelease(const CGraph *const graph) {
  const CGraphView *view = VIEW(graph);
  free(view->vertNext);
  free(view->edgeHead);
  free(view->edgeNext);
  free(view->endpoints);
}

CGraphId cgraphAddVert(CGraph *const graph) {
  if (graph->vertNum == graph->vertCap) {
    // realloc
  }

  CGraphView *view = VIEW(graph);
  const CGraphId vid = graph->vertFree;
  cgraphUnlink(view->vertNext, &graph->vertFree);
  cgraphInsert(view->vertNext, &view->vertHead, vid);
  if (vid == view->vertRange) ++view->vertRange;
  ++graph->vertNum;
  return vid;
}

void cgraphReserveVert(CGraph *graph, const CGraphSize num) {
  for (CGraphSize i = 0; i != num; ++i) {
    cgraphAddVert(graph);
  }
}

CGraphId cgraphAddEdge(CGraph *const graph, const CGraphId from,
                       const CGraphId to, const CGraphBool directed) {
  if (graph->edgeNum == graph->edgeCap) {
    // realloc
  }

  CGraphView *view = VIEW(graph);
  const CGraphId did = graph->edgeFree;
  cgraphUnlink(view->edgeNext, &graph->edgeFree);
  cgraphInsertEdge(view, from, did);
  if (!directed) cgraphInsertEdge(view, to, REVERSE(did));

  const CGraphId eid = view->directed ? did : (did >> 1);
  view->endpoints[eid] = (CGraphEndpoint){.to = to, .from = from};
  if (eid == view->edgeRange) ++view->edgeRange;
  ++graph->edgeNum;
  return eid;
}

void cgraphDeleteVert(CGraph *graph, const CGraphId vid) {
  CGraphView *view = VIEW(graph);
  CGraphId *predNext = cgraphFind(view->vertNext, &view->vertHead, vid);
  cgraphUnlink(view->vertNext, predNext);
  cgraphInsert(view->vertNext, &graph->vertFree, vid);
  if (vid == view->vertRange - 1) view->vertRange = vid;
  --graph->vertNum;
}

void cgraphDeleteEdge(CGraph *graph, const CGraphId eid) {
  CGraphView *view = VIEW(graph);
  const CGraphId did = view->directed ? eid : (eid << 1);
  const CGraphId to = view->endpoints[eid].to;
  const CGraphId from = view->endpoints[eid].from;

  CGraphId *predNext = cgraphFind(view->edgeNext, view->edgeHead + from, did);
  cgraphUnlink(view->edgeNext, predNext);
  cgraphInsert(view->edgeNext, &graph->edgeFree, did);
  if (!view->directed) {
    predNext = cgraphFind(view->edgeNext, view->edgeHead + to, REVERSE(did));
    if (predNext) cgraphUnlink(view->edgeNext, predNext);
  }
  if (eid == view->edgeRange - 1) view->edgeRange = eid;
  --graph->edgeNum;
}

void cgraphEdgeTraverseV(const CGraphView *view, void *userData,
                         void (*callback)(CGraphId, CGraphId, CGraphId,
                                          void *)) {
  for (CGraphId from = view->vertHead; from != INVALID_ID;
       from = view->vertNext[from]) {
    for (CGraphId did_ = view->edgeHead[from], eid, to; did_ != INVALID_ID;
         did_ = view->edgeNext[did_]) {
      cgraphIterParseF(view, did_, &eid, &to);
      callback(from, eid, to, userData);
    }
  }
}

void cgraphEdgeTraverse(const CGraph *graph, void *userData,
                        void (*callback)(CGraphId, CGraphId, CGraphId,
                                         void *)) {
  cgraphEdgeTraverseV(VIEW(graph), userData, callback);
}