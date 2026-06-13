#include "cgraph/struct/disjoint_set.h"
#include <stdlib.h>
#include <string.h>

CGraphDisjointSet *cgraph_new_disjoint(const CGraphSize size) {
  CGraphDisjointSet *set = malloc(size * sizeof(CGraphId));
  memset(set, -1, size * sizeof(CGraphId));
  return set;
}

void cgraph_delete_disjoint(CGraphDisjointSet *set) { free(set); }