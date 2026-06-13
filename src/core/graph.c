#include "cgraph/graph.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define OUT CGRAPH_OUT
#define IN CGRAPH_IN
#define FREE_NEXT next[OUT]

static void *safe_malloc(const size_t size) {
  void *mem = malloc(size);
  if (!mem) abort();
  return mem;
}

static void *safe_realloc(void *memory, const size_t newSize) {
  void *mem = realloc(memory, newSize);
  if (!mem) abort();
  return mem;
}

static void reserve_graph(CGraph *graph, const CGraphBool directed, const CGraphSize vert_cap,
                          const CGraphSize edge_cap) {
  graph->vert.capacity = vert_cap;
  graph->vert.array = safe_malloc(vert_cap * sizeof(CGraphId));
  graph->vert.degree[OUT] = safe_malloc(vert_cap * sizeof(CGraphInt));
  graph->vert.degree[IN] = directed
                             ? safe_malloc(vert_cap * sizeof(CGraphInt))
                             : graph->vert.degree[OUT];

  graph->edge.directed = directed;
  graph->edge.capacity = edge_cap;
  graph->edge.head[OUT] = safe_malloc(vert_cap * sizeof(CGraphId));
  graph->edge.head[IN] = safe_malloc(vert_cap * sizeof(CGraphId));
  graph->edge.next[OUT] = safe_malloc(edge_cap * sizeof(CGraphId));
  graph->edge.next[IN] = safe_malloc(edge_cap * sizeof(CGraphId));
  graph->edge.xor_ = safe_malloc(edge_cap * sizeof(CGraphId));
  graph->edge.to = safe_malloc(edge_cap * sizeof(CGraphId));
}

void cgraph_clear_edges(CGraph *graph) {
  graph->edge.count = 0;
  graph->edge.range = 0;
  graph->edge.free = INVALID_ID;
}

void cgraph_clear(CGraph *graph) {
  graph->vert.count = 0;
  graph->vert.range = 0;
  cgraph_clear_edges(graph);
}

void cgraph_init(CGraph *graph, const CGraphBool directed, const CGraphSize vert_cap,
                 const CGraphSize edge_cap) {
  reserve_graph(graph, directed, vert_cap, edge_cap);
  cgraph_clear(graph);
}

void cgraph_copy_vertices(CGraph *dst, const CGraph *src) {
  reserve_graph(dst, src->edge.directed, src->vert.range, src->edge.range);
  dst->vert.count = src->vert.count;
  dst->vert.range = src->vert.range;
  cgraph_clear_edges(dst);

  memcpy(dst->vert.array, src->vert.array, dst->vert.capacity * sizeof(CGraphId));
  memset(dst->vert.degree[OUT], 0, dst->vert.capacity * sizeof(CGraphInt));
  memset(dst->edge.head[OUT], INVALID_ID, dst->vert.capacity * sizeof(CGraphId));
  memset(dst->edge.head[IN], INVALID_ID, dst->vert.capacity * sizeof(CGraphId));

  if (dst->edge.directed) {
    memset(dst->vert.degree[IN], 0, dst->vert.capacity * sizeof(CGraphInt));
  }
}

void cgraph_copy(CGraph *dst, const CGraph *src) {
  reserve_graph(dst, src->edge.directed, src->vert.range, src->edge.range);
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
  memcpy(dst->edge.xor_, src->edge.xor_, dst->edge.capacity * sizeof(CGraphId));
  memcpy(dst->edge.to, src->edge.to, dst->edge.capacity * sizeof(CGraphId));

  if (src->edge.directed) {
    memcpy(dst->vert.degree[IN], src->vert.degree[IN], dst->vert.capacity * sizeof(CGraphInt));
  }
}

static CGraphSize resize_new_capacity(const CGraphSize capacity) {
  if (capacity <= 4) return 8;
  if (capacity < 1024) return capacity * 2;
  return capacity + capacity / 2;
}

static void vert_resize(CGraph *graph, const CGraphSize capacity) {
  const CGraphSize new_cap = resize_new_capacity(capacity);

  graph->vert.array = safe_realloc(graph->vert.array, new_cap * sizeof(CGraphId));
  graph->vert.degree[OUT] = safe_realloc(graph->vert.degree[OUT], new_cap * sizeof(CGraphInt));
  graph->vert.degree[IN] = graph->edge.directed
                             ? safe_realloc(graph->vert.degree[IN], new_cap * sizeof(CGraphInt))
                             : graph->vert.degree[OUT];

  graph->edge.head[OUT] = safe_realloc(graph->edge.head[OUT], new_cap * sizeof(CGraphId));
  graph->edge.head[IN] = safe_realloc(graph->edge.head[IN], new_cap * sizeof(CGraphId));

  graph->vert.capacity = new_cap;
}

