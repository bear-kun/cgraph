#include "cgraph/alg.h"

/*
 * Floyd-Warshall算法
 * 不限正负权
 * 原理：对路径经过的顶点动态规划
 */

void cgraph_shortest_floyd_warshall(WeightType **weight, const CGraphSize vert_count,
                                    CGraphId **path, WeightType **distance) {
  for (CGraphId i = 0; i < vert_count; i++) {
    for (CGraphId j = 0; j < vert_count; j++) {
      distance[i][j] = weight[i][j];
      path[i][j] = j;
    }
  }
  for (CGraphId middle = 0; middle < vert_count; ++middle) {
    for (CGraphId source = 0; source < vert_count; ++source) {
      if (distance[source][middle] == CGRAPH_INF) continue;

      for (CGraphId target = 0; target < vert_count; ++target) {
        if (distance[middle][target] == CGRAPH_INF) continue;

        if (distance[source][middle] + distance[middle][target] < distance[source][target]) {
          distance[source][target] = distance[source][middle] + distance[middle][target];
          path[source][target] = middle;
        }
      }
    }
  }
}