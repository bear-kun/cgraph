#include "cgraph/graph.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#ifdef CGRAPH_ASSERT_MODE
#define cgraph_check(expr, code) assert(expr)
#else
#define cgraph_check(expr, code) if(!(expr)) return code
#endif

#define OUT CGRAPH_OUT
#define IN CGRAPH_IN
#define FREE_NEXT next[OUT]
#define STATE next[IN] // edge is deleted or not
#define DELETED (-2)

// ----- basic -----

typedef struct {
  size_t old_count;
  int size;
  CGraphInt *stack[16];
} MemoryAllocator;

static CGraphBool memory_alloc(MemoryAllocator *alloc, CGraphId **ptr, const size_t count) {
  void *mem = malloc(count * sizeof(CGraphInt));
  if (!mem) return false;
  alloc->stack[alloc->size++] = *ptr = mem;
  return true;
}

static CGraphBool memory_realloc(MemoryAllocator *alloc, CGraphId **ptr, const size_t new_count) {
  void *mem = realloc(*ptr, new_count * sizeof(CGraphInt));
  if (!mem) return false;
  alloc->stack[alloc->size++] = *ptr = mem;
  return true;
}

static void memory_free(const MemoryAllocator *alloc) {
  for (int i = 0; i < alloc->size; i++) {
    if (alloc->old_count == 0) {
      free(alloc->stack[i]);
    } else {
      realloc(alloc->stack[i], alloc->old_count);
    }
  }
}

static CGraphBool graph_reserve(CGraph *graph, const CGraphBool directed,
                                const CGraphSize vert_cap, const CGraphSize edge_cap) {
  MemoryAllocator alloc = {0};

  graph->vert.capacity = vert_cap;
  if (!memory_alloc(&alloc, &graph->vert.indices, vert_cap)) goto fail;
  if (!memory_alloc(&alloc, &graph->vert.array, vert_cap)) goto fail;
  if (!memory_alloc(&alloc, &graph->vert.degree[OUT], vert_cap)) goto fail;
  if (directed) {
    if (!memory_alloc(&alloc, &graph->vert.degree[IN], vert_cap)) goto fail;
  } else {
    graph->vert.degree[IN] = graph->vert.degree[OUT];
  }

  graph->edge.directed = directed;
  graph->edge.capacity = edge_cap;
  if (!memory_alloc(&alloc, &graph->edge.head[OUT], vert_cap)) goto fail;
  if (!memory_alloc(&alloc, &graph->edge.head[IN], vert_cap)) goto fail;
  if (!memory_alloc(&alloc, &graph->edge.next[OUT], edge_cap)) goto fail;
  if (!memory_alloc(&alloc, &graph->edge.next[IN], edge_cap)) goto fail;
  if (!memory_alloc(&alloc, &graph->edge.xor_, edge_cap)) goto fail;
  if (!memory_alloc(&alloc, &graph->edge.to, edge_cap)) goto fail;

  return true;

fail:
  memory_free(&alloc);
  return false;
}

static void vert_copy(CGraph *dst, const CGraph *src) {
  dst->vert.count = src->vert.count;
  dst->vert.range = src->vert.range;
  memcpy(dst->vert.indices, src->vert.indices, dst->vert.capacity * sizeof(CGraphId));
  memcpy(dst->vert.array, src->vert.array, dst->vert.capacity * sizeof(CGraphId));
}

static void edge_clear(CGraph *graph) {
  cgraph_clear_edges(graph);
  memset(graph->edge.head[OUT], CGRAPH_INV_ID, graph->vert.capacity * sizeof(CGraphId));
  memset(graph->edge.head[IN], CGRAPH_INV_ID, graph->vert.capacity * sizeof(CGraphId));
  memset(graph->vert.degree[OUT], 0, graph->vert.capacity * sizeof(CGraphInt));
  if (graph->edge.directed) {
    memset(graph->vert.degree[IN], 0, graph->vert.capacity * sizeof(CGraphInt));
  }
}

static void edge_copy(CGraph *dst, const CGraph *src) {
  dst->edge.count = src->edge.count;
  dst->edge.range = src->edge.range;
  dst->edge.free = src->edge.free;

  memcpy(dst->edge.head[OUT], src->edge.head[OUT], dst->vert.capacity * sizeof(CGraphId));
  memcpy(dst->edge.head[IN], src->edge.head[IN], dst->vert.capacity * sizeof(CGraphId));
  memcpy(dst->edge.next[OUT], src->edge.next[OUT], dst->edge.capacity * sizeof(CGraphId));
  memcpy(dst->edge.next[IN], src->edge.next[IN], dst->edge.capacity * sizeof(CGraphId));
  memcpy(dst->edge.xor_, src->edge.xor_, dst->edge.capacity * sizeof(CGraphId));
  memcpy(dst->edge.to, src->edge.to, dst->edge.capacity * sizeof(CGraphId));

  memcpy(dst->vert.degree[OUT], src->vert.degree[OUT], dst->vert.capacity * sizeof(CGraphInt));
  if (src->edge.directed) {
    memcpy(dst->vert.degree[IN], src->vert.degree[IN], dst->vert.capacity * sizeof(CGraphInt));
  }
}

