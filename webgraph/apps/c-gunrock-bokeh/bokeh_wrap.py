import sys
import traceback

import numpy as np

# Ugly hack because tables (imported by bokeh) borks if sys.argv isn't defined
sys.argv = ['app']
sys.path.append("../../vis/bokeh")    
import hyperlink_plot
import boxviolin

print("loading bokeh.wrap")

def visualize( nodes, in_deg, out_deg ):
    try:
        print("Visualizing here")

        print(nodes);
        print(in_deg);
        print(out_deg);
        
        np.save("degrees", in_deg+out_deg);

        #hyperlink_plot.cluster_vals(nodes, in_deg)
        boxviolin.plot_graph({'total degrees': in_deg + out_deg,
                              #'in degree': in_deg,
                              #'out degree':out_deg,
                             },
                             100)
    except:
        traceback.print_exc()
    return 0

