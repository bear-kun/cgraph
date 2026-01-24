#include "graph.h"
#include "internal/developer.h"
#include <stdlib.h>
#include <string.h>

void cgraphInit(CGraph *const graph, const CGraphBool directed,
                const CGraphSize vertCap, const CGraphSize edgeCap) {
  graph->vertCap = vertCap;
  graph->edgeCap = edgeCap;
  graph->edgeNum = graph->vertNum = 0;
  graph->vertFree = graph->edgeFree = 0;
  graph->vertResize = graph->edgeResize = NULL;

  CGraphView *const view = VIEW(graph);
  view->vertRange = 0;
  view->vertHead = INVALID_ID;
  view->vertNext = malloc(vertCap * sizeof(CGraphId));
  for (CGraphId i = 0; i < vertCap; i++) view->vertNext[i] = i + 1;

  view->directed = directed;
  view->edgeRange = 0;
  view->edgeHead = malloc(vertCap * sizeof(CGraphId));
  memset(view->edgeHead, INVALID_ID, vertCap * sizeof(CGraphId));
  view->endpoints = malloc(edgeCap * sizeof(CGraphEndpoint));

  if (directed) {
    view->edgeNext = malloc(edgeCap * sizeof(CGraphId));
    for (CGraphId i = 0; i < edgeCap; i++) view->edgeNext[i] = i + 1;
  } else {
    view->edgeNext = malloc(2 * edgeCap * sizeof(CGraphId));
    for (CGraphId i = 0; i < edgeCap; i++) view->edgeNext[2 * i] = 2 * (i + 1);
  }
}

void cgraphCopy(CGraph *dst, const CGraph *src) {
  CGraphView *dstView = VIEW(dst);
  const CGraphView *srcView = VIEW(src);

  *dst = *src;
  dst->vertCap = srcView->vertRange;
  dst->edgeCap = srcView->edgeRange;
  dst->vertResize = dst->edgeResize = NULL;

  dstView->vertNext = malloc(dst->vertCap * sizeof(CGraphId));
  memcpy(dstView->vertNext, srcView->vertNext, dst->vertCap * sizeof(CGraphId));

  dstView->edgeHead = malloc(dst->vertCap * sizeof(CGraphId));
  memcpy(dstView->edgeHead, srcView->edgeHead, dst->vertCap * sizeof(CGraphId));

  const CGraphSize edgeNextSize = (dstView->directed ? 1 : 2) * dst->edgeCap;
  dstView->edgeNext = malloc(edgeNextSize * sizeof(CGraphId));
  memcpy(dstView->edgeNext, srcView->edgeNext, edgeNextSize * sizeof(CGraphId));

  dstView->endpoints = malloc(dst->edgeCap * sizeof(CGraphEndpoint));
  memcpy(dstView->endpoints, srcView->endpoints,
         dst->edgeCap * sizeof(CGraphEndpoint));
}

static void cgraphVertResize(CGraph *graph) {
  CGraphView *const view = VIEW(graph);
  const CGraphSize halfCap = graph->vertCap;
  graph->vertCap *= 2;
  void *mem = realloc(view->vertNext, graph->vertCap * sizeof(CGraphId));
  if (!mem) abort();
  view->vertNext = mem;
  for (CGraphId i = (CGraphId)halfCap; i < graph->vertCap; i++)
    view->vertNext[i] = i + 1;

  mem = realloc(view->edgeHead, graph->vertCap * sizeof(CGraphId));
  if (!mem) abort();
  view->edgeHead = mem;
  memset(view->edgeHead + halfCap, INVALID_ID, halfCap * sizeof(CGraphId));

  if (graph->vertResize) graph->vertResize(graph->vertCap);
}