static CGraphSize resize_new_capacity(const CGraphSize capacity) {
  if (capacity <= 4) return 8;
  if (capacity < 1024) return capacity * 2;
  return capacity + capacity / 2;
}

static CGraphBool vert_resize(CGraph *graph, const CGraphSize capacity) {
  MemoryAllocator alloc = {graph->vert.capacity};
  const CGraphSize new_cap = resize_new_capacity(capacity);

  if (!memory_realloc(&alloc, &graph->vert.indices, new_cap)) goto fail;
  if (!memory_realloc(&alloc, &graph->vert.array, new_cap)) goto fail;
  if (!memory_realloc(&alloc, &graph->vert.degree[OUT], new_cap)) goto fail;
  if (graph->edge.directed) {
    if (!memory_realloc(&alloc, &graph->vert.degree[IN], new_cap)) goto fail;
  } else {
    graph->vert.degree[IN] = graph->vert.degree[OUT];
  }

  if (!memory_realloc(&alloc, &graph->edge.head[OUT], new_cap)) goto fail;
  if (!memory_realloc(&alloc, &graph->edge.head[IN], new_cap)) goto fail;

  graph->vert.capacity = new_cap;
  return true;

fail:
  memory_free(&alloc);
  return false;
}

static CGraphBool edge_resize(CGraph *graph, const CGraphSize capacity) {
  MemoryAllocator alloc = {graph->edge.capacity};
  const CGraphSize new_cap = resize_new_capacity(capacity);

  if (!memory_realloc(&alloc, &graph->edge.next[OUT], new_cap)) goto fail;
  if (!memory_realloc(&alloc, &graph->edge.next[IN], new_cap)) goto fail;
  if (!memory_realloc(&alloc, &graph->edge.xor_, new_cap)) goto fail;
  if (!memory_realloc(&alloc, &graph->edge.to, new_cap)) goto fail;

  graph->edge.capacity = new_cap;
  return true;

fail:
  memory_free(&alloc);
  return false;
}

static void swap(CGraphId *array, const CGraphSize i, const CGraphSize j) {
  const CGraphId tmp = array[i];
  array[i] = array[j];
  array[j] = tmp;
}

CGraphBool cgraph_is_valid_vertex(const CGraph *graph, const CGraphId vid) {
  return vid >= 0 && vid < graph->vert.range && graph->vert.indices[vid] < graph->vert.count;
}

static void vert_fix_free(const CGraph *graph) {
  for (CGraphSize i = graph->vert.count, j = i; i < graph->vert.range; i++) {
    while (graph->vert.array[j] >= graph->vert.range) j++;

    const CGraphId vid = graph->vert.array[j++];
    graph->vert.array[i] = vid;
    graph->vert.indices[vid] = (CGraphId)i;
  }
}

static CGraphId vert_insert_new(CGraph *graph) {
  CGraphId vid;
  if (graph->vert.count == graph->vert.range) {
    vid = (CGraphId)graph->vert.range++;
    graph->vert.indices[vid] = vid;
    graph->vert.array[vid] = vid;

    // init
    graph->vert.degree[OUT][vid] = 0;
    graph->vert.degree[IN][vid] = 0;
    graph->edge.head[OUT][vid] = CGRAPH_INV_ID;
    graph->edge.head[IN][vid] = CGRAPH_INV_ID;
  } else {
    const CGraphId back = graph->vert.array[graph->vert.count];

    // always make the range grow linearly
    if (back < graph->vert.range) {
      vid = back;
    } else if (back == graph->vert.range) {
      vid = back;
      graph->vert.range++;
    } else {
      swap(graph->vert.array, graph->vert.count, graph->vert.indices[graph->vert.range]);
      swap(graph->vert.indices, back, graph->vert.range);
      vid = (CGraphId)graph->vert.range++;
    }
  }
  graph->vert.count++;
  return vid;
}

