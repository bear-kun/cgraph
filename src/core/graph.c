#include "cgraph/graph.h"
#include <stdlib.h>
#include <string.h>

#define OUT CGRAPH_OUT
#define IN CGRAPH_IN

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

static void reserveGraph(CGraph *graph, const CGraphBool directed, const CGraphSize vertCap,
                         const CGraphSize edgeCap) {
  graph->vert.capacity = vertCap;
  graph->vert.array = safeMalloc(vertCap * sizeof(CGraphId));
  graph->vert.degree[OUT] = safeMalloc(vertCap * sizeof(CGraphInt));
  graph->vert.degree[IN] = directed
                             ? safeMalloc(vertCap * sizeof(CGraphInt))
                             : graph->vert.degree[OUT];
  graph->vert.resize = NULL;

  graph->edge.directed = directed;
  graph->edge.capacity = edgeCap;
  graph->edge.head[OUT] = safeMalloc(vertCap * sizeof(CGraphId));
  graph->edge.head[IN] = safeMalloc(vertCap * sizeof(CGraphId));
  graph->edge.next[OUT] = safeMalloc(edgeCap * sizeof(CGraphId));
  graph->edge.next[IN] = safeMalloc(edgeCap * sizeof(CGraphId));
  graph->edge.xor = safeMalloc(edgeCap * sizeof(CGraphId));
  graph->edge.to = safeMalloc(edgeCap * sizeof(CGraphId));
  graph->edge.resize = NULL;
}

void cgraphClearEdges(CGraph *graph) {
  graph->edge.count = 0;
  graph->edge.range = 0;
  graph->edge.free = INVALID_ID;
}

void cgraphClear(CGraph *graph) {
  graph->vert.count = 0;
  graph->vert.range = 0;
  cgraphClearEdges(graph);
}

void cgraphInit(CGraph *graph, const CGraphBool directed, const CGraphSize vertCap,
                const CGraphSize edgeCap) {
  reserveGraph(graph, directed, vertCap, edgeCap);
  cgraphClear(graph);
}

void cgraphCopyVertices(CGraph *dst, const CGraph *src) {
  reserveGraph(dst, src->edge.directed, src->vert.range, src->edge.range);
  dst->vert.count = src->vert.count;
  dst->vert.range = src->vert.range;
  cgraphClearEdges(dst);

  memcpy(dst->vert.array, src->vert.array, dst->vert.capacity * sizeof(CGraphId));
  memset(dst->vert.degree[OUT], 0, dst->vert.capacity * sizeof(CGraphInt));
  memset(dst->edge.head[OUT], INVALID_ID, dst->vert.capacity * sizeof(CGraphId));
  memset(dst->edge.head[IN], INVALID_ID, dst->vert.capacity * sizeof(CGraphId));
  if (dst->edge.directed) {
    memset(dst->vert.degree[IN], 0, dst->vert.capacity * sizeof(CGraphInt));
  }
}

void cgraphCopy(CGraph *dst, const CGraph *src) {
  reserveGraph(dst, src->edge.directed, src->vert.range, src->edge.range);
  dst->vert.count = src->vert.count;
  dst->vert.range = src->vert.range;
  dst->edge.count = src->edge.count;
  dst->edge.range = src->edge.range;
  dst->edge.free = src->edge.free;

  memcpy(dst->vert.array, src->vert.array, dst->vert.capacity * sizeof(CGraphId));
  memcpy(dst->vert.degree[OUT], src->vert.degree[OUT], dst->vert.capacity * sizeof(CGraphInt));
  memcpy(dst->edge.head[OUT], src->edge.head[OUT], dst->vert.capacity * sizeof(CGraphId));
  memcpy(dst->edge.head[IN], src->edge.head[IN], dst->vert.capacity * sizeof(CGraphId));
  memcpy(dst->edge.next[OUT], src->edge.next[OUT], dst->edge.capacity * sizeof(CGraphId));
  memcpy(dst->edge.next[IN], src->edge.next[IN], dst->edge.capacity * sizeof(CGraphId));
  memcpy(dst->edge.xor, src->edge.xor, dst->edge.capacity * sizeof(CGraphId));
  memcpy(dst->edge.to, src->edge.to, dst->edge.capacity * sizeof(CGraphId));

  if (src->edge.directed) {
    memcpy(dst->vert.degree[IN], src->vert.degree[IN], dst->vert.capacity * sizeof(CGraphInt));
  }
}

