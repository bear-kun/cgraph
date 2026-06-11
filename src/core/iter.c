#include "cgraph/iter.h"
#include <stdlib.h>
#include <string.h>

#define OUT CGRAPH_OUT
#define IN CGRAPH_IN

CGraphExplorer *cgraphGetExplorer(const CGraph *graph, const CGraphBool dir) {
  CGraphExplorer *iter;
  if (graph->edge.directed) {
    iter = malloc(sizeof(CGraphExplorer) + graph->vert.range * sizeof(CGraphId));
    iter->dir_current = NULL;
  } else {
    iter = malloc(
        sizeof(CGraphExplorer) + graph->vert.range * (sizeof(CGraphId) + sizeof(CGraphBool)));
    iter->dir_current = (CGraphBool *)(iter->edge + graph->vert.range);
  }
  iter->view = graph;
  cgraphExplorerResetVert(iter);
  cgraphExplorerResetAllEdges(iter, dir);
  return iter;
}

void cgraphExplorerRelease(CGraphExplorer *iter) { free(iter); }

void cgraphExplorerResetVert(CGraphExplorer *iter) {
  iter->vert = 0;
}

void cgraphExplorerResetEdge(CGraphExplorer *iter, const CGraphId vid) {
  iter->edge[vid] = iter->view->edge.head[iter->dir_global][vid];
  if (iter->dir_current) iter->dir_current[vid] = iter->dir_global;
}

void cgraphExplorerResetAllEdges(CGraphExplorer *iter, const CGraphBool dir) {
  iter->dir_global = dir;
  memcpy(iter->edge, iter->view->edge.head[dir], iter->view->vert.range * sizeof(CGraphId));
  if (iter->dir_current)
    memset(iter->dir_current, dir,
           iter->view->vert.range * sizeof(CGraphBool));
}

CGraphBool cgraphExplorerNextVert(CGraphExplorer *iter, CGraphId *vid) {
  if (iter->vert == iter->view->vert.count) return false;
  *vid = iter->view->vert.array[iter->vert++];
  return true;
}

CGraphBool cgraphExplorerNextEdge(CGraphExplorer *iter, const CGraphId vid, CGraphId *eid,
                                  CGraphId *other) {
  CGraphId *curr = iter->edge + vid;

  // undirected
  if (iter->dir_current) {
    CGraphBool *dir = iter->dir_current + vid;

  again:
    if (*curr == INVALID_ID) {
      if (*dir == iter->dir_global) {
        *dir = !*dir;
        *curr = iter->view->edge.head[*dir][vid];
        goto again;
      }
      return false;
    }
    *eid = *curr;
    *other = iter->view->edge.xor[*curr] ^ vid;
    *curr = iter->view->edge.next[*dir][*curr];
    return true;
  }

  if (*curr == INVALID_ID) return false;
  *eid = *curr;
  *other = iter->view->edge.xor[*curr] ^ vid;
  *curr = iter->view->edge.next[iter->dir_global][*curr];
  return true;
}

CGraphIter cgraphGetVertIter(const CGraph *graph) {
  return (CGraphIter){graph, 0};
}

CGraphIter cgraphGetEdgeIter(const CGraph *graph, const CGraphId vid, const CGraphBool dir) {
  return (CGraphIter){
      .view = graph,
      .vert = vid,
      .edge = graph->edge.head[dir][vid],
      .dir_current = dir,
      .dir_global = dir,
      .undirected = !graph->edge.directed
  };
}

CGraphBool cgraphIterNextVert(CGraphIter *iter, CGraphId *vid) {
  if (iter->vert == iter->view->vert.count) return false;
  *vid = iter->view->vert.array[iter->vert++];
  return true;
}

CGraphBool cgraphIterNextEdge(CGraphIter *iter, CGraphId *eid, CGraphId *other) {
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
  *other = iter->view->edge.xor[iter->edge] ^ iter->vert;
  iter->edge = iter->view->edge.next[iter->dir_current][iter->edge];
  return true;
}


void cgraphTraverseEdges(const CGraph *graph, void *data,
                         void (*callback)(CGraphId, CGraphId, CGraphId, void *)) {
  const CGraphId *head = graph->edge.head[OUT], *next = graph->edge.next[OUT];
  for (CGraphId v = 0; v < graph->vert.count; v++) {
    const CGraphId from = graph->vert.array[v];
    for (CGraphId eid = head[from]; eid != INVALID_ID; eid = next[eid]) {
      callback(from, eid, graph->edge.xor[eid] ^ from, data);
    }
  }
}