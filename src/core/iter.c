#include "cgraph/iter.h"
#include <stdlib.h>
#include <string.h>

#define OUT CGRAPH_OUT
#define IN CGRAPH_IN

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

void cgraph_explorer_reset_vertex(CGraphExplorer *explorer) {
  explorer->vert = 0;
}

void cgraph_explorer_reset_edge(CGraphExplorer *explorer, const CGraphId vid) {
  explorer->edge[vid] = explorer->view->edge.head[explorer->dir_global][vid];
  if (explorer->dir_current) explorer->dir_current[vid] = explorer->dir_global;
}

void cgraph_explorer_reset_all_edges(CGraphExplorer *explorer, const CGraphBool dir) {
  explorer->dir_global = dir;
  memcpy(explorer->edge, explorer->view->edge.head[dir],
         explorer->view->vert.range * sizeof(CGraphId));
  if (explorer->dir_current)
    memset(explorer->dir_current, dir,
           explorer->view->vert.range * sizeof(CGraphBool));
}

CGraphBool cgraph_explorer_next_vertex(CGraphExplorer *explorer, CGraphId *vid) {
  if (explorer->vert == explorer->view->vert.count) return false;
  *vid = explorer->view->vert.array[explorer->vert++];
  return true;
}

CGraphBool cgraph_explorer_next_edge(CGraphExplorer *explorer, const CGraphId vid, CGraphId *eid,
                                     CGraphId *other) {
  CGraphId *curr = explorer->edge + vid;

  // undirected
  if (explorer->dir_current) {
    CGraphBool *dir = explorer->dir_current + vid;

  again:
    if (*curr == INVALID_ID) {
      if (*dir == explorer->dir_global) {
        *dir = !*dir;
        *curr = explorer->view->edge.head[*dir][vid];
        goto again;
      }
      return false;
    }
    *eid = *curr;
    *other = explorer->view->edge.xor_[*curr] ^ vid;
    *curr = explorer->view->edge.next[*dir][*curr];
    return true;
  }

  if (*curr == INVALID_ID) return false;
  *eid = *curr;
  *other = explorer->view->edge.xor_[*curr] ^ vid;
  *curr = explorer->view->edge.next[explorer->dir_global][*curr];
  return true;
}

CGraphIter cgraph_get_vertex_iter(const CGraph *graph) {
  return (CGraphIter){graph, 0};
}

CGraphIter cgraph_get_edge_iter(const CGraph *graph, const CGraphId vid, const CGraphBool dir) {
  return (CGraphIter){
      .view = graph,
      .vert = vid,
      .edge = graph->edge.head[dir][vid],
      .dir_current = dir,
      .dir_global = dir,
      .undirected = !graph->edge.directed
  };
}

CGraphBool cgraph_iter_next_vertex(CGraphIter *iter, CGraphId *vid) {
  if (iter->vert == iter->view->vert.count) return false;
  *vid = iter->view->vert.array[iter->vert++];
  return true;
}

CGraphBool cgraph_iter_next_edge(CGraphIter *iter, CGraphId *eid, CGraphId *other) {
again:
  if (iter->edge == INVALID_ID) {
    if (iter->undirected && iter->dir_current == iter->dir_global) {
      iter->dir_current = !iter->dir_current;
      iter->edge = iter->view->edge.head[iter->dir_current][iter->vert];
      goto again;
    }
    return false;
  }

  *eid = iter->edge;
  *other = iter->view->edge.xor_[iter->edge] ^ iter->vert;
  iter->edge = iter->view->edge.next[iter->dir_current][iter->edge];
  return true;
}

void cgraph_traverse_edges(const CGraph *graph, void *data,
                           void (*callback)(CGraphId, CGraphId, CGraphId, void *)) {
  const CGraphId *head = graph->edge.head[OUT], *next = graph->edge.next[OUT];
  for (CGraphId v = 0; v < graph->vert.count; v++) {
    const CGraphId from = graph->vert.array[v];
    for (CGraphId eid = head[from]; eid != INVALID_ID; eid = next[eid]) {
      callback(from, eid, graph->edge.xor_[eid] ^ from, data);
    }
  }
}