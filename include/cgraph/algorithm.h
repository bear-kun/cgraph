#ifndef CGRAPH_ALGORITHM_H
#define CGRAPH_ALGORITHM_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void cgraph_critical_path(const CGraph *aoa, const TimeType duration[], CGraphId successor[],
                          TimeType early_start[], TimeType late_start[]);

// path: src -> ... -> dst | len = E + 1
CGraphBool cgraph_eulerian_circuit(const CGraph *graph, CGraphId path[], CGraphId src);
CGraphBool cgraph_eulerian_path(const CGraph *graph, CGraphId path[], CGraphId src, CGraphId dst);

/*
  return count of articulations

  if (*articulations == NULL) {
    *articulations = new memory need to free;
  } else {
    *articulations need enough memory
  }
 */
CGraphInt cgraph_articulations(const CGraph *graph, CGraphId **articulations);

// Strongly Connected Components
// return ComponentID * V
void cgraph_strongly_connected(const CGraph *graph, CGraphId components[]);

// only directed-network
FlowType cgraph_max_flow_edmonds_karp(const CGraph *network, const FlowType capacity[],
                                      FlowType flow[], CGraphId source, CGraphId sink);

// only undirected-graph
void cgraph_spanning_tree_prim(const CGraph *graph, const WeightType weights[],
                               CGraphId predecessor[], CGraphId root);
// only undirected-graph
void cgraph_spanning_tree_kruskal(const CGraph *graph, const WeightType weights[],
                                  CGraphId edges[]);

void cgraph_topo_sort(const CGraph *graph, CGraphId sort[]);

void cgraph_topo_path(const CGraph *graph, CGraphId predecessor[]);

void cgraph_unweighted_shortest(const CGraph *graph, CGraphId predecessor[], CGraphId source,
                                CGraphId target);

void cgraph_shortest_dijkstra(const CGraph *graph, const WeightType weights[],
                              CGraphId predecessor[], CGraphId source, CGraphId target);

// only directed-graph
CGraphBool cgraph_shortest_bellman_ford(const CGraph *graph, const WeightType weights[],
                                        CGraphId predecessor[], CGraphId source);

// matrix
void cgraph_shortest_floyd_warshall(WeightType **weight, CGraphSize vert_count, CGraphId **path,
                                    WeightType **distance);

#ifdef __cplusplus
}
#endif
#endif // CGRAPH_ALGORITHM_H