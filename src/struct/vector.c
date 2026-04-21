#include "struct/vector.h"
#include <stdlib.h>

void cgraphVectorRelease(const CGraphVector *vector) {
  free(vector->elems);
}

void cgraphVectorPush(CGraphVector *vector, const CGraphId item) {
  if (vector->size == vector->capacity) {
    vector->capacity = (vector->capacity + 4) * 2;
    void *mem = realloc(vector->elems, vector->capacity * sizeof(CGraphId));
    if (!mem) abort();
    vector->elems = mem;
  }
  vector->elems[vector->size++] = item;
}