#include "cgraph/struct/vector.h"
#include <stdlib.h>

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