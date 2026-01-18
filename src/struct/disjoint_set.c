#include "struct/disjoint_set.h"
#include <stdlib.h>
#include <string.h>

CGraphDisjointSet *cgraphDisjointCreate(const CGraphSize size) {
  CGraphDisjointSet *set = malloc(size * sizeof(CGraphId));
  memset(set, -1, size * sizeof(CGraphId));
  return set;
}

void cgraphDisjointRelease(CGraphDisjointSet *set) { free(set); }