static void vert_unlink_and_delete(CGraph *graph, const CGraphId vid) {
  const CGraphId back = graph->vert.array[graph->vert.count - 1];
  if (back != vid) {
    swap(graph->vert.array, graph->vert.indices[vid], graph->vert.count - 1);
    swap(graph->vert.indices, vid, back);
  }
  graph->vert.count--;

  if (vid == graph->vert.range - 1) {
    if (graph->vert.count == 0) {
      graph->vert.range = 0;
      return;
    }

    // shrink range
    graph->vert.range--;
    while (graph->vert.indices[graph->vert.range - 1] >= graph->vert.count) graph->vert.range--;
  }
}

CGraphBool cgraph_is_valid_edge(const CGraph *graph, const CGraphId eid) {
  return eid >= 0 && eid < graph->edge.range && graph->edge.STATE[eid] != DELETED;
}

static void edge_fix_free(CGraph *graph) {
  if (graph->edge.count == graph->edge.range) {
    graph->edge.free = CGRAPH_INV_ID;
    return;
  }

  CGraphId f = graph->edge.free, *prev = &graph->edge.free;
  while (f != CGRAPH_INV_ID) {
    const CGraphId next = graph->edge.FREE_NEXT[f];
    if (f >= graph->edge.range) {
      *prev = next;
    } else {
      prev = &graph->edge.FREE_NEXT[f];
    }
    f = next;
  }
}

static CGraphId edge_get_new(CGraph *graph, const CGraphId from, const CGraphId to) {
  CGraphId eid;
  if (graph->edge.count == graph->edge.range) {
    graph->edge.free = CGRAPH_INV_ID;
    eid = (CGraphId)graph->edge.range++;
  } else {
    do {
      eid = graph->edge.free;
      graph->edge.free = graph->edge.FREE_NEXT[eid];
    } while (eid >= graph->edge.range);
  }

  graph->edge.xor_[eid] = from ^ to;
  graph->edge.to[eid] = to;
  graph->edge.count++;
  return eid;
}

static void edge_delete(CGraph *graph, const CGraphId eid) {
  graph->edge.FREE_NEXT[eid] = graph->edge.free;
  graph->edge.free = eid;
  graph->edge.STATE[eid] = DELETED;
  graph->edge.count--;

  if (eid == graph->edge.range - 1) {
    if (graph->edge.count == 0) {
      graph->edge.range = 0;
      return;
    }

    graph->edge.range--;
    while (graph->edge.STATE[graph->edge.range - 1] == DELETED) graph->edge.range--;
  }
}

static void edge_insert(const CGraph *graph, const CGraphId vid, const CGraphId eid,
                        const CGraphBool dir) {
  graph->edge.next[dir][eid] = graph->edge.head[dir][vid];
  graph->edge.head[dir][vid] = eid;
  graph->vert.degree[dir][vid]++;
}

static void edge_unlink(const CGraph *graph, const CGraphId vid, const CGraphId eid,
                        const CGraphBool dir) {
  CGraphId *next = graph->edge.next[dir];
  for (CGraphId *ptr = graph->edge.head[dir] + vid; *ptr != CGRAPH_INV_ID; ptr = next + *ptr) {
    if (*ptr == eid) {
      *ptr = next[*ptr];
      graph->vert.degree[dir][vid]--;
      break;
    }
  }
}

CGraphStatus cgraph_init(CGraph *graph, const CGraphBool directed, const CGraphSize vert_cap,
                         const CGraphSize edge_cap) {
  cgraph_check(graph_reserve(graph, directed, vert_cap, edge_cap), CGRAPH_ERR_MEMORY);
  cgraph_clear(graph);
  return CGRAPH_OK;
}

