#include "graph.h"
#include <stdlib.h>
#include <string.h>

#define REVERSE(did) ((did) ^ 1)
#define DID(eid) ((eid) << 1 | (eid) >> (sizeof(CGraphId) * 8 - 1))
#define EDGE_NEXT_SIZE(edgeCap, directed) (directed ? edgeCap : 2 * edgeCap)

static void *allocIds(const size_t size) {
  void *mem = malloc(size * sizeof(CGraphId));
  if (!mem) abort();
  return mem;
}

static void *reallocIds(void *memory, const size_t newSize) {
  void *mem = realloc(memory, newSize * sizeof(CGraphId));
  if (!mem) abort();
  return mem;
}

static void reserveGraph(CGraph *graph, const CGraphSize vertCap,
                         const CGraphSize edgeCap, const CGraphBool directed) {
  graph->vertCap = vertCap;
  graph->vertNext = allocIds(vertCap);
  graph->vertResize = NULL;

  graph->directed = directed;
  graph->edgeCap = edgeCap;
  graph->edgeHead = allocIds(vertCap);
  graph->edgeNext = allocIds(EDGE_NEXT_SIZE(edgeCap, directed));
  graph->edgeFrom = allocIds(edgeCap);
  graph->edgeTo = allocIds(edgeCap);
  graph->edgeResize = NULL;
}

void cgraphInit(CGraph *const graph, const CGraphBool directed,
                const CGraphSize vertCap, const CGraphSize edgeCap) {
  reserveGraph(graph, vertCap, edgeCap, directed);
  cgraphClear(graph);
}

void cgraphCopyVert(CGraph *dst, const CGraph *src) {
  *dst = *src;
  reserveGraph(dst, src->vertRange, src->edgeRange, src->directed);

  cgraphClearEdges(dst);
  memcpy(dst->vertNext, src->vertNext, dst->vertCap * sizeof(CGraphId));
}

void cgraphCopy(CGraph *dst, const CGraph *src) {
  *dst = *src;
  reserveGraph(dst, src->vertRange, src->edgeRange, src->directed);

  const CGraphSize edgeNextSize = EDGE_NEXT_SIZE(dst->edgeCap, dst->directed);
  memcpy(dst->vertNext, src->vertNext, dst->vertCap * sizeof(CGraphId));
  memcpy(dst->edgeHead, src->edgeHead, dst->vertCap * sizeof(CGraphId));
  memcpy(dst->edgeNext, src->edgeNext, edgeNextSize * sizeof(CGraphId));
  memcpy(dst->edgeFrom, src->edgeFrom, dst->edgeCap * sizeof(CGraphId));
  memcpy(dst->edgeTo, src->edgeTo, dst->edgeCap * sizeof(CGraphId));
}

static void initNextList(CGraphId *list, const CGraphSize start,
                         const CGraphSize end) {
  for (CGraphSize i = start; i < end; i++) list[i] = (CGraphId)(i + 1);
}

static void initNextList2(CGraphId *list, const CGraphSize start,
                          const CGraphSize end) {
  for (CGraphSize i = start; i < end; i++) list[2 * i] = (CGraphId)(2 * i + 2);
}

void cgraphClearEdges(CGraph *graph) {
  graph->edgeNum = 0;
  graph->edgeFree = 0;
  graph->edgeRange = 0;
  memset(graph->edgeHead, INVALID_ID, graph->vertCap * sizeof(CGraphId));
  if (graph->directed) {
    initNextList(graph->edgeNext, 0, graph->edgeCap);
  } else {
    initNextList2(graph->edgeNext, 0, graph->edgeCap);
  }
}

void cgraphClear(CGraph *graph) {
  graph->vertNum = 0;
  graph->vertFree = 0;
  graph->vertRange = 0;
  graph->vertHead = INVALID_ID;
  initNextList(graph->vertNext, 0, graph->vertCap);
  cgraphClearEdges(graph);
}

static void cgraphVertResize(CGraph *graph) {
  const CGraphSize oldCap = graph->vertCap;
  const CGraphSize newCap = (graph->vertCap + 1) << 1;

  graph->vertNext = reallocIds(graph->vertNext, newCap);
  initNextList(graph->vertNext, oldCap, newCap);

  graph->edgeHead = reallocIds(graph->edgeHead, newCap);
  memset(graph->edgeHead + oldCap, INVALID_ID,
         (newCap - oldCap) * sizeof(CGraphId));

  graph->vertCap = newCap;
  if (graph->vertResize) graph->vertResize(oldCap, newCap);
}

static void cgraphEdgeResize(CGraph *graph) {
  const CGraphSize oldCap = graph->edgeCap;
  const CGraphSize newCap = (graph->edgeCap + 1) << 1;

  const CGraphSize edgeNextSize = EDGE_NEXT_SIZE(newCap, graph->directed);
  graph->edgeNext = reallocIds(graph->edgeNext, edgeNextSize);
  graph->edgeFrom = reallocIds(graph->edgeFrom, newCap);
  graph->edgeTo = reallocIds(graph->edgeTo, newCap);

  if (graph->directed) {
    initNextList(graph->edgeNext, oldCap, newCap);
  } else {
    initNextList2(graph->edgeNext, oldCap, newCap);
  }

  graph->edgeCap = newCap;
  if (graph->edgeResize) graph->edgeResize(oldCap, newCap);
}

void cgraphRelease(const CGraph *const graph) {
  free(graph->vertNext);
  free(graph->edgeHead);
  free(graph->edgeNext);
  free(graph->edgeFrom);
  free(graph->edgeTo);
}

