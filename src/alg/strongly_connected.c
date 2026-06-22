#include "cgraph/algorithm.h"
#include "cgraph/graph.h"
#include "cgraph/struct/stack.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
  CGraphExplorer *explorer;
  CGraphStack *stack;
  CGraphBool *flag;
  CGraphId *components;
  CGraphId counter;
} Package;

static void forward(Package *pkg, const CGraphId from) {
  CGraphId eid, to;
  pkg->flag[from] = 1;
  while (cgraph_explorer_next_edge(pkg->explorer, from, &eid, &to)) {
    if (!pkg->flag[to]) forward(pkg, to);
  }
  cgraph_stack_push(pkg->stack, from);
}

static void backward(Package *pkg, const CGraphId to) {
  CGraphId eid, from;
  pkg->flag[to] = 0;
  pkg->components[to] = pkg->counter;
  while (cgraph_explorer_next_edge(pkg->explorer, to, &eid, &from)) {
    if (pkg->flag[from]) backward(pkg, from);
  }
}

void cgraph_strongly_connected(const CGraph *graph, CGraphId components[]) {
  Package pkg = {
      .explorer = cgraph_new_explorer(graph, CGRAPH_OUT),
      .stack = cgraph_new_stack(graph->vert.count),
      .flag = calloc(graph->vert.range, sizeof(CGraphBool)),
      .components = components,
      .counter = 0,
  };
  memset(components, CGRAPH_INV_ID, graph->vert.range * sizeof(CGraphId));

  CGraphId from;
  while (cgraph_explorer_next_vertex(pkg.explorer, &from)) {
    if (pkg.flag[from] == 0) forward(&pkg, from);
  }

  cgraph_explorer_reset_all_edges(pkg.explorer, CGRAPH_IN);
  while (!cgraph_stack_empty(pkg.stack)) {
    const CGraphId vert = cgraph_stack_pop(pkg.stack);
    if (pkg.flag[vert] == 1) {
      backward(&pkg, vert);
      pkg.counter++;
    }
  }

  free(pkg.flag);
  cgraph_delete_explorer(pkg.explorer);
  cgraph_delete_stack(pkg.stack);
}