static void cgraphEdgeResize(CGraph *graph) {
  CGraphView *const view = VIEW(graph);
  const CGraphSize halfCap = graph->edgeCap;
  graph->edgeCap *= 2;
  void *mem = realloc(view->endpoints, graph->edgeCap * sizeof(CGraphEndpoint));
  if (!mem) abort();
  view->endpoints = mem;
  if (view->directed) {
    mem = realloc(view->edgeNext, graph->edgeCap * sizeof(CGraphId));
    if (!mem) abort();
    view->edgeNext = mem;
    for (CGraphId i = (CGraphId)halfCap; i < graph->edgeCap; i++) {
      view->edgeNext[i] = i + 1;
    }
  } else {
    mem = realloc(view->edgeNext, 2 * graph->edgeCap * sizeof(CGraphId));
    if (!mem) abort();
    view->edgeNext = mem;
    for (CGraphId i = (CGraphId)halfCap; i < graph->edgeCap; i++) {
      view->edgeNext[2 * i] = 2 * (i + 1);
    }
  }
  if (graph->edgeResize) graph->edgeResize(graph->edgeCap);
}

void cgraphRelease(const CGraph *const graph) {
  const CGraphView *view = VIEW(graph);
  free(view->vertNext);
  free(view->edgeHead);
  free(view->edgeNext);
  free(view->endpoints);
}

CGraphId cgraphAddVert(CGraph *const graph) {
  if (graph->vertNum == graph->vertCap) cgraphVertResize(graph);

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
  if (graph->edgeNum == graph->edgeCap) cgraphEdgeResize(graph);

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

CGraphId *cgraphFind(CGraphId *next, CGraphId *head, const CGraphId id) {
  CGraphId *predNext = head;
  while (*predNext != INVALID_ID && *predNext != id)
    predNext = next + *predNext;
  return *predNext == INVALID_ID ? 0 : predNext;
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

void cgraphClearEdges(CGraph *graph) {
  const CGraphSize vertCap = graph->vertCap;
  const CGraphSize edgeCap = graph->edgeCap;
  CGraphView *view = VIEW(graph);

  view->edgeRange = 0;
  memset(view->edgeHead, INVALID_ID, vertCap * sizeof(CGraphId));

  if (view->directed) {
    for (CGraphId i = 0; i < edgeCap; i++) view->edgeNext[i] = i + 1;
  } else {
    for (CGraphId i = 0; i < edgeCap; i++) view->edgeNext[2 * i] = 2 * (i + 1);
  }
}

void cgraphClear(CGraph *graph) {
  CGraphView *view = VIEW(graph);
  view->vertRange = 0;
  view->vertHead = INVALID_ID;
  for (CGraphId i = 0; i < graph->vertCap; i++) view->vertNext[i] = i + 1;
  cgraphClearEdges(graph);
}

CGraphId cgraphFindEdgeId(const CGraph *graph, const CGraphId from,
                          const CGraphId to) {
  const CGraphView *view = VIEW(graph);
  for (CGraphId eid = view->edgeHead[from]; eid != INVALID_ID;
       eid = view->edgeNext[eid]) {
    if (view->endpoints[eid].to == to) return eid;
    if (!view->directed && view->endpoints[eid].from == from) return eid;
  }
  return INVALID_ID;
}

void cgraphParseEdgeId(const CGraph *graph, const CGraphId eid, CGraphId *from,
                       CGraphId *to) {
  const CGraphView *view = VIEW(graph);
  const CGraphEndpoint endpoint = view->endpoints[eid];
  *from = endpoint.from;
  *to = endpoint.to;
}

void cgraphTraverseEdgeV(const CGraphView *view, void *userData,
                         void (*callback)(CGraphId, CGraphId, CGraphId,
                                          void *)) {
  for (CGraphId from = view->vertHead; from != INVALID_ID;
       from = view->vertNext[from]) {
    for (CGraphId did = view->edgeHead[from], eid, to; did != INVALID_ID;
         did = view->edgeNext[did]) {
      cgraphIterParseF(view, did, &eid, &to);
      callback(from, eid, to, userData);
    }
  }
}

void cgraphTraverseEdges(const CGraph *graph, void *userData,
                         void (*callback)(CGraphId, CGraphId, CGraphId,
                                          void *)) {
  cgraphTraverseEdgeV(VIEW(graph), userData, callback);
}

void cgraphSetVertResizeCallback(CGraph *graph,
                                 const CGraphResizeCallback callback) {
  graph->vertResize = callback;
}

void cgraphSetEdgeResizeCallback(CGraph *graph,
                                 const CGraphResizeCallback callback) {
  graph->edgeResize = callback;
}