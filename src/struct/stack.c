#include "struct/stack.h"
#include <stdlib.h>

CGraphStack *cgraphStackCreate(const CGraphSize capacity) {
  CGraphStack *stack =
      malloc(sizeof(CGraphStack) + capacity * sizeof(CGraphId));
  stack->size = 0;
  return stack;
}

void cgraphStackRelease(CGraphStack *const stack) { free(stack); }