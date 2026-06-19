#ifndef CGRAPH_GRAPH_H
#define CGRAPH_GRAPH_H

#include "types.h"

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
// do not check validity
void cgraph_delete_edge(CGraph *graph, CGraphId eid);
// do not check validity
void cgraph_reverse_edge(const CGraph *graph, CGraphId eid);
// do not check validity
CGraphId cgraph_find_edge(const CGraph *graph, CGraphId from, CGraphId to);
// do not check validity
void cgraph_where_edge_from_to(const CGraph *graph, CGraphId eid, CGraphId *from, CGraphId *to);

CGraphBool cgraph_save_binary(CGraph *graph, const char *path);
CGraphBool cgraph_load_binary(CGraph *graph, const char *path); // Don't init graph !

#ifdef __cplusplus
}
#endif
#endif // CGRAPH_GRAPH_H