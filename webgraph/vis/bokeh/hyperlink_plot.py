import bokeh.plotting as plt
from bokeh.charts import Bar
import brewer2mpl as b2m
import pandas as pds


def example_hyperlink_plot():
    idx_df = pds.read_csv("example_index.txt", sep="\t", names=["url", "id"]) 
    arc_df = pds.read_csv("example_arcs.txt", sep="\t", names=["src", "dest"])

    out_deg = arc_df.groupby(["src"]).size()
    out_deg.sort(ascending=False)
    in_deg = arc_df.groupby(["dest"]).size()
    in_deg.sort(ascending=False)

    color_map_max = out_deg.max()
    pallette = b2m.get_map("Dark2", "Qualitative", 8).hex_colors
    num_colors = len(pallette) - 1
    color_map = lambda x: pallette[int(num_colors*(x/color_map_max))]
    color_vals = out_deg[in_deg.index].fillna(0)
    colors = color_vals.map(color_map)

    plt.output_file("in_deg.html", title="In degree rankings")
    plt.scatter(range(in_deg.size), in_deg, color=colors, fill_alpha=0.2,
                size=10, name="In Degree")
    plt.show()

def cluster_vals(nodes, values):
    plt.output_file("in_deg.html", title="In degree rankings")
    bar = Bar(nodes, values)
    bar.show()
