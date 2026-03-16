#include "cgraph/graph.h"
#include "cgraph/iter.h"
#include "struct/disjoint_set.h"
#include "struct/heap.h"
#include "struct/pairing_heap.h"
#include <stdlib.h>

void cgraphMSTPrim(const CGraph *graph, const WeightType weights[],
                   CGraphId predecessor[], const CGraphId root) {
  CGraphIter *iter = cgraphGetIter(graph);
  CGraphBool *visited = calloc(graph->vertRange, sizeof(CGraphBool));
  WeightType *minWeight = malloc(graph->vertRange * sizeof(WeightType));
  CGraphPairingHeap *heap = cgraphPairingHeapCreate(graph->vertNum, minWeight);
  for (CGraphId i = 0; i < graph->vertRange; i++) minWeight[i] = CGRAPH_INF;

  visited[root] = true;
  predecessor[root] = INVALID_ID;
  cgraphPairingHeapPush(heap, root);
  while (!cgraphPairingHeapEmpty(heap)) {
    const CGraphId from = cgraphPairingHeapPop(heap);

    CGraphId eid, to;
    while (cgraphIterNextEdge(iter, from, &eid, &to)) {
      if (weights[eid] < minWeight[to]) {
        minWeight[to] = weights[eid];
        predecessor[to] = from;

        if (visited[to]) {
          cgraphPairingHeapPush(heap, to);
        } else {
          visited[to] = true;
          cgraphPairingHeapUpdate(heap, to);
        }
      }
    }
  }

  free(visited);
  free(minWeight);
  cgraphIterRelease(iter);
  cgraphPairingHeapRelease(heap);
}

static void callback(CGraphId from, const CGraphId eid, CGraphId to,
                     void *userData) {
  CGraphHeap *heap = *(void **)userData;
  CGraphBool *isInHeap = *((void **)userData + 1);
  // 去除反向边
  if (!isInHeap[eid]) {
    isInHeap[eid] = true;
    cgraphHeapPreBuild(heap, eid);
  }
}

static void KruskalHeapInit(const CGraph *graph, CGraphHeap *heap) {
  CGraphBool *isInHeap = calloc(graph->edgeRange, sizeof(CGraphBool));
  void *userData[] = {heap, isInHeap};
  cgraphTraverseEdges(graph, userData, callback);
  free(isInHeap);
  cgraphHeapBuild(heap);
}

void cgraphMSTKruskal(const CGraph *graph, const WeightType weight[],
                      CGraphId edges[]) {
  CGraphHeap *heap = cgraphHeapCreate(graph->edgeNum, weight);
  CGraphDisjointSet *disjointSet = cgraphDisjointCreate(graph->vertNum);
  KruskalHeapInit(graph, heap);

  CGraphSize counter = 0;
  while (!cgraphHeapEmpty(heap)) {
    const CGraphId eid = cgraphHeapPop(heap);
    const CGraphId cls1 = cgraphDisjointFind(disjointSet, graph->edgeFrom[eid]);
    const CGraphId cls2 = cgraphDisjointFind(disjointSet, graph->edgeTo[eid]);

    if (cls1 != cls2) {
      edges[counter++] = eid;
      cgraphDisjointUnion(disjointSet, cls1, cls2);
    }
  }
  if (counter != graph->vertNum - 1) {
    // No spanning tree
  }

  cgraphHeapRelease(heap);
  cgraphDisjointRelease(disjointSet);
}