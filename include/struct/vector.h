#ifndef GRAPH_LINKED_PATH_H
#define GRAPH_LINKED_PATH_H

#include "cgraph/types.h"

typedef struct {
  CGraphSize capacity, size;
  CGraphId *elems;
} CGraphVector;

#define cgraphVectorCreate() (CGraphVector){0}

void cgraphVectorRelease(const CGraphVector *vector);
void cgraphVectorPush(CGraphVector *vector, CGraphId item);

static CGraphId *cgraphVectorGetData(const CGraphVector *vector) {
  return vector->elems;
}

#endif // GRAPH_LINKED_PATH_H