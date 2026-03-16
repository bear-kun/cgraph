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

/*
 * 割点：若删除该点及其关联的边后，图会被分割成两个或多个不连通的子图
 * 本函数只用在无向图，且连通
 */
void cgraphFindArticulation(const CGraph *graph,
                            CGraphLinkedNode **articulations);

// 强连接分支 Strongly Connected Component
void cgraphFindScc(const CGraph *graph, CGraphId connectionId[]);

FlowType cgraphMaxFlowEdmondsKarp(const CGraph *network,
                                  const FlowType capacity[], FlowType flow[],
                                  CGraphId source, CGraphId sink);

/*
 * 只支持无向图
 * 输出顶点前驱（树状）
 */
void cgraphMSTPrim(const CGraph *graph, const WeightType weights[],
                   CGraphId predecessor[], CGraphId root);

/*
 * 只支持无向图
 * 输出边id
 */
void cgraphMSTKruskal(const CGraph *graph, const WeightType weights[],
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
// 不限正负权值
void FloydWarshallWeightedPath(WeightType **weight, CGraphSize vertNum,
                               CGraphId **path, WeightType **distance);

#endif // GRAPH_ALG_H
