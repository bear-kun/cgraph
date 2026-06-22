#include "cgraph/algorithm.h"
#include "cgraph/struct/vector.h"
#include <stdlib.h>

typedef struct Vertex_ Vertex;

struct Vertex_ {
  CGraphBool visited;
  CGraphId preorder; // dfs中第一次访问节点的序数
  CGraphId lowest; // 该点所在的所有圈的所有顶点中最小的序数（一个点也视作圈）
  Vertex *pred;
};

typedef struct {
  CGraphExplorer *explorer;
  Vertex *vertices;
  CGraphId topo;
  CGraphVector arts;
} Package;

static void recursion(Package *pkg, const CGraphId from) {
  Vertex *vertex = pkg->vertices + from;
  vertex->visited = true;
  vertex->lowest = vertex->preorder = pkg->topo++;

  CGraphBool is_art = false;
  CGraphSize child_count = 0;

  CGraphId eid, to;
  while (cgraph_explorer_next_edge(pkg->explorer, from, &eid, &to)) {
    Vertex *adjacent = pkg->vertices + to;

    if (!adjacent->visited) {
      child_count++;
      adjacent->pred = vertex;
      recursion(pkg, to);

      // 仅非根节点有效
      // 若target所在的圈不包含vertex,则vertex为割点
      // 使用isArt，只添加一次
      if (vertex->pred != NULL && adjacent->lowest >= vertex->preorder && !is_art) {
        is_art = true;
        cgraph_vector_push(&pkg->arts, from);
      }

      // 递归更新lowest
      if (adjacent->lowest < vertex->lowest) vertex->lowest = adjacent->lowest;
    }
    /*
     * 排除反向边；
     * 若出现访问过的点，说明有圈，
     * 因为单向DFS在无圈图中的遍历是拓扑排序的；
     * 更新lowest
     */
    else if (adjacent != vertex->pred && adjacent->preorder < vertex->lowest) {
      vertex->lowest = adjacent->preorder;
    }
  }

  // 根节点根据子树数量判断
  if (vertex->pred == NULL && child_count >= 2) {
    cgraph_vector_push(&pkg->arts, from);
  }
}

CGraphInt cgraph_articulations(const CGraph *graph, CGraphId **articulations) {
  Package pkg = {
      .explorer = cgraph_new_explorer(graph, CGRAPH_OUT),
      .vertices = calloc(graph->vert.range, sizeof(Vertex)),
      .topo = 0,
      .arts = cgraph_new_vector()
  };
  if (*articulations) {
    pkg.arts.capacity = graph->vert.capacity;
    pkg.arts.elems = *articulations;
  }

  CGraphId vid;
  while (cgraph_explorer_next_vertex(pkg.explorer, &vid)) {
    if (!pkg.vertices[vid].visited) recursion(&pkg, vid);
  }

  free(pkg.vertices);
  cgraph_delete_explorer(pkg.explorer);

  *articulations = pkg.arts.elems;
  return (CGraphInt)pkg.arts.size;
}