#ifndef CGRAPH_GRAPH_H
#define CGRAPH_GRAPH_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <float.h>

// set CGRAPH_EPSILON = 0 if WeightType is Integer

#define CGRAPH_VERSION 0
#define CGRAPH_INV_ID (-1)
#define CGRAPH_INF FLT_MAX
#define CGRAPH_EPS 1e-6f
#define CGRAPH_OUT 0
#define CGRAPH_IN 1

typedef bool CGraphBool;
typedef int32_t CGraphInt;
typedef uint32_t CGraphUint;
typedef CGraphInt CGraphId;
typedef CGraphUint CGraphSize;

typedef float WeightType;
typedef WeightType TimeType; // aoa
typedef WeightType FlowType; // flow

typedef enum {
  CGRAPH_OK = 0,

  CGRAPH_ERR_GENERAL,
  CGRAPH_ERR_MEMORY,
  CGRAPH_ERR_INVALID_ID,

  CGRAPH_ERR_FILE_OPEN,
  CGRAPH_ERR_FILE_FORMAT,
  CGRAPH_ERR_FILE_VERSION,
  CGRAPH_ERR_FILE_READ,
  CGRAPH_ERR_FILE_WRITE,
} CGraphStatus;

// O(5 * V + 4 * E)

typedef struct {
  struct {
    CGraphSize capacity, count, range;
    CGraphId *indices, *array;
    CGraphInt *degree[2];
  } vert;

  struct {
    CGraphBool directed;
    CGraphSize capacity, count, range;
    CGraphId free, *head[2], *next[2];
    CGraphId *xor_, *to;
  } edge;
} CGraph;

typedef struct {
  const CGraph *view;
  CGraphId vert; // not vid
  CGraphBool dir_global;
  CGraphBool *dir_current;
  CGraphId edge[];
} CGraphExplorer;

typedef struct {
  const CGraph *view;
  CGraphId vert; // not vid if vert-iter
  CGraphId edge;
  CGraphBool undirected;
  CGraphBool dir_global;
  CGraphBool dir_current;
} CGraphIterator;

#ifdef __cplusplus
extern "C" {
#endif

CGraphBool cgraph_init(CGraph *graph, CGraphBool directed, CGraphSize vert_cap,
                       CGraphSize edge_cap);
void cgraph_release(const CGraph *graph);
CGraphBool cgraph_copy(CGraph *dst, CGraph *src);
CGraphBool cgraph_copy_vertices(CGraph *dst, const CGraph *src);
void cgraph_clear_edges(CGraph *graph);
void cgraph_clear(CGraph *graph);

CGraphId cgraph_add_vertex(CGraph *graph);
CGraphBool cgraph_add_vertices(CGraph *graph, CGraphSize count);
void cgraph_delete_vertex(CGraph *graph, CGraphId vid);

CGraphId cgraph_add_edge(CGraph *graph, CGraphId from, CGraphId to);
CGraphBool cgraph_add_edges(CGraph *graph, CGraphSize count, const CGraphId endpoints[][2]);
void cgraph_delete_edge(CGraph *graph, CGraphId eid);
void cgraph_reverse_edge(const CGraph *graph, CGraphId eid);
CGraphId cgraph_find_edge(const CGraph *graph, CGraphId from, CGraphId to);
void cgraph_where_edge_from_to(const CGraph *graph, CGraphId eid, CGraphId *from, CGraphId *to);

CGraphStatus cgraph_save_binary(CGraph *graph, const char *path);
CGraphStatus cgraph_save_binary_s(CGraph *graph, FILE *stream);
CGraphStatus cgraph_load_binary(CGraph *graph, const char *path); // Don't init graph !
CGraphStatus cgraph_load_binary_s(CGraph *graph, FILE *stream); // Don't init graph !

// ----- iterator -----
CGraphIterator cgraph_get_vertex_iterator(const CGraph *graph);
CGraphIterator cgraph_get_edge_iterator(const CGraph *graph, CGraphId vid, CGraphBool dir);

CGraphBool cgraph_iterator_next_vertex(CGraphIterator *iter, CGraphId *vid);
// reverse of the insertion order
CGraphBool cgraph_iterator_next_edge(CGraphIterator *iter, CGraphId *eid, CGraphId *other);

CGraphExplorer *cgraph_new_explorer(const CGraph *graph, CGraphBool dir);
void cgraph_delete_explorer(CGraphExplorer *explorer);

void cgraph_explorer_reset_vertex(CGraphExplorer *explorer);
void cgraph_explorer_reset_edge(CGraphExplorer *explorer, CGraphId vid);
void cgraph_explorer_reset_all_edges(CGraphExplorer *explorer, CGraphBool dir);

CGraphBool cgraph_explorer_next_vertex(CGraphExplorer *explorer, CGraphId *vid);
// reverse of the insertion order
CGraphBool cgraph_explorer_next_edge(CGraphExplorer *explorer, CGraphId vid, CGraphId *eid,
                                     CGraphId *other);

void cgraph_traverse_edges(const CGraph *graph, void *data,
                           void (*callback)(CGraphId from, CGraphId eid, CGraphId to, void *data));

#ifdef __cplusplus
}
#endif
#endif // CGRAPH_GRAPH_H