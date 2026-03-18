#include "iter.h"
#include <stdlib.h>
#include <string.h>

CGraphIter *cgraphGetIter(const CGraph *graph) {
  CGraphIter *iter =
      malloc(sizeof(CGraphIter) + graph->vertRange * sizeof(CGraphId));
  iter->view = graph;
  iter->vertCurr = graph->vertHead;
  memcpy(iter->edgeCurr, graph->edgeHead, graph->vertRange * sizeof(CGraphId));
  return iter;
}

void cgraphIterRelease(CGraphIter *iter) { free(iter); }

void cgraphIterResetVert(CGraphIter *iter) {
  iter->vertCurr = iter->view->vertHead;
}

void cgraphIterResetEdge(CGraphIter *iter, const CGraphId from) {
  const CGraph *view = iter->view;
  if (from == INVALID_ID) {
    memcpy(iter->edgeCurr, view->edgeHead, view->vertRange * sizeof(CGraphId));
  } else {
    iter->edgeCurr[from] = view->edgeHead[from];
  }
}

static void parseF(const CGraph *graph, const CGraphId did,
                                   CGraphId *eid, CGraphId *to) {
  // 高度重复可预测，保留分支版本
  if (graph->directed) {
    *eid = did;
    *to = graph->edgeTo[did];
  } else {
    *eid = did >> 1;
    *to = (did & 1 ? graph->edgeFrom : graph->edgeTo)[*eid];
  }
}

void cgraphIterCurr(const CGraphIter *iter, CGraphId *from, CGraphId *eid,
                    CGraphId *to) {
  *from = iter->vertCurr;
  if (*from == INVALID_ID) return;
  *eid = iter->edgeCurr[*from];
  if (*eid == INVALID_ID) return;
  parseF(iter->view, *eid, eid, to);
}

CGraphBool cgraphIterNextVert(CGraphIter *iter, CGraphId *vid) {
  if (iter->vertCurr == INVALID_ID) return false;
  *vid = iter->vertCurr;
  iter->vertCurr = iter->view->vertNext[iter->vertCurr];
  return true;
}

CGraphBool cgraphIterNextEdge(CGraphIter *iter, const CGraphId from,
                              CGraphId *eid, CGraphId *to) {
  CGraphId *curr = iter->edgeCurr + from;
  if (*curr == INVALID_ID) return false;
  parseF(iter->view, *curr, eid, to);
  *curr = iter->view->edgeNext[*curr];
  return true;
}

CGraphIterLite cgraphGetVertIter(const CGraph *graph) {
  return (CGraphIterLite){graph, graph->vertHead};
}

CGraphIterLite cgraphGetEdgeIter(const CGraph *graph, const CGraphId from) {
  return (CGraphIterLite){graph, graph->edgeHead[from]};
}

CGraphBool cgraphIterLiteNextVert(CGraphIterLite *iter, CGraphId *vid) {
  if (iter->curr == INVALID_ID) return false;
  *vid = iter->curr;
  iter->curr = iter->view->vertNext[iter->curr];
  return true;
}

CGraphBool cgraphIterLiteNextEdge(CGraphIterLite *iter, CGraphId *eid, CGraphId *to) {
  if (iter->curr == INVALID_ID) return false;
  parseF(iter->view, iter->curr, eid, to);
  iter->curr = iter->view->edgeNext[iter->curr];
  return true;
}

void cgraphTraverseEdges(const CGraph *graph, void *userData,
                         void (*callback)(CGraphId, CGraphId, CGraphId,
                                          void *)) {
  for (CGraphId from = graph->vertHead; from != INVALID_ID;
       from = graph->vertNext[from]) {
    for (CGraphId did = graph->edgeHead[from], eid, to; did != INVALID_ID;
         did = graph->edgeNext[did]) {
      parseF(graph, did, &eid, &to);
      callback(from, eid, to, userData);
    }
  }
}