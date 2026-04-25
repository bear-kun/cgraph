#include "cgraph/alg.h"
#include "cgraph/graph.h"
#include "cgraph/iter.h"
#include "struct/stack.h"
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
  while (cgraphExplorerNextEdge(pkg->explorer, from, &eid, &to)) {
    if (!pkg->flag[to]) forward(pkg, to);
  }
  cgraphStackPush(pkg->stack, from);
}

static void backward(Package *pkg, const CGraphId to) {
  CGraphId eid, from;
  pkg->flag[to] = 0;
  pkg->components[to] = pkg->counter;
  while (cgraphExplorerNextEdgeRev(pkg->explorer, to, &eid, &from)) {
    if (pkg->flag[from]) backward(pkg, from);
  }
}

void cgraphStronglyConnected(const CGraph *graph, CGraphId components[]) {
  Package pkg ={
    .explorer = cgraphGetExplorer(graph),
    .stack = cgraphStackCreate(graph->vert.count),
    .flag = calloc(graph->vert.range, sizeof(CGraphBool)),
    .components = components,
    .counter = 0,
  };
  memset(components, INVALID_ID, graph->vert.range * sizeof(CGraphId));

  CGraphId from;
  while (cgraphExplorerNextVert(pkg.explorer, &from)) {
    if (pkg.flag[from] == 0) forward(&pkg, from);
  }

  cgraphExplorerResetAllEdgesRev(pkg.explorer);
  while (!cgraphStackEmpty(pkg.stack)) {
    const CGraphId vert = cgraphStackPop(pkg.stack);
    if (pkg.flag[vert] == 1) {
      backward(&pkg, vert);
      pkg.counter++;
    }
  }

  free(pkg.flag);
  cgraphExplorerRelease(pkg.explorer);
  cgraphStackRelease(pkg.stack);
}