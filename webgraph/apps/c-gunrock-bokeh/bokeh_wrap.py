import sys
import traceback


def visualize( nodes, values ):
    try:
        print("Visualizing here")

        print(nodes);
        print(values);

        # Ugly hack because tables (imported by bokeh) borks if sys.argv isn't defined
        sys.argv = ['app']
        sys.path.append("../../vis/bokeh")    
        import hyperlink_plot
        hyperlink_plot.cluster_vals(nodes, values)
    except:
        traceback.print_exc()
    return 0

