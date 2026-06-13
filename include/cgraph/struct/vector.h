#ifndef CGRAPH_LINKED_PATH_H
#define CGRAPH_LINKED_PATH_H

#include "cgraph/types.h"

typedef struct {
  CGraphSize capacity, size;
  CGraphId *elems;
} CGraphVector;

#define cgraph_new_vector() (CGraphVector){0}

void cgraph_delete_vector(const CGraphVector *vector);
void cgraph_vector_push(CGraphVector *vector, CGraphId item);

#endif // CGRAPH_LINKED_PATH_H