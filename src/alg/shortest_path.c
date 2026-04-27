#include "cgraph/alg.h"
#include "cgraph/iter.h"
#include "struct/pairing_heap.h"
#include "struct/queue.h"
#include <stdlib.h>
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
    CGraphIter iter = cgraphGetEdgeIter(graph, from);
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

void cgraphShortestDijkstra(const CGraph *const graph, const WeightType weights[],
                            CGraphId predecessor[], const CGraphId source, const CGraphId target) {
  CGraphBool *visited = calloc(graph->vert.range, sizeof(CGraphBool));
  WeightType *distance = malloc(graph->vert.range * sizeof(WeightType));
  CGraphPairingHeap *heap = cgraphPairingHeapCreate(graph->vert.count, distance);
  memset(predecessor, INVALID_ID, graph->vert.range * sizeof(CGraphId));
  for (CGraphId i = 0; i < graph->vert.range; i++) distance[i] = CGRAPH_INF;

  visited[source] = true;
  distance[source] = 0;
  cgraphPairingHeapPush(heap, source);
  while (!cgraphPairingHeapEmpty(heap)) {
    const CGraphId from = cgraphPairingHeapPop(heap);
    if (from == target) break;

    CGraphId eid, to;
    CGraphIter iter = cgraphGetEdgeIter(graph, from);
    while (cgraphIterNextEdge(&iter, &eid, &to)) {
      if (distance[from] + weights[eid] < distance[to]) {
        distance[to] = distance[from] + weights[eid];
        predecessor[to] = from;

        if (visited[to]) {
          cgraphPairingHeapUpdate(heap, to);
        } else {
          visited[to] = true;
          cgraphPairingHeapPush(heap, to);
        }
      }
    }
  }

  free(visited);
  free(distance);
  cgraphPairingHeapRelease(heap);
}