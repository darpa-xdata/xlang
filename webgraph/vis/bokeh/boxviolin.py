
import numpy as np
import pandas as pd

from bokeh.objects import (CategoricalAxis, ColumnDataSource, DataRange1d, 
        FactorRange, Glyph, Grid, LinearAxis, Plot, PanTool, WheelZoomTool)
from bokeh.glyphs import Rect
from bokeh.plotting import output_file, show, curdoc, save
from bokeh.resources import Resources



def make_box_violin_plot(data, num_bins, maxwidth=0.9):
    """ 
    data: dict[Str -> List[Number]]
    maxwidth: float
        Maximum width of tornado plot within each factor/facet

    Returns the plot object 
    """

    df = pd.DataFrame(columns=["group", "centers", "width", "height"])
    # Compute histograms, while keeping track of max Y values and max counts
    for group, vals in data.iteritems():
        hist, edges = np.histogram(vals, num_bins, density=True)
        df = df.append(pd.DataFrame(dict(
            group = group,
            centers = (edges[:-1] + edges[1:])/2.0,
            width = hist,
            height = edges[1:] - edges[:-1])))

    # Normalize the widths
    df["width"] *= (maxwidth / df["width"].max())
    
    ds = ColumnDataSource(df)

    xdr = FactorRange(factors=sorted(df["group"].unique()))
    ydr = DataRange1d(sources=[ds.columns("centers")])

    plot = Plot(data_sources=[ds], x_range=xdr, y_range=ydr, title="Degree Distribution",
            plot_width=750, plot_height=600, 
            tools=[])
    xaxis = CategoricalAxis(plot=plot, location="bottom", axis_label="number of nodes")
    yaxis = LinearAxis(plot=plot, location="left", axis_label="degree")
    plot.below.append(xaxis)
    plot.above.append(yaxis)

    xgrid = Grid(plot=plot, dimension=0, axis=xaxis)
    ygrid = Grid(plot=plot, dimension=1, axis=yaxis)

    glyph = Rect(x="group", y="centers", width="width", height="height")
    plot.renderers.append(Glyph(data_source=ds, xdata_range=xdr, ydata_range=ydr,
        glyph=glyph))

    return plot


def plot_graph(data, bins):
    p = make_box_violin_plot(data, bins)
    doc = curdoc()
    doc.add(p)
    doc._current_plot = p   #TODO: Fix this!
    output_file("violin.html")
    show()
    #save("violin.html", obj=doc)



if __name__ == "__main__":
    import sys
    data = dict(a = np.random.randn(100)*30 + 10,
                b = np.random.randn(30)*20 + 5,
                c = np.random.randn(300)*35 + 15)
    plot_graph(data, 10)


