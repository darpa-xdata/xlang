"""Simple flask app for serving webgraph queries.

DESCRIPTION

This application runs a restful server giving back requests for node neighborhoods.

EXAMPLES



AUTHOR(S)
    Andy R. Terrel <aterrel@continuum.io>
"""
from __future__ import absolute_import, division, print_function

import json
from traceback import print_exc

import numpy as np
from flask import Flask
app = Flask(__name__)

NODES = None
CSR_OFFSETS = None
CSR_INDICES = None
CSC_OFFSETS = None
CSC_INDICES = None


@app.route('/name=<node_name>')
def get_node_neighborhood(node_name):
    """Returns a json rep of the node with a list of in and out edges.
    
    For example:
    >>> get_nod_neighborhood("kitware.com")
    {"id": 534543,
     "name": "kitware.com",
     "in": [4324,654546, ...],
     "out": [53254, ...]},
    """
    try:
        ret_dict = {"id": -1, "name": node_name, "in": [], "out": []}
        print(node_name)
        print(NODES)
        node_idx = NODES.searchsorted(node_name)
        if NODES[node_idx] != node_name:
            return json.dumps(ret_dict)
        else:
            ret_dict['id'] = node_idx
            print(CSR_INDICES)
            print(CSR_OFFSETS)
            out_nodes = CSR_INDICES[CSR_OFFSETS[node_idx]: CSR_OFFSETS[node_idx + 1]]
            in_nodes = CSC_INDICES[CSC_OFFSETS[node_idx]: CSC_OFFSETS[node_idx + 1]]
            ret_dict["in"] = list(in_nodes)
            ret_dict["out"] = list(out_nodes)
    except:
        print_exc()
        import pdb; pdb.set_trace()

    return json.dumps(ret_dict)


def start_app():
    app.run()


def parse_cluster_file(filename):
    pass

def run_test():
    global NODES
    NODES = np.array(['continuum.io', 'darpa.mil', 'kitware.com', 'nasa.gov'])
    global CSR_OFFSETS 
    CSR_OFFSETS = np.array([0, 2, 3, 5, 8])
    global CSR_INDICES 
    CSR_INDICES = np.array([1, 2, 2, 1, 3, 0, 1, 2])
    global CSC_OFFSETS 
    CSC_OFFSETS = np.array([0, 1, 4, 7, 8])
    global CSC_INDICES
    CSC_INDICES = np.array([3, 0, 2, 3, 0, 1, 3, 2])
    start_app()

if __name__ == "__main__":
    run_test()
