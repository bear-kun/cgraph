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
  struct {
    CGraphSize capacity, count;
    CGraphId range, free;
    CGraphId head, *next;
    CGraphInt *indegree, *outdegree;
    CGraphResizeCallback resize;
  } vert;

  struct {
    CGraphBool directed;
    CGraphSize capacity, count;
    CGraphId range, free;
    CGraphId *head, *next;
    CGraphId *from, *to;
    CGraphResizeCallback resize;
  } edge;
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