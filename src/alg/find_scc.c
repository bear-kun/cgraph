#include "cgraph/graph.h"
#include "cgraph/iter.h"
#include "struct/stack.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
  CGraph *reverse;
  CGraphIter *iter;
  CGraphStack *stack;
  CGraphBool *flag;
  CGraphId *connectionId;
  CGraphId counter;
} Package;

static void forward(Package *pkg, const CGraphId from) {
  CGraphId eid, to;
  pkg->flag[from] = 1;
  while (cgraphIterNextEdge(pkg->iter, from, &eid, &to)) {
    if (!pkg->flag[to]) forward(pkg, to);
    cgraphAddEdge(pkg->reverse, to, from); // 边转向
  }
  cgraphStackPush(pkg->stack, from);
}

static void backward(Package *pkg, const CGraphId from) {
  CGraphId eid, to;
  pkg->connectionId[from] = pkg->counter;
  pkg->flag[from] = 0;
  while (cgraphIterNextEdge(pkg->iter, from, &eid, &to)) {
    if (pkg->flag[to]) backward(pkg, to);
  }
}

void cgraphFindScc(const CGraph *graph, CGraphId connectionId[]) {
  CGraph reverse;
  cgraphCopyVert(&reverse, graph);
  CGraphIter *iter = cgraphGetIter(graph);
  CGraphStack *stack = cgraphStackCreate(graph->vertNum);
  CGraphBool *flag = calloc(graph->vertRange, sizeof(CGraphBool));
  Package pkg = {&reverse, iter, stack, flag, connectionId, 0};
  memset(reverse.edgeHead, INVALID_ID, graph->vertRange * sizeof(CGraphId));
  memset(connectionId, INVALID_ID, graph->vertRange * sizeof(CGraphId));

  // 正序
  CGraphId from;
  while (cgraphIterNextVert(pkg.iter, &from)) {
    if (flag[from] == 0) forward(&pkg, from);
  }

  // 逆序
  iter->view = &reverse;
  cgraphIterResetEdge(pkg.iter, INVALID_ID);
  while (!cgraphStackEmpty(stack)) {
    const CGraphId vert = cgraphStackPop(stack);
    if (flag[vert] == 1) backward(&pkg, vert);
    ++pkg.counter;
  }

  free(flag);
  cgraphIterRelease(iter);
  cgraphRelease(&reverse);
  cgraphStackRelease(stack);
}
