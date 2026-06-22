#ifndef CGRAPH_STACK_H
#define CGRAPH_STACK_H

#include "cgraph/graph.h"

typedef struct {
  CGraphSize size;
  CGraphId elems[];
} CGraphStack;

CGraphStack *cgraph_new_stack(CGraphSize capacity);
void cgraph_delete_stack(CGraphStack *stack);

static inline void cgraph_stack_push(CGraphStack *const stack, const CGraphId item) {
  stack->elems[stack->size++] = item;
}

static inline CGraphId cgraph_stack_pop(CGraphStack *const stack) {
  return stack->elems[--stack->size];
}

static inline CGraphBool cgraph_stack_empty(const CGraphStack *const stack) {
  return stack->size == 0;
}

#endif // CGRAPH_STACK_H
