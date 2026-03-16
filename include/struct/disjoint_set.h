#ifndef DISJOINT_SET_H
#define DISJOINT_SET_H

#include "cgraph/types.h"

typedef CGraphId CGraphDisjointSet;

CGraphDisjointSet *cgraphDisjointCreate(CGraphSize size);
void cgraphDisjointRelease(CGraphDisjointSet *set);

static void cgraphDisjointUnion(CGraphDisjointSet *set, const CGraphId class1,
                               const CGraphId class2) {
  CGraphId *neg_height = set;
  if (neg_height[class1] > neg_height[class2]) {
    set[class1] = class2;
  } else {
    if (neg_height[class1] == neg_height[class2]) --neg_height[class1];
    set[class2] = class1;
  }
}

static CGraphId cgraphDisjointFind(CGraphDisjointSet *set, const CGraphId id) {
  CGraphId cls;
  for (cls = id; set[cls] >= 0; cls = set[cls]);
  for (CGraphId i = id; i != cls;) {
    const CGraphId next = set[i];
    set[i] = cls;
    i = next;
  }
  return cls;
}

#endif // DISJOINT_SET_H
