#include "cgraph/alg.h"
#include "cgraph/iter.h"
#include "cgraph/struct/queue.h"
#include <stdlib.h>
#include <string.h>

static void indegree_init_queue(const CGraph *graph, const CGraphInt indegree[],
                                CGraphQueue *queue) {
  CGraphId vid;
  CGraphIter vertices = cgraph_get_vertex_iter(graph);
  while (cgraph_iter_next_vertex(&vertices, &vid)) {
    if (indegree[vid] == 0) cgraph_queue_push(queue, vid);
  }
}

void cgraph_topo_path(const CGraph *const graph, CGraphId predecessor[]) {
  CGraphQueue *queue = cgraph_new_queue(graph->vert.count);
  CGraphInt *indegree = malloc(graph->vert.range * sizeof(CGraphInt));
  memcpy(indegree, graph->vert.degree[CGRAPH_IN], graph->vert.range * sizeof(CGraphInt));
  memset(predecessor, INVALID_ID, graph->vert.range * sizeof(CGraphId));
  indegree_init_queue(graph, indegree, queue);

  CGraphInt counter = 0;
  while (!cgraph_queue_empty(queue)) {
    const CGraphId from = cgraph_queue_pop(queue);
    counter++;

    CGraphId eid, to;
    CGraphIter iter = cgraph_get_edge_iter(graph, from, CGRAPH_OUT);
    while (cgraph_iter_next_edge(&iter, &eid, &to)) {
      if (predecessor[to] == -1) predecessor[to] = from;
      if (--indegree[to] == 0) cgraph_queue_push(queue, to);
    }
  }

  if (counter != graph->vert.count) {
    /* ERROR: 圈 */
  }

  free(indegree);
  cgraph_delete_queue(queue);
}

void cgraph_topo_sort(const CGraph *const graph, CGraphId sort[]) {
  CGraphQueue *queue = cgraph_new_queue(graph->vert.count);
  CGraphInt *indegree = malloc(graph->vert.range * sizeof(CGraphInt));
  memcpy(indegree, graph->vert.degree[CGRAPH_IN], graph->vert.range * sizeof(CGraphInt));
  indegree_init_queue(graph, indegree, queue);

  CGraphInt counter = 0;
  while (!cgraph_queue_empty(queue)) {
    const CGraphId from = cgraph_queue_pop(queue);
    sort[counter++] = from;

    CGraphId eid, to;
    CGraphIter iter = cgraph_get_edge_iter(graph, from, CGRAPH_OUT);
    while (cgraph_iter_next_edge(&iter, &eid, &to)) {
      if (--indegree[to] == 0) cgraph_queue_push(queue, to);
    }
  }

  if (counter != graph->vert.count) {
    /* ERROR: 圈 */
  }

  free(indegree);
  cgraph_delete_queue(queue);
}