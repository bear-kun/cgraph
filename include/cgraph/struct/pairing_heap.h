#ifndef PAIRING_HEAP_H
#define PAIRING_HEAP_H

#include "../type.h"

// 视作只有右子树符合约定的搜索二叉树
typedef struct PairingHeapNode_ PairingHeapNode;
struct PairingHeapNode_ {
  WeightType weight;
  PairingHeapNode *left;  // sibling
  PairingHeapNode *right; // child
  PairingHeapNode **parent;
};

typedef struct {
  const WeightType *weights;
  PairingHeapNode *root;
  PairingHeapNode **stack;
  PairingHeapNode nodes[0];
} CGraphPairingHeap;

CGraphPairingHeap *cgraphPairingHeapCreate(CGraphSize capacity,
                                         const WeightType *weights);
void cgraphPairingHeapRelease(CGraphPairingHeap *heap);
void cgraphPairingHeapPush(CGraphPairingHeap *heap, CGraphId id);
CGraphId cgraphPairingHeapPop(CGraphPairingHeap *heap);
void cgraphPairingHeapUpdate(CGraphPairingHeap *heap, CGraphId id);

static inline CGraphBool cgraphPairingHeapEmpty(const CGraphPairingHeap *heap) {
  return !heap->root;
}

#endif // PAIRING_HEAP_H
