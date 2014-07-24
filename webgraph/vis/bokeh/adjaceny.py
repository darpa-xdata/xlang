from __future__ import absolute_import, division, print_function

import bokeh.plotting as plt
import numpy as np



def load_random_graph():
    N = 500
    M = 10
    values = np.random.random(N)
    rows = np.empty(N)
    idx = 0
    for i in range(N):
        idx += np.random.randint(0, M)
        rows[i] = idx
    rows = rows.astype(np.int)
    cols = np.random.randint(0, N, rows[-1]+M)
    return rows, cols


def compute_adj(rows, cols):
    N = len(rows)
    adj = np.zeros((N, N))
    for i in range(N-2):
        start, end = rows[i], rows[i+1]
        for j in range(start, end):
            adj[i, cols[j]] = 100
    for j in range(rows[-1], len(cols)):
        adj[N-1, cols[j]] = 100
    return adj


def plot_adj(adj):
    N, _ = adj.shape
    plt.output_file("adjacency.html", title="Adjacency")
    TOOLS="pan,wheel_zoom,box_zoom,reset,click,previewsave"
    plt.figure(plot_width=N+100, plot_height=N+100, tools=TOOLS)
    plt.image(image=[adj], x =[0], y=[0], dw=[N], dh=[N], 
      x_range=[0, N], y_range=[0, N],palette=["YlGnBu-3"])
    plt.show()


def main():
    r, c = load_random_graph()
    adj = compute_adj(r, c)
    plot_adj(adj)

if __name__ == "__main__":
    main()
