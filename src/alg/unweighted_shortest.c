#include "cgraph/alg.h"
#include "cgraph/iter.h"
#include "cgraph/struct/queue.h"
#include <string.h>

void cgraphUnweightedShortest(const CGraph *const graph, CGraphId predecessor[],
                              const CGraphId source, const CGraphId target) {
  CGraphQueue *queue = cgraphQueueCreate(graph->vert.count);
  memset(predecessor, INVALID_ID, sizeof(CGraphId) * graph->vert.range);

  predecessor[source] = source;
  cgraphQueuePush(queue, source);
  while (!cgraphQueueEmpty(queue)) {
    const CGraphId from = cgraphQueuePop(queue);

    CGraphId eid, to;
    CGraphIter iter = cgraphGetEdgeIter(graph, from, CGRAPH_OUT);
    while (cgraphIterNextEdge(&iter, &eid, &to)) {
      if (predecessor[to] == INVALID_ID) {
        predecessor[to] = from;
        if (to == target) goto end;
        cgraphQueuePush(queue, to);
      }
    }
  }

  end:
    cgraphQueueRelease(queue);
}