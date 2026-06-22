#include "cgraph/algorithm.h"

/*
 * Floyd-Warshall算法
 * 不限正负权
 * 原理：对路径经过的顶点动态规划
 */

void cgraph_shortest_floyd_warshall(WeightType **weight, const CGraphSize vert_count,
                                    CGraphId **path, WeightType **distance) {
  for (CGraphSize i = 0; i < vert_count; i++) {
    for (CGraphSize j = 0; j < vert_count; j++) {
      distance[i][j] = weight[i][j];
      path[i][j] = (CGraphId)j;
    }
  }
  for (CGraphSize middle = 0; middle < vert_count; ++middle) {
    for (CGraphSize source = 0; source < vert_count; ++source) {
      if (distance[source][middle] == CGRAPH_INF) continue;

      for (CGraphSize target = 0; target < vert_count; ++target) {
        if (distance[middle][target] == CGRAPH_INF) continue;

        if (distance[source][middle] + distance[middle][target] < distance[source][target]) {
          distance[source][target] = distance[source][middle] + distance[middle][target];
          path[source][target] = (CGraphId)middle;
        }
      }
    }
  }
}