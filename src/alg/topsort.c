#include "cgraph/alg.h"
#include "cgraph/iter.h"
#include "cgraph/struct/queue.h"
#include <stdlib.h>
#include <string.h>

static void indegreeInitQueue(const CGraph *graph, const CGraphInt indegree[], CGraphQueue *queue) {
  CGraphId vid;
  CGraphIter vertices = cgraphGetVertIter(graph);
  while (cgraphIterNextVert(&vertices, &vid)) {
    if (indegree[vid] == 0) cgraphQueuePush(queue, vid);
  }
}

void cgraphTopoPath(const CGraph *const graph, CGraphId predecessor[]) {
  CGraphQueue *queue = cgraphQueueCreate(graph->vert.count);
  CGraphInt *indegree = malloc(graph->vert.range * sizeof(CGraphInt));
  memcpy(indegree, graph->vert.degree[CGRAPH_IN], graph->vert.range * sizeof(CGraphInt));
  memset(predecessor, INVALID_ID, graph->vert.range * sizeof(CGraphId));
  indegreeInitQueue(graph, indegree, queue);

  CGraphInt counter = 0;
  while (!cgraphQueueEmpty(queue)) {
    const CGraphId from = cgraphQueuePop(queue);
    counter++;

    CGraphId eid, to;
    CGraphIter iter = cgraphGetEdgeIter(graph, from, CGRAPH_OUT);
    while (cgraphIterNextEdge(&iter, &eid, &to)) {
      if (predecessor[to] == -1) predecessor[to] = from;
      if (--indegree[to] == 0) cgraphQueuePush(queue, to);
    }
  }

  if (counter != graph->vert.count) {
    /* ERROR: 圈 */
  }

  free(indegree);
  cgraphQueueRelease(queue);
}

void cgraphTopoSort(const CGraph *const graph, CGraphId sort[]) {
  CGraphQueue *queue = cgraphQueueCreate(graph->vert.count);
  CGraphInt *indegree = malloc(graph->vert.range * sizeof(CGraphInt));
  memcpy(indegree, graph->vert.degree[CGRAPH_IN], graph->vert.range * sizeof(CGraphInt));
  indegreeInitQueue(graph, indegree, queue);

  CGraphInt counter = 0;
  while (!cgraphQueueEmpty(queue)) {
    const CGraphId from = cgraphQueuePop(queue);
    sort[counter++] = from;

    CGraphId eid, to;
    CGraphIter iter = cgraphGetEdgeIter(graph, from, CGRAPH_OUT);
    while (cgraphIterNextEdge(&iter, &eid, &to)) {
      if (--indegree[to] == 0) cgraphQueuePush(queue, to);
    }
  }

  if (counter != graph->vert.count) {
    /* ERROR: 圈 */
  }

  free(indegree);
  cgraphQueueRelease(queue);
}