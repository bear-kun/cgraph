#include "cgraph/struct/heap.h"
#include <stdlib.h>

static void graph_heapify_down(CGraphHeap *heap, CGraphSize father) {
  const CGraphId top = heap->elems[father];
  const WeightType top_value = heap->weights[top];

  for (CGraphSize child; (child = father << 1) <= heap->size; father = child) {
    if (child != heap->size && heap->weights[heap->elems[child + 1]] <
        heap->weights[heap->elems[child]]) {
      ++child;
    }
    if (heap->weights[heap->elems[child]] < top_value) {
      heap->elems[father] = heap->elems[child];
    } else break;
  }
  heap->elems[father] = top;
}

CGraphHeap *cgraph_new_heap(const CGraphSize capacity, const WeightType *weights) {
  CGraphHeap *heap = malloc(sizeof(CGraphHeap) + (capacity + 1) * sizeof(CGraphId));
  heap->capacity = capacity;
  heap->size = 0;
  heap->weights = weights;
  return heap;
}

void cgraph_delete_heap(CGraphHeap *heap) { free(heap); }

void cgraph_heap_push(CGraphHeap *heap, const CGraphId id) {
  const WeightType value = heap->weights[id];
  CGraphSize child = ++heap->size;
  for (CGraphSize father; ((father = child >> 1)) && value < heap->weights[heap->elems[father]];
       child = father) {
    heap->elems[child] = heap->elems[father];
  }
  heap->elems[child] = id;
}

CGraphId cgraph_heap_pop(CGraphHeap *heap) {
  const CGraphId ret = heap->elems[1];
  heap->elems[1] = heap->elems[heap->size--];
  graph_heapify_down(heap, 1);
  return ret;
}

void cgraph_heap_build(CGraphHeap *heap) {
  for (CGraphSize i = heap->size >> 1; i; --i) graph_heapify_down(heap, i);
}