static CGraphSize resizeNewCapacity(const CGraphSize capacity) {
  if (capacity <= 4) return 8;
  if (capacity < 1024) return capacity * 2;
  return capacity + capacity / 2;
}

static void vertResize(CGraph *graph, const CGraphSize capacity) {
  const CGraphSize oldCap = graph->vert.capacity;
  const CGraphSize newCap = resizeNewCapacity(capacity);

  graph->vert.array = safeRealloc(graph->vert.array, newCap * sizeof(CGraphId));
  graph->vert.degree[OUT] = safeRealloc(graph->vert.degree[OUT], newCap * sizeof(CGraphInt));
  graph->vert.degree[IN] = graph->edge.directed
                             ? safeRealloc(graph->vert.degree[IN], newCap * sizeof(CGraphInt))
                             : graph->vert.degree[OUT];

  graph->edge.head[OUT] = safeRealloc(graph->edge.head[OUT], newCap * sizeof(CGraphId));
  graph->edge.head[IN] = safeRealloc(graph->edge.head[IN], newCap * sizeof(CGraphId));

  graph->vert.capacity = newCap;
  if (graph->vert.resize) graph->vert.resize(oldCap, newCap);
}

static void edgeResize(CGraph *graph, const CGraphSize capacity) {
  const CGraphSize oldCap = graph->edge.capacity;
  const CGraphSize newCap = resizeNewCapacity(capacity);

  graph->edge.xor = safeRealloc(graph->edge.xor, newCap * sizeof(CGraphId));
  graph->edge.to = safeRealloc(graph->edge.to, newCap * sizeof(CGraphId));
  graph->edge.next[OUT] = safeRealloc(graph->edge.next[OUT], newCap * sizeof(CGraphId));
  graph->edge.next[IN] = safeRealloc(graph->edge.next[IN], newCap * sizeof(CGraphId));

  graph->edge.capacity = newCap;
  if (graph->edge.resize) graph->edge.resize(oldCap, newCap);
}

void cgraphRelease(const CGraph *const graph) {
  free(graph->vert.array);
  free(graph->vert.degree[OUT]);
  if (graph->edge.directed) free(graph->vert.degree[IN]);

  free(graph->edge.head[OUT]);
  free(graph->edge.head[IN]);
  free(graph->edge.next[OUT]);
  free(graph->edge.next[IN]);
  free(graph->edge.xor);
  free(graph->edge.to);
}

static void swap(CGraphId *array, CGraphSize i, CGraphSize j) {
  const CGraphId tmp = array[i];
  array[i] = array[j];
  array[j] = tmp;
}

static CGraphId vertInsertNew(CGraph *graph) {
  CGraphId vid;
  if (graph->vert.count == graph->vert.range) {
    vid = (CGraphId)graph->vert.range++;
    graph->vert.array[vid] = vid;
    graph->vert.degree[OUT][vid] = 0;
    graph->vert.degree[IN][vid] = 0;
    graph->edge.head[OUT][vid] = INVALID_ID;
    graph->edge.head[IN][vid] = INVALID_ID;
  } else {
    vid = graph->vert.array[graph->vert.count];
  }
  graph->vert.count++;
  return vid;
}

static void vertUnlinkAndDelete(CGraph *graph, const CGraphId vid) {
  CGraphSize i = 0;
  while (i < graph->vert.count && graph->vert.array[i] != vid) i++;
  if (i == graph->vert.count) return;

  swap(graph->vert.array, i, --graph->vert.count);
}

CGraphId cgraphAddVert(CGraph *const graph) {
  if (graph->vert.count == graph->vert.capacity) vertResize(graph, graph->vert.capacity);
  return vertInsertNew(graph);
}

void cgraphAddVertices(CGraph *graph, const CGraphSize count) {
  if (graph->vert.count + count > graph->vert.capacity) {
    vertResize(graph, graph->vert.count + count);
  }
  for (CGraphSize i = 0; i < count; i++) {
    vertInsertNew(graph);
  }
}

void cgraphDeleteVert(CGraph *graph, const CGraphId vid) {
  vertUnlinkAndDelete(graph, vid);
}

#define FREE_NEXT next[OUT]

