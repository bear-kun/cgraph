#ifndef GRAPH_VERTEX_EDGE_H
#define GRAPH_VERTEX_EDGE_H

#include <stdbool.h>
#include <stdint.h>

typedef bool CGraphBool;
typedef int64_t CGraphInt;
typedef int64_t CGraphId;
typedef uint64_t CGraphSize;
typedef int64_t WeightType;
typedef WeightType TimeType; // aoa
typedef WeightType FlowType; // flow

typedef void (*CGraphResizeCallback)(CGraphSize oldCap, CGraphSize newCap);

// 最小图结构
typedef struct {
  CGraphSize vertRange;
  CGraphId vertHead, *vertNext;

  CGraphBool directed;
  CGraphSize edgeRange;
  CGraphId *edgeHead, *edgeNext;
  CGraphId *edgeFrom, *edgeTo;
} CGraphView;

typedef struct {
  CGraphSize vertCap, edgeCap;
  CGraphSize vertNum, edgeNum;
  CGraphId vertFree, edgeFree;
  CGraphResizeCallback vertResize, edgeResize;
  CGraphView view;
} CGraph;

typedef struct {
  const CGraphView *view;
  CGraphId vertCurr;
  CGraphId edgeCurr[0];
} CGraphIter;

#endif // GRAPH_VERTEX_EDGE_H