static void edge_resize(CGraph *graph, const CGraphSize capacity) {
  const CGraphSize new_cap = resize_new_capacity(capacity);

  graph->edge.xor_ = safe_realloc(graph->edge.xor_, new_cap * sizeof(CGraphId));
  graph->edge.to = safe_realloc(graph->edge.to, new_cap * sizeof(CGraphId));
  graph->edge.next[OUT] = safe_realloc(graph->edge.next[OUT], new_cap * sizeof(CGraphId));
  graph->edge.next[IN] = safe_realloc(graph->edge.next[IN], new_cap * sizeof(CGraphId));

  graph->edge.capacity = new_cap;
}

void cgraph_release(const CGraph *const graph) {
  free(graph->vert.array);
  free(graph->vert.degree[OUT]);
  if (graph->edge.directed) free(graph->vert.degree[IN]);

  free(graph->edge.head[OUT]);
  free(graph->edge.head[IN]);
  free(graph->edge.next[OUT]);
  free(graph->edge.next[IN]);
  free(graph->edge.xor_);
  free(graph->edge.to);
}

static void swap(CGraphId *array, const CGraphSize i, const CGraphSize j) {
  const CGraphId tmp = array[i];
  array[i] = array[j];
  array[j] = tmp;
}

static CGraphId vert_insert_new(CGraph *graph) {
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

static void vert_unlink_and_delete(CGraph *graph, const CGraphId vid) {
  CGraphSize i = 0;
  while (i < graph->vert.count && graph->vert.array[i] != vid) i++;
  if (i == graph->vert.count) return;

  swap(graph->vert.array, i, --graph->vert.count);
}

static CGraphId edge_get_new(CGraph *graph, const CGraphId from, const CGraphId to) {
  CGraphId eid;
  if (graph->edge.count == graph->edge.range) {
    eid = (CGraphId)graph->edge.range++;
  } else {
    eid = graph->edge.free;
    graph->edge.free = graph->edge.FREE_NEXT[eid];
  }

  graph->edge.xor_[eid] = from ^ to;
  graph->edge.to[eid] = to;
  graph->edge.count++;
  return eid;
}

static void edge_delete(CGraph *graph, const CGraphId eid) {
  graph->edge.FREE_NEXT[eid] = graph->edge.free;
  graph->edge.free = eid;
  graph->edge.count--;
}

static void edge_insert(const CGraph *graph, const CGraphId vid, const CGraphId eid,
                        const CGraphBool dir) {
  graph->edge.next[dir][eid] = graph->edge.head[dir][vid];
  graph->edge.head[dir][vid] = eid;
  graph->vert.degree[dir][vid]++;
}

static CGraphBool edge_unlink(const CGraph *graph, const CGraphId vid, const CGraphId eid,
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

CGraphId cgraph_add_vertex(CGraph *const graph) {
  if (graph->vert.count == graph->vert.capacity) vert_resize(graph, graph->vert.capacity);
  return vert_insert_new(graph);
}

void cgraph_add_vertices(CGraph *graph, const CGraphSize count) {
  if (graph->vert.count + count > graph->vert.capacity) {
    vert_resize(graph, graph->vert.count + count);
  }
  for (CGraphSize i = 0; i < count; i++) {
    vert_insert_new(graph);
  }
}

void cgraph_delete_vertex(CGraph *graph, const CGraphId vid) {
  CGraphId eid = graph->edge.head[OUT][vid];
  while (eid != INVALID_ID) {
    const CGraphId next = graph->edge.next[OUT][eid];
    edge_unlink(graph, graph->edge.xor_[eid] ^ vid, eid, IN);
    edge_delete(graph, eid);
    eid = next;
  }
  graph->edge.head[OUT][vid] = INVALID_ID;

  eid = graph->edge.head[IN][vid];
  while (eid != INVALID_ID) {
    const CGraphId next = graph->edge.next[IN][eid];
    edge_unlink(graph, graph->edge.xor_[eid] ^ vid, eid, OUT);
    edge_delete(graph, eid);
    eid = next;
  }
  graph->edge.head[IN][vid] = INVALID_ID;

  vert_unlink_and_delete(graph, vid);
}

CGraphId cgraph_add_edge(CGraph *const graph, const CGraphId from, const CGraphId to) {
  if (graph->edge.count == graph->edge.capacity) edge_resize(graph, graph->edge.capacity);

  const CGraphId eid = edge_get_new(graph, from, to);
  edge_insert(graph, from, eid, OUT);
  edge_insert(graph, to, eid, IN);
  return eid;
}

void cgraph_add_edges(CGraph *graph, const CGraphSize count, const CGraphId endpoints[][2]) {
  if (graph->edge.count + count > graph->edge.capacity) {
    edge_resize(graph, graph->edge.count + count);
  }

  for (CGraphSize i = 0; i != count; i++) {
    const CGraphId from = endpoints[i][0];
    const CGraphId to = endpoints[i][1];
    const CGraphId eid = edge_get_new(graph, from, to);
    edge_insert(graph, from, eid, OUT);
    edge_insert(graph, to, eid, IN);
  }
}

void cgraph_delete_edge(CGraph *graph, const CGraphId eid) {
  const CGraphId to = graph->edge.to[eid];
  const CGraphId from = graph->edge.xor_[eid] ^ to;

  if (!edge_unlink(graph, from, eid, OUT)) return;
  edge_unlink(graph, to, eid, IN);
  edge_delete(graph, eid);
}

void cgraph_reverse_edge(const CGraph *const graph, const CGraphId eid) {
  if (!graph->edge.directed) return;

  const CGraphId to = graph->edge.to[eid];
  const CGraphId from = graph->edge.xor_[eid] ^ to;

  edge_unlink(graph, from, eid, OUT);
  edge_unlink(graph, to, eid, IN);
  edge_insert(graph, from, eid, IN);
  edge_insert(graph, to, eid, OUT);

  graph->edge.to[eid] = from;
}

CGraphId cgraph_find_edge(const CGraph *graph, const CGraphId from, const CGraphId to) {
  const CGraphId *next = graph->edge.next[OUT];
  for (CGraphId eid = graph->edge.head[OUT][from]; eid != INVALID_ID; eid = next[eid]) {
    if (graph->edge.to[eid] == to) return eid;
  }
  if (!graph->edge.directed) {
    next = graph->edge.next[IN];
    for (CGraphId eid = graph->edge.head[IN][from]; eid != INVALID_ID; eid = next[eid]) {
      if ((graph->edge.xor_[eid] ^ from) == to) return eid;
    }
  }
  return INVALID_ID;
}

void cgraph_where_edge_from_to(const CGraph *graph, const CGraphId eid, CGraphId *from,
                               CGraphId *to) {
  *to = graph->edge.to[eid];
  *from = graph->edge.xor_[eid] ^ *to;
}

typedef struct {
  struct {
    CGraphSize count, range;
  } vert;

  struct {
    CGraphBool directed;
    CGraphSize count, range;
    CGraphId free;
  } edge;
} CGraphHeader;

void cgraph_save_binary(const CGraph *graph, const char *path) {
  FILE *file = fopen(path, "wb");
  if (!file) {
    perror("cgraph");
    return;
  }

  const CGraphHeader header = {
      .vert = {
          .count = graph->vert.count,
          .range = graph->vert.range
      },

      .edge = {
          .directed = graph->edge.directed,
          .count = graph->edge.count,
          .range = graph->edge.range,
          .free = graph->edge.free
      }
  };

  fwrite(&header, sizeof(CGraphHeader), 1, file);
  fwrite(graph->vert.array, sizeof(CGraphId), header.vert.range, file);
  fwrite(graph->edge.head[OUT], sizeof(CGraphId), header.vert.range, file);
  fwrite(graph->edge.next[OUT], sizeof(CGraphId), header.edge.range, file);
  fwrite(graph->edge.to, sizeof(CGraphId), header.edge.range, file);
  fclose(file);
}

void cgraph_load_binary(CGraph *graph, const char *path) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    perror("cgraph");
    return;
  }

  CGraphHeader header;
  size_t read = fread(&header, sizeof(CGraphHeader), 1, file);
  if (read != sizeof(CGraphHeader)) goto err1;

  reserve_graph(graph, header.edge.directed, header.vert.range, header.edge.range);
  graph->vert.count = header.vert.count;
  graph->vert.range = header.vert.range;
  graph->edge.count = header.edge.count;
  graph->edge.range = header.edge.range;
  graph->edge.free = header.edge.free;

  read = fread(graph->vert.array, sizeof(CGraphId), header.vert.range, file);
  if (read != header.vert.range) goto err2;

  read = fread(graph->edge.head[OUT], sizeof(CGraphId), header.vert.range, file);
  if (read != header.vert.range) goto err2;

  read = fread(graph->edge.next[OUT], sizeof(CGraphId), header.edge.range, file);
  if (read != header.edge.range) goto err2;

  read = fread(graph->edge.to, sizeof(CGraphId), header.edge.range, file);
  if (read != header.edge.range) goto err2;

  memset(graph->vert.degree[OUT], 0, sizeof(CGraphInt) * header.vert.range);
  memset(graph->edge.head[IN], INVALID_ID, sizeof(CGraphId) * header.vert.range);
  if (header.edge.directed) {
    memset(graph->vert.degree[IN], 0, sizeof(CGraphInt) * header.vert.range);
  }

  const CGraphId *next = graph->edge.next[OUT];
  for (CGraphSize i = 0; i < header.vert.count; i++) {
    const CGraphId from = graph->vert.array[i];
    for (CGraphId eid = graph->edge.head[OUT][from]; eid != INVALID_ID; eid = next[eid]) {
      const CGraphId to = graph->edge.to[eid];
      graph->vert.degree[OUT][from]++;
      edge_insert(graph, to, eid, IN);
      graph->edge.xor_[eid] = from ^ to;
    }
  }

  fclose(file);
  return;

err2:
  cgraph_release(graph);
err1:
  fprintf(stderr, "cgraph: failed to read or incomplete : %s .\n", path);
  fclose(file);
}