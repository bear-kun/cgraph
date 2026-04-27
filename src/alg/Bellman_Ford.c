#include "cgraph/alg.h"
#include "cgraph/iter.h"
#include "struct/queue.h"
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

static void initCSR(CSR *csr, const CGraph *graph, const WeightType *weights) {
  csr->offset = malloc((graph->vert.range + 1) * sizeof(CGraphId));
  csr->edges = malloc(graph->edge.count * sizeof(BFEdge));
  csr->cursor = csr->offset + 1;

  CGraphId begin = 0;
  csr->offset[0] = 0;
  for (CGraphSize v = 0; v < graph->vert.range; v++) {
    csr->cursor[v] = begin;
    begin += graph->vert.degree[0][v];
  }

  const void *data[2] = {csr, weights};
  cgraphTraverseEdges(graph, data, callback);
}

static void releaseCSR(const CSR *csr) {
  free(csr->offset);
  free(csr->edges);
}

// SPFA, Shortest Path Faster Algorithm
CGraphBool cgraphShortestBellmanFord(const CGraph *graph, const WeightType weights[],
                                     CGraphId predecessor[], const CGraphId source) {
  CSR csr;
  initCSR(&csr, graph, weights);
  CGraphQueue *queue = cgraphQueueCreate(graph->vert.count);
  CGraphBool *isInQueue = calloc(graph->vert.range, sizeof(CGraphBool));
  CGraphSize *depth = calloc(graph->vert.range, sizeof(CGraphSize));
  WeightType *distance = malloc(graph->vert.range * sizeof(WeightType));
  memset(predecessor, INVALID_ID, graph->vert.range * sizeof(CGraphId));
  for (CGraphId i = 0; i < graph->vert.range; i++) distance[i] = CGRAPH_INF;

  CGraphBool success = true;
  depth[source] = 1;
  distance[source] = 0;
  cgraphQueuePush(queue, source);
  while (!cgraphQueueEmpty(queue)) {
    const CGraphId from = cgraphQueuePop(queue);
    isInQueue[from] = false;

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

      if (!isInQueue[to]) {
        cgraphQueuePush(queue, to);
        isInQueue[to] = true;
      }
    }
  }

end:
  free(depth);
  free(isInQueue);
  free(distance);
  cgraphQueueRelease(queue);
  releaseCSR(&csr);
  return success;
}