void cgraph_release(const CGraph *graph) {
  free(graph->vert.indices);
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

void cgraph_clear_edges(CGraph *graph) {
  graph->edge.count = 0;
  graph->edge.range = 0;
  graph->edge.free = CGRAPH_INV_ID;
}

void cgraph_clear(CGraph *graph) {
  graph->vert.count = 0;
  graph->vert.range = 0;
  cgraph_clear_edges(graph);
}

CGraphStatus cgraph_copy_vertices(CGraph *dst, const CGraph *src) {
  vert_fix_free(src);

  if (!graph_reserve(dst, src->edge.directed, src->vert.range, src->edge.range)) {
    return CGRAPH_ERR_MEMORY;
  }

  vert_copy(dst, src);
  edge_clear(dst);
  return CGRAPH_OK;
}

CGraphStatus cgraph_copy(CGraph *dst, CGraph *src) {
  vert_fix_free(src);
  edge_fix_free(src);

  if (!graph_reserve(dst, src->edge.directed, src->vert.range, src->edge.range)) {
    return CGRAPH_ERR_MEMORY;
  }

  vert_copy(dst, src);
  edge_copy(dst, src);
  return CGRAPH_OK;
}

CGraphStatus cgraph_add_vertex(CGraph *graph, CGraphId *vid) {
  if (graph->vert.count == graph->vert.capacity) {
    if (!vert_resize(graph, graph->vert.capacity + 1)) return CGRAPH_ERR_MEMORY;
  }

  *vid = vert_insert_new(graph);
  return CGRAPH_OK;
}

CGraphStatus cgraph_add_vertices(CGraph *graph, const CGraphSize count) {
  if (graph->vert.count + count > graph->vert.capacity) {
    if (!vert_resize(graph, graph->vert.count + count)) return CGRAPH_ERR_MEMORY;
  }

  for (CGraphSize i = 0; i < count; i++) vert_insert_new(graph);

  return CGRAPH_OK;
}

CGraphStatus cgraph_delete_vertex(CGraph *graph, const CGraphId vid) {
  cgraph_check(cgraph_is_valid_vertex(graph, vid), CGRAPH_ERR_INVALID_ID);

  CGraphId eid = graph->edge.head[OUT][vid];
  while (eid != CGRAPH_INV_ID) {
    const CGraphId next = graph->edge.next[OUT][eid];
    edge_unlink(graph, graph->edge.xor_[eid] ^ vid, eid, IN);
    edge_delete(graph, eid);
    eid = next;
  }
  graph->vert.degree[OUT][vid] = 0;
  graph->edge.head[OUT][vid] = CGRAPH_INV_ID;

  eid = graph->edge.head[IN][vid];
  while (eid != CGRAPH_INV_ID) {
    const CGraphId next = graph->edge.next[IN][eid];
    edge_unlink(graph, graph->edge.xor_[eid] ^ vid, eid, OUT);
    edge_delete(graph, eid);
    eid = next;
  }
  graph->vert.degree[IN][vid] = 0;
  graph->edge.head[IN][vid] = CGRAPH_INV_ID;

  vert_unlink_and_delete(graph, vid);
  return CGRAPH_OK;
}

CGraphStatus cgraph_add_edge(CGraph *graph, CGraphId *eid, const CGraphId from, const CGraphId to) {
  cgraph_check(cgraph_is_valid_vertex(graph, from) && cgraph_is_valid_vertex(graph, to),
               CGRAPH_ERR_INVALID_ID);

  if (graph->edge.count == graph->edge.capacity) {
    if (!edge_resize(graph, graph->edge.capacity + 1)) return CGRAPH_ERR_MEMORY;
  }

  *eid = edge_get_new(graph, from, to);
  edge_insert(graph, from, *eid, OUT);
  edge_insert(graph, to, *eid, IN);
  return CGRAPH_OK;
}

CGraphStatus cgraph_add_edges(CGraph *graph, const CGraphSize count,
                              const CGraphId endpoints[][2]) {
  for (CGraphSize i = 0; i < count; i++) {
    const CGraphId from = endpoints[i][0];
    const CGraphId to = endpoints[i][1];
    cgraph_check(cgraph_is_valid_vertex(graph, from) && cgraph_is_valid_vertex(graph, to),
                 CGRAPH_ERR_INVALID_ID);
  }

  if (graph->edge.count + count > graph->edge.capacity) {
    if (!edge_resize(graph, graph->edge.count + count)) return CGRAPH_ERR_MEMORY;
  }

  for (CGraphSize i = 0; i != count; i++) {
    const CGraphId from = endpoints[i][0];
    const CGraphId to = endpoints[i][1];
    const CGraphId eid = edge_get_new(graph, from, to);
    edge_insert(graph, from, eid, OUT);
    edge_insert(graph, to, eid, IN);
  }

  return CGRAPH_OK;
}

CGraphStatus cgraph_delete_edge(CGraph *graph, const CGraphId eid) {
  cgraph_check(cgraph_is_valid_edge(graph, eid), CGRAPH_ERR_INVALID_ID);

  const CGraphId to = graph->edge.to[eid];
  const CGraphId from = graph->edge.xor_[eid] ^ to;

  edge_unlink(graph, from, eid, OUT);
  edge_unlink(graph, to, eid, IN);
  edge_delete(graph, eid);
  return CGRAPH_OK;
}

CGraphStatus cgraph_reverse_edge(CGraph *graph, const CGraphId eid) {
  cgraph_check(cgraph_is_valid_edge(graph, eid), CGRAPH_ERR_INVALID_ID);

  if (!graph->edge.directed) return CGRAPH_OK;

  const CGraphId to = graph->edge.to[eid];
  const CGraphId from = graph->edge.xor_[eid] ^ to;

  edge_unlink(graph, from, eid, OUT);
  edge_unlink(graph, to, eid, IN);
  edge_insert(graph, from, eid, IN);
  edge_insert(graph, to, eid, OUT);

  graph->edge.to[eid] = from;
  return CGRAPH_OK;
}

CGraphStatus cgraph_find_edge(const CGraph *graph, CGraphId *eid, const CGraphId from,
                              const CGraphId to) {
  cgraph_check(cgraph_is_valid_vertex(graph, from) && cgraph_is_valid_vertex(graph, to),
               CGRAPH_ERR_INVALID_ID);

  const CGraphId xor_ = from ^ to;
  const CGraphId *next = graph->edge.next[OUT];

  for (CGraphId e = graph->edge.head[OUT][from]; e != CGRAPH_INV_ID; e = next[e]) {
    if (graph->edge.xor_[e] == xor_) {
      *eid = e;
      return CGRAPH_OK;
    }
  }

  if (!graph->edge.directed) {
    next = graph->edge.next[IN];
    for (CGraphId e = graph->edge.head[IN][from]; e != CGRAPH_INV_ID; e = next[e]) {
      if (graph->edge.xor_[e] == xor_) {
        *eid = e;
        return e;
      }
    }
  }

  *eid = CGRAPH_INV_ID;
  return CGRAPH_OK;
}

CGraphStatus cgraph_where_edge_from_to(const CGraph *graph, const CGraphId eid, CGraphId *from,
                                       CGraphId *to) {
  cgraph_check(cgraph_is_valid_edge(graph, eid), CGRAPH_ERR_INVALID_ID);

  *to = graph->edge.to[eid];
  *from = graph->edge.xor_[eid] ^ *to;
  return CGRAPH_OK;
}

// ----- iterator -----

CGraphIterator cgraph_get_vertex_iterator(const CGraph *graph) {
  return (CGraphIterator){graph, 0};
}

CGraphIterator cgraph_get_edge_iterator(const CGraph *graph, const CGraphId vid,
                                        const CGraphBool dir) {
  return (CGraphIterator){
      .view = graph,
      .vert = vid,
      .edge = graph->edge.head[dir][vid],
      .undirected = !graph->edge.directed,
      .dir_global = dir,
      .dir_current = dir
  };
}

CGraphBool cgraph_iterator_next_vertex(CGraphIterator *iter, CGraphId *vid) {
  if (iter->vert == iter->view->vert.count) return false;
  *vid = iter->view->vert.array[iter->vert++];
  return true;
}

CGraphBool cgraph_iterator_next_edge(CGraphIterator *iter, CGraphId *eid, CGraphId *res) {
again:
  if (iter->edge == CGRAPH_INV_ID) {
    if (iter->undirected && iter->dir_current == iter->dir_global) {
      iter->dir_current = !iter->dir_current;
      iter->edge = iter->view->edge.head[iter->dir_current][iter->vert];
      goto again;
    }
    return false;
  }

  *eid = iter->edge;
  *res = iter->view->edge.xor_[iter->edge] ^ iter->vert;
  iter->edge = iter->view->edge.next[iter->dir_current][iter->edge];
  return true;
}

CGraphExplorer *cgraph_new_explorer(const CGraph *graph, const CGraphBool dir) {
  CGraphExplorer *explorer;
  if (graph->edge.directed) {
    explorer = malloc(sizeof(CGraphExplorer) + graph->vert.range * sizeof(CGraphId));
    explorer->dir_current = NULL;
  } else {
    explorer = malloc(
        sizeof(CGraphExplorer) + graph->vert.range * (sizeof(CGraphId) + sizeof(CGraphBool)));
    explorer->dir_current = (CGraphBool *)(explorer->edge + graph->vert.range);
  }
  explorer->view = graph;
  cgraph_explorer_reset_vertex(explorer);
  cgraph_explorer_reset_all_edges(explorer, dir);
  return explorer;
}

void cgraph_delete_explorer(CGraphExplorer *explorer) { free(explorer); }

void cgraph_explorer_reset_vertex(CGraphExplorer *explorer) { explorer->vert = 0; }

void cgraph_explorer_reset_edge(CGraphExplorer *explorer, const CGraphId vid) {
  explorer->edge[vid] = explorer->view->edge.head[explorer->dir_global][vid];
  if (explorer->dir_current) explorer->dir_current[vid] = explorer->dir_global;
}

void cgraph_explorer_reset_all_edges(CGraphExplorer *explorer, const CGraphBool dir) {
  explorer->dir_global = dir;
  memcpy(explorer->edge, explorer->view->edge.head[dir],
         explorer->view->vert.range * sizeof(CGraphId));
  if (explorer->dir_current) {
    memset(explorer->dir_current, dir, explorer->view->vert.range * sizeof(CGraphBool));
  }
}

CGraphBool cgraph_explorer_next_vertex(CGraphExplorer *explorer, CGraphId *vid) {
  if (explorer->vert == explorer->view->vert.count) return false;
  *vid = explorer->view->vert.array[explorer->vert++];
  return true;
}

CGraphBool cgraph_explorer_next_edge(CGraphExplorer *explorer, const CGraphId vid, CGraphId *eid,
                                     CGraphId *res) {
  CGraphId *curr = explorer->edge + vid;

  // undirected
  if (explorer->dir_current) {
    CGraphBool *dir = explorer->dir_current + vid;

  again:
    if (*curr == CGRAPH_INV_ID) {
      if (*dir == explorer->dir_global) {
        *dir = !*dir;
        *curr = explorer->view->edge.head[*dir][vid];
        goto again;
      }
      return false;
    }
    *eid = *curr;
    *res = explorer->view->edge.xor_[*curr] ^ vid;
    *curr = explorer->view->edge.next[*dir][*curr];
    return true;
  }

  if (*curr == CGRAPH_INV_ID) return false;
  *eid = *curr;
  *res = explorer->view->edge.xor_[*curr] ^ vid;
  *curr = explorer->view->edge.next[explorer->dir_global][*curr];
  return true;
}

void cgraph_traverse_edges(const CGraph *graph, void *data,
                           void (*callback)(CGraphId, CGraphId, CGraphId, void *)) {
  const CGraphId *head = graph->edge.head[OUT], *next = graph->edge.next[OUT];
  for (CGraphId v = 0; v < graph->vert.count; v++) {
    const CGraphId from = graph->vert.array[v];
    for (CGraphId eid = head[from]; eid != CGRAPH_INV_ID; eid = next[eid]) {
      callback(from, eid, graph->edge.xor_[eid] ^ from, data);
    }
  }
}

// ----- file ------

#include "endian.h"

#define BATCH_SIZE (128 * 1024)
#define CGRAPH_MAGIC 0x06790608

typedef struct {
  // 64 bytes
  uint32_t magic;
  uint32_t version;
  uint32_t header_size;
  uint32_t data_offset;
  uint32_t int_bytes;

  uint32_t flags;
  uint64_t vert_count;
  uint64_t vert_range;
  uint64_t edge_count;
  uint64_t edge_range;
  uint64_t edge_free;
  // -----------------

  uint8_t reserved[]; // 64 bytes
} CGraphFileHeader;


static uint8_t *write_buffer_16bits(uint8_t *buffer, const uint16_t data) {
  const uint16_t data_net = hton16(data);
  memcpy(buffer, &data_net, 2);
  return buffer + 2;
}

static uint8_t *write_buffer_32bits(uint8_t *buffer, const uint32_t data) {
  const uint32_t data_net = hton32(data);
  memcpy(buffer, &data_net, 4);
  return buffer + 4;
}

static uint8_t *write_buffer_64bits(uint8_t *buffer, const uint64_t data) {
  const uint64_t data_net = hton64(data);
  memcpy(buffer, &data_net, 8);
  return buffer + 8;
}

static uint8_t *write_buffer_cgraph_int(uint8_t *buffer, const CGraphInt data) {
  switch (sizeof(CGraphInt)) {
  default: // case 1
    *buffer = data;
    return buffer + 1;
  case 2:
    return write_buffer_16bits(buffer, data);
  case 4:
    return write_buffer_32bits(buffer, data);
  case 8:
    return write_buffer_64bits(buffer, data);
  }
}

static CGraphBool write_header(FILE *stream, const CGraphFileHeader *header) {
  uint8_t buffer[128] = {0};
  uint8_t *ptr = buffer;
  ptr = write_buffer_32bits(ptr, header->magic);
  ptr = write_buffer_32bits(ptr, header->version);
  ptr = write_buffer_32bits(ptr, header->header_size);
  ptr = write_buffer_32bits(ptr, header->data_offset);
  ptr = write_buffer_32bits(ptr, header->int_bytes);
  ptr = write_buffer_32bits(ptr, header->flags);
  ptr = write_buffer_64bits(ptr, header->vert_count);
  ptr = write_buffer_64bits(ptr, header->vert_range);
  ptr = write_buffer_64bits(ptr, header->edge_count);
  ptr = write_buffer_64bits(ptr, header->edge_range);
  write_buffer_64bits(ptr, header->edge_free);
  return fwrite(buffer, 128, 1, stream);
}

static CGraphBool write_array(FILE *stream, const CGraphInt *array, const CGraphSize len,
                              uint8_t *buffer) {
  const CGraphSize rows = len / BATCH_SIZE;
  const CGraphInt *data_ptr = array;
  uint8_t *buff_ptr = buffer;

  for (CGraphSize r = 0; r < rows; r++) {
    for (CGraphSize c = 0; c < BATCH_SIZE; c++) {
      buff_ptr = write_buffer_cgraph_int(buff_ptr, *data_ptr++);
    }

    const size_t write = fwrite(buffer, sizeof(CGraphInt), BATCH_SIZE, stream);
    if (write != BATCH_SIZE) return false;
    buff_ptr = buffer;
  }

  const size_t remaining = len - rows * BATCH_SIZE;
  if (remaining == 0) return true;

  for (size_t i = 0; i < remaining; i++) {
    buff_ptr = write_buffer_cgraph_int(buff_ptr, *data_ptr++);
  }
  return remaining == fwrite(buffer, sizeof(CGraphInt), remaining, stream);
}

CGraphStatus cgraph_save_binary_s(CGraph *graph, FILE *stream) {
  vert_fix_free(graph);
  edge_fix_free(graph);

  const CGraphFileHeader header = {
      .magic = CGRAPH_MAGIC,
      .version = CGRAPH_VERSION,
      .header_size = 64,
      .data_offset = 128,
      .int_bytes = sizeof(CGraphInt),

      .flags = graph->edge.directed,
      .vert_count = graph->vert.count,
      .vert_range = graph->vert.range,
      .edge_count = graph->edge.count,
      .edge_range = graph->edge.range,
      .edge_free = graph->edge.free,
  };

  if (!write_header(stream, &header)) return CGRAPH_ERR_FILE_WRITE;

  uint8_t *buffer = malloc(BATCH_SIZE * sizeof(CGraphInt));
  if (buffer == NULL) return CGRAPH_ERR_MEMORY;

  if (write_array(stream, graph->vert.array, graph->vert.range, buffer)
      && write_array(stream, graph->edge.head[OUT], graph->vert.range, buffer)
      && write_array(stream, graph->edge.next[OUT], graph->edge.range, buffer)
      && write_array(stream, graph->edge.to, graph->edge.range, buffer)) {
    free(buffer);
    return CGRAPH_OK;
  }

  free(buffer);
  return CGRAPH_ERR_FILE_WRITE;
}

CGraphStatus cgraph_save_binary(CGraph *graph, const char *path) {
  FILE *file = fopen(path, "wb");
  if (!file) return CGRAPH_ERR_FILE_OPEN;

  const CGraphStatus status = cgraph_save_binary_s(graph, file);
  fclose(file);
  return status;
}

static uint8_t *read_buffer_16bits(uint8_t *buffer, uint16_t *data) {
  memcpy(data, buffer, 2);
  *data = ntoh16(*data);
  return buffer + 2;
}

static uint8_t *read_buffer_32bits(uint8_t *buffer, uint32_t *data) {
  memcpy(data, buffer, 4);
  *data = ntoh32(*data);
  return buffer + 4;
}

static uint8_t *read_buffer_64bits(uint8_t *buffer, uint64_t *data) {
  memcpy(data, buffer, 8);
  *data = ntoh64(*data);
  return buffer + 8;
}

static uint8_t *read_buffer_cgraph_int(uint8_t *buffer, CGraphInt *data) {
  switch (sizeof(CGraphInt)) {
  default: // case 1
    *data = *buffer;
    return buffer + 1;
  case 2:
    return read_buffer_16bits(buffer, (uint16_t *)data);
  case 4:
    return read_buffer_32bits(buffer, (uint32_t *)data);
  case 8:
    return read_buffer_64bits(buffer, (uint64_t *)data);
  }
}

static CGraphBool read_header(FILE *stream, CGraphFileHeader *header) {
  uint8_t buffer[128];
  if (!fread(buffer, 128, 1, stream)) return false;

  uint8_t *ptr = buffer;
  ptr = read_buffer_32bits(ptr, &header->magic);
  ptr = read_buffer_32bits(ptr, &header->version);
  ptr = read_buffer_32bits(ptr, &header->header_size);
  ptr = read_buffer_32bits(ptr, &header->data_offset);
  ptr = read_buffer_32bits(ptr, &header->int_bytes);
  ptr = read_buffer_32bits(ptr, &header->flags);
  ptr = read_buffer_64bits(ptr, &header->vert_count);
  ptr = read_buffer_64bits(ptr, &header->vert_range);
  ptr = read_buffer_64bits(ptr, &header->edge_count);
  ptr = read_buffer_64bits(ptr, &header->edge_range);
  read_buffer_64bits(ptr, &header->edge_free);
  return true;
}

static CGraphBool read_array(FILE *stream, CGraphInt *array, const CGraphSize len,
                             uint8_t *buffer) {
  const CGraphSize rows = len / BATCH_SIZE;
  CGraphInt *data_ptr = array;
  uint8_t *buff_ptr = buffer;

  for (CGraphSize r = 0; r < rows; r++) {
    const size_t read = fread(buff_ptr, sizeof(CGraphInt), BATCH_SIZE, stream);
    if (read < BATCH_SIZE) return false;

    for (CGraphSize c = 0; c < BATCH_SIZE; c++) {
      buff_ptr = read_buffer_cgraph_int(buff_ptr, data_ptr++);
    }
    buff_ptr = buffer;
  }

  const size_t remaining = len - rows * BATCH_SIZE;
  if (remaining == 0) return true;

  const size_t read = fread(buff_ptr, sizeof(CGraphInt), remaining, stream);
  if (read < remaining) return false;

  for (size_t i = 0; i < remaining; i++) {
    buff_ptr = read_buffer_cgraph_int(buff_ptr, data_ptr++);
  }
  return true;
}

static void rebuild_graph(const CGraph *graph) {
  memset(graph->edge.head[IN], CGRAPH_INV_ID, sizeof(CGraphId) * graph->vert.range);
  memset(graph->vert.degree[OUT], 0, sizeof(CGraphInt) * graph->vert.range);
  if (graph->edge.directed) {
    memset(graph->vert.degree[IN], 0, sizeof(CGraphInt) * graph->vert.range);
  }

  const CGraphId *head = graph->edge.head[OUT], *next = graph->edge.next[OUT];
  for (CGraphSize i = 0; i < graph->vert.range; i++) {
    const CGraphId from = graph->vert.array[i];
    graph->vert.indices[from] = (CGraphId)i;
    if (i >= graph->vert.count) continue;

    for (CGraphId eid = head[from]; eid != CGRAPH_INV_ID; eid = next[eid]) {
      const CGraphId to = graph->edge.to[eid];
      graph->vert.degree[OUT][from]++;
      edge_insert(graph, to, eid, IN);
      graph->edge.xor_[eid] = from ^ to;
    }
  }

  for (CGraphId f = graph->edge.free; f != CGRAPH_INV_ID; f = graph->edge.FREE_NEXT[f]) {
    graph->edge.STATE[f] = DELETED;
  }
}

CGraphStatus cgraph_load_binary_s(CGraph *graph, FILE *stream) {
  CGraphFileHeader header;
  if (!read_header(stream, &header)) return CGRAPH_ERR_FILE_READ;

  if (header.magic != CGRAPH_MAGIC) return CGRAPH_ERR_FILE_FORMAT;
  if (header.version != CGRAPH_VERSION) return CGRAPH_ERR_FILE_VERSION;
  if (header.int_bytes != sizeof(CGraphInt)) return CGRAPH_ERR_GENERAL;

  if (!graph_reserve(graph, header.flags, header.vert_range, header.edge_range)) {
    return CGRAPH_ERR_MEMORY;
  }

  graph->vert.count = header.vert_count;
  graph->vert.range = header.vert_range;
  graph->edge.count = header.edge_count;
  graph->edge.range = header.edge_range;
  graph->edge.free = (CGraphId)header.edge_free;

  uint8_t *buffer = malloc(BATCH_SIZE * sizeof(CGraphInt));
  if (buffer == NULL) return CGRAPH_ERR_MEMORY;

  if (read_array(stream, graph->vert.array, graph->vert.range, buffer)
      && read_array(stream, graph->edge.head[OUT], graph->vert.range, buffer)
      && read_array(stream, graph->edge.next[OUT], graph->edge.range, buffer)
      && read_array(stream, graph->edge.to, graph->edge.range, buffer)) {
    free(buffer);
    rebuild_graph(graph);
    return CGRAPH_OK;
  }

  free(buffer);
  cgraph_release(graph);
  return CGRAPH_ERR_FILE_READ;
}

CGraphStatus cgraph_load_binary(CGraph *graph, const char *path) {
  FILE *file = fopen(path, "rb");
  if (!file) return CGRAPH_ERR_FILE_OPEN;

  const CGraphStatus status = cgraph_load_binary_s(graph, file);
  fclose(file);
  return status;
}