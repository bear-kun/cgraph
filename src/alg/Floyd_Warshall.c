#include "internal/developer.h"

/*
 * Floyd-Warshall算法
 * 不限正负权
 * 原理：对路径经过的顶点动态规划
 */

void FloydWarshallWeightedPath(WeightType **weight, const CGraphSize vertNum,
                               CGraphId **path, WeightType **distance) {
  for (CGraphSize i = 0; i < vertNum; i++) {
    for (CGraphSize j = 0; j < vertNum; j++) {
      distance[i][j] = weight[i][j];
      path[i][j] = j;
    }
  }
  for (CGraphSize middle = 0; middle < vertNum; ++middle) {
    for (CGraphSize source = 0; source < vertNum; ++source) {
      if (distance[source][middle] == UNREACHABLE) continue;

      for (CGraphSize target = 0; target < vertNum; ++target) {
        if (distance[middle][target] == UNREACHABLE) continue;

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