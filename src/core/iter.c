#include "iter.h"
#include "internal/developer.h"
#include <stdlib.h>
#include <string.h>

CGraphIter *cgraphIterFromView(const CGraphView *view) {
  CGraphIter *iter =
      malloc(sizeof(CGraphIter) + view->vertRange * sizeof(CGraphId));
  iter->view = view;
  iter->vertCurr = view->vertHead;
  memcpy(iter->edgeCurr, view->edgeHead, view->vertRange * sizeof(CGraphId));
  return iter;
}

CGraphIter *cgraphGetIter(const CGraph *graph) {
  return cgraphIterFromView(VIEW(graph));
}

void cgraphIterRelease(CGraphIter *iter) { free(iter); }

void cgraphIterResetVert(CGraphIter *iter) {
  iter->vertCurr = iter->view->vertHead;
}

void cgraphIterResetEdge(CGraphIter *iter, const CGraphId from) {
  const CGraphView *view = iter->view;
  if (from == INVALID_ID) {
    memcpy(iter->edgeCurr, view->edgeHead, view->vertRange * sizeof(CGraphId));
  } else {
    iter->edgeCurr[from] = view->edgeHead[from];
  }
}

void cgraphIterCurr(const CGraphIter *iter, CGraphId *from, CGraphId *eid,
                    CGraphId *to) {
  *from = iter->vertCurr;
  if (*from == INVALID_ID) return;
  *eid = iter->edgeCurr[*from];
  if (*eid == INVALID_ID) return;
  cgraphIterParseF(iter->view, *eid, eid, to);
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
  cgraphIterParseF(iter->view, *curr, eid, to);
  *curr = iter->view->edgeNext[*curr];
  return true;
}

void cgraphIterParseF(const CGraphView *view, const CGraphId did, CGraphId *eid,
                      CGraphId *to) {
  // 高度重复可预测，保留分支版本
  if (view->directed) {
    *eid = did;
    *to = view->endpoints[did].to;
  } else {
    *eid = did >> 1;
    *to = ((CGraphId *)view->endpoints)[did];
  }
}

void cgraphIterParseB(const CGraphView *view, const CGraphId did, CGraphId *eid,
                      CGraphId *from) {
  if (view->directed) {
    *eid = did;
    *from = view->endpoints[did].from;
  } else {
    *eid = did >> 1;
    *from = ((CGraphId *)view->endpoints)[REVERSE(did)];
  }
}

CGraphBool cgraphIterNextDirect(CGraphIter *iter, const CGraphId from,
                                CGraphId *did) {
  CGraphId *curr = iter->edgeCurr + from;
  if (*curr == INVALID_ID) return false;
  *did = *curr;
  *curr = iter->view->edgeNext[*curr];
  return true;
}