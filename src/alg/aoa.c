#include "cgraph/alg.h"
#include "cgraph/iter.h"
#include "struct/queue.h"
#include <stdlib.h>
#include <string.h>

static void indegreeInitQueue(const CGraph *graph, const CGraphInt indegree[], CGraphQueue *queue) {
  CGraphId vid;
  CGraphIter vertices = cgraphGetVertIter(graph);
  while (cgraphIterNextVert(&vertices, &vid)) {
    if (indegree[vid] == 0) cgraphQueuePush(queue, vid);
  }
}

typedef struct {
  const CGraph *graph;
  CGraphQueue *queue;
  CGraphInt *indegree;
  const TimeType *duration;
  TimeType *earlyStart, *lateStart;
  CGraphId *successor;
} Package;

static void forward(const Package *pkg) {
  while (!cgraphQueueEmpty(pkg->queue)) {
    const CGraphId from = cgraphQueuePop(pkg->queue);

    CGraphId eid, to;
    CGraphIter iter = cgraphGetEdgeIter(pkg->graph, from, CGRAPH_OUT);
    while (cgraphIterNextEdge(&iter, &eid, &to)) {
      if (pkg->earlyStart[to] < pkg->earlyStart[from] + pkg->duration[eid]) {
        pkg->earlyStart[to] = pkg->earlyStart[from] + pkg->duration[eid];
      }
      if (--pkg->indegree[to] == 0) cgraphQueuePush(pkg->queue, to);
    }
  }
}

static void backward(const Package *pkg, const CGraphId *begin, const CGraphId *end) {
  const CGraphId *p = end;
  do {
    const CGraphId from = *--p;
    CGraphId eid, to;
    CGraphIter iter = cgraphGetEdgeIter(pkg->graph, from, CGRAPH_OUT);
    while (cgraphIterNextEdge(&iter, &eid, &to)) {
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

static void init(Package *pkg, const CGraph *graph) {
  const CGraphSize vertRange = graph->vert.range;

  pkg->queue = cgraphQueueCreate(vertRange);
  pkg->indegree = malloc(vertRange * sizeof(CGraphInt));
  memcpy(pkg->indegree, graph->vert.degree[CGRAPH_IN], vertRange * sizeof(CGraphInt));
  memset(pkg->earlyStart, 0, vertRange * sizeof(TimeType));
  memset(pkg->successor, INVALID_ID, vertRange * sizeof(CGraphId));
  for (CGraphId i = 0; i < vertRange; i++) pkg->lateStart[i] = CGRAPH_INF;
  indegreeInitQueue(graph, pkg->indegree, pkg->queue);
}

void cgraphCriticalPath(const CGraph *aoa, const TimeType duration[], CGraphId successor[],
                        TimeType earlyStart[], TimeType lateStart[]) {
  Package pkg;
  pkg.duration = duration;
  pkg.earlyStart = earlyStart;
  pkg.lateStart = lateStart;
  pkg.successor = successor;
  init(&pkg, aoa);

  forward(&pkg);

  const CGraphId *last = pkg.queue->elems + aoa->vert.count - 1;
  lateStart[*last] = earlyStart[*last];

  backward(&pkg, pkg.queue->elems, last);

  free(pkg.indegree);
  cgraphQueueRelease(pkg.queue);
}