static void listUnlink(const CGraphId *next, CGraphId *predNext) {
  *predNext = next[*predNext];
}

static void listInsert(CGraphId *next, CGraphId *predNext,
                       const CGraphId id) {
  next[id] = *predNext;
  *predNext = id;
}

static void cgraphInsertEdge(const CGraph *graph, const CGraphId from,
                             const CGraphId did) {
  listInsert(graph->edgeNext, graph->edgeHead + from, did);
}

static CGraphId *listFind(CGraphId *next, CGraphId *head, const CGraphId id) {
  for (CGraphId *ptr = head; *ptr != INVALID_ID; ptr = next + *ptr) {
    if (*ptr == id) return ptr;
  }
  return NULL;
}

CGraphId cgraphAddVert(CGraph *const graph) {
  if (graph->vertNum == graph->vertCap) cgraphVertResize(graph);

  const CGraphId vid = graph->vertFree;
  listUnlink(graph->vertNext, &graph->vertFree);
  listInsert(graph->vertNext, &graph->vertHead, vid);
  if (vid == graph->vertRange) graph->vertRange++;
  graph->vertNum++;
  return vid;
}

void cgraphReserveVert(CGraph *graph, const CGraphSize num) {
  for (CGraphSize i = 0; i != num; ++i) {
    cgraphAddVert(graph);
  }
}

CGraphId cgraphAddEdge(CGraph *const graph, const CGraphId from,
                       const CGraphId to) {
  if (graph->edgeNum == graph->edgeCap) cgraphEdgeResize(graph);

  const CGraphId did = graph->edgeFree;
  listUnlink(graph->edgeNext, &graph->edgeFree);
  cgraphInsertEdge(graph, from, did);
  if (!graph->directed) cgraphInsertEdge(graph, to, REVERSE(did));

  const CGraphId eid = graph->directed ? did : (did >> 1);
  graph->edgeFrom[eid] = from;
  graph->edgeTo[eid] = to;
  if (eid == graph->edgeRange) graph->edgeRange++;
  graph->edgeNum++;
  return eid;
}

CGraphId cgraphPushEdgeBack(CGraph *const graph, const CGraphId from,
                            const CGraphId to) {
  if (graph->edgeNum == graph->edgeCap) cgraphEdgeResize(graph);

  const CGraphId eid = graph->edgeFree;
  listUnlink(graph->edgeNext, &graph->edgeFree);
  CGraphId *back = graph->edgeHead + from;
  while (*back != INVALID_ID) back = graph->edgeNext + *back;
  listInsert(graph->edgeNext, back, eid);

  graph->edgeFrom[eid] = from;
  graph->edgeTo[eid] = to;
  if (eid == graph->edgeRange) graph->edgeRange++;
  graph->edgeNum++;
  return eid;
}

void cgraphDeleteVert(CGraph *graph, const CGraphId vid) {
  CGraphId *predNext = listFind(graph->vertNext, &graph->vertHead, vid);
  if (!predNext) return;

  listUnlink(graph->vertNext, predNext);
  listInsert(graph->vertNext, &graph->vertFree, vid);
  if (vid == graph->vertRange - 1) graph->vertRange--;
  graph->vertNum--;
}

void cgraphDeleteEdge(CGraph *graph, const CGraphId eid) {
  const CGraphId did = graph->directed ? eid : (eid << 1);
  const CGraphId from = graph->edgeFrom[eid];

  CGraphId *predNext = listFind(graph->edgeNext, graph->edgeHead + from, did);
  if (!predNext) return;

  listUnlink(graph->edgeNext, predNext);
  listInsert(graph->edgeNext, &graph->edgeFree, did);

  if (!graph->directed) {
    const CGraphId to = graph->edgeTo[eid];
    predNext = listFind(graph->edgeNext, graph->edgeHead + to, REVERSE(did));
    listUnlink(graph->edgeNext, predNext);
  }

  if (eid == graph->edgeRange - 1) graph->edgeRange--;
  graph->edgeNum--;
}

void cgraphReverseEdge(const CGraph *const graph, const CGraphId eid) {
  if (!graph->directed) return;

  const CGraphId from = graph->edgeFrom[eid];
  const CGraphId to = graph->edgeTo[eid];

  CGraphId *predNext = listFind(graph->edgeNext, graph->edgeHead + from, eid);
  if (!predNext) return;

  listUnlink(graph->edgeNext, predNext);
  cgraphInsertEdge(graph, to, eid);

  graph->edgeFrom[eid] = to;
  graph->edgeTo[eid] = from;
}

CGraphId cgraphFindEdgeId(const CGraph *graph, const CGraphId from,
                          const CGraphId to) {
  for (CGraphId eid = graph->edgeHead[from]; eid != INVALID_ID;
       eid = graph->edgeNext[eid]) {
    if (graph->edgeTo[eid] == to) return eid;
    if (!graph->directed && graph->edgeFrom[eid] == from) return eid;
  }
  return INVALID_ID;
}

void cgraphParseEdgeId(const CGraph *graph, const CGraphId eid, CGraphId *from,
                       CGraphId *to) {
  if (from) *from = graph->edgeFrom[eid];
  if (to) *to = graph->edgeTo[eid];
}

void cgraphSetVertResizeCallback(CGraph *graph,
                                 const CGraphResizeCallback callback) {
  graph->vertResize = callback;
}

void cgraphSetEdgeResizeCallback(CGraph *graph,
                                 const CGraphResizeCallback callback) {
  graph->edgeResize = callback;
}