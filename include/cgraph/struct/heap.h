#ifndef CGRAPH_HEAP_H
#define CGRAPH_HEAP_H

#include "cgraph/graph.h"

typedef struct {
  CGraphSize capacity, size;
  const WeightType *weights;
  CGraphId elems[];
} CGraphHeap;

CGraphHeap *cgraph_new_heap(CGraphSize capacity, const WeightType *weights);
void cgraph_delete_heap(CGraphHeap *heap);
void cgraph_heap_push(CGraphHeap *heap, CGraphId id);
CGraphId cgraph_heap_pop(CGraphHeap *heap);
void cgraph_heap_build(CGraphHeap *heap);

static inline CGraphBool cgraph_heap_empty(const CGraphHeap *const heap) {
  return heap->size == 0;
}

static void cgraph_heap_prebuild(CGraphHeap *heap, const CGraphId id) {
  heap->elems[++heap->size] = id;
}

#endif // CGRAPH_HEAP_H
