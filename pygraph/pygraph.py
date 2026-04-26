import ctypes
import numpy as np

cgraph = ctypes.CDLL('./cgraph.dll')

c_void = None
c_ref = ctypes.byref
c_ptr = ctypes.POINTER
c_bool_t = ctypes.c_bool
c_int_t = ctypes.c_int32
c_id_t = ctypes.c_int32
c_size_t = ctypes.c_uint32
c_resize_cb_t = ctypes.CFUNCTYPE(c_void, c_size_t, c_size_t)
c_weight_t = ctypes.c_float


class CGraph(ctypes.Structure):
    class _Vertices(ctypes.Structure):
        _fields_ = [
            ('capacity', c_size_t), ('count', c_size_t), ('range', c_size_t),
            ('free', c_id_t), ('head', c_id_t), ('next', c_ptr(c_id_t)),
            ('degree', c_ptr(c_int_t) * 2),
            ('resize', c_resize_cb_t)
        ]

    class _Edges(ctypes.Structure):
        _fields_ = [
            ('directed', c_bool_t),
            ('capacity', c_size_t), ('count', c_size_t), ('range', c_size_t),
            ('free', c_id_t), ('head', c_ptr(c_id_t) * 2), ('next', c_ptr(c_id_t) * 2),
            ('xor', c_ptr(c_id_t)), ('to', c_ptr(c_id_t)),
            ('resize', c_resize_cb_t)
        ]

    _fields_ = [
        ('vert', _Vertices), ('edge', _Edges)
    ]


class CGraphIter(ctypes.Structure):
    _fields_ = [
        ('view', c_ptr(CGraph)),
        ('curr', c_id_t),
        ('vert', c_id_t),
        ('dir', c_bool_t),
        ('undirected', c_bool_t)
    ]


c_graph_ptr = c_ptr(CGraph)

# Core
cgraph_init = cgraph.cgraphInit
cgraph_init.argtypes = (c_graph_ptr, c_bool_t, c_size_t, c_size_t)
cgraph_release = cgraph.cgraphRelease
cgraph_release.argtypes = (c_graph_ptr,)
cgraph_copy = cgraph.cgraphCopy
cgraph_copy.argtypes = (c_graph_ptr, c_graph_ptr)
cgraph_clear = cgraph.cgraphClear
cgraph_clear.argtypes = (c_graph_ptr,)

cgraph_add_vert = cgraph.cgraphAddVert
cgraph_add_vert.restype = c_id_t
cgraph_add_vert.argtypes = (c_graph_ptr,)
cgraph_add_vertices = cgraph.cgraphAddVertices
cgraph_add_vertices.argtypes = (c_graph_ptr, c_size_t)
cgraph_delete_vert = cgraph.cgraphDeleteVert
cgraph_delete_vert.argtypes = (c_graph_ptr, c_id_t)

cgraph_add_edge = cgraph.cgraphAddEdge
cgraph_add_edge.restype = c_id_t
cgraph_add_edge.argtypes = (c_graph_ptr, c_id_t, c_id_t)
cgraph_add_edges = cgraph.cgraphAddEdges
cgraph_add_edges.argtypes = (c_graph_ptr, c_size_t, c_ptr(c_id_t * 2))
cgraph_delete_edge = cgraph.cgraphDeleteEdge
cgraph_delete_edge.argtypes = (c_graph_ptr, c_id_t)
cgraph_reverse_edge = cgraph.cgraphReverseEdge
cgraph_reverse_edge.argtypes = (c_graph_ptr, c_id_t)
cgraph_find_edge = cgraph.cgraphFindEdge
cgraph_find_edge.restype = c_id_t
cgraph_find_edge.argtypes = (c_graph_ptr, c_id_t, c_id_t)
cgraph_where_edge_from_to = cgraph.cgraphWhereEdgeFromTo
cgraph_where_edge_from_to.argtypes = (c_graph_ptr, c_id_t, c_ptr(c_id_t), c_ptr(c_id_t))

# Iterator
cgraph_get_vert_iter = cgraph.cgraphGetVertIter
cgraph_get_vert_iter.restype = CGraphIter
cgraph_get_vert_iter.argtypes = (c_graph_ptr,)
cgraph_get_edge_iter = cgraph.cgraphGetEdgeIter
cgraph_get_edge_iter.restype = CGraphIter
cgraph_get_edge_iter.argtypes = (c_graph_ptr, c_id_t)
cgraph_get_edge_iter_rev = cgraph.cgraphGetEdgeIterRev
cgraph_get_edge_iter_rev.restype = CGraphIter
cgraph_get_edge_iter_rev.argtypes = (c_graph_ptr, c_id_t)

