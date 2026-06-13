#ifndef CGRAPH_TYPES_H
#define CGRAPH_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <float.h>

// set CGRAPH_EPSILON = 0 if WeightType is Integer
#define CGRAPH_INV_ID (-1)
#define CGRAPH_INF FLT_MAX
#define CGRAPH_EPS 1e-6f
#define CGRAPH_OUT 0
#define CGRAPH_IN 1

typedef bool CGraphBool;
typedef int32_t CGraphInt;
typedef int32_t CGraphId;
typedef uint32_t CGraphSize;
typedef float WeightType;
typedef WeightType TimeType; // aoa
typedef WeightType FlowType; // flow

// O(5 * V + 4 * E)
typedef struct {
  struct {
    CGraphSize capacity, count, range;
    CGraphId *array;
    CGraphInt *degree[2];
  } vert;

  struct {
    CGraphBool directed;
    CGraphSize capacity, count, range;
    CGraphId free, *head[2], *next[2];
    CGraphId *xor_, *to;
  } edge;
} CGraph;

typedef struct {
  const CGraph *view;
  CGraphId vert; // not vid
  CGraphBool dir_global;
  CGraphBool *dir_current;
  CGraphId edge[];
} CGraphExplorer;

typedef struct {
  const CGraph *view;
  CGraphId vert; // not vid if vert-iter
  CGraphId edge;
  CGraphBool dir_current;
  CGraphBool dir_global;
  CGraphBool undirected;
} CGraphIterator;

#endif // CGRAPH_TYPES_H