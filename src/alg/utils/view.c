#include "internal/developer.h"
#include <stdlib.h>
#include <string.h>

#define DID(eid) ((eid) << 1 | (eid) >> (sizeof(CGraphId) * 8 - 1))

CGraphView *cgraphViewReserveEdge(const CGraphView *view,
                                  const CGraphBool directed) {
  char *const buff = malloc(sizeof(CGraphView) +
                            (view->vertRange + (view->edgeRange << !directed)) *
                                sizeof(CGraphId));
  CGraphView *copy = (CGraphView *)buff;
  copy->vertRange = view->vertRange;
  copy->vertHead = view->vertHead;
  copy->vertNext = view->vertNext;

  copy->directed = directed;
  copy->edgeRange = view->edgeRange;
  copy->edgeHead = (CGraphId *)(buff + sizeof(CGraphView));
  copy->edgeNext = copy->edgeHead + view->vertRange;
  copy->endpoints = view->endpoints;
  return copy;
}

void cgraphViewCopyEdge(const CGraphView *view, const CGraphView *copy) {
  switch (view->directed << 1 | copy->directed) {
  case 3:
    memcpy(copy->edgeHead, view->edgeHead, copy->vertRange * sizeof(CGraphId));
    memcpy(copy->edgeNext, view->edgeNext, copy->edgeRange * sizeof(CGraphId));
    break;
  case 0:
    memcpy(copy->edgeHead, view->edgeHead, copy->vertRange * sizeof(CGraphId));
    memcpy(copy->edgeNext, view->edgeNext,
           2 * copy->edgeRange * sizeof(CGraphId));
    break;
  case 2:
    for (CGraphSize i = 0; i < copy->vertRange; ++i)
      copy->edgeHead[i] = DID(view->edgeHead[i]);
    for (CGraphSize i = 0; i < copy->edgeRange; ++i)
      copy->edgeNext[i << 1] = DID(view->edgeNext[i]);
    break;
  default:;
  }
}