static CGraphId edgeGetNew(CGraph *graph, const CGraphId from, const CGraphId to) {
  CGraphId eid;
  if (graph->edge.count == graph->edge.range) {
    eid = (CGraphId)graph->edge.range++;
  } else {
    eid = graph->edge.free;
    graph->edge.free = graph->edge.FREE_NEXT[eid];
  }

  graph->edge.xor[eid] = from ^ to;
  graph->edge.to[eid] = to;
  graph->edge.count++;
  return eid;
}

static void edgeDelete(CGraph *graph, const CGraphId eid) {
  graph->edge.FREE_NEXT[eid] = graph->edge.free;
  graph->edge.free = eid;
  graph->edge.count--;
}

static void edgeInsert(const CGraph *graph, const CGraphId vid, const CGraphId eid,
                       const CGraphBool dir) {
  graph->edge.next[dir][eid] = graph->edge.head[dir][vid];
  graph->edge.head[dir][vid] = eid;
  graph->vert.degree[dir][vid]++;
}

static CGraphBool edgeUnlink(const CGraph *graph, const CGraphId vid, const CGraphId eid,
                             const CGraphBool dir) {
  CGraphId *next = graph->edge.next[dir];
  for (CGraphId *ptr = graph->edge.head[dir] + vid; *ptr != INVALID_ID; ptr = next + *ptr) {
    if (*ptr == eid) {
      *ptr = next[*ptr];
      graph->vert.degree[dir][vid]--;
      return true;
    }
  }
  return false;
}

CGraphId cgraphAddEdge(CGraph *const graph, const CGraphId from, const CGraphId to) {
  if (graph->edge.count == graph->edge.capacity) edgeResize(graph, graph->edge.capacity);

  const CGraphId eid = edgeGetNew(graph, from, to);
  edgeInsert(graph, from, eid, OUT);
  edgeInsert(graph, to, eid, IN);
  return eid;
}

void cgraphAddEdges(CGraph *graph, const CGraphSize count, const CGraphId endpoints[][2]) {
  if (graph->edge.count + count > graph->edge.capacity) {
    edgeResize(graph, graph->edge.count + count);
  }

  for (CGraphSize i = 0; i != count; i++) {
    const CGraphId from = endpoints[i][0];
    const CGraphId to = endpoints[i][1];
    const CGraphId eid = edgeGetNew(graph, from, to);
    edgeInsert(graph, from, eid, OUT);
    edgeInsert(graph, to, eid, IN);
  }
}

void cgraphDeleteEdge(CGraph *graph, const CGraphId eid) {
  const CGraphId to = graph->edge.to[eid];
  const CGraphId from = graph->edge.xor[eid] ^ to;

  if (!edgeUnlink(graph, from, eid, OUT)) return;
  edgeUnlink(graph, to, eid, IN);
  edgeDelete(graph, eid);
}

void cgraphReverseEdge(const CGraph *const graph, const CGraphId eid) {
  if (!graph->edge.directed) return;

  const CGraphId to = graph->edge.to[eid];
  const CGraphId from = graph->edge.xor[eid] ^ to;

  edgeUnlink(graph, from, eid, OUT);
  edgeUnlink(graph, to, eid, IN);
  edgeInsert(graph, from, eid, IN);
  edgeInsert(graph, to, eid, OUT);

  graph->edge.to[eid] = from;
}

CGraphId cgraphFindEdge(const CGraph *graph, const CGraphId from, const CGraphId to) {
  const CGraphId *next = graph->edge.next[OUT];
  for (CGraphId eid = graph->edge.head[OUT][from]; eid != INVALID_ID; eid = next[eid]) {
    if (graph->edge.to[eid] == to) return eid;
  }
  if (!graph->edge.directed) {
    next = graph->edge.next[IN];
    for (CGraphId eid = graph->edge.head[IN][from]; eid != INVALID_ID; eid = next[eid]) {
      if ((graph->edge.xor[eid] ^ from) == to) return eid;
    }
  }
  return INVALID_ID;
}

void cgraphWhereEdgeFromTo(const CGraph *graph, const CGraphId eid, CGraphId *from, CGraphId *to) {
  *to = graph->edge.to[eid];
  *from = graph->edge.xor[eid] ^ *to;
}

void cgraphSetVertResizeCallback(CGraph *graph, const CGraphResizeCallback callback) {
  graph->vert.resize = callback;
}

void cgraphSetEdgeResizeCallback(CGraph *graph, const CGraphResizeCallback callback) {
  graph->edge.resize = callback;
}