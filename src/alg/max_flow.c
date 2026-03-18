#include "cgraph/graph.h"
#include "cgraph/iter.h"
#include "struct/queue.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
  const CGraph *network;
  CGraph *residual;
  CGraphId src, sink;
  CGraphId *pred; // eid
  const FlowType *cap;
  FlowType *curr;
  FlowType *flow;
} Package;

/*
 * 广度优先搜索寻找最短路径，
 * 之所以不用贪心寻找可扩容最大的边，
 * 是因为这可能会导致capacity大的边被反复反转，
 * 不如最短路径收敛稳定 O(V * E^2)
 */
static CGraphBool bfs(const Package *pkg, CGraphQueue *const queue) {
  cgraphQueueClear(queue);
  cgraphQueuePush(queue, pkg->src);
  while (!cgraphQueueEmpty(queue)) {
    const CGraphId from = cgraphQueuePop(queue);

    CGraphId eid, to;
    CGraphIterLite iter = cgraphGetEdgeIter(pkg->residual, from);
    while (cgraphIterLiteNextEdge(&iter, &eid, &to)) {
      if (to != pkg->src && pkg->pred[to] == INVALID_ID) {
        pkg->pred[to] = eid;
        if (to == pkg->sink) return true;
        cgraphQueuePush(queue, to);
      }
    }
  }
  return false;
}

// 寻找路径可调整的flow = min(capacity - flow)
static FlowType pathFlow(const Package *pkg) {
  FlowType flow = CGRAPH_INF;
  CGraphId eid = pkg->pred[pkg->sink];
  while (eid != INVALID_ID) {
    const CGraphId from = cgraphParseEdgeFrom(pkg->residual, eid);
    if (flow > pkg->cap[eid] - pkg->curr[eid]) {
      flow = pkg->cap[eid] - pkg->curr[eid];
    }
    eid = pkg->pred[from];
  }
  return flow;
}

static void update(const Package *pkg, const FlowType step) {
  CGraphId eid = pkg->pred[pkg->sink];
  while (eid != INVALID_ID) {
    const CGraphId networkFrom = cgraphParseEdgeFrom(pkg->network, eid);
    const CGraphId residualFrom = cgraphParseEdgeFrom(pkg->residual, eid);

    if (networkFrom == residualFrom) {
      pkg->flow[eid] += step;
    } else {
      pkg->flow[eid] -= step;
    }

    pkg->curr[eid] += step;
    if (pkg->curr[eid] == pkg->cap[eid]) {
      // 若残余网络的边的flow满容，则反转，
      // 视作原网络边可释放的flow
      pkg->curr[eid] = 0;
      cgraphReverseEdge(pkg->residual, eid);
    }

    eid = pkg->pred[residualFrom];
  }
}

FlowType cgraphMaxFlowEdmondsKarp(const CGraph *network,
                                  const FlowType capacity[], FlowType flow[],
                                  const CGraphId source, const CGraphId sink) {
  CGraph residual;
  cgraphCopy(&residual, network);
  CGraphQueue *queue = cgraphQueueCreate(network->vertNum);
  CGraphId *pred = malloc(network->vertRange * sizeof(CGraphId));
  FlowType *curr = calloc(network->edgeRange, sizeof(FlowType));
  memset(flow, 0, network->edgeRange * sizeof(FlowType));

  const Package pkg = {network, &residual, source, sink,
                       pred, capacity, curr, flow};

  FlowType maxFlow = 0;
  while (true) {
    memset(pred, INVALID_ID, network->vertRange * sizeof(CGraphId));
    if (!bfs(&pkg, queue)) break;

    const FlowType step = pathFlow(&pkg);
    update(&pkg, step);
    maxFlow += step;
  }

  free(pred);
  free(curr);
  cgraphRelease(&residual);
  cgraphQueueRelease(queue);
  return maxFlow;
}