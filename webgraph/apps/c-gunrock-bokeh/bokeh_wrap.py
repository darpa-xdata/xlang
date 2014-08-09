import sys
import traceback

print("loading bokeh.wrap")

def visualize( nodes, in_deg, out_deg ):
    try:
        print("Visualizing here")

        print(nodes);
        print(in_deg);
        print(out_deg);

        # Ugly hack because tables (imported by bokeh) borks if sys.argv isn't defined
        sys.argv = ['app']
        sys.path.append("../../vis/bokeh")    
        import hyperlink_plot
        hyperlink_plot.cluster_vals(nodes, in_deg)
    except:
        traceback.print_exc()
    return 0

