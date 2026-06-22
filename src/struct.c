#include "cgraph/struct/vector.h"
#include "cgraph/struct/queue.h"
#include "cgraph/struct/stack.h"
#include "cgraph/struct/disjoint_set.h"
#include "cgraph/struct/heap.h"
#include "cgraph/struct/pairing_heap.h"
#include <stdlib.h>
#include <string.h>

// ================ vector ================

void cgraph_delete_vector(const CGraphVector *vector) {
  free(vector->elems);
}

void cgraph_vector_push(CGraphVector *vector, const CGraphId item) {
  if (vector->size == vector->capacity) {
    vector->capacity = (vector->capacity + 4) * 2;
    void *mem = realloc(vector->elems, vector->capacity * sizeof(CGraphId));
    if (!mem) abort();
    vector->elems = mem;
  }
  vector->elems[vector->size++] = item;
}

// ================ queue ================

CGraphQueue *cgraph_new_queue(const CGraphSize capacity) {
  CGraphQueue *queue = malloc(sizeof(CGraphQueue) + capacity * sizeof(CGraphId));
  queue->capacity = capacity;
  queue->size = queue->front = queue->rear = 0;
  return queue;
}

void cgraph_delete_queue(CGraphQueue *queue) { free(queue); }

// ================ stack ================

CGraphStack *cgraph_new_stack(const CGraphSize capacity) {
  CGraphStack *stack = malloc(sizeof(CGraphStack) + capacity * sizeof(CGraphId));
  stack->size = 0;
  return stack;
}

void cgraph_delete_stack(CGraphStack *const stack) { free(stack); }

// ================ disjoint set ================

CGraphDisjointSet *cgraph_new_disjoint(const CGraphSize size) {
  CGraphDisjointSet *set = malloc(size * sizeof(CGraphId));
  memset(set, -1, size * sizeof(CGraphId));
  return set;
}

void cgraph_delete_disjoint(CGraphDisjointSet *set) { free(set); }

// ================ heap ================

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

// ================ pairing heap ================

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