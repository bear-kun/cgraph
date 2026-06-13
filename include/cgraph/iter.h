#ifndef GRAPH_ITER_H
#define GRAPH_ITER_H

#include "types.h"

#ifdef __cplusplus
extern "C" {


#endif

// explorer
CGraphExplorer *cgraph_new_explorer(const CGraph *graph, CGraphBool dir);
void cgraph_delete_explorer(CGraphExplorer *explorer);

void cgraph_explorer_reset_vertex(CGraphExplorer *explorer);
void cgraph_explorer_reset_edge(CGraphExplorer *explorer, CGraphId vid);
void cgraph_explorer_reset_all_edges(CGraphExplorer *explorer, CGraphBool dir);

CGraphBool cgraph_explorer_next_vertex(CGraphExplorer *explorer, CGraphId *vid);
// reverse of the insertion order
CGraphBool cgraph_explorer_next_edge(CGraphExplorer *explorer, CGraphId vid, CGraphId *eid,
                                     CGraphId *other);

// iter
CGraphIter cgraph_get_vertex_iter(const CGraph *graph);
CGraphIter cgraph_get_edge_iter(const CGraph *graph, CGraphId vid, CGraphBool dir);

CGraphBool cgraph_iter_next_vertex(CGraphIter *iter, CGraphId *vid);
// reverse of the insertion order
CGraphBool cgraph_iter_next_edge(CGraphIter *iter, CGraphId *eid, CGraphId *other);

void cgraph_traverse_edges(const CGraph *graph, void *data,
                           void (*callback)(CGraphId from, CGraphId eid, CGraphId to, void *data));

#ifdef __cplusplus
}
#endif
#endif // GRAPH_ITER_H