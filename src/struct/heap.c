#include "struct/heap.h"
#include <stdlib.h>

static void graphHeapifyDown(CGraphHeap *heap, CGraphSize father) {
  const CGraphId top = heap->elems[father];
  const WeightType topValue = heap->weights[top];

  for (CGraphSize child; (child = father << 1) <= heap->size; father = child) {
    if (child != heap->size && heap->weights[heap->elems[child + 1]] <
                                   heap->weights[heap->elems[child]])
      ++child;
    if (heap->weights[heap->elems[child]] < topValue)
      heap->elems[father] = heap->elems[child];
    else
      break;
  }
  heap->elems[father] = top;
}

CGraphHeap *cgraphHeapCreate(const CGraphSize capacity,
                             const WeightType *weights) {
  CGraphHeap *heap =
      malloc(sizeof(CGraphHeap) + (capacity + 1) * sizeof(CGraphId));
  heap->capacity = capacity;
  heap->size = 0;
  heap->weights = weights;
  return heap;
}

void cgraphHeapRelease(CGraphHeap *heap) { free(heap); }

void cgraphHeapPush(CGraphHeap *heap, const CGraphId id) {
  const WeightType value = heap->weights[id];
  CGraphSize child = ++heap->size;
  for (CGraphSize father;
       ((father = child >> 1)) && value < heap->weights[heap->elems[father]];
       child = father) {
    heap->elems[child] = heap->elems[father];
  }
  heap->elems[child] = id;
}

CGraphId cgraphHeapPop(CGraphHeap *heap) {
  const CGraphId ret = heap->elems[1];
  heap->elems[1] = heap->elems[heap->size--];
  graphHeapifyDown(heap, 1);
  return ret;
}

void cgraphHeapBuild(CGraphHeap *heap) {
  for (uint64_t i = heap->size >> 1; i; --i) graphHeapifyDown(heap, i);
}
