#include "internal/developer.h"
#include "struct/linked_list.h"
#include <stdlib.h>

typedef struct VertexAttribute_ Vertex;
struct VertexAttribute_ {
  CGraphBool visited;
  CGraphId preorder; // dfs中第一次访问节点的序数
  CGraphId lowest;   // 该点所在的所有圈的所有顶点中最小的序数（一个点也视作圈）
  Vertex *pred;
};

typedef struct {
  CGraphIter *iter;
  Vertex *vertices;
  CGraphLinkedNode **arts;
  CGraphId topo;
} Package;

static void findArticulationStep(Package *pkg, const CGraphId from) {
  // 排除根节点，单独处理
  Vertex *vertex = pkg->vertices + from;
  CGraphBool isArt = vertex->pred != NULL;
  CGraphId eid, to;

  vertex->visited = 1;
  vertex->lowest = vertex->preorder = pkg->topo++;
  while (cgraphIterNextEdge(pkg->iter, from, &eid, &to)) {
    Vertex *adjacent = pkg->vertices + to;

    if (!adjacent->visited) {
      adjacent->pred = vertex;
      findArticulationStep(pkg, to);

      // 若target所在的圈不包含vertex,则vertex为割点
      // 使用isArt，只添加一次
      if (adjacent->lowest >= vertex->preorder && !isArt) {
        isArt = 1;
        cgraphLinkedInsert(pkg->arts, from);
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
}

void cgraphFindArticulation(const CGraph *const graph,
                            CGraphLinkedNode **articulations) {
  const CGraphView *view = VIEW(graph);
  Vertex *vertices = calloc(view->vertRange, sizeof(Vertex));

  Package pkg = {cgraphIterFromView(view), vertices, articulations, 0};
  const CGraphId root = pkg.iter->vertCurr;
  findArticulationStep(&pkg, root);

  // 若根节点有两个及以上的子树，则为割点
  CGraphId id, to;
  unsigned children = 0;
  cgraphIterResetEdge(pkg.iter, root);
  while (cgraphIterNextEdge(pkg.iter, root, &id, &to)) {
    if (vertices[to].pred == vertices && ++children == 2) {
      cgraphLinkedInsert(articulations, 0);
      break;
    }
  }
  free(vertices);
  cgraphIterRelease(pkg.iter);
}
