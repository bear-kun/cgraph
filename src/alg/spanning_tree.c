#include "cgraph/alg.h"
#include "cgraph/graph.h"
#include "cgraph/iter.h"
#include "struct/disjoint_set.h"
#include "struct/heap.h"
#include "struct/pairing_heap.h"
#include <stdlib.h>

void cgraphSpanningTreePrim(const CGraph *graph, const WeightType weights[], CGraphId predecessor[],
                            const CGraphId root) {
  CGraphBool *visited = calloc(graph->vert.range, sizeof(CGraphBool));
  WeightType *minWeight = malloc(graph->vert.range * sizeof(WeightType));
  CGraphPairingHeap *heap = cgraphPairingHeapCreate(graph->vert.count, minWeight);
  for (CGraphId i = 0; i < graph->vert.range; i++) minWeight[i] = CGRAPH_INF;

  visited[root] = true;
  predecessor[root] = INVALID_ID;
  cgraphPairingHeapPush(heap, root);
  while (!cgraphPairingHeapEmpty(heap)) {
    const CGraphId from = cgraphPairingHeapPop(heap);

    CGraphId eid, to;
    CGraphIter iter = cgraphGetEdgeIter(graph, from);
    while (cgraphIterNextEdge(&iter, &eid, &to)) {
      if (weights[eid] < minWeight[to]) {
        minWeight[to] = weights[eid];
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
  free(minWeight);
  cgraphPairingHeapRelease(heap);
}

static void callback(CGraphId from, const CGraphId eid, CGraphId to, void *userData) {
  CGraphHeap *heap = *(void **)userData;
  CGraphBool *isInHeap = *((void **)userData + 1);
  // 去除反向边
  if (!isInHeap[eid]) {
    isInHeap[eid] = true;
    cgraphHeapPreBuild(heap, eid);
  }
}

static void KruskalHeapInit(const CGraph *graph, CGraphHeap *heap) {
  CGraphBool *isInHeap = calloc(graph->edge.range, sizeof(CGraphBool));
  void *userData[] = {heap, isInHeap};
  cgraphTraverseEdges(graph, userData, callback);
  free(isInHeap);
  cgraphHeapBuild(heap);
}

void cgraphSpanningTreeKruskal(const CGraph *graph, const WeightType weights[], CGraphId edges[]) {
  CGraphHeap *heap = cgraphHeapCreate(graph->edge.count, weights);
  CGraphDisjointSet *disjointSet = cgraphDisjointCreate(graph->vert.count);
  KruskalHeapInit(graph, heap);

  CGraphSize counter = 0;
  while (!cgraphHeapEmpty(heap)) {
    CGraphId from, to;
    const CGraphId eid = cgraphHeapPop(heap);
    cgraphWhereEdgeFromTo(graph, eid, &from, &to);
    const CGraphId cls1 = cgraphDisjointFind(disjointSet, from);
    const CGraphId cls2 = cgraphDisjointFind(disjointSet, to);

    if (cls1 != cls2) {
      edges[counter++] = eid;
      cgraphDisjointUnion(disjointSet, cls1, cls2);
    }
  }
  if (counter != graph->vert.count - 1) {
    // No spanning tree
  }

  cgraphHeapRelease(heap);
  cgraphDisjointRelease(disjointSet);
}