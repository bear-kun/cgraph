#include "cgraph/alg.h"
#include "cgraph/graph.h"
#include "cgraph/iter.h"
#include "cgraph/struct/pairing_heap.h"
#include "cgraph/struct/disjoint_set.h"
#include "cgraph/struct/heap.h"
#include <stdlib.h>

void cgraph_spanning_tree_prim(const CGraph *graph, const WeightType weights[],
                               CGraphId predecessor[], const CGraphId root) {
  CGraphBool *visited = calloc(graph->vert.range, sizeof(CGraphBool));
  WeightType *min_weight = malloc(graph->vert.range * sizeof(WeightType));
  CGraphPairingHeap *heap = cgraph_new_pairing_heap(graph->vert.count, min_weight);
  for (CGraphId i = 0; i < graph->vert.range; i++) min_weight[i] = CGRAPH_INF;

  visited[root] = true;
  predecessor[root] = INVALID_ID;
  cgraph_pairing_heap_push(heap, root);
  while (!cgraph_pairing_heap_empty(heap)) {
    const CGraphId from = cgraph_pairing_heap_pop(heap);

    CGraphId eid, to;
    CGraphIter iter = cgraph_get_edge_iter(graph, from, CGRAPH_OUT);
    while (cgraph_iter_next_edge(&iter, &eid, &to)) {
      if (weights[eid] < min_weight[to]) {
        min_weight[to] = weights[eid];
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
  free(min_weight);
  cgraph_delete_pairing_heap(heap);
}

static void callback(CGraphId from, const CGraphId eid, CGraphId to, void *data) {
  CGraphHeap *heap = *(void **)data;
  CGraphBool *in_heap = *((void **)data + 1);
  // 去除反向边
  if (!in_heap[eid]) {
    in_heap[eid] = true;
    cgraph_heap_prebuild(heap, eid);
  }
}

static void init_kruskal_heap(const CGraph *graph, CGraphHeap *heap) {
  CGraphBool *isInHeap = calloc(graph->edge.range, sizeof(CGraphBool));
  void *userData[] = {heap, isInHeap};
  cgraph_traverse_edges(graph, userData, callback);
  free(isInHeap);
  cgraph_heap_build(heap);
}

void cgraph_spanning_tree_kruskal(const CGraph *graph, const WeightType weights[],
                                  CGraphId edges[]) {
  CGraphHeap *heap = cgraph_new_heap(graph->edge.count, weights);
  CGraphDisjointSet *disjoint_set = cgraph_new_disjoint(graph->vert.count);
  init_kruskal_heap(graph, heap);

  CGraphSize counter = 0;
  while (!cgraph_heap_empty(heap)) {
    CGraphId from, to;
    const CGraphId eid = cgraph_heap_pop(heap);
    cgraph_where_edge_from_to(graph, eid, &from, &to);
    const CGraphId cls1 = cgraph_disjoint_find(disjoint_set, from);
    const CGraphId cls2 = cgraph_disjoint_find(disjoint_set, to);

    if (cls1 != cls2) {
      edges[counter++] = eid;
      cgraph_disjoint_union(disjoint_set, cls1, cls2);
    }
  }
  if (counter != graph->vert.count - 1) {
    // No spanning tree
  }

  cgraph_delete_heap(heap);
  cgraph_delete_disjoint(disjoint_set);
}