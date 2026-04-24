#include "cgraph/iter.h"
#include <stdlib.h>
#include <string.h>

#define OUT 0
#define IN 1

CGraphIter *cgraphGetIter(const CGraph *graph) {
  CGraphIter *iter;
  if (graph->edge.directed) {
    iter = malloc(sizeof(CGraphIter) + graph->vert.range * sizeof(CGraphId));
    iter->curr.dir = NULL;
  } else {
    iter = malloc(sizeof(CGraphIter) + graph->vert.range * (sizeof(CGraphId) + sizeof(CGraphBool)));
    iter->curr.dir = (CGraphBool *)(iter->curr.edge + graph->vert.range);
  }
  iter->view = graph;
  cgraphIterResetVert(iter);
  cgraphIterResetAllEdges(iter);
  return iter;
}

void cgraphIterRelease(CGraphIter *iter) { free(iter); }

void cgraphIterResetVert(CGraphIter *iter) {
  iter->curr.vert = iter->view->vert.head;
}

void cgraphIterResetEdge(CGraphIter *iter, const CGraphId from) {
  iter->curr.edge[from] = iter->view->edge.head[OUT][from];
  if (iter->curr.dir) iter->curr.dir[from] = 0;
}

void cgraphIterResetAllEdges(CGraphIter *iter) {
  memcpy(iter->curr.edge, iter->view->edge.head[OUT], iter->view->vert.range * sizeof(CGraphId));
  if (iter->curr.dir) memset(iter->curr.dir, 0, iter->view->vert.range * sizeof(CGraphBool));
}

CGraphBool cgraphIterNextVert(CGraphIter *iter, CGraphId *vid) {
  if (iter->curr.vert == INVALID_ID) return false;
  *vid = iter->curr.vert;
  iter->curr.vert = iter->view->vert.next[*vid];
  return true;
}

CGraphBool cgraphIterNextEdge(CGraphIter *iter, const CGraphId from, CGraphId *eid, CGraphId *to) {
  CGraphId *curr = iter->curr.edge + from;
  if (iter->curr.dir) {
    CGraphBool *dir = iter->curr.dir + from;
    again:
    if (*curr == INVALID_ID) {
      if (*dir == OUT) {
        *curr = iter->view->edge.head[IN][from];
        *dir = IN;
        goto again;
      }
      return false;
    }
    *eid = *curr;
    *curr = iter->view->edge.next[*dir][*eid];
  } else {
    if (*curr == INVALID_ID) return false;
    *eid = *curr;
    *curr = iter->view->edge.next[OUT][*eid];
  }
  *to = iter->view->edge.xor[*eid] ^ from;
  return true;
}

CGraphIterLite cgraphGetVertIter(const CGraph *graph) {
  return (CGraphIterLite){graph, graph->vert.head};
}

CGraphIterLite cgraphGetEdgeIter(const CGraph *graph, const CGraphId from) {
  return (CGraphIterLite){graph, graph->edge.head[OUT][from], from, OUT};
}

CGraphIterLite cgraphGetEdgeIterRev(const CGraph *graph, const CGraphId to) {
  return (CGraphIterLite){graph, graph->edge.head[IN][to], to, IN};
}

CGraphBool cgraphIterLiteNextVert(CGraphIterLite *iter, CGraphId *vid) {
  if (iter->curr == INVALID_ID) return false;
  *vid = iter->curr;
  iter->curr = iter->view->vert.next[iter->curr];
  return true;
}

CGraphBool cgraphIterLiteNextEdge(CGraphIterLite *iter, CGraphId *eid, CGraphId *to) {
again:
  if (iter->curr == INVALID_ID) {
    if (!iter->view->edge.directed && iter->dir == OUT) {
      iter->curr = iter->view->edge.head[IN][iter->vert];
      iter->dir = IN;
      goto again;
    }
    return false;
  }

  *eid = iter->curr;
  *to = iter->view->edge.xor[iter->curr] ^ iter->vert;
  iter->curr = iter->view->edge.next[iter->dir][iter->curr];
  return true;
}

CGraphBool cgraphIterLiteNextEdgeRev(CGraphIterLite *iter, CGraphId *eid, CGraphId *from) {
again:
  if (iter->curr == INVALID_ID) {
    if (!iter->view->edge.directed && iter->dir == IN) {
      iter->curr = iter->view->edge.head[OUT][iter->vert];
      iter->dir = OUT;
      goto again;
    }
    return false;
  }

  *eid = iter->curr;
  *from = iter->view->edge.xor[iter->curr] ^ iter->vert;
  iter->curr = iter->view->edge.next[iter->dir][iter->curr];
  return true;
}

void cgraphTraverseEdges(const CGraph *graph, void *userData,
                         void (*callback)(CGraphId, CGraphId, CGraphId, void *)) {
  const CGraphId *next = graph->edge.next[OUT];
  for (CGraphId from = graph->vert.head; from != INVALID_ID; from = graph->vert.next[from]) {
    for (CGraphId eid = graph->edge.head[OUT][from]; eid != INVALID_ID; eid = next[eid]) {
      callback(from, eid, graph->edge.xor[eid] ^ from, userData);
    }
  }
}