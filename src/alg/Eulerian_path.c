#include "cgraph/alg.h"
#include "cgraph/iter.h"
#include "struct/stack.h"

#include <stdio.h>
#include <stdlib.h>

/*
 * 寻找初始路径 -> 回溯插入所有回环
 *
 * 1. S -> A      ->      B -> T  (初始路径)
 *          -> C -> D -> A        (A -> C -> D -> A 回环)
 *
 * 2. S -> A -> C -> D -> B -> T  (回溯插入)
 */

typedef struct {
  CGraphIter *iter;
  CGraphBool *visited;
  CGraphId *path;
  CGraphId target; // 当前回环或路径的临时目标点target
  CGraphId to;
} Package;

static CGraphBool getTargetEdge(Package *pkg, const CGraphId from) {
  CGraphId eid;
  while (cgraphIterNextEdge(pkg->iter, from, &eid, &pkg->to)) {
    if (!pkg->visited[eid]) {
      pkg->visited[eid] = true;
      return true;
    }
  }
  return false;
}

static inline CGraphBool insert(Package *pkg, const CGraphId from) {
  *--pkg->path = from;
  if (pkg->target == INVALID_ID) {
    pkg->target = from;
    return true;
  }
  return from == pkg->target;
}

// 递归实现
static CGraphBool EulerianPath_recursive(Package *pkg, const CGraphId from) {
  while (1) {
    if (!getTargetEdge(pkg, from)) return insert(pkg, from);
    if (!EulerianPath_recursive(pkg, pkg->to)) return false;
    pkg->target = from;
  }
}

// 栈实现
static CGraphBool EulerianPath_stack(Package *pkg, CGraphStack *stack,
                                     CGraphId from) {
  while (1) {
    if (getTargetEdge(pkg, from)) {
      // 调用
      cgraphStackPush(stack, from);
      from = pkg->to;
      continue;
    }

    if (!insert(pkg, from)) return false;

    if (cgraphStackEmpty(stack)) return true; // 结束
    from = cgraphStackPop(stack); // 返回
    pkg->target = from;
  }
}

CGraphBool cgraphEulerianPath(const CGraph *const graph, CGraphId path[],
                              const CGraphId src, const CGraphId dst) {
  Package pkg = {
      .iter = cgraphGetIter(graph),
      .visited = calloc(graph->edgeRange, sizeof(CGraphBool)),
      .path = path + graph->edgeNum + 1,
      .target = dst
  };

  path[0] = INVALID_ID;
  // const CGraphBool res = EulerianPath_recursive(&pkg, src);
  CGraphStack *stack = cgraphStackCreate(graph->edgeNum);
  const CGraphBool res = EulerianPath_stack(&pkg, stack, src);

  free(pkg.visited);
  cgraphStackRelease(stack);
  cgraphIterRelease(pkg.iter);
  return res && path[0] != INVALID_ID;
}

CGraphBool cgraphEulerianCircuit(const CGraph *const graph, CGraphId path[],
                                 const CGraphId src) {
  return cgraphEulerianPath(graph, path, src, src);
}