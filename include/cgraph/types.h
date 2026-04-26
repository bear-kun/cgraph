#ifndef GRAPH_VERTEX_EDGE_H
#define GRAPH_VERTEX_EDGE_H

#include <stdbool.h>
#include <stdint.h>
#include <float.h>

// set CGRAPH_EPSILON = 0 if WeightType is Integer
#define INVALID_ID (-1)
#define CGRAPH_INF FLT_MAX
#define CGRAPH_EPSILON 1e-6f

typedef bool CGraphBool;
typedef int32_t CGraphInt;
typedef int32_t CGraphId;
typedef uint32_t CGraphSize;
typedef float WeightType;
typedef WeightType TimeType; // aoa
typedef WeightType FlowType; // flow

typedef void (*CGraphResizeCallback)(CGraphSize oldCap, CGraphSize newCap);

// O(6 * V + 4 * E)
typedef struct {
  struct {
    CGraphSize capacity, count, range;
    CGraphId free, head, *next;
    CGraphInt *degree[2];
    CGraphResizeCallback resize;
  } vert;

  struct {
    CGraphBool directed;
    CGraphSize capacity, count, range;
    CGraphId free, *head[2], *next[2];
    CGraphId *xor, *to;
    CGraphResizeCallback resize;
  } edge;
} CGraph;

typedef struct {
  const CGraph *view;

  struct {
    CGraphId vert;
    CGraphBool *dir;
    CGraphId edge[];
  } curr;
} CGraphExplorer;

typedef struct {
  const CGraph *view;
  CGraphId curr;
  CGraphId vert;
  CGraphBool dir;
  CGraphBool undirected;
} CGraphIter;

#endif // GRAPH_VERTEX_EDGE_H