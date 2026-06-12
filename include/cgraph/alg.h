#ifndef GRAPH_ALG_H
#define GRAPH_ALG_H

#include "types.h"

#ifdef __cplusplus
extern "C" {

#endif

void cgraphCriticalPath(const CGraph *aoa, const TimeType duration[], CGraphId successor[],
                        TimeType earlyStart[], TimeType lateStart[]);

// path: src -> ... -> dst | len = E + 1
CGraphBool cgraphEulerianCircuit(const CGraph *graph, CGraphId path[], CGraphId src);
CGraphBool cgraphEulerianPath(const CGraph *graph, CGraphId path[], CGraphId src, CGraphId dst);

/*
  return count of articulations

  if (*articulations == NULL) {
    *articulations = new memory need to free;
  } else {
    *articulations need enough memory
  }
 */
CGraphInt cgraphArticulations(const CGraph *graph, CGraphId **articulations);

// Strongly Connected Components
// return ComponentID * V
void cgraphStronglyConnected(const CGraph *graph, CGraphId components[]);

// only directed-network
FlowType cgraphMaxFlowEdmondsKarp(const CGraph *network, const FlowType capacity[], FlowType flow[],
                                  CGraphId source, CGraphId sink);

// only undirected-graph
void cgraphSpanningTreePrim(const CGraph *graph, const WeightType weights[], CGraphId predecessor[],
                            CGraphId root);
// only undirected-graph
void cgraphSpanningTreeKruskal(const CGraph *graph, const WeightType weights[], CGraphId edges[]);

void cgraphTopoSort(const CGraph *graph, CGraphId sort[]);

void cgraphTopoPath(const CGraph *graph, CGraphId predecessor[]);

void cgraphUnweightedShortest(const CGraph *graph, CGraphId predecessor[], CGraphId source,
                              CGraphId target);

void cgraphShortestDijkstra(const CGraph *graph, const WeightType weights[], CGraphId predecessor[],
                            CGraphId source, CGraphId target);

// only directed-graph
CGraphBool cgraphShortestBellmanFord(const CGraph *graph, const WeightType weights[],
                                     CGraphId predecessor[], CGraphId source);

// matrix
void FloydWarshallWeightedPath(WeightType **weight, CGraphSize vertNum, CGraphId **path,
                               WeightType **distance);

#ifdef __cplusplus
}
#endif
#endif // GRAPH_ALG_H