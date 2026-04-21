#include "cgraph/alg.h"
#include "cgraph/iter.h"
#include "struct/queue.h"
#include <stdlib.h>
#include <string.h>

static void indegreeInitQueue(const CGraph *graph, const CGraphInt indegree[],
                              CGraphQueue *queue) {
  CGraphId vid;
  CGraphIterLite vertices = cgraphGetVertIter(graph);
  while (cgraphIterLiteNextVert(&vertices, &vid)) {
    if (indegree[vid] == 0) cgraphQueuePush(queue, vid);
  }
}

void cgraphTopoPath(const CGraph *const graph, CGraphId predecessor[]) {
  CGraphQueue *queue = cgraphQueueCreate(graph->vertNum);
  CGraphInt *indegree = malloc(graph->vertRange * sizeof(CGraphInt));
  memcpy(indegree, graph->indegree, graph->vertRange * sizeof(CGraphInt));
  memset(predecessor, INVALID_ID, graph->vertRange * sizeof(CGraphId));
  indegreeInitQueue(graph, indegree, queue);

  CGraphInt counter = 0;
  while (!cgraphQueueEmpty(queue)) {
    const CGraphId from = cgraphQueuePop(queue);
    counter++;

    CGraphId eid, to;
    CGraphIterLite iter = cgraphGetEdgeIter(graph, from);
    while (cgraphIterLiteNextEdge(&iter, &eid, &to)) {
      if (predecessor[to] == -1) predecessor[to] = from;
      if (--indegree[to] == 0) cgraphQueuePush(queue, to);
    }
  }

  if (counter != graph->vertNum) {
    /* ERROR: 圈 */
  }

  free(indegree);
  cgraphQueueRelease(queue);
}

void cgraphTopoSort(const CGraph *const graph, CGraphId sort[]) {
  CGraphQueue *queue = cgraphQueueCreate(graph->vertNum);
  CGraphInt *indegree = malloc(graph->vertRange * sizeof(CGraphInt));
  memcpy(indegree, graph->indegree, graph->vertRange * sizeof(CGraphInt));
  indegreeInitQueue(graph, indegree, queue);

  CGraphInt counter = 0;
  while (!cgraphQueueEmpty(queue)) {
    const CGraphId from = cgraphQueuePop(queue);
    sort[counter++] = from;

    CGraphId eid, to;
    CGraphIterLite iter = cgraphGetEdgeIter(graph, from);
    while (cgraphIterLiteNextEdge(&iter, &eid, &to)) {
      if (--indegree[to] == 0) cgraphQueuePush(queue, to);
    }
  }

  if (counter != graph->vertNum) {
    /* ERROR: 圈 */
  }

  free(indegree);
  cgraphQueueRelease(queue);
}