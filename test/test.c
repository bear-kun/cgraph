#include "cgraph/graph.h"
#include "cgraph/alg.h"
#include "cgraph/iter.h"
#include <stdio.h>

typedef struct {
  CGraphId from, to;
} Endpoint;

static void addEdges(CGraph *graph, const CGraphSize num, Endpoint edges[]) {
  for (CGraphSize i = 0; i < num; ++i) {
    cgraphAddEdge(graph, edges[i].from, edges[i].to, true);
  }
}

static void addEdgesU(CGraph *graph, const CGraphSize num, Endpoint edges[]) {
  for (CGraphSize i = 0; i < num; ++i) {
    cgraphAddEdge(graph, edges[i].from, edges[i].to, false);
  }
}

int testIter() {
  CGraph graph;
  cgraphInit(&graph, false, 10, 10);

  cgraphReserveVert(&graph, 5);
  cgraphAddEdge(&graph, 0, 1, false);
  cgraphAddEdge(&graph, 1, 2, true);
  cgraphAddEdge(&graph, 2, 3, true);
  cgraphDeleteEdge(&graph, 0);
  cgraphAddEdge(&graph, 3, 4, false);
  cgraphDeleteEdge(&graph, 2);
  cgraphAddEdge(&graph, 4, 0, true);

  CGraphIter *iter = cgraphGetIter(&graph);
  CGraphId from, to, edge;
  printf("from\teid\t\tto\n");
  while (cgraphIterNextVert(iter, &from)) {
    while (cgraphIterNextEdge(iter, from, &edge, &to)) {
      printf("%lld\t\t%lld\t\t%lld\n", from, to, edge);
    }
  }
  cgraphIterRelease(iter);
  cgraphRelease(&graph);
  return 0;
}

int testMaxFlow() {
  CGraph graph;
  cgraphInit(&graph, true, 10, 15);

  cgraphReserveVert(&graph, 6);
  addEdges(&graph, 9,
           (Endpoint[]){{0, 1},
                        {0, 3},
                        {1, 3},
                        {1, 2},
                        {4, 1},
                        {3, 4},
                        {4, 2},
                        {2, 5},
                        {4, 5}});

  const FlowType capacity[9] = {3, 5, 1, 4, 2, 2, 1, 5, 2};
  FlowType flow[9];
  const FlowType maxFlow =
      cgraphMaxFlowEdmondsKarp(&graph, capacity, flow, 0, 5);

  printf("max flow: %lld\n", maxFlow);
  for (CGraphSize i = 0; i < 9; ++i) {
    printf("%lld ", flow[i]);
  }
  putchar('\n');

  cgraphRelease(&graph);
  return 0;
}

int testWeightedPath() {
  CGraph graph;
  cgraphInit(&graph, true, 10, 15);

  cgraphReserveVert(&graph, 7);
  addEdges(&graph, 12,
           (Endpoint[]){{0, 1},
                        {0, 2},
                        {0, 3},
                        {1, 4},
                        {2, 1},
                        {1, 2},
                        {2, 3},
                        {3, 5},
                        {2, 4},
                        {2, 5},
                        {4, 6},
                        {5, 6}});

  const WeightType weights[12] = {11, 9, 12, 9, 4, 3, 5, 7, 13, 12, 15, 14};
  CGraphId predecessor[7];

  cgraphShortestDijkstra(&graph, weights, predecessor, 0, 6);
  printf("Dijkstra: 6");
  for (CGraphId i = predecessor[6]; i != -1; i = predecessor[i]) {
    printf(" <- %lld", i);
  }

  cgraphShortestBellmanFord(&graph, weights, predecessor, 0);
  printf("\nBellmanFord: 6");
  for (CGraphId i = predecessor[6]; i != -1; i = predecessor[i]) {
    printf(" <- %lld", i);
  }
  putchar('\n');

  cgraphRelease(&graph);
  return 0;
}

int testEulerPath() {
  CGraph graph;
  cgraphInit(&graph, false, 10, 15);

  cgraphReserveVert(&graph, 8);
  addEdgesU(&graph, 10,
            (Endpoint[]){{0, 1},
                         {0, 2},
                         {0, 3},
                         {1, 2},
                         {3, 4},
                         {3, 5},
                         {4, 5},
                         {5, 6},
                         {5, 7},
                         {6, 7}});

  CGraphId path[11];
  cgraphEulerPath(&graph, path, 0, 3);

  printf("Euler Path: 0");
  for (CGraphSize i = 1; i < 11; ++i) {
    printf(" -> %lld", path[i]);
  }
  putchar('\n');

  cgraphRelease(&graph);
  return 0;
}

int main() {
  testIter();
  testMaxFlow();
  testWeightedPath();
  testEulerPath();
}