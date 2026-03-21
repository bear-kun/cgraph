#include "cgraph/iter.h"
#include "struct/pairing_heap.h"
#include "struct/queue.h"
#include <stdlib.h>
#include <string.h>

void cgraphUnweightedShortest(const CGraph *const graph, CGraphId predecessor[],
                              const CGraphId source, const CGraphId target) {
  CGraphQueue *queue = cgraphQueueCreate(graph->vertNum);
  memset(predecessor, INVALID_ID, sizeof(CGraphId) * graph->vertRange);

  predecessor[source] = source;
  cgraphQueuePush(queue, source);
  while (!cgraphQueueEmpty(queue)) {
    const CGraphId from = cgraphQueuePop(queue);

    CGraphId eid, to;
    CGraphIterLite iter = cgraphGetEdgeIter(graph, from);
    while (cgraphIterLiteNextEdge(&iter, &eid, &to)) {
      if (predecessor[to] == INVALID_ID) {
        predecessor[to] = from;
        if (to == target) break;
        cgraphQueuePush(queue, to);
      }
    }
  }
  cgraphQueueRelease(queue);
}

void cgraphShortestDijkstra(const CGraph *const graph,
                            const WeightType weights[], CGraphId predecessor[],
                            const CGraphId source, const CGraphId target) {
  CGraphBool *visited = calloc(graph->vertRange, sizeof(CGraphBool));
  WeightType *distance = malloc(graph->vertRange * sizeof(WeightType));
  CGraphPairingHeap *heap = cgraphPairingHeapCreate(graph->vertNum, distance);
  memset(predecessor, INVALID_ID, graph->vertRange * sizeof(CGraphId));
  for (CGraphId i = 0; i < graph->vertRange; i++) distance[i] = CGRAPH_INF;

  visited[source] = true;
  distance[source] = 0;
  cgraphPairingHeapPush(heap, source);
  while (!cgraphPairingHeapEmpty(heap)) {
    const CGraphId from = cgraphPairingHeapPop(heap);
    if (from == target) break;

    CGraphId eid, to;
    CGraphIterLite iter = cgraphGetEdgeIter(graph, from);
    while (cgraphIterLiteNextEdge(&iter, &eid, &to)) {
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

// 无负值圈
void cgraphShortestBellmanFord(const CGraph *const graph,
                               const WeightType weights[],
                               CGraphId predecessor[], const CGraphId source) {
  CGraphQueue *queue = cgraphQueueCreate(graph->vertNum);
  CGraphBool *isInQueue = calloc(graph->vertRange, sizeof(CGraphBool));
  WeightType *distance = malloc(graph->vertRange * sizeof(WeightType));
  memset(predecessor, INVALID_ID, graph->vertRange * sizeof(CGraphId));
  for (CGraphId i = 0; i < graph->vertRange; i++) distance[i] = CGRAPH_INF;

  distance[source] = 0;
  cgraphQueuePush(queue, source);
  while (!cgraphQueueEmpty(queue)) {
    const CGraphId from = cgraphQueuePop(queue);
    isInQueue[from] = false;

    CGraphId eid, to;
    CGraphIterLite iter = cgraphGetEdgeIter(graph, from);
    while (cgraphIterLiteNextEdge(&iter, &eid, &to)) {
      if (distance[to] <= distance[from] + weights[eid]) continue;

      distance[to] = distance[from] + weights[eid];
      predecessor[to] = from;

      if (!isInQueue[to]) {
        cgraphQueuePush(queue, to);
        isInQueue[to] = true;
      }
    }
  }

  free(isInQueue);
  free(distance);
  cgraphQueueRelease(queue);
}