#include "cgraph/alg.h"
#include "cgraph/iter.h"
#include "cgraph/struct/pairing_heap.h"
#include <stdlib.h>
#include <string.h>

void cgraph_shortest_dijkstra(const CGraph *const graph, const WeightType weights[],
                              CGraphId predecessor[], const CGraphId source,
                              const CGraphId target) {
  CGraphBool *visited = calloc(graph->vert.range, sizeof(CGraphBool));
  WeightType *distance = malloc(graph->vert.range * sizeof(WeightType));
  CGraphPairingHeap *heap = cgraph_new_pairing_heap(graph->vert.count, distance);
  memset(predecessor, INVALID_ID, graph->vert.range * sizeof(CGraphId));
  for (CGraphId i = 0; i < graph->vert.range; i++) distance[i] = CGRAPH_INF;

  visited[source] = true;
  distance[source] = 0;
  cgraph_pairing_heap_push(heap, source);
  while (!cgraph_pairing_heap_empty(heap)) {
    const CGraphId from = cgraph_pairing_heap_pop(heap);
    if (from == target) break;

    CGraphId eid, to;
    CGraphIter iter = cgraph_get_edge_iter(graph, from, CGRAPH_OUT);
    while (cgraph_iter_next_edge(&iter, &eid, &to)) {
      if (distance[from] + weights[eid] < distance[to]) {
        distance[to] = distance[from] + weights[eid];
        predecessor[to] = from;

        if (visited[to]) {
          cgraph_pairing_heap_update(heap, to);
        } else {
          visited[to] = true;
          cgraph_pairing_heap_push(heap, to);
        }
      }
    }
  }

  free(visited);
  free(distance);
  cgraph_delete_pairing_heap(heap);
}