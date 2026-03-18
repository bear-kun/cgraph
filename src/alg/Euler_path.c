#include "cgraph/iter.h"
#include "struct/stack.h"
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
  CGraphId currTgt; // 当前回环或路径的临时目标点target
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
  return from == pkg->currTgt;
}

// 递归实现
static CGraphBool EulerPath_recursive(Package *pkg, const CGraphId from) {
  while (1) {
    if (!getTargetEdge(pkg, from)) return insert(pkg, from);
    if (!EulerPath_recursive(pkg, pkg->to)) return false;
    pkg->currTgt = from;
  }
}

// 栈实现
static CGraphBool EulerPath_stack(Package *pkg, CGraphStack *stack,
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
    pkg->currTgt = from;
  }
}

void cgraphEulerPath(const CGraph *const graph, CGraphId path[],
                     const CGraphId src, const CGraphId dst) {
  Package pkg = {.iter = cgraphGetIter(graph),
                 .visited = calloc(graph->edgeRange, sizeof(CGraphBool)),
                 .path = path + graph->edgeNum + 1,
                 .currTgt = dst};

  // if (!EulerPath_recursive(&pkg, src))
  CGraphStack *stack = cgraphStackCreate(graph->edgeNum);
  if (!EulerPath_stack(&pkg, stack, src)) {
    *path = INVALID_ID;
  }

  free(pkg.visited);
  cgraphStackRelease(stack);
  cgraphIterRelease(pkg.iter);
}

void cgraphEulerCircuit(const CGraph *const graph, CGraphId path[],
                        const CGraphId src) {
  cgraphEulerPath(graph, path, src, src);
}