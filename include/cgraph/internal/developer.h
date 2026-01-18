#ifndef CGRAPH_UTILS_H
#define CGRAPH_UTILS_H

#include "../iter.h"

#define INVALID_ID (-1)
#define UNREACHABLE 0x7f7f7f7f7f7f7f7f
#define UNREACHABLE_BYTE 0x7f
#define REVERSE(did) ((did) ^ 1)
#define VIEW(graph) (&(graph)->view)

CGraphIter *cgraphIterFromView(const CGraphView *view);
void cgraphIterParseF(const CGraphView *view, CGraphId did, CGraphId *eid, CGraphId *to);
void cgraphIterParseB(const CGraphView *view, CGraphId did, CGraphId *eid, CGraphId *from);
CGraphBool cgraphIterNextDirect(CGraphIter *iter, CGraphId from, CGraphId *did);

CGraphView *cgraphViewReserveEdge(const CGraphView *view, CGraphBool directed);
void cgraphViewCopyEdge(const CGraphView *view, const CGraphView *copy);
void cgraphEdgeTraverseV(const CGraphView *view, void *userData,
                        void (*callback)(CGraphId from, CGraphId eid, CGraphId to, void *userData));

CGraphId *cgraphFind(CGraphId *next, CGraphId *head, CGraphId id);

static inline void cgraphUnlink(const CGraphId *next, CGraphId *predNext) {
  *predNext = next[*predNext];
}

static inline void cgraphInsert(CGraphId *next, CGraphId *predNext,
                                const CGraphId id) {
  next[id] = *predNext;
  *predNext = id;
}

static inline void cgraphInsertEdge(const CGraphView *view, const CGraphId from,
                                    const CGraphId did) {
  cgraphInsert(view->edgeNext, view->edgeHead + from, did);
}

#endif // CGRAPH_UTILS_H
