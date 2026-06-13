#include "cgraph/struct/stack.h"
#include <stdlib.h>

CGraphStack *cgraph_new_stack(const CGraphSize capacity) {
  CGraphStack *stack = malloc(sizeof(CGraphStack) + capacity * sizeof(CGraphId));
  stack->size = 0;
  return stack;
}

void cgraph_delete_stack(CGraphStack *const stack) { free(stack); }