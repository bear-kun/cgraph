#include "cgraph/iter.h"
#include <stdlib.h>
#include <string.h>

#define OUT 0
#define IN 1

static CGraphExplorer *getNewExplorer(const CGraph *graph) {
  CGraphExplorer *iter;
  if (graph->edge.directed) {
    iter = malloc(sizeof(CGraphExplorer) + graph->vert.range * sizeof(CGraphId));
    iter->curr.dir = NULL;
  } else {
    iter = malloc(
        sizeof(CGraphExplorer) + graph->vert.range * (sizeof(CGraphId) + sizeof(CGraphBool)));
    iter->curr.dir = (CGraphBool *)(iter->curr.edge + graph->vert.range);
  }
  iter->view = graph;
  cgraphExplorerResetVert(iter);
  return iter;
}

CGraphExplorer *cgraphGetExplorer(const CGraph *graph) {
  CGraphExplorer *iter = getNewExplorer(graph);
  cgraphExplorerResetAllEdges(iter);
  return iter;
}

CGraphExplorer *cgraphGetExplorerRev(const CGraph *graph) {
  CGraphExplorer *iter = getNewExplorer(graph);
  cgraphExplorerResetAllEdgesRev(iter);
  return iter;
}

void cgraphExplorerRelease(CGraphExplorer *iter) { free(iter); }

void cgraphExplorerResetVert(CGraphExplorer *iter) {
  iter->curr.vert = iter->view->vert.head;
}

void cgraphExplorerResetEdge(CGraphExplorer *iter, const CGraphId from) {
  iter->curr.edge[from] = iter->view->edge.head[OUT][from];
  if (iter->curr.dir) iter->curr.dir[from] = OUT;
}

void cgraphExplorerResetAllEdges(CGraphExplorer *iter) {
  memcpy(iter->curr.edge, iter->view->edge.head[OUT], iter->view->vert.range * sizeof(CGraphId));
  if (iter->curr.dir) memset(iter->curr.dir, OUT, iter->view->vert.range * sizeof(CGraphBool));
}

void cgraphExplorerResetEdgeRev(CGraphExplorer *iter, const CGraphId to) {
  iter->curr.edge[to] = iter->view->edge.head[IN][to];
  if (iter->curr.dir) iter->curr.dir[to] = IN;
}

void cgraphExplorerResetAllEdgesRev(CGraphExplorer *iter) {
  memcpy(iter->curr.edge, iter->view->edge.head[IN], iter->view->vert.range * sizeof(CGraphId));
  if (iter->curr.dir) memset(iter->curr.dir, IN, iter->view->vert.range * sizeof(CGraphBool));
}

CGraphBool cgraphExplorerNextVert(CGraphExplorer *iter, CGraphId *vid) {
  if (iter->curr.vert == INVALID_ID) return false;
  *vid = iter->curr.vert;
  iter->curr.vert = iter->view->vert.next[*vid];
  return true;
}

static CGraphBool explorerNextEdge(CGraphExplorer *iter, const CGraphId vid, CGraphId *eid,
                                   CGraphId *other, const CGraphBool D) {
  CGraphId *curr = iter->curr.edge + vid;

  if (iter->curr.dir) {
    CGraphBool *dir = iter->curr.dir + vid;
  again:
    if (*curr == INVALID_ID) {
      if (*dir == D) {
        *curr = iter->view->edge.head[!D][vid];
        *dir = !D;
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
  *curr = iter->view->edge.next[D][*curr];
  return true;
}

CGraphBool cgraphExplorerNextEdge(CGraphExplorer *iter, const CGraphId from, CGraphId *eid,
                                  CGraphId *to) {
  return explorerNextEdge(iter, from, eid, to, OUT);
}

CGraphBool cgraphExplorerNextEdgeRev(CGraphExplorer *iter, const CGraphId to, CGraphId *eid,
                                     CGraphId *from) {
  return explorerNextEdge(iter, to, eid, from, IN);
}

CGraphIter cgraphGetVertIter(const CGraph *graph) {
  return (CGraphIter){graph, graph->vert.head};
}

CGraphIter cgraphGetEdgeIter(const CGraph *graph, const CGraphId from) {
  return (CGraphIter){graph, graph->edge.head[OUT][from], from, OUT, !graph->edge.directed};
}

CGraphIter cgraphGetEdgeIterRev(const CGraph *graph, const CGraphId to) {
  return (CGraphIter){graph, graph->edge.head[IN][to], to, IN, !graph->edge.directed};
}

CGraphBool cgraphIterNextVert(CGraphIter *iter, CGraphId *vid) {
  if (iter->curr == INVALID_ID) return false;
  *vid = iter->curr;
  iter->curr = iter->view->vert.next[iter->curr];
  return true;
}

static CGraphBool iterNextEdge(CGraphIter *iter, CGraphId *eid, CGraphId *other,
                               const CGraphBool dir) {
again:
  if (iter->curr == INVALID_ID) {
    if (iter->undirected && iter->dir == dir) {
      iter->curr = iter->view->edge.head[!dir][iter->vert];
      iter->dir = !dir;
      goto again;
    }
    return false;
  }

  *eid = iter->curr;
  *other = iter->view->edge.xor[iter->curr] ^ iter->vert;
  iter->curr = iter->view->edge.next[iter->dir][iter->curr];
  return true;
}

CGraphBool cgraphIterNextEdge(CGraphIter *iter, CGraphId *eid, CGraphId *to) {
  return iterNextEdge(iter, eid, to, OUT);
}

CGraphBool cgraphIterNextEdgeRev(CGraphIter *iter, CGraphId *eid, CGraphId *from) {
  return iterNextEdge(iter, eid, from, IN);
}

void cgraphTraverseEdges(const CGraph *graph, void *data,
                         void (*callback)(CGraphId, CGraphId, CGraphId, void *)) {
  const CGraphId *next = graph->edge.next[OUT];
  for (CGraphId from = graph->vert.head; from != INVALID_ID; from = graph->vert.next[from]) {
    for (CGraphId eid = graph->edge.head[OUT][from]; eid != INVALID_ID; eid = next[eid]) {
      callback(from, eid, graph->edge.xor[eid] ^ from, data);
    }
  }
}