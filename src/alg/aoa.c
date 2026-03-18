#include "cgraph/graph.h"
#include "cgraph/iter.h"
#include "struct/queue.h"
#include <stdlib.h>
#include <string.h>

static void callback(CGraphId from, CGraphId eid, const CGraphId to,
                     void *userdata) {
  const CGraphInt *indegree = *(void **)userdata;
  CGraphQueue *queue = *((void **)userdata + 1);
  if (indegree[to] == 0) cgraphQueuePush(queue, to);
}

static void indegreeInitQueue(const CGraph *graph,
                              const CGraphInt indegree[], CGraphQueue *queue) {
  void *userData[] = {(void *)indegree, queue};
  cgraphTraverseEdges(graph, userData, callback);
}

typedef struct {
  const CGraph *graph;
  CGraphQueue *queue;
  CGraphInt *indegree;
  const TimeType *duration;
  TimeType *earlyStart, *lateStart;
  CGraphId *successor;
} Package;

static void forward(const Package *const pkg) {
  while (!cgraphQueueEmpty(pkg->queue)) {
    const CGraphId from = cgraphQueuePop(pkg->queue);

    CGraphId eid, to;
    CGraphIterLite iter = cgraphGetEdgeIter(pkg->graph, from);
    while (cgraphIterLiteNextEdge(&iter, &eid, &to)) {
      if (pkg->earlyStart[to] < pkg->earlyStart[from] + pkg->duration[eid]) {
        pkg->earlyStart[to] = pkg->earlyStart[from] + pkg->duration[eid];
      }
      if (--pkg->indegree[to] == 0) cgraphQueuePush(pkg->queue, to);
    }
  }
}

static void backward(const Package *pkg, const CGraphId *const begin,
                     const CGraphId *const end) {
  const CGraphId *p = end;
  do {
    const CGraphId from = *--p;
    CGraphId eid, to;
    CGraphIterLite iter = cgraphGetEdgeIter(pkg->graph, from);
    while (cgraphIterLiteNextEdge(&iter, &eid, &to)) {
      if (pkg->lateStart[from] > pkg->lateStart[to] - pkg->duration[eid]) {
        pkg->lateStart[from] = pkg->lateStart[to] - pkg->duration[eid];
        if (pkg->lateStart[from] == pkg->earlyStart[from]) {
          pkg->successor[from] = to;
          break;
        }
      }
    }
  } while (p != begin);
}

static void init(Package *pkg, const CGraph *graph,
                 const CGraphInt indegree[]) {
  const CGraphSize vertRange = graph->vertRange;

  pkg->queue = cgraphQueueCreate(vertRange);
  indegreeInitQueue(graph, pkg->indegree, pkg->queue);
  pkg->indegree = malloc(vertRange * sizeof(CGraphInt));
  memcpy(pkg->indegree, indegree, vertRange * sizeof(CGraphInt));
  memset(pkg->earlyStart, 0, vertRange * sizeof(TimeType));
  memset(pkg->successor, INVALID_ID, vertRange * sizeof(CGraphId));
  for (CGraphId i = 0; i < vertRange; i++) pkg->lateStart[i] = CGRAPH_INF;
}

void cgraphCriticalPath(const CGraph *aoa, const CGraphInt indegree[],
                        const TimeType duration[], CGraphId successor[],
                        TimeType earlyStart[], TimeType lateStart[]) {
  Package pkg;
  pkg.duration = duration;
  pkg.earlyStart = earlyStart;
  pkg.lateStart = lateStart;
  pkg.successor = successor;
  init(&pkg, aoa, indegree);

  forward(&pkg);

  const CGraphId *last = pkg.queue->elems + aoa->vertNum - 1;
  lateStart[*last] = earlyStart[*last];

  backward(&pkg, pkg.queue->elems, last);

  free(pkg.indegree);
  cgraphQueueRelease(pkg.queue);
}