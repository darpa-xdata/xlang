try:
    import sys
    import traceback
    
    import numpy as np

    # Ugly hack because tables (imported by bokeh) borks if sys.argv isn't defined
    sys.argv = ['app']
    sys.path.append("../../vis/bokeh")    
    import hyperlink_plot
    import boxviolin
    
    print("loading bokeh.wrap")
except:
    traceback.print_exc()

def visualize(csr_offsets, csr_indices, top_nodes, in_deg, out_deg ):
    try:
        print("Visualizing here")

        print(nodes);
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
    except:
        traceback.print_exc()
    return 0

