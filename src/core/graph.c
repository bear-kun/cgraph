#include "cgraph/graph.h"
#include <stdlib.h>
#include <string.h>

#define REVERSE(did) ((did) ^ 1)
#define DID(eid) ((eid) << 1 | (eid) >> (sizeof(CGraphId) * 8 - 1))

static void *safeMalloc(const size_t size) {
  void *mem = malloc(size);
  if (!mem) abort();
  return mem;
}

static void *safeRealloc(void *memory, const size_t newSize) {
  void *mem = realloc(memory, newSize);
  if (!mem) abort();
  return mem;
}

static void reserveGraph(CGraph *graph, const CGraphSize vertCap, const CGraphSize edgeCap,
                         const CGraphBool directed) {
  graph->vert.capacity = vertCap;
  graph->vert.next = safeMalloc(vertCap * sizeof(CGraphId));
  graph->vert.indegree = safeMalloc(vertCap * sizeof(CGraphInt));
  graph->vert.resize = NULL;

  graph->edge.directed = directed;
  graph->edge.capacity = edgeCap;
  graph->edge.head = safeMalloc(vertCap * sizeof(CGraphId));
  graph->edge.from = safeMalloc(edgeCap * sizeof(CGraphId));
  graph->edge.to = safeMalloc(edgeCap * sizeof(CGraphId));
  graph->edge.resize = NULL;

  if (directed) {
    graph->vert.outdegree = safeMalloc(vertCap * sizeof(CGraphInt));
    graph->edge.next = safeMalloc(edgeCap * sizeof(CGraphId));
  } else {
    graph->vert.outdegree = graph->vert.indegree;
    graph->edge.next = safeMalloc(2 * edgeCap * sizeof(CGraphId));
  }
}

void cgraphInit(CGraph *const graph, const CGraphBool directed, const CGraphSize vertCap,
                const CGraphSize edgeCap) {
  reserveGraph(graph, vertCap, edgeCap, directed);
  cgraphClear(graph);
}

void cgraphCopyVert(CGraph *dst, const CGraph *src) {
  *dst = *src;
  reserveGraph(dst, src->vert.range, src->edge.range, src->edge.directed);

  cgraphClearEdges(dst);
  memcpy(dst->vert.next, src->vert.next, dst->vert.capacity * sizeof(CGraphId));
}

void cgraphCopy(CGraph *dst, const CGraph *src) {
  *dst = *src;
  reserveGraph(dst, src->vert.range, src->edge.range, src->edge.directed);

  memcpy(dst->vert.next, src->vert.next, dst->vert.capacity * sizeof(CGraphId));
  memcpy(dst->vert.indegree, src->vert.indegree, dst->vert.capacity * sizeof(CGraphInt));
  memcpy(dst->edge.head, src->edge.head, dst->vert.capacity * sizeof(CGraphId));
  memcpy(dst->edge.from, src->edge.from, dst->edge.capacity * sizeof(CGraphId));
  memcpy(dst->edge.to, src->edge.to, dst->edge.capacity * sizeof(CGraphId));

  if (src->edge.directed) {
    memcpy(dst->vert.outdegree, src->vert.outdegree, dst->vert.capacity * sizeof(CGraphInt));
    memcpy(dst->edge.next, src->edge.next, dst->edge.capacity * sizeof(CGraphId));
  } else {
    memcpy(dst->edge.next, src->edge.next, 2 * dst->edge.capacity * sizeof(CGraphId));
  }
}

static void initNextList(CGraphId *list, const CGraphSize start, const CGraphSize end) {
  for (CGraphSize i = start; i < end; i++) list[i] = (CGraphId)(i + 1);
}

static void initNextList2(CGraphId *list, const CGraphSize start,
                          const CGraphSize end) {
  for (CGraphSize i = start; i < end; i++) list[2 * i] = (CGraphId)(2 * i + 2);
}

void cgraphClearEdges(CGraph *graph) {
  graph->edge.count = 0;
  graph->edge.free = 0;
  graph->edge.range = 0;
  memset(graph->edge.head, INVALID_ID, graph->vert.capacity * sizeof(CGraphId));
  memset(graph->vert.indegree, 0, graph->vert.capacity * sizeof(CGraphInt));

  if (graph->edge.directed) {
    memset(graph->vert.outdegree, 0, graph->vert.capacity * sizeof(CGraphInt));
    initNextList(graph->edge.next, 0, graph->edge.capacity);
  } else {
    initNextList2(graph->edge.next, 0, graph->edge.capacity);
  }
}

