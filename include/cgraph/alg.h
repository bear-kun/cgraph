#ifndef GRAPH_ALG_H
#define GRAPH_ALG_H

#include "types.h"
#include "struct/linked_list.h"

void cgraphCriticalPath(const CGraph *aoa, const CGraphInt indegree[],
                        const TimeType duration[], CGraphId successor[],
                        TimeType earlyStart[], TimeType lateStart[]);

// path: src -> ... -> dst
void cgraphEulerCircuit(const CGraph *graph, CGraphId path[], CGraphId src);
void cgraphEulerPath(const CGraph *graph, CGraphId path[], CGraphId src,
                     CGraphId dst);

void cgraphArticulation(const CGraph *graph, CGraphLinkedNode **articulations);

// Strongly Connected Components
// return ComponentID * V
void cgraphStronglyConnected(const CGraph *graph, CGraphId components[]);

FlowType cgraphMaxFlowEdmondsKarp(const CGraph *network,
                                  const FlowType capacity[],
                                  FlowType flow[], CGraphId source,
                                  CGraphId sink);

void cgraphSpanningTreePrim(const CGraph *graph, const WeightType weights[],
                            CGraphId predecessor[], CGraphId root);

void cgraphSpanningTreeKruskal(const CGraph *graph, const WeightType weights[],
                               CGraphId edges[]);

void cgraphTopoSort(const CGraph *graph, const CGraphInt indegree[],
                    CGraphId sort[]);

void cgraphTopoPath(const CGraph *graph, const CGraphInt indegree[],
                    CGraphId predecessor[]);

void cgraphUnweightedShortest(const CGraph *graph, CGraphId predecessor[],
                              CGraphId source, CGraphId target);

void cgraphShortestDijkstra(const CGraph *graph, const WeightType weights[],
                            CGraphId predecessor[], CGraphId source,
                            CGraphId target);

void cgraphShortestBellmanFord(const CGraph *graph, const WeightType weights[],
                               CGraphId predecessor[], CGraphId source);

// matrix
void FloydWarshallWeightedPath(WeightType **weight, CGraphSize vertNum,
                               CGraphId **path, WeightType **distance);

#endif // GRAPH_ALG_H