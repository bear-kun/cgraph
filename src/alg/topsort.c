#include "cgraph/graph.h"
#include "cgraph/iter.h"
#include "struct/queue.h"
#include <stdlib.h>
#include <string.h>

static void callback(CGraphId from, CGraphId eid, CGraphId to, void *userdata) {
  const CGraphInt *indegree = *(void **)userdata;
  CGraphQueue *queue = *((void **)userdata + 1);
  if (indegree[to] == 0) cgraphQueuePush(queue, to);
}

static void indegreeInitQueue(const CGraph *graph,
                              const CGraphInt indegree[], CGraphQueue *queue) {
  void *userData[] = {(void *)indegree, queue};
  cgraphTraverseEdges(graph, userData, callback);
}

void cgraphTopoPath(const CGraph *const graph, const CGraphInt indegree[],
                    CGraphId predecessor[]) {
  CGraphIter *iter = cgraphGetIter(graph);
  CGraphQueue *queue = cgraphQueueCreate(graph->vertNum);
  CGraphInt *copiedIndegree = malloc(graph->vertRange * sizeof(CGraphInt));
  memcpy(copiedIndegree, indegree, graph->vertRange * sizeof(CGraphInt));
  memset(predecessor, INVALID_ID, graph->vertRange * sizeof(CGraphId));
  indegreeInitQueue(graph, indegree, queue);

  CGraphInt counter = 0;
  CGraphId eid, to;
  while (!cgraphQueueEmpty(queue)) {
    const CGraphId from = cgraphQueuePop(queue);
    ++counter;
    while (cgraphIterNextEdge(iter, from, &eid, &to)) {
      if (predecessor[to] == -1) predecessor[to] = from;
      if (--copiedIndegree[to] == 0) cgraphQueuePush(queue, to);
    }
  }

  if (counter != graph->vertNum) {
    /* ERROR: 圈 */
  }

  free(copiedIndegree);
  cgraphIterRelease(iter);
  cgraphQueueRelease(queue);
}

void cgraphTopoSort(const CGraph *const graph, const CGraphInt indegree[],
                    CGraphId sort[]) {
  CGraphIter *iter = cgraphGetIter(graph);
  CGraphQueue *queue = cgraphQueueCreate(graph->vertNum);
  CGraphInt *copiedIndegree = malloc(graph->vertRange * sizeof(CGraphInt));
  memcpy(copiedIndegree, indegree, graph->vertRange * sizeof(CGraphInt));
  indegreeInitQueue(graph, indegree, queue);

  CGraphInt counter = 0;
  CGraphId id, to;
  while (!cgraphQueueEmpty(queue)) {
    const CGraphId from = cgraphQueuePop(queue);
    sort[counter++] = from;

    while (cgraphIterNextEdge(iter, from, &id, &to)) {
      if (--copiedIndegree[to] == 0) cgraphQueuePush(queue, to);
    }
  }

  if (counter != graph->vertNum) {
    /* ERROR: 圈 */
  }

  free(copiedIndegree);
  cgraphIterRelease(iter);
  cgraphQueueRelease(queue);
}