import sys
import traceback

sys.path.append("../../vis/bokeh")

import hyperlink_plot

def visualize( nodes, values ):
    try:
        print("Visualizing here")
        hyperlink_plot.cluster_vals(nodes, values)
    except:
        traceback.print_exc()
    return 0

