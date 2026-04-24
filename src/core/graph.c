#include "cgraph/graph.h"
#include <stdlib.h>
#include <string.h>

#define OUT 0
#define IN 1

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
  graph->vert.degree[OUT] = safeMalloc(vertCap * sizeof(CGraphInt));
  graph->vert.resize = NULL;

  graph->edge.directed = directed;
  graph->edge.capacity = edgeCap;
  graph->edge.head[OUT] = safeMalloc(vertCap * sizeof(CGraphId));
  graph->edge.next[OUT] = safeMalloc(edgeCap * sizeof(CGraphId));
  graph->edge.next[IN] = safeMalloc(edgeCap * sizeof(CGraphId));
  graph->edge.from = safeMalloc(edgeCap * sizeof(CGraphId));
  graph->edge.to = safeMalloc(edgeCap * sizeof(CGraphId));
  graph->edge.resize = NULL;

  if (directed) {
    graph->vert.degree[IN] = safeMalloc(vertCap * sizeof(CGraphInt));
    graph->edge.head[IN] = safeMalloc(vertCap * sizeof(CGraphId));
  } else {
    graph->vert.degree[IN] = graph->vert.degree[OUT];
    graph->edge.head[IN] = graph->edge.head[OUT];
  }
}

void cgraphInit(CGraph *graph, const CGraphBool directed, const CGraphSize vertCap,
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
  memcpy(dst->vert.degree[OUT], src->vert.degree[OUT], dst->vert.capacity * sizeof(CGraphInt));
  memcpy(dst->edge.head[OUT], src->edge.head[OUT], dst->vert.capacity * sizeof(CGraphId));
  memcpy(dst->edge.next[OUT], src->edge.next[OUT], dst->edge.capacity * sizeof(CGraphId));
  memcpy(dst->edge.next[IN], src->edge.next[IN], dst->edge.capacity * sizeof(CGraphId));
  memcpy(dst->edge.from, src->edge.from, dst->edge.capacity * sizeof(CGraphId));
  memcpy(dst->edge.to, src->edge.to, dst->edge.capacity * sizeof(CGraphId));

  if (src->edge.directed) {
    memcpy(dst->vert.degree[IN], src->vert.degree[IN], dst->vert.capacity * sizeof(CGraphInt));
    memcpy(dst->edge.head[IN], src->edge.head[IN], dst->vert.capacity * sizeof(CGraphId));
  }
}

static void initNextList(CGraphId *list, const CGraphSize start, const CGraphSize end) {
  for (CGraphSize i = start; i < end; i++) list[i] = (CGraphId)(i + 1);
}

