#ifndef GRAPH_VERTEX_EDGE_H
#define GRAPH_VERTEX_EDGE_H

#include <stdbool.h>
#include <stdint.h>

#if CGRAPH_INT_BITS == 32
typedef int32_t CGraphInt, CGraphId;
typedef uint32_t CGraphSize;
#else
typedef int64_t CGraphInt, CGraphId;
typedef uint64_t CGraphSize;
#endif

typedef bool CGraphBool;
typedef int64_t WeightType;
typedef WeightType TimeType; // aoa
typedef WeightType FlowType; // flow

typedef struct {
  CGraphId to, from;
} CGraphEndpoint;

// 最小图结构
typedef struct {
  CGraphSize vertRange;
  CGraphId vertHead, *vertNext;

  CGraphBool directed;
  CGraphSize edgeRange;
  CGraphId *edgeHead, *edgeNext;
  CGraphEndpoint *endpoints;
} CGraphView;

typedef struct {
  CGraphSize vertCap, edgeCap;
  CGraphSize vertNum, edgeNum;
  CGraphId vertFree, edgeFree;
  CGraphView view;
} CGraph;

typedef struct {
  const CGraphView *view;
  CGraphId vertCurr;
  CGraphId edgeCurr[0];
} CGraphIter;

#endif // GRAPH_VERTEX_EDGE_H
