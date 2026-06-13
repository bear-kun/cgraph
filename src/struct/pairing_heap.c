#include "cgraph/struct/pairing_heap.h"
#include <stdlib.h>

CGraphPairingHeap *cgraph_new_pairing_heap(const CGraphSize capacity, const WeightType *weights) {
  CGraphPairingHeap *heap =
      malloc(sizeof(CGraphPairingHeap)
             + capacity * sizeof(PairingHeapNode) // nodes
             + (capacity - 1) * sizeof(PairingHeapNode *)); // stack
  heap->weights = weights;
  heap->root = NULL;
  heap->stack = (PairingHeapNode **)(heap->nodes + capacity);
  return heap;
}

void cgraph_delete_pairing_heap(CGraphPairingHeap *heap) { free(heap); }

static PairingHeapNode *meld(PairingHeapNode *left, PairingHeapNode *right) {
  if (!left) return right;
  if (!right) return left;

  if (left->weight < right->weight) {
    right->left = left->right;
    if (right->left) right->left->parent = &right->left;
    left->right = right;
    right->parent = &left->right;
    left->left = NULL;
    return left;
  }
  left->left = right->right;
  if (left->left) left->left->parent = &left->left;
  right->right = left;
  left->parent = &right->right;
  right->left = NULL;
  return right;
}

static PairingHeapNode *combine(PairingHeapNode *parent, PairingHeapNode **stack) {
  if (!parent || !parent->left) return parent;

  PairingHeapNode **top = stack - 1;
  do {
    PairingHeapNode *x = parent;
    PairingHeapNode *y = parent->left;
    parent = y ? y->left : NULL;
    *++top = meld(y, x);
  } while (parent);

  for (PairingHeapNode **pred; top != stack; top = pred) {
    pred = top - 1;
    *pred = meld(*top, *pred);
  }

  return *stack;
}

void cgraph_pairing_heap_push(CGraphPairingHeap *heap, const CGraphId id) {
  PairingHeapNode *node = heap->nodes + id;
  node->weight = heap->weights[id];
  node->right = NULL;
  if (!heap->root) {
    node->left = NULL;
    heap->root = node;
    return;
  }
  heap->root = meld(heap->root, node);
}

CGraphId cgraph_pairing_heap_pop(CGraphPairingHeap *heap) {
  const CGraphId root = (CGraphId)(heap->root - heap->nodes);
  heap->root = combine(heap->root->right, heap->stack);
  return root;
}

void cgraph_pairing_heap_update(CGraphPairingHeap *heap, const CGraphId id) {
  PairingHeapNode *node = heap->nodes + id;
  node->weight = heap->weights[id];

  if (node == heap->root) return;

  *node->parent = node->left;
  if (node->left) node->left->parent = node->parent;
  heap->root = meld(heap->root, node);
}