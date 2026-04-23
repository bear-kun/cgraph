#include "cgraph/iter.h"
#include <stdlib.h>
#include <string.h>

CGraphIter *cgraphGetIter(const CGraph *graph) {
  CGraphIter *iter =
      malloc(sizeof(CGraphIter) + graph->vert.range * sizeof(CGraphId));
  iter->view = graph;
  iter->vertCurr = graph->vert.head;
  memcpy(iter->edgeCurr, graph->edge.head, graph->vert.range * sizeof(CGraphId));
  return iter;
}

void cgraphIterRelease(CGraphIter *iter) { free(iter); }

void cgraphIterResetVert(CGraphIter *iter) {
  iter->vertCurr = iter->view->vert.head;
}

void cgraphIterResetEdge(CGraphIter *iter, const CGraphId from) {
  const CGraph *view = iter->view;
  if (from == INVALID_ID) {
    memcpy(iter->edgeCurr, view->edge.head, view->vert.range * sizeof(CGraphId));
  } else {
    iter->edgeCurr[from] = view->edge.head[from];
  }
}

static void parseF(const CGraph *graph, const CGraphId did, CGraphId *eid, CGraphId *to) {
  // 高度重复可预测，保留分支版本
  if (graph->edge.directed) {
    *eid = did;
    *to = graph->edge.to[did];
  } else {
    *eid = did >> 1;
    *to = (did & 1 ? graph->edge.from : graph->edge.to)[*eid];
  }
}

void cgraphIterCurr(const CGraphIter *iter, CGraphId *from, CGraphId *eid, CGraphId *to) {
  *from = iter->vertCurr;
  if (*from == INVALID_ID) return;
  *eid = iter->edgeCurr[*from];
  if (*eid == INVALID_ID) return;
  parseF(iter->view, *eid, eid, to);
}

CGraphBool cgraphIterNextVert(CGraphIter *iter, CGraphId *vid) {
  if (iter->vertCurr == INVALID_ID) return false;
  *vid = iter->vertCurr;
  iter->vertCurr = iter->view->vert.next[iter->vertCurr];
  return true;
}

CGraphBool cgraphIterNextEdge(CGraphIter *iter, const CGraphId from, CGraphId *eid, CGraphId *to) {
  CGraphId *curr = iter->edgeCurr + from;
  if (*curr == INVALID_ID) return false;
  parseF(iter->view, *curr, eid, to);
  *curr = iter->view->edge.next[*curr];
  return true;
}

CGraphIterLite cgraphGetVertIter(const CGraph *graph) {
  return (CGraphIterLite){graph, graph->vert.head};
}

CGraphIterLite cgraphGetEdgeIter(const CGraph *graph, const CGraphId from) {
  return (CGraphIterLite){graph, graph->edge.head[from]};
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
  iter->curr = iter->view->edge.next[iter->curr];
  return true;
}

void cgraphTraverseEdges(const CGraph *graph, void *userData,
                         void (*callback)(CGraphId, CGraphId, CGraphId, void *)) {
  CGraphId eid, to;
  for (CGraphId from = graph->vert.head; from != INVALID_ID; from = graph->vert.next[from]) {
    for (CGraphId did = graph->edge.head[from]; did != INVALID_ID; did = graph->edge.next[did]) {
      parseF(graph, did, &eid, &to);
      callback(from, eid, to, userData);
    }
  }
}