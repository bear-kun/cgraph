#ifndef CGRAPH_PAIRING_HEAP_H
#define CGRAPH_PAIRING_HEAP_H

#include "cgraph/types.h"

// 视作只有右子树符合约定的搜索二叉树
typedef struct PairingHeapNode_ PairingHeapNode;

struct PairingHeapNode_ {
  WeightType weight;
  PairingHeapNode *left; // sibling
  PairingHeapNode *right; // child
  PairingHeapNode **parent;
};

typedef struct {
  const WeightType *weights;
  PairingHeapNode *root;
  PairingHeapNode **stack;
  PairingHeapNode nodes[0];
} CGraphPairingHeap;

CGraphPairingHeap *cgraph_new_pairing_heap(CGraphSize capacity, const WeightType *weights);
void cgraph_delete_pairing_heap(CGraphPairingHeap *heap);
void cgraph_pairing_heap_push(CGraphPairingHeap *heap, CGraphId id);
CGraphId cgraph_pairing_heap_pop(CGraphPairingHeap *heap);
void cgraph_pairing_heap_update(CGraphPairingHeap *heap, CGraphId id);

static inline CGraphBool cgraph_pairing_heap_empty(const CGraphPairingHeap *heap) {
  return !heap->root;
}

#endif // CGRAPH_PAIRING_HEAP_H