void cgraphClear(CGraph *graph) {
  graph->vert.count = 0;
  graph->vert.free = 0;
  graph->vert.range = 0;
  graph->vert.head = INVALID_ID;
  initNextList(graph->vert.next, 0, graph->vert.capacity);
  cgraphClearEdges(graph);
}

static void cgraphVertResize(CGraph *graph) {
  const CGraphSize oldCap = graph->vert.capacity;
  const CGraphSize newCap = (graph->vert.capacity + 1) << 1;

  graph->vert.next = safeRealloc(graph->vert.next, newCap * sizeof(CGraphId));
  graph->edge.head = safeRealloc(graph->edge.head, newCap * sizeof(CGraphId));
  graph->vert.indegree = safeRealloc(graph->vert.indegree, newCap * sizeof(CGraphInt));

  initNextList(graph->vert.next, oldCap, newCap);
  memset(graph->edge.head + oldCap, INVALID_ID, (newCap - oldCap) * sizeof(CGraphId));
  memset(graph->vert.indegree + oldCap, 0, (newCap - oldCap) * sizeof(CGraphInt));

  if (graph->edge.directed) {
    graph->vert.outdegree = safeRealloc(graph->vert.outdegree, newCap * sizeof(CGraphInt));
    memset(graph->vert.outdegree + oldCap, 0, (newCap - oldCap) * sizeof(CGraphInt));
  } else {
    graph->vert.outdegree = graph->vert.indegree;
  }

  graph->vert.capacity = newCap;
  if (graph->vert.resize) graph->vert.resize(oldCap, newCap);
}

static void cgraphEdgeResize(CGraph *graph) {
  const CGraphSize oldCap = graph->edge.capacity;
  const CGraphSize newCap = (graph->edge.capacity + 1) << 1;

  graph->edge.from = safeRealloc(graph->edge.from, newCap * sizeof(CGraphId));
  graph->edge.to = safeRealloc(graph->edge.to, newCap * sizeof(CGraphId));

  if (graph->edge.directed) {
    graph->edge.next = safeRealloc(graph->edge.next, newCap * sizeof(CGraphId));
    initNextList(graph->edge.next, oldCap, newCap);
  } else {
    graph->edge.next = safeRealloc(graph->edge.next, 2 * newCap * sizeof(CGraphId));
    initNextList2(graph->edge.next, oldCap, newCap);
  }

  graph->edge.capacity = newCap;
  if (graph->edge.resize) graph->edge.resize(oldCap, newCap);
}

void cgraphRelease(const CGraph *const graph) {
  free(graph->vert.next);
  free(graph->vert.indegree);
  if (graph->edge.directed) free(graph->vert.outdegree);

  free(graph->edge.head);
  free(graph->edge.next);
  free(graph->edge.from);
  free(graph->edge.to);
}

static void listUnlink(const CGraphId *next, CGraphId *predNext) {
  *predNext = next[*predNext];
}

static void listInsert(CGraphId *next, CGraphId *predNext, const CGraphId id) {
  next[id] = *predNext;
  *predNext = id;
}

static void cgraphInsertEdge(const CGraph *graph, const CGraphId from, const CGraphId did) {
  listInsert(graph->edge.next, graph->edge.head + from, did);
}

static CGraphId *listFind(CGraphId *next, CGraphId *head, const CGraphId id) {
  for (CGraphId *ptr = head; *ptr != INVALID_ID; ptr = next + *ptr) {
    if (*ptr == id) return ptr;
  }
  return NULL;
}

CGraphId cgraphAddVert(CGraph *const graph) {
  if (graph->vert.count == graph->vert.capacity) cgraphVertResize(graph);

  const CGraphId vid = graph->vert.free;
  listUnlink(graph->vert.next, &graph->vert.free);
  listInsert(graph->vert.next, &graph->vert.head, vid);
  if (vid == graph->vert.range) graph->vert.range++;
  graph->vert.count++;
  return vid;
}

void cgraphReserveVert(CGraph *graph, const CGraphSize num) {
  for (CGraphSize i = 0; i != num; ++i) {
    cgraphAddVert(graph);
  }
}