cgraph_iter_next_vert = cgraph.cgraphIterNextVert
cgraph_iter_next_vert.restype = c_bool_t
cgraph_iter_next_vert.argtypes = (c_ptr(CGraphIter), c_ptr(c_id_t))
cgraph_iter_next_edge = cgraph.cgraphIterNextEdge
cgraph_iter_next_edge.restype = c_bool_t
cgraph_iter_next_edge.argtypes = (c_ptr(CGraphIter), c_ptr(c_id_t), c_ptr(c_id_t))
cgraph_iter_next_edge_rev = cgraph.cgraphIterNextEdgeRev
cgraph_iter_next_edge_rev.restype = c_bool_t
cgraph_iter_next_edge_rev.argtypes = (c_graph_ptr, c_ptr(c_id_t), c_ptr(c_id_t))

# Algorithm
cgraph_eulerian_path = cgraph.cgraphEulerianPath
cgraph_eulerian_path.restype = c_bool_t
cgraph_eulerian_path.argtypes = (c_graph_ptr, c_ptr(c_id_t), c_id_t, c_id_t)
cgraph_articulations = cgraph.cgraphArticulations
cgraph_articulations.restype = c_int_t
cgraph_articulations.argtypes = (c_graph_ptr, c_ptr(c_ptr(c_id_t)))
cgraph_strongly_connected = cgraph.cgraphStronglyConnected
cgraph_strongly_connected.argtypes = (c_graph_ptr, c_ptr(c_id_t))
cgraph_max_flow_edmonds_karp = cgraph.cgraphMaxFlowEdmondsKarp
cgraph_max_flow_edmonds_karp.restype = c_weight_t
cgraph_max_flow_edmonds_karp.argtypes = (c_graph_ptr, c_ptr(c_weight_t), c_ptr(c_weight_t), c_id_t, c_id_t)
cgraph_spanning_tree_kruskal = cgraph.cgraphSpanningTreeKruskal
cgraph_spanning_tree_kruskal.argtypes = (c_graph_ptr, c_ptr(c_weight_t), c_ptr(c_id_t))
cgraph_topo_sort = cgraph.cgraphTopoSort
cgraph_topo_sort.argtypes = (c_graph_ptr, c_ptr(c_id_t))
cgraph_unweighted_shortest = cgraph.cgraphUnweightedShortest
cgraph_unweighted_shortest.argtypes = (c_graph_ptr, c_ptr(c_id_t), c_id_t, c_id_t)
cgraph_shortest_dijkstra = cgraph.cgraphShortestDijkstra
cgraph_shortest_dijkstra.argtypes = (c_graph_ptr, c_ptr(c_weight_t), c_ptr(c_id_t), c_id_t, c_id_t)
cgraph_shortest_bellman_ford = cgraph.cgraphShortestBellmanFord
cgraph_shortest_bellman_ford.restype = c_bool_t
cgraph_shortest_bellman_ford.argtypes = (c_graph_ptr, c_ptr(c_weight_t), c_ptr(c_id_t), c_id_t)


class GraphEdges:
    def __init__(self, cgraph_ptr, vid):
        self.__cg_ptr = cgraph_ptr
        self.__vid = vid

    def __iter__(self):
        self.__iter = cgraph_get_edge_iter(self.__cg_ptr, self.__vid)
        return self

    def __next__(self):
        eid, to = c_id_t(), c_id_t()
        if cgraph_iter_next_edge(c_ref(self.__iter), c_ref(eid), c_ref(to)):
            return eid.value, to.value
        raise StopIteration


class GraphVertices:
    def __init__(self, cgraph_ptr):
        self.__cg_ptr = cgraph_ptr

    def __iter__(self):
        self.__iter = cgraph_get_vert_iter(self.__cg_ptr)
        return self

    def __next__(self):
        vid = c_id_t()
        if cgraph_iter_next_vert(c_ref(self.__iter), c_ref(vid)):
            return vid.value, GraphEdges(self.__cg_ptr, vid)
        raise StopIteration


_numpy2ctypes = np.ctypeslib.as_ctypes


