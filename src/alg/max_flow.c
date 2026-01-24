#include "internal/developer.h"
#include "struct/queue.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
  CGraphId src, sink;
  CGraphView *residual;
  CGraphIter *iter;
  CGraphId *pred;
  const FlowType *cap;
  FlowType *curr;
  FlowType *flow;
} Package;

static CGraphView *buildResidualNetwork(const CGraphView *src) {
  char *mem =
      malloc(sizeof(CGraphView) +
             (src->vertRange + 2 * src->edgeRange) * sizeof(CGraphId));
  CGraphView *residual = (CGraphView *)mem;
  *residual = *src;
  residual->directed = false;
  residual->edgeHead = (CGraphId *)(mem + sizeof(CGraphView));
  residual->edgeNext = residual->edgeHead + src->vertRange;

  for (CGraphSize i = 0; i < residual->vertRange; ++i)
    residual->edgeHead[i] = DID(src->edgeHead[i]);
  for (CGraphSize i = 0; i < residual->edgeRange; ++i)
    residual->edgeNext[i << 1] = DID(src->edgeNext[i]);
  return residual;
}

/*
 * 广度优先搜索寻找最短路径，
 * 之所以不用贪心寻找可扩容最大的边，
 * 是因为这可能会导致capacity大的边被反复反转，
 * 不如最短路径收敛稳定 O(V * E^2)
 */
static CGraphBool bfs(const Package *pkg, CGraphQueue *const queue) {
  CGraphId did, eid, to;
  cgraphQueueClear(queue);
  cgraphQueuePush(queue, pkg->src);
  while (!cgraphQueueEmpty(queue)) {
    const CGraphId from = cgraphQueuePop(queue);

    while (cgraphIterNextDirect(pkg->iter, from, &did)) {
      cgraphIterParseF(pkg->residual, did, &eid, &to);
      if (pkg->pred[to] != INVALID_ID || to == pkg->src) continue;

      pkg->pred[to] = did;

      if (to == pkg->sink) return true;
      cgraphQueuePush(queue, to);
    }
  }
  return false;
}

// 寻找路径可调整的flow = min(capacity - flow)
static FlowType pathFlow(const Package *pkg) {
  FlowType flow = UNREACHABLE;
  CGraphId eid, from;
  for (CGraphId did = pkg->pred[pkg->sink]; did != INVALID_ID;
       did = pkg->pred[from]) {
    cgraphIterParseB(pkg->residual, did, &eid, &from);
    if (flow > pkg->cap[eid] - pkg->curr[eid]) {
      flow = pkg->cap[eid] - pkg->curr[eid];
    }
  }
  return flow;
}

static void reverse(const CGraphView *const residual, const CGraphId did,
                    const CGraphId from, const CGraphId to) {
  CGraphId *predNext =
      cgraphFind(residual->edgeNext, residual->edgeHead + from, did);
  cgraphUnlink(residual->edgeNext, predNext);
  cgraphInsertEdge(residual, to, REVERSE(did));
}

static void update(const Package *pkg, const FlowType step) {
  CGraphId eid, from, to = pkg->sink;
  for (CGraphId did = pkg->pred[to]; did != INVALID_ID; did = pkg->pred[from]) {
    cgraphIterParseB(pkg->residual, did, &eid, &from);
    pkg->curr[eid] += step;

    // 如果edge是正向的，则flow的增加是同向的；否则相反
    if (did & 1)
      pkg->flow[eid] -= step;
    else
      pkg->flow[eid] += step;

    if (pkg->curr[eid] == pkg->cap[eid]) {
      // 若残余网络的边的flow满容，则反转，
      // 视作原网络边可释放的flow
      pkg->curr[eid] = 0;
      reverse(pkg->residual, did, from, to);
    }
    to = from;
  }
  cgraphIterResetEdge(pkg->iter, INVALID_ID);
}

FlowType cgraphMaxFlowEdmondsKarp(const CGraph *network,
                                  const FlowType capacity[], FlowType flow[],
                                  const CGraphId source, const CGraphId sink) {
  const CGraphView *view = VIEW(network);
  CGraphView *residual = buildResidualNetwork(view);

  CGraphIter *iter = cgraphIterFromView(residual);
  CGraphQueue *queue = cgraphQueueCreate(network->vertNum);
  CGraphId *pred = malloc(view->vertRange * sizeof(CGraphId));
  FlowType *curr = calloc(view->edgeRange, sizeof(FlowType));
  memset(flow, 0, view->edgeRange * sizeof(FlowType));

  const Package pkg = {source, sink,     residual, iter,
                       pred,   capacity, curr,     flow};

  FlowType maxFlow = 0;
  while (1) {
    memset(pred, INVALID_ID, view->vertRange * sizeof(CGraphId));
    if (!bfs(&pkg, queue)) break;

    const FlowType step = pathFlow(&pkg);
    update(&pkg, step);
    maxFlow += step;
  }

  free(pred);
  free(curr);
  free(residual);
  cgraphIterRelease(iter);
  cgraphQueueRelease(queue);
  return maxFlow;
}