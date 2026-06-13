#include "cgraph/alg.h"
#include "cgraph/iter.h"
#include "cgraph/struct/queue.h"
#include <string.h>

void cgraph_unweighted_shortest(const CGraph *const graph, CGraphId predecessor[],
                                const CGraphId source, const CGraphId target) {
  CGraphQueue *queue = cgraph_new_queue(graph->vert.count);
  memset(predecessor, INVALID_ID, sizeof(CGraphId) * graph->vert.range);

  predecessor[source] = source;
  cgraph_queue_push(queue, source);
  while (!cgraph_queue_empty(queue)) {
    const CGraphId from = cgraph_queue_pop(queue);

    CGraphId eid, to;
    CGraphIter iter = cgraph_get_edge_iter(graph, from, CGRAPH_OUT);
    while (cgraph_iter_next_edge(&iter, &eid, &to)) {
      if (predecessor[to] == INVALID_ID) {
        predecessor[to] = from;
        if (to == target) goto end;
        cgraph_queue_push(queue, to);
      }
    }
  }

end:
  cgraph_delete_queue(queue);
}