class Graph:
    def __init__(self, directed=True, vert_capacity=32, edge_capacity=128):
        self.__cgraph = CGraph()
        self.__cg_ptr = c_ref(self.__cgraph)
        cgraph_init(self.__cg_ptr, directed, vert_capacity, edge_capacity)

    def release(self):
        if self.__cg_ptr is not None:
            cgraph_release(self.__cg_ptr)
            self.__cg_ptr = None

    def __del__(self):
        self.release()

    @property
    def vert_count(self):
        return self.__cgraph.vert.count

    @property
    def edge_count(self):
        return self.__cgraph.edge.count

    def copy(self):
        graph = Graph()
        cgraph_copy(self.__cg_ptr, c_ref(graph))
        return graph

    def clear(self):
        cgraph_clear(self.__cg_ptr)

    def add_vert(self):
        return cgraph_add_vert(self.__cg_ptr)

    def add_vertices(self, count):
        cgraph_add_vertices(self.__cg_ptr, count)

    def add_edge(self, from_vid, to_vid):
        return cgraph_add_edge(self.__cg_ptr, from_vid, to_vid)

    def add_edges(self, endpoints):
        endpoints = np.asarray(endpoints, dtype=np.dtype(c_id_t))
        return cgraph_add_edges(self.__cg_ptr, len(endpoints), _numpy2ctypes(endpoints))

    def delete_vert(self, vid):
        cgraph_delete_vert(self.__cg_ptr, vid)

    def delete_edge(self, eid):
        cgraph_delete_edge(self.__cg_ptr, eid)

    def find_edge(self, from_vid, to_vid):
        return cgraph_find_edge(self.__cg_ptr, from_vid, to_vid)

    def where_edge_from_to(self, eid):
        from_vid, to_vid = c_id_t(), c_id_t()
        cgraph_where_edge_from_to(self.__cg_ptr, eid, c_ref(from_vid), c_ref(to_vid))
        return from_vid.value, to_vid.value

    def __iter__(self):
        return iter(GraphVertices(self.__cg_ptr))

    def edges(self, vid):
        return GraphEdges(self.__cg_ptr, vid)

    def eulerian_path(self, source, target=None):
        target = -1 if target is None else target
        path = np.empty(self.__cgraph.edge.count + 1, dtype=np.dtype(c_id_t))
        if not cgraph_eulerian_path(self.__cg_ptr, _numpy2ctypes(path), source, target):
            return None
        return path

    def articulations(self):
        arts = np.empty(self.__cgraph.vert.count, dtype=np.dtype(c_id_t))
        c_ptr2 = ctypes.pointer(ctypes.cast(_numpy2ctypes(arts), c_ptr(c_id_t)))
        count = cgraph_articulations(self.__cg_ptr, c_ptr2)
        arts.resize(count, refcheck=False)
        return arts

    def strongly_connected(self):
        components = np.empty(self.__cgraph.vert.range, dtype=np.dtype(c_id_t))
        cgraph_strongly_connected(self.__cg_ptr, _numpy2ctypes(components))
        return components

    def max_flow(self, capacity, source, sink):
        capacity = np.asarray(capacity, dtype=np.dtype(c_weight_t))
        flow = np.empty(self.__cgraph.edge.count, dtype=np.dtype(c_weight_t))
        max_flow = cgraph_max_flow_edmonds_karp(self.__cg_ptr,
                                                _numpy2ctypes(capacity), _numpy2ctypes(flow), source, sink)
        return max_flow, flow

    def spanning_tree(self, weights):
        weights = np.asarray(weights, dtype=np.dtype(c_weight_t))
        edges = np.empty(self.__cgraph.vert.count - 1, dtype=np.dtype(c_id_t))
        cgraph_spanning_tree_kruskal(self.__cg_ptr, _numpy2ctypes(weights), _numpy2ctypes(edges))
        return edges

    def topo_sort(self):
        sort = np.empty(self.__cgraph.vert.count, dtype=np.dtype(c_id_t))
        cgraph_topo_sort(self.__cg_ptr, _numpy2ctypes(sort))
        return sort

    def shortest_path(self, source, target=None, weights=None, method='dijkstra'):
        target = -1 if target is None else target
        pred = np.empty(self.__cgraph.vert.count, dtype=np.dtype(c_id_t))
        if weights is None:
            cgraph_unweighted_shortest(self.__cg_ptr, _numpy2ctypes(pred), source, target)
        else:
            weights = np.asarray(weights, dtype=np.dtype(c_weight_t))
            if method.lower() == 'dijkstra':
                cgraph_shortest_dijkstra(self.__cg_ptr, _numpy2ctypes(weights), _numpy2ctypes(pred), source, target)
            elif method.lower() == 'bellman-ford':
                if not cgraph_shortest_bellman_ford(self.__cg_ptr, _numpy2ctypes(weights), _numpy2ctypes(pred), source):
                    return None
            else:
                raise NotImplementedError
        return pred
