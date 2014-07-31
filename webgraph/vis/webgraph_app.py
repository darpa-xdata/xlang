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
from flask import Flask, request


app = Flask(__name__)
NODES = None
CSR_OFFSETS = None
CSR_INDICES = None
CSC_OFFSETS = None
CSC_INDICES = None


def neighborhood_from_idx(node_idx):
    out_nodes = CSR_INDICES[CSR_OFFSETS[node_idx]: CSR_OFFSETS[node_idx + 1]]
    in_nodes = CSC_INDICES[CSC_OFFSETS[node_idx]: CSC_OFFSETS[node_idx + 1]]
    return {"id": node_idx, "name": NODES[node_idx], 
            "in": map(int, in_nodes), "out": map(int, out_nodes)}
    

def neighborhood_from_name(node_name):
    node_idx = NODES.searchsorted(node_name)
    if NODES[node_idx] != node_name:
        return json.dumps({})
    else:
        return neighborhood_from_idx(node_idx)


@app.route('/site', methods=['GET'])
def site():
    return json.dumps(neighborhood_from_name(request.args.get("name")))
    

@app.route('/subgraph', methods=['GET'])
def subgraph():
    try:
        ids = request.args.get('ids')
        ids = map(int, ids.split(','))
        print("ids:", ids)
        ret_dict = map(neighborhood_from_idx, ids)
        print("ret_dict:", ret_dict)
        print("json:", json.dumps(ret_dict))
    except:
        print_exc()
        import pdb; pdb.set_trace()
    return json.dumps(ret_dict)


def start_app(debug=False):
    app.run(debug=debug)


def parse_cluster_file(filename):
    pass


def run_app(nodes, csr_offsets, csr_indices, csc_offsets, csc_indices):
    try:
        import sys;
        sys.argv = ["webgraph_app.py"]
        global NODES, CSR_OFFSETS, CSR_INDICES, CSC_OFFSETS, CSC_INDICES 
        NODES = nodes
        CSR_OFFSETS = csr_offsets
        CSR_INDICES = csr_indices
        CSC_OFFSETS = csc_offsets
        CSC_INDICES = csc_indices
        print("Starting app with:")
        print("  nodes =", NODES)
        print("  CSR_OFFSETS =", CSR_OFFSETS)
        print("  CSR_INDICES =", CSR_INDICES)
        print("  CSC_OFFSETS =", CSC_OFFSETS)
        print("  CSC_INDICES =", CSC_INDICES)
        start_app()
    except:
        print_exc()
    return 0


def run_test():
    global NODES, CSR_OFFSETS, CSR_INDICES, CSC_OFFSETS, CSC_INDICES 
    NODES = np.array(['continuum.io', 'darpa.mil', 'kitware.com', 'nasa.gov'])
    CSR_OFFSETS = np.array([0, 2, 3, 5, 8])
    CSR_INDICES = np.array([1, 2, 2, 1, 3, 0, 1, 2])
    CSC_OFFSETS = np.array([0, 1, 4, 7, 8])
    CSC_INDICES = np.array([3, 0, 2, 3, 0, 1, 3, 2])
    start_app(debug=True)


if __name__ == "__main__":
    run_test()
