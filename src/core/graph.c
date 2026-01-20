#include "internal/developer.h"
#include <stdlib.h>
#include <string.h>

#define DID(eid) ((eid) << 1 | (eid) >> (sizeof(CGraphId) * 8 - 1))

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
    for (CGraphId i = 0; i < edgeCap; i++)
      view->edgeNext[2 * i] = 2 * (i + 1);
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
  CGraphView *view = VIEW(graph);
  if (graph->vertNum == graph->vertCap) {
    const CGraphSize oldCap = graph->vertCap;
    graph->vertCap *= 2;
    void *mem = realloc(view->vertNext, graph->vertCap * sizeof(CGraphId));
    view->vertNext = mem;
    for (CGraphId i = (CGraphId)oldCap; i < graph->vertCap; i++) {
      view->vertNext[i] = i + 1;
    }
    if (graph->vertResize) graph->vertResize(graph->vertCap);
  }

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
  CGraphView *view = VIEW(graph);
  if (graph->edgeNum == graph->edgeCap) {
    const CGraphSize oldCap = graph->edgeCap;
    graph->edgeCap *= 2;
    void *mem = realloc(view->edgeHead, graph->edgeCap * sizeof(CGraphId));
    view->edgeHead = mem;
    memset(view->edgeHead + oldCap, INVALID_ID, graph->edgeCap - oldCap);
    mem = realloc(view->endpoints, graph->edgeCap * sizeof(CGraphEndpoint));
    view->endpoints = mem;
    if (directed) {
      mem = realloc(view->edgeNext, graph->edgeCap * sizeof(CGraphId));
      view->edgeNext = mem;
      for (CGraphId i = (CGraphId)oldCap; i < graph->edgeCap; i++) {
        view->edgeNext[i] = i + 1;
      }
    } else {
      mem = realloc(view->edgeNext, 2 * graph->edgeCap * sizeof(CGraphId));
      view->edgeNext = mem;
      for (CGraphId i = (CGraphId)oldCap; i < graph->edgeCap; i++) {
        view->edgeNext[2 * i] = 2 * (i + 1);
      }
      if (graph->edgeResize) graph->edgeResize(graph->edgeCap);
    }
  }

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

void cgraphCopyEdgeV(const CGraphView *view, const CGraphView *copy) {
  switch (view->directed << 1 | copy->directed) {
  case 3:
    memcpy(copy->edgeHead, view->edgeHead, copy->vertRange * sizeof(CGraphId));
    memcpy(copy->edgeNext, view->edgeNext, copy->edgeRange * sizeof(CGraphId));
    break;
  case 0:
    memcpy(copy->edgeHead, view->edgeHead, copy->vertRange * sizeof(CGraphId));
    memcpy(copy->edgeNext, view->edgeNext,
           2 * copy->edgeRange * sizeof(CGraphId));
    break;
  case 2:
    for (CGraphSize i = 0; i < copy->vertRange; ++i)
      copy->edgeHead[i] = DID(view->edgeHead[i]);
    for (CGraphSize i = 0; i < copy->edgeRange; ++i)
      copy->edgeNext[i << 1] = DID(view->edgeNext[i]);
    break;
  default:;
  }
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

void cgraphSetVertResizeCallback(CGraph *graph,
                                 const CGraphResizeCallback callback) {
  graph->vertResize = callback;
}
void cgraphSetEdgeResizeCallback(CGraph *graph,
                                 const CGraphResizeCallback callback) {
  graph->edgeResize = callback;
}
