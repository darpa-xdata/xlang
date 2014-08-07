from __future__ import absolute_import, division, print_function

import ctypes
from traceback import print_exc
import numpy

class GraphT(ctypes.Structure):
    _fields_ = [('numNodes', ctypes.c_int),
                ('numEdges', ctypes.c_int),
                ('nodeNames', ctypes.POINTER(ctypes.c_char_p)),
                ('edgeValues', ctypes.POINTER(ctypes.c_double)),
                ('nodeValues', ctypes.POINTER(ctypes.c_double)),
                ('rowOffsets', ctypes.POINTER(ctypes.c_int)),
                ('colIndices', ctypes.POINTER(ctypes.c_int))]  

def print_graph(out_graph_addr, in_graph_addr):
    try:
        in_graph = GraphT.from_address(in_graph_addr)
        print(in_graph.numNodes)
        p = ctypes.cast(in_graph.nodeNames, ctypes.POINTER(ctypes.c_size_t))
        nodes = numpy.ctypeslib.as_array(p, shape=(in_graph.numNodes,))
        print(nodes, nodes.dtype)
        c = nodes.ctypes.data_as(ctypes.POINTER(ctypes.c_char_p))
        print(c[0], c[1])
    except:
        print_exc()
        raise
