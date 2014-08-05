from __future__ import absolute_import, division, print_function

import ctypes


class GraphT(ctypes.Structure):
    _fields_ = [('numNodes', ctypes.c_int),
                ('numEdges', ctypes.c_int),
                ('nodeNames', ctypes.POINTER(ctypes.c_char_p)),
                ('edgeValues', ctypes.POINTER(ctypes.c_double)),
                ('nodeValues', ctypes.POINTER(ctypes.c_double)),
                ('rowOffsets', ctypes.POINTER(ctypes.c_int)),
                ('colIndices', ctypes.POINTER(ctypes.c_int))]  

def print_graph(out_graph_addr, in_graph_addr):
    in_graph = GraphT.from_address(in_graph_addr)
    print(in_graph.numNodes)
#    import pdb; pdb.set_trace()
    for i in xrange(in_graph.numNodes):
        print(in_graph.nodeNames[i])
