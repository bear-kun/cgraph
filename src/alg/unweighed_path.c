#include "internal/developer.h"
#include "struct/queue.h"
#include <string.h>

void cgraphUnweightedShortest(const CGraph *const graph, CGraphId predecessor[],
                              const CGraphId source, const CGraphId target) {
  const CGraphView *view = VIEW(graph);
  CGraphIter *iter = cgraphIterFromView(view);
  CGraphQueue *queue = cgraphQueueCreate(graph->vertNum);
  memset(predecessor, INVALID_ID, sizeof(CGraphId) * view->vertRange);

  CGraphId id, to;
  cgraphQueuePush(queue, source);
  while (!cgraphQueueEmpty(queue)) {
    const CGraphId from = cgraphQueuePop(queue);

    while (cgraphIterNextEdge(iter, from, &id, &to)) {
      if (predecessor[to] == -1) {
        predecessor[to] = from;
        if (to == target) return;
        cgraphQueuePush(queue, to);
      }
    }
  }
  cgraphIterRelease(iter);
  cgraphQueueRelease(queue);
}