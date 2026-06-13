#include "cgraph/alg.h"
#include "cgraph/iter.h"
#include "cgraph/struct/queue.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
  CGraphId to;
  WeightType weight;
} BFEdge;

typedef struct {
  CGraphId *offset;
  BFEdge *edges;
  CGraphId *cursor;
} CSR;

static void callback(const CGraphId from, const CGraphId eid, const CGraphId to, void *data) {
  const CSR *csr = *(void **)data;
  const WeightType *weights = *((void **)data + 1);
  csr->edges[csr->cursor[from]++] = (BFEdge){to, weights[eid]};
}

static void init_csr(CSR *csr, const CGraph *graph, const WeightType *weights) {
  csr->offset = malloc((graph->vert.range + 1) * sizeof(CGraphId));
  csr->edges = malloc(graph->edge.count * sizeof(BFEdge));
  csr->cursor = csr->offset + 1;

  CGraphId begin = 0;
  csr->offset[0] = 0;
  for (CGraphSize v = 0; v < graph->vert.range; v++) {
    csr->cursor[v] = begin;
    begin += graph->vert.degree[CGRAPH_OUT][v];
  }

  const void *data[2] = {csr, weights};
  cgraph_traverse_edges(graph, data, callback);
}

static void release_csr(const CSR *csr) {
  free(csr->offset);
  free(csr->edges);
}

// SPFA, Shortest Path Faster Algorithm
CGraphBool cgraph_shortest_bellman_ford(const CGraph *graph, const WeightType weights[],
                                        CGraphId predecessor[], const CGraphId source) {
  CSR csr;
  init_csr(&csr, graph, weights);
  CGraphQueue *queue = cgraph_new_queue(graph->vert.count);
  CGraphBool *in_queue = calloc(graph->vert.range, sizeof(CGraphBool));
  CGraphSize *depth = calloc(graph->vert.range, sizeof(CGraphSize));
  WeightType *distance = malloc(graph->vert.range * sizeof(WeightType));
  memset(predecessor, INVALID_ID, graph->vert.range * sizeof(CGraphId));
  for (CGraphId i = 0; i < graph->vert.range; i++) distance[i] = CGRAPH_INF;

  CGraphBool success = true;
  depth[source] = 1;
  distance[source] = 0;
  cgraph_queue_push(queue, source);
  while (!cgraph_queue_empty(queue)) {
    const CGraphId from = cgraph_queue_pop(queue);
    in_queue[from] = false;

    const CGraphId end = csr.offset[from + 1];
    for (CGraphId e = csr.offset[from]; e < end; e++) {
      const BFEdge *edge = csr.edges + e;
      const CGraphId to = edge->to;

      if (distance[to] <= distance[from] + edge->weight) continue;
      distance[to] = distance[from] + edge->weight;
      predecessor[to] = from;

      depth[to] = depth[from] + 1;
      if (depth[to] == graph->vert.count) {
        success = false;
        goto end;
      }

      if (!in_queue[to]) {
        cgraph_queue_push(queue, to);
        in_queue[to] = true;
      }
    }
  }

end:
  free(depth);
  free(in_queue);
  free(distance);
  cgraph_delete_queue(queue);
  release_csr(&csr);
  return success;
}