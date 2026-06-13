#include "cgraph/algorithm.h"
#include "cgraph/graph.h"
#include "cgraph/iterator.h"
#include "cgraph/struct/queue.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
  CGraphId *offset;
  CGraphId *edges;
  CGraphId *edge_xor; // from ^ to
  CGraphId *cursor;
} Residual;

typedef struct {
  const Residual *residual;
  CGraphId from;
  CGraphId current, end;
} ResidualIter;

static void callback(const CGraphId from, const CGraphId eid, const CGraphId to, void *data) {
  const Residual *residual = data;
  residual->edges[residual->cursor[from]++] = eid;
  residual->edges[residual->cursor[to]++] = ~eid;
}

static void residual_init(Residual *residual, const CGraph *network) {
  residual->offset = malloc((network->vert.range + 1) * sizeof(CGraphId));
  residual->edges = malloc(2 * network->edge.count * sizeof(CGraphId));
  residual->edge_xor = network->edge.xor_;
  residual->cursor = residual->offset + 1;

  CGraphId begin = 0;
  residual->offset[0] = 0;
  for (CGraphSize v = 0; v < network->vert.range; v++) {
    residual->cursor[v] = begin;
    begin += network->vert.degree[CGRAPH_OUT][v] + network->vert.degree[CGRAPH_IN][v];
  }
  cgraph_traverse_edges(network, residual, callback);
}

static void residual_release(const Residual *residual) {
  free(residual->offset);
  free(residual->edges);
}

static void residual_reverse(const Residual *residual, const CGraphId from, const CGraphId eid,
                             const CGraphId to) {
  const CGraphId end1 = residual->offset[from + 1];
  for (CGraphId i = residual->offset[from]; i != end1; i++) {
    if (residual->edges[i] == eid) {
      residual->edges[i] = ~eid;
      break;
    }
  }
  const CGraphId end2 = residual->offset[to + 1];
  for (CGraphId i = residual->offset[to]; i != end2; i++) {
    if (residual->edges[i] == ~eid) {
      residual->edges[i] = eid;
      break;
    }
  }
}

static ResidualIter residual_get_iter(const Residual *residual, const CGraphId from) {
  ResidualIter iter;
  iter.residual = residual;
  iter.from = from;
  iter.current = residual->offset[from];
  iter.end = residual->offset[from + 1];
  return iter;
}

static CGraphBool residual_iter_next_edge(ResidualIter *iter, CGraphId *eid, CGraphId *to) {
  while (iter->current != iter->end) {
    const CGraphId did = iter->residual->edges[iter->current++];
    if (did < 0) continue;
    *eid = did;
    *to = iter->from ^ iter->residual->edge_xor[did];
    return true;
  }
  return false;
}

typedef struct {
  Residual *residual;
  CGraphId source, sink;

  struct {
    const FlowType *capacity;
    FlowType *current;
    CGraphBool *reverse;
  } flow;

  struct {
    CGraphInt *version;
    CGraphId *incoming;
  } bfs;
} Package;

static CGraphBool bfs(const Package *pkg, CGraphQueue *queue, const CGraphInt version) {
  pkg->bfs.version[pkg->source] = version;

  cgraph_queue_clear(queue);
  cgraph_queue_push(queue, pkg->source);
  while (!cgraph_queue_empty(queue)) {
    const CGraphId from = cgraph_queue_pop(queue);

    CGraphId eid, to;
    ResidualIter iter = residual_get_iter(pkg->residual, from);
    while (residual_iter_next_edge(&iter, &eid, &to)) {
      if (pkg->bfs.version[to] != version) {
        pkg->bfs.version[to] = version;
        pkg->bfs.incoming[to] = eid;
        if (to == pkg->sink) return true;
        cgraph_queue_push(queue, to);
      }
    }
  }
  return false;
}

static FlowType path_flow(const Package *pkg) {
  FlowType flow = CGRAPH_INF;
  CGraphId to = pkg->sink;
  while (to != pkg->source) {
    const CGraphId eid = pkg->bfs.incoming[to];
    const CGraphId from = pkg->residual->edge_xor[eid] ^ to;
    if (flow > pkg->flow.capacity[eid] - pkg->flow.current[eid]) {
      flow = pkg->flow.capacity[eid] - pkg->flow.current[eid];
    }
    to = from;
  }
  return flow;
}

static void update(const Package *pkg, const FlowType step) {
  CGraphId to = pkg->sink;
  while (to != pkg->source) {
    const CGraphId eid = pkg->bfs.incoming[to];
    const CGraphId from = pkg->residual->edge_xor[eid] ^ to;
    pkg->flow.current[eid] += step;

    if (pkg->flow.current[eid] >= pkg->flow.capacity[eid] * (1 - CGRAPH_EPS)) {
      pkg->flow.current[eid] = 0;
      pkg->flow.reverse[eid] = !pkg->flow.reverse[eid];
      residual_reverse(pkg->residual, from, eid, to);
    }
    to = from;
  }
}

FlowType cgraph_max_flow_edmonds_karp(const CGraph *network, const FlowType capacity[],
                                      FlowType flow[],
                                      const CGraphId source, const CGraphId sink) {
  Residual residual;
  residual_init(&residual, network);
  CGraphQueue *queue = cgraph_new_queue(network->vert.count);
  memset(flow, 0, network->edge.range * sizeof(FlowType));

  const Package pkg = {
      .residual = &residual,
      .source = source,
      .sink = sink,
      .flow.capacity = capacity,
      .flow.current = flow,
      .flow.reverse = calloc(network->edge.range, sizeof(CGraphBool)),
      .bfs.version = calloc(network->vert.range, sizeof(CGraphInt)),
      .bfs.incoming = malloc(network->vert.range * sizeof(CGraphId)),
  };

  FlowType max_flow = 0;
  CGraphInt version = 0;
  while (true) {
    if (!bfs(&pkg, queue, ++version)) break;

    const FlowType step = path_flow(&pkg);
    update(&pkg, step);
    max_flow += step;
  }

  for (CGraphSize e = 0; e < network->edge.range; e++) {
    if (pkg.flow.reverse[e]) {
      flow[e] = capacity[e] - flow[e];
    }
  }

  free(pkg.flow.reverse);
  free(pkg.bfs.version);
  free(pkg.bfs.incoming);
  residual_release(&residual);
  cgraph_delete_queue(queue);
  return max_flow;
}