void cgraphClearEdges(CGraph *graph) {
  graph->edge.count = 0;
  graph->edge.free = 0;
  graph->edge.range = 0;

  memset(graph->vert.degree[OUT], 0, graph->vert.capacity * sizeof(CGraphInt));
  memset(graph->edge.head[OUT], INVALID_ID, graph->vert.capacity * sizeof(CGraphId));
  initNextList(graph->edge.next[OUT], 0, graph->edge.capacity);

  if (graph->edge.directed) {
    memset(graph->vert.degree[IN], 0, graph->vert.capacity * sizeof(CGraphInt));
    memset(graph->edge.head[IN], INVALID_ID, graph->vert.capacity * sizeof(CGraphId));
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

static void vertResize(CGraph *graph) {
  const CGraphSize oldCap = graph->vert.capacity;
  const CGraphSize newCap = (graph->vert.capacity + 1) << 1;

  graph->vert.next = safeRealloc(graph->vert.next, newCap * sizeof(CGraphId));
  graph->vert.degree[OUT] = safeRealloc(graph->vert.degree[OUT], newCap * sizeof(CGraphInt));
  graph->edge.head[OUT] = safeRealloc(graph->edge.head[OUT], newCap * sizeof(CGraphId));

  initNextList(graph->vert.next, oldCap, newCap);
  memset(graph->vert.degree[OUT] + oldCap, 0, (newCap - oldCap) * sizeof(CGraphInt));
  memset(graph->edge.head[OUT] + oldCap, INVALID_ID, (newCap - oldCap) * sizeof(CGraphId));

  if (graph->edge.directed) {
    graph->vert.degree[IN] = safeRealloc(graph->vert.degree[IN], newCap * sizeof(CGraphInt));
    graph->edge.head[IN] = safeRealloc(graph->edge.head[IN], newCap * sizeof(CGraphId));

    memset(graph->vert.degree[IN] + oldCap, 0, (newCap - oldCap) * sizeof(CGraphInt));
    memset(graph->edge.head[IN] + oldCap, INVALID_ID, (newCap - oldCap) * sizeof(CGraphId));
  } else {
    graph->vert.degree[IN] = graph->vert.degree[OUT];
    graph->edge.head[IN] = graph->edge.head[OUT];
  }

  graph->vert.capacity = newCap;
  if (graph->vert.resize) graph->vert.resize(oldCap, newCap);
}

static void edgeResize(CGraph *graph) {
  const CGraphSize oldCap = graph->edge.capacity;
  const CGraphSize newCap = (graph->edge.capacity + 1) << 1;

  graph->edge.from = safeRealloc(graph->edge.from, newCap * sizeof(CGraphId));
  graph->edge.to = safeRealloc(graph->edge.to, newCap * sizeof(CGraphId));
  graph->edge.next[OUT] = safeRealloc(graph->edge.next[OUT], newCap * sizeof(CGraphId));
  graph->edge.next[IN] = safeRealloc(graph->edge.next[IN], newCap * sizeof(CGraphId));

  initNextList(graph->edge.next[OUT], oldCap, newCap);

  graph->edge.capacity = newCap;
  if (graph->edge.resize) graph->edge.resize(oldCap, newCap);
}

void cgraphRelease(const CGraph *const graph) {
  free(graph->vert.next);
  free(graph->vert.degree[OUT]);

  free(graph->edge.head[OUT]);
  free(graph->edge.next[OUT]);
  free(graph->edge.next[IN]);
  free(graph->edge.from);
  free(graph->edge.to);

  if (graph->edge.directed) {
    free(graph->vert.degree[IN]);
    free(graph->edge.head[IN]);
  }
}

static CGraphId vertGetNew(CGraph *graph) {
  const CGraphId vid = graph->vert.free;
  graph->vert.free = graph->vert.next[vid];
  if (vid == graph->vert.range) graph->vert.range++;
  graph->vert.count++;
  return vid;
}

static void vertDelete(CGraph *graph, const CGraphId vid) {
  graph->vert.next[vid] = graph->vert.free;
  graph->vert.free = vid;
  if (vid == graph->vert.range - 1) graph->vert.range--;
  graph->vert.count--;
}

static void vertInsert(CGraph *graph, const CGraphId vid) {
  graph->vert.next[vid] = graph->vert.head;
  graph->vert.head = vid;
}

static void vertUnlink(CGraph *graph, const CGraphId vid) {
  for (CGraphId *ptr = &graph->vert.head; *ptr != INVALID_ID; ptr = graph->vert.next + *ptr) {
    if (*ptr == vid) {
      *ptr = graph->vert.next[*ptr];
      break;
    }
  }
}

static CGraphId edgeGetNew(CGraph *graph, const CGraphId from, const CGraphId to) {
  const CGraphId eid = graph->edge.free;
  graph->edge.free = graph->edge.next[OUT][eid];
  graph->edge.from[eid] = from;
  graph->edge.to[eid] = to;
  if (eid == graph->edge.range) graph->edge.range++;
  graph->edge.count++;
  return eid;
}

static void edgeDelete(CGraph *graph, const CGraphId eid) {
  graph->edge.next[OUT][eid] = graph->edge.free;
  graph->edge.free = eid;
  if (eid == graph->edge.range - 1) graph->edge.range--;
  graph->edge.count--;
}

static void edgeInsert(const CGraph *graph, const CGraphId vid, const CGraphId eid,
                       const CGraphBool dir) {
  graph->edge.next[dir][eid] = graph->edge.head[dir][vid];
  graph->edge.head[dir][vid] = (eid << 1) | dir;
  graph->vert.degree[dir][vid]++;
}

static void edgeUnlink(const CGraph *graph, const CGraphId vid, const CGraphId eid,
                       const CGraphBool dir) {
  CGraphId *ptr = graph->edge.head[dir] + vid;
  while (*ptr != INVALID_ID) {
    if (*ptr >> 1 == eid) {
      *ptr = graph->edge.next[*ptr & 1][*ptr >> 1];
      graph->vert.degree[dir][vid]--;
      break;
    }
    ptr = graph->edge.next[*ptr & 1] + (*ptr >> 1);
  }
}

CGraphId cgraphAddVert(CGraph *const graph) {
  if (graph->vert.count == graph->vert.capacity) vertResize(graph);

  const CGraphId vid = vertGetNew(graph);
  vertInsert(graph, vid);
  return vid;
}

void cgraphReserveVert(CGraph *graph, const CGraphSize num) {
  for (CGraphSize i = 0; i != num; ++i) {
    cgraphAddVert(graph);
  }
}

CGraphId cgraphAddEdge(CGraph *const graph, const CGraphId from, const CGraphId to) {
  if (graph->edge.count == graph->edge.capacity) edgeResize(graph);

  const CGraphId eid = edgeGetNew(graph, from, to);
  edgeInsert(graph, from, eid, OUT);
  edgeInsert(graph, to, eid, IN);
  return eid;
}

void cgraphDeleteVert(CGraph *graph, const CGraphId vid) {
  vertUnlink(graph, vid);
  vertDelete(graph, vid);
}

void cgraphDeleteEdge(CGraph *graph, const CGraphId eid) {
  const CGraphId from = graph->edge.from[eid];
  const CGraphId to = graph->edge.to[eid];

  edgeUnlink(graph, from, eid, OUT);
  edgeUnlink(graph, to, eid, IN);
  edgeDelete(graph, eid);
}

void cgraphReverseEdge(const CGraph *const graph, const CGraphId eid) {
  if (!graph->edge.directed) return;

  const CGraphId from = graph->edge.from[eid];
  const CGraphId to = graph->edge.to[eid];

  edgeUnlink(graph, from, eid, OUT);
  edgeUnlink(graph, to, eid, IN);
  edgeInsert(graph, from, eid, IN);
  edgeInsert(graph, to, eid, OUT);

  graph->edge.from[eid] = to;
  graph->edge.to[eid] = from;
}

CGraphId cgraphFindEdge(const CGraph *graph, const CGraphId from, const CGraphId to) {
  CGraphId did = graph->edge.head[OUT][from];
  while (did != INVALID_ID) {
    const CGraphId eid = did >> 1;
    if (graph->edge.to[eid] == to) return eid;
    if (!graph->edge.directed && graph->edge.to[eid] == from) return eid;
    did = graph->edge.next[did & 1][eid];
  }
  return INVALID_ID;
}

void cgraphWhereEdgeFromTo(const CGraph *graph, const CGraphId eid, CGraphId *from, CGraphId *to) {
  *from = graph->edge.from[eid];
  *to = graph->edge.to[eid];
}

void cgraphSetVertResizeCallback(CGraph *graph, const CGraphResizeCallback callback) {
  graph->vert.resize = callback;
}

void cgraphSetEdgeResizeCallback(CGraph *graph, const CGraphResizeCallback callback) {
  graph->edge.resize = callback;
}