CGraphId cgraphAddEdge(CGraph *const graph, const CGraphId from, const CGraphId to) {
  if (graph->edge.count == graph->edge.capacity) cgraphEdgeResize(graph);

  const CGraphId did = graph->edge.free;
  listUnlink(graph->edge.next, &graph->edge.free);
  cgraphInsertEdge(graph, from, did);

  graph->vert.indegree[to]++;
  graph->vert.outdegree[from]++;
  if (!graph->edge.directed) {
    cgraphInsertEdge(graph, to, REVERSE(did));
    graph->vert.indegree[from]++;
    graph->vert.outdegree[to]++;
  }

  const CGraphId eid = graph->edge.directed ? did : (did >> 1);
  graph->edge.from[eid] = from;
  graph->edge.to[eid] = to;
  if (eid == graph->edge.range) graph->edge.range++;
  graph->edge.count++;
  return eid;
}

CGraphId cgraphPushEdgeBack(CGraph *const graph, const CGraphId from, const CGraphId to) {
  if (graph->edge.count == graph->edge.capacity) cgraphEdgeResize(graph);

  const CGraphId eid = graph->edge.free;
  listUnlink(graph->edge.next, &graph->edge.free);
  CGraphId *back = graph->edge.head + from;
  while (*back != INVALID_ID) back = graph->edge.next + *back;
  listInsert(graph->edge.next, back, eid);

  graph->vert.indegree[to]++;
  graph->vert.outdegree[from]++;
  graph->edge.from[eid] = from;
  graph->edge.to[eid] = to;
  if (eid == graph->edge.range) graph->edge.range++;
  graph->edge.count++;
  return eid;
}

void cgraphDeleteVert(CGraph *graph, const CGraphId vid) {
  CGraphId *predNext = listFind(graph->vert.next, &graph->vert.head, vid);
  if (!predNext) return;

  listUnlink(graph->vert.next, predNext);
  listInsert(graph->vert.next, &graph->vert.free, vid);
  if (vid == graph->vert.range - 1) graph->vert.range--;
  graph->vert.count--;
}

void cgraphDeleteEdge(CGraph *graph, const CGraphId eid) {
  const CGraphId did = graph->edge.directed ? eid : (eid << 1);
  const CGraphId from = graph->edge.from[eid];

  CGraphId *predNext = listFind(graph->edge.next, graph->edge.head + from, did);
  if (!predNext) return;

  const CGraphId to = graph->edge.to[eid];
  listUnlink(graph->edge.next, predNext);
  listInsert(graph->edge.next, &graph->edge.free, did);
  graph->vert.indegree[to]--;
  graph->vert.outdegree[from]--;

  if (!graph->edge.directed) {
    predNext = listFind(graph->edge.next, graph->edge.head + to, REVERSE(did));
    listUnlink(graph->edge.next, predNext);
    graph->vert.indegree[from]--;
    graph->vert.outdegree[to]--;
  }

  if (eid == graph->edge.range - 1) graph->edge.range--;
  graph->edge.count--;
}

void cgraphReverseEdge(const CGraph *const graph, const CGraphId eid) {
  if (!graph->edge.directed) return;

  const CGraphId from = graph->edge.from[eid];
  const CGraphId to = graph->edge.to[eid];

  CGraphId *predNext = listFind(graph->edge.next, graph->edge.head + from, eid);
  if (!predNext) return;

  listUnlink(graph->edge.next, predNext);
  cgraphInsertEdge(graph, to, eid);

  graph->edge.from[eid] = to;
  graph->edge.to[eid] = from;

  graph->vert.indegree[to]--;
  graph->vert.outdegree[from]--;
  graph->vert.indegree[from]++;
  graph->vert.outdegree[to]++;
}

CGraphId cgraphFindEdge(const CGraph *graph, const CGraphId from, const CGraphId to) {
  for (CGraphId eid = graph->edge.head[from]; eid != INVALID_ID; eid = graph->edge.next[eid]) {
    if (graph->edge.to[eid] == to) return eid;
    if (!graph->edge.directed && graph->edge.from[eid] == from) return eid;
  }
  return INVALID_ID;
}

CGraphId cgraphWhereEdgeFrom(const CGraph *graph, const CGraphId eid) {
  return graph->edge.from[eid];
}

CGraphId cgraphWhereEdgeTo(const CGraph *graph, const CGraphId eid) {
  return graph->edge.to[eid];
}

void cgraphSetVertResizeCallback(CGraph *graph, const CGraphResizeCallback callback) {
  graph->vert.resize = callback;
}

void cgraphSetEdgeResizeCallback(CGraph *graph, const CGraphResizeCallback callback) {
  graph->edge.resize = callback;
}