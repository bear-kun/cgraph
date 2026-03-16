#ifndef GRAPH_STACK_H
#define GRAPH_STACK_H

#include "cgraph/types.h"

typedef struct {
  CGraphSize size;
  CGraphId elems[0];
} CGraphStack;

CGraphStack *cgraphStackCreate(CGraphSize capacity);
void cgraphStackRelease(CGraphStack *stack);

static inline void cgraphStackPush(CGraphStack *const stack, const CGraphId item) {
  stack->elems[stack->size++] = item;
}

static inline CGraphId cgraphStackPop(CGraphStack *const stack) {
  return stack->elems[--stack->size];
}

static inline CGraphBool cgraphStackEmpty(const CGraphStack *const stack) {
  return stack->size == 0;
}

#endif // GRAPH_STACK_H
