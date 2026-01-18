#ifndef HEAP_H
#define HEAP_H

#include "../type.h"

typedef struct {
  CGraphSize capacity, size;
  const WeightType *weights;
  CGraphId elems[];
} CGraphHeap;

CGraphHeap *cgraphHeapCreate(CGraphSize capacity, const WeightType *weights);
void cgraphHeapPush(CGraphHeap *heap, CGraphId id);
CGraphId cgraphHeapPop(CGraphHeap *heap);
void cgraphHeapRelease(CGraphHeap *heap);
void cgraphHeapBuild(CGraphHeap *heap);

static inline CGraphBool cgraphHeapEmpty(const CGraphHeap *const heap) {
  return heap->size == 0;
}

static void cgraphHeapPreBuild(CGraphHeap *heap, const CGraphId id) {
  heap->elems[++heap->size] = id;
}

#endif // HEAP_H
