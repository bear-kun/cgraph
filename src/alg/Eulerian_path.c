#include "cgraph/alg.h"
#include "cgraph/iter.h"
#include "cgraph/struct/stack.h"
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
  CGraphExplorer *explorer;
  CGraphBool *visited;
  CGraphId *path;
  CGraphId target; // 当前回环或路径的临时目标点target
  CGraphId to;
} Package;

static CGraphBool get_target_edge(Package *pkg, const CGraphId from) {
  CGraphId eid;
  while (cgraph_explorer_next_edge(pkg->explorer, from, &eid, &pkg->to)) {
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
static CGraphBool eulerian_path_recursive(Package *pkg, const CGraphId from) {
  while (1) {
    if (!get_target_edge(pkg, from)) return insert(pkg, from);
    if (!eulerian_path_recursive(pkg, pkg->to)) return false;
    pkg->target = from;
  }
}

// 栈实现
static CGraphBool eulerian_path_stack(Package *pkg, CGraphStack *stack, CGraphId from) {
  while (1) {
    if (get_target_edge(pkg, from)) {
      // 调用
      cgraph_stack_push(stack, from);
      from = pkg->to;
      continue;
    }

    if (!insert(pkg, from)) return false;

    if (cgraph_stack_empty(stack)) return true; // 结束
    from = cgraph_stack_pop(stack); // 返回
    pkg->target = from;
  }
}

CGraphBool cgraph_eulerian_path(const CGraph *graph, CGraphId path[], const CGraphId src,
                                const CGraphId dst) {
  Package pkg = {
      .explorer = cgraph_new_explorer(graph, CGRAPH_OUT),
      .visited = calloc(graph->edge.range, sizeof(CGraphBool)),
      .path = path + graph->edge.count + 1,
      .target = dst
  };

  path[0] = INVALID_ID;
  // const CGraphBool res = EulerianPath_recursive(&pkg, src);
  CGraphStack *stack = cgraph_new_stack(graph->edge.count);
  const CGraphBool res = eulerian_path_stack(&pkg, stack, src);

  free(pkg.visited);
  cgraph_delete_stack(stack);
  cgraph_delete_explorer(pkg.explorer);
  return res && path[0] != INVALID_ID;
}

CGraphBool cgraph_eulerian_circuit(const CGraph *graph, CGraphId path[], const CGraphId src) {
  return cgraph_eulerian_path(graph, path, src, src);
}