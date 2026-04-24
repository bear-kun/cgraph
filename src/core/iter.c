#include "cgraph/iter.h"
#include <stdlib.h>
#include <string.h>

#define OUT 0
#define IN 1

CGraphIter *cgraphGetIter(const CGraph *graph) {
  CGraphIter *iter = malloc(sizeof(CGraphIter) + graph->vert.range * sizeof(CGraphId));
  iter->view = graph;
  iter->vertCurr = graph->vert.head;
  memcpy(iter->edgeCurr, graph->edge.head[OUT], graph->vert.range * sizeof(CGraphId));
  return iter;
}

void cgraphIterRelease(CGraphIter *iter) { free(iter); }

void cgraphIterResetVert(CGraphIter *iter) {
  iter->vertCurr = iter->view->vert.head;
}

void cgraphIterResetEdge(CGraphIter *iter, const CGraphId from) {
  const CGraph *view = iter->view;
  if (from == INVALID_ID) {
    memcpy(iter->edgeCurr, view->edge.head[OUT], view->vert.range * sizeof(CGraphId));
  } else {
    iter->edgeCurr[from] = view->edge.head[OUT][from];
  }
}

static void parseF(const CGraph *graph, const CGraphId did, CGraphId *eid, CGraphId *to) {
  *eid = did >> 1;
  *to = (did & 1 ? graph->edge.from : graph->edge.to)[*eid];
}

static void parseB(const CGraph *graph, const CGraphId did, CGraphId *eid, CGraphId *from) {
  *eid = did >> 1;
  *from = (did & 1 ? graph->edge.to : graph->edge.from)[*eid];
}

CGraphBool cgraphIterNextVert(CGraphIter *iter, CGraphId *vid) {
  if (iter->vertCurr == INVALID_ID) return false;
  *vid = iter->vertCurr;
  iter->vertCurr = iter->view->vert.next[iter->vertCurr];
  return true;
}

CGraphBool cgraphIterNextEdge(CGraphIter *iter, const CGraphId from, CGraphId *eid, CGraphId *to) {
  CGraphId curr = iter->edgeCurr[from];
  if (curr == INVALID_ID) return false;
  parseF(iter->view, curr, eid, to);
  iter->edgeCurr[from] = iter->view->edge.next[curr & 1][curr >> 1];
  return true;
}

CGraphIterLite cgraphGetVertIter(const CGraph *graph) {
  return (CGraphIterLite){graph, graph->vert.head};
}

CGraphIterLite cgraphGetEdgeIter(const CGraph *graph, const CGraphId from) {
  return (CGraphIterLite){graph, graph->edge.head[OUT][from]};
}

CGraphIterLite cgraphGetEdgeIterRev(const CGraph *graph, const CGraphId to) {
  return (CGraphIterLite){graph, graph->edge.head[IN][to]};
}

CGraphBool cgraphIterLiteNextVert(CGraphIterLite *iter, CGraphId *vid) {
  if (iter->curr == INVALID_ID) return false;
  *vid = iter->curr;
  iter->curr = iter->view->vert.next[iter->curr];
  return true;
}

CGraphBool cgraphIterLiteNextEdge(CGraphIterLite *iter, CGraphId *eid, CGraphId *to) {
  if (iter->curr == INVALID_ID) return false;
  parseF(iter->view, iter->curr, eid, to);
  iter->curr = iter->view->edge.next[iter->curr & 1][iter->curr >> 1];
  return true;
}

CGraphBool cgraphIterLiteNextEdgeRev(CGraphIterLite *iter, CGraphId *eid, CGraphId *from) {
  if (iter->curr == INVALID_ID) return false;
  parseB(iter->view, iter->curr, eid, from);
  iter->curr = iter->view->edge.next[iter->curr & 1][iter->curr >> 1];
  return true;
}

void cgraphTraverseEdges(const CGraph *graph, void *userData,
                         void (*callback)(CGraphId, CGraphId, CGraphId, void *)) {
  CGraphId eid, to;
  for (CGraphId from = graph->vert.head; from != INVALID_ID; from = graph->vert.next[from]) {
    for (CGraphId did = graph->edge.head[OUT][from]; did != INVALID_ID;
         did = graph->edge.next[did & 1][did >> 1]) {
      parseF(graph, did, &eid, &to);
      callback(from, eid, to, userData);
    }
  }
}