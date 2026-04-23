#ifndef GRAPH_VERTEX_EDGE_H
#define GRAPH_VERTEX_EDGE_H

#include <stdbool.h>
#include <stdint.h>
#include <float.h>

// set CGRAPH_EPSILON = 0 if WeightType is Integer
#define INVALID_ID (-1)
#define CGRAPH_INF DBL_MAX
#define CGRAPH_EPSILON 1e-12

typedef bool CGraphBool;
typedef int64_t CGraphInt;
typedef int64_t CGraphId;
typedef uint64_t CGraphSize;
typedef double WeightType;
typedef WeightType TimeType; // aoa
typedef WeightType FlowType; // flow

typedef void (*CGraphResizeCallback)(CGraphSize oldCap, CGraphSize newCap);

typedef struct {
  CGraphSize vertCap, vertNum;
  CGraphId vertRange, vertFree;
  CGraphId vertHead, *vertNext;
  CGraphInt *indegree, *outdegree;
  CGraphResizeCallback vertResize;

  CGraphBool directed;
  CGraphSize edgeCap, edgeNum;
  CGraphId edgeRange, edgeFree;
  CGraphId *edgeHead, *edgeNext;
  CGraphId *edgeFrom, *edgeTo;
  CGraphResizeCallback edgeResize;
} CGraph;

typedef struct {
  const CGraph *view;
  CGraphId vertCurr;
  CGraphId edgeCurr[0];
} CGraphIter;

typedef struct {
  const CGraph *view;
  CGraphId curr;
} CGraphIterLite;

#endif // GRAPH_VERTEX_EDGE_H