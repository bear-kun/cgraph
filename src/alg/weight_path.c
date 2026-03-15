#include "internal/developer.h"
#include "struct/pairing_heap.h"
#include "struct/queue.h"
#include <stdlib.h>
#include <string.h>

void cgraphShortestDijkstra(const CGraph *const graph,
                            const WeightType weights[], CGraphId predecessor[],
                            const CGraphId source, const CGraphId target) {
  const CGraphView *view = VIEW(graph);
  CGraphIter *iter = cgraphIterFromView(view);
  CGraphBool *visited = calloc(view->vertRange, sizeof(CGraphBool));
  WeightType *distance = malloc(view->vertRange * sizeof(WeightType));
  CGraphPairingHeap *heap = cgraphPairingHeapCreate(graph->vertNum, distance);
  memset(distance, UNREACHABLE_BYTE, view->vertRange * sizeof(WeightType));
  memset(predecessor, INVALID_ID, view->vertRange * sizeof(CGraphId));

  visited[source] = true;
  distance[source] = 0;
  cgraphPairingHeapPush(heap, source);
  while (!cgraphPairingHeapEmpty(heap)) {
    const CGraphId from = cgraphPairingHeapPop(heap);
    if (from == target) break;

    CGraphId eid, to;
    while (cgraphIterNextEdge(iter, from, &eid, &to)) {
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
  cgraphIterRelease(iter);
  cgraphPairingHeapRelease(heap);
}

// 无负值圈
void cgraphShortestBellmanFord(const CGraph *const graph,
                               const WeightType weights[],
                               CGraphId predecessor[], const CGraphId source) {
  const CGraphView *view = VIEW(graph);
  CGraphIter *iter = cgraphIterFromView(view);
  CGraphQueue *queue = cgraphQueueCreate(graph->vertNum);
  CGraphBool *isInQueue = calloc(view->vertRange, sizeof(CGraphBool));
  WeightType *distance = malloc(view->vertRange * sizeof(WeightType));
  memset(distance, UNREACHABLE_BYTE, view->vertRange * sizeof(WeightType));
  memset(predecessor, INVALID_ID, view->vertRange * sizeof(CGraphId));

  CGraphId eid, to;
  distance[source] = 0;
  cgraphQueuePush(queue, source);
  while (!cgraphQueueEmpty(queue)) {
    const CGraphId from = cgraphQueuePop(queue);
    isInQueue[from] = false;

    while (cgraphIterNextEdge(iter, from, &eid, &to)) {
      if (distance[to] <= distance[from] + weights[eid]) continue;

      distance[to] = distance[from] + weights[eid];
      predecessor[to] = from;

      if (!isInQueue[to]) {
        cgraphQueuePush(queue, to);
        cgraphIterResetEdge(iter, to);
        isInQueue[to] = true;
      }
    }
  }

  free(isInQueue);
  free(distance);
  cgraphIterRelease(iter);
  cgraphQueueRelease(queue);
}