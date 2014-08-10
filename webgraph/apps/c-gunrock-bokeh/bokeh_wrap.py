import sys
import traceback
        
import numpy as np

# Ugly hack because tables (imported by bokeh) borks if sys.argv isn't defined
sys.argv = ['app']
sys.path.append("../../vis/bokeh")    
import hyperlink_plot
import boxviolin
import adjacency

print("loading bokeh.wrap")

def visualize(csr_offsets, csr_indices, top_nodes, in_deg, out_deg ):
    try:

        print("Visualizing here")

        print(top_nodes);
        print(in_deg);
        print(out_deg);

        np.save("csr_offsets", csr_offsets);
        np.save("csr_indices", csr_indices);
        np.save("top_nodes", top_nodes);
        np.save("degrees", in_deg+out_deg);
        np.save("out_degrees", out_deg);
        np.save("in_degrees", in_deg);

        #hyperlink_plot.cluster_vals(nodes, in_deg)
        boxviolin.plot_graph({'total degrees': in_deg + out_deg,
                              #'in degree': in_deg,
                              #'out degree':out_deg,
                             },
                             100)
        adjacency.plot_graph(csr_offsets, csr_indices)
        
    except:
        traceback.print_exc()
    return 0

def test():
    return 0
