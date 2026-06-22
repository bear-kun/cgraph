#include "cgraph/algorithm.h"
#include "cgraph/struct/queue.h"
#include <stdlib.h>
#include <string.h>

static void indegree_init_queue(const CGraph *graph, const CGraphInt indegree[],
                                CGraphQueue *queue) {
  CGraphId vid;
  CGraphIterator vertices = cgraph_get_vertex_iterator(graph);
  while (cgraph_iterator_next_vertex(&vertices, &vid)) {
    if (indegree[vid] == 0) cgraph_queue_push(queue, vid);
  }
}

typedef struct {
  const CGraph *graph;
  CGraphQueue *queue;
  CGraphInt *indegree;
  const TimeType *duration;
  TimeType *early_start, *late_start;
  CGraphId *successor;
} Package;

static void forward(const Package *pkg) {
  while (!cgraph_queue_empty(pkg->queue)) {
    const CGraphId from = cgraph_queue_pop(pkg->queue);

    CGraphId eid, to;
    CGraphIterator iter = cgraph_get_edge_iterator(pkg->graph, from, CGRAPH_OUT);
    while (cgraph_iterator_next_edge(&iter, &eid, &to)) {
      if (pkg->early_start[to] < pkg->early_start[from] + pkg->duration[eid]) {
        pkg->early_start[to] = pkg->early_start[from] + pkg->duration[eid];
      }
      if (--pkg->indegree[to] == 0) cgraph_queue_push(pkg->queue, to);
    }
  }
}

static void backward(const Package *pkg, const CGraphId *begin, const CGraphId *end) {
  const CGraphId *p = end;
  do {
    const CGraphId from = *--p;
    CGraphId eid, to;
    CGraphIterator iter = cgraph_get_edge_iterator(pkg->graph, from, CGRAPH_OUT);
    while (cgraph_iterator_next_edge(&iter, &eid, &to)) {
      if (pkg->late_start[from] > pkg->late_start[to] - pkg->duration[eid]) {
        pkg->late_start[from] = pkg->late_start[to] - pkg->duration[eid];
        if (pkg->late_start[from] == pkg->early_start[from]) {
          pkg->successor[from] = to;
          break;
        }
      }
    }
  } while (p != begin);
}

static void init(Package *pkg, const CGraph *graph) {
  const CGraphSize vert_range = graph->vert.range;

  pkg->queue = cgraph_new_queue(vert_range);
  pkg->indegree = malloc(vert_range * sizeof(CGraphInt));
  memcpy(pkg->indegree, graph->vert.degree[CGRAPH_IN], vert_range * sizeof(CGraphInt));
  memset(pkg->early_start, 0, vert_range * sizeof(TimeType));
  memset(pkg->successor, CGRAPH_INV_ID, vert_range * sizeof(CGraphId));
  for (CGraphSize i = 0; i < vert_range; i++) pkg->late_start[i] = CGRAPH_INF;
  indegree_init_queue(graph, pkg->indegree, pkg->queue);
}

void cgraph_critical_path(const CGraph *aoa, const TimeType duration[], CGraphId successor[],
                          TimeType early_start[], TimeType late_start[]) {
  Package pkg;
  pkg.duration = duration;
  pkg.early_start = early_start;
  pkg.late_start = late_start;
  pkg.successor = successor;
  init(&pkg, aoa);

  forward(&pkg);

  const CGraphId *last = pkg.queue->elems + aoa->vert.count - 1;
  late_start[*last] = early_start[*last];

  backward(&pkg, pkg.queue->elems, last);

  free(pkg.indegree);
  cgraph_delete_queue(pkg.queue);
}