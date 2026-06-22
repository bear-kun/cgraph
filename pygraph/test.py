import random
import time

import networkx as nx

import pygraph


def random_graph(directed, vert_count, edge_count):
    edges = [(random.randint(0, vert_count - 1), random.randint(0, vert_count - 1)) for _ in range(edge_count)]

    start = time.time()
    pg = pygraph.Graph(directed)
    pg.add_vertices(vert_count)
    pg.add_edges(edges)
    end1 = time.time() - start

    start = time.time()
    ng = nx.MultiDiGraph() if directed else nx.MultiGraph()
    ng.add_nodes_from(range(vert_count))
    ng.add_edges_from(edges)
    end2 = time.time() - start

    print(f'Random Graph: {end1} | {end2}')
    return pg, ng


def random_path(directed, source, target, vert_count, edge_count):
    path = [random.randint(0, vert_count - 1) for _ in range(edge_count - 1)]

    pg = pygraph.Graph(directed)
    pg.add_vertices(vert_count)

    ng = nx.MultiDiGraph() if directed else nx.MultiGraph()
    ng.add_nodes_from(path + [0])

    for p in path:
        pg.add_edge(source, p)
        ng.add_edge(source, p)
        source = p
    pg.add_edge(source, target)
    ng.add_edge(source, target)
    return pg, ng


def eulerian_path(vert_count, edge_count):
    pg, ng = random_path(False, 0, 1, vert_count, edge_count)

    start = time.time()
    path1 = pg.eulerian_path(0)
    end1 = time.time() - start

    start = time.time()
    path2 = tuple(nx.eulerian_path(ng, 0))
    end2 = time.time() - start

    print(f'Eulerian Path: {end1} | {end2}')


def strongly_connected(vert_count, edge_count):
    pg, ng = random_graph(True, vert_count, edge_count)

    start = time.time()
    ssc1 = pg.strongly_connected()
    end1 = time.time() - start

    start = time.time()
    ssc2 = tuple(nx.strongly_connected_components(ng))
    end2 = time.time() - start

    print(f'Strongly Connected Components: {end1} | {end2}')


def articulations(vert_count, edge_count):
    pg, ng = random_graph(False, vert_count, edge_count)

    start = time.time()
    art1 = pg.articulations()
    end1 = time.time() - start

    start = time.time()
    art2 = tuple(nx.articulation_points(ng))
    end2 = time.time() - start

    if len(art1) != len(art2):
        print(f'Articulations error!')
    else:
        for p in art2:
            if p not in art1:
                print(f'Articulations error!')

    print(f'Articulations: {end1} | {end2}')


def max_flow(vert_count, edge_count):
    edges = list(set((random.randint(0, vert_count - 1), random.randint(0, vert_count - 1)) for _ in range(edge_count)))
    edge_count = len(edges)
    capacity = [random.uniform(0, 1000000) for _ in range(edge_count)]

    pg = pygraph.Graph(True)
    pg.add_vertices(vert_count)
    pg.add_edges(edges)

    ng = nx.DiGraph()
    ng.add_nodes_from(range(vert_count))
    for (u, v), w in zip(edges, capacity):
        ng.add_edge(u, v, capacity=w)

    source = edges[0][0]
    sink = edges[-1][1]

    start = time.time()
    max1, flow1 = pg.max_flow(capacity, source, sink)
    end1 = time.time() - start

    start = time.time()
    max2, flow2 = nx.maximum_flow(ng, source, sink)
    end2 = time.time() - start

    if abs(max1 - max2) / max2 > 1e-6:
        print(f'Maximum Flow Error! {max1} | {max2}')

    print(f'Max Flow: {end1} | {end2}')


def shortest_path(vert_count, edge_count):
    path = [random.randint(0, vert_count - 1) for _ in range(edge_count - 1)]
    weights1 = [random.uniform(0, 10000) for _ in range(edge_count)]
    weights2 = [random.uniform(-10, 10000) for _ in range(edge_count)]

    pg = pygraph.Graph(True)
    pg.add_vertices(vert_count)

    ng = nx.MultiDiGraph()
    ng.add_nodes_from(path + [0])

    src = 0
    for i, p in enumerate(path):
        pg.add_edge(src, p)
        ng.add_edge(src, p, weight1=weights1[i], weight2=weights2[i])
        src = p
    pg.add_edge(src, 1)
    ng.add_edge(src, 1, weight1=weights1[-1], weight2=weights2[-1])

    start = time.time()
    path1 = pg.shortest_path(0, weights=weights1, method='dijkstra')
    end1 = time.time() - start

    start = time.time()
    path2 = nx.shortest_path(ng, 0, weight='weight1', method='dijkstra')
    end2 = time.time() - start

    print(f'Dijkstra: {end1} | {end2}')

    start = time.time()
    path1 = pg.shortest_path(0, weights=weights2, method='bellman-ford')
    end1 = time.time() - start

    try:
        start = time.time()
        path2 = nx.shortest_path(ng, 0, weight='weight2', method='bellman-ford')
    except nx.exception.NetworkXUnbounded:
        path2 = None
    end2 = time.time() - start

    print(f'Bellman-Ford: {end1} | {end2}')


def main():
    eulerian_path(10000, 100000)
    strongly_connected(10000, 1000000)
    articulations(10000, 1000000)
    max_flow(10000, 100000)
    shortest_path(10000, 100000)
    return


if __name__ == '__main__':
    main()
