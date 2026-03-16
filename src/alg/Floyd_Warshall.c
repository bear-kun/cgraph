#include "cgraph/types.h"

/*
 * Floyd-Warshall算法
 * 不限正负权
 * 原理：对路径经过的顶点动态规划
 */

void FloydWarshallWeightedPath(WeightType **weight, const CGraphSize vertNum,
                               CGraphId **path, WeightType **distance) {
  for (CGraphId i = 0; i < vertNum; i++) {
    for (CGraphId j = 0; j < vertNum; j++) {
      distance[i][j] = weight[i][j];
      path[i][j] = j;
    }
  }
  for (CGraphId middle = 0; middle < vertNum; ++middle) {
    for (CGraphId source = 0; source < vertNum; ++source) {
      if (distance[source][middle] == CGRAPH_INF) continue;

      for (CGraphId target = 0; target < vertNum; ++target) {
        if (distance[middle][target] == CGRAPH_INF) continue;

        if (distance[source][middle] + distance[middle][target] <
            distance[source][target]) {
          distance[source][target] =
              distance[source][middle] + distance[middle][target];
          path[source][target] = middle;
        }
      }
    }
  }
}