
import numpy as np
import pandas as pd

from bokeh.objects import (CategoricalAxis, ColumnDataSource, DataRange1d, 
        FactorRange, Glyph, Grid, LinearAxis, Plot, PanTool, WheelZoomTool)
from bokeh.glyphs import Rect


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

    plot = Plot(data_sources=[ds], x_range=xdr, y_range=ydr, title="Violin Box Plot",
            plot_width=800, plot_height=600, 
            tools=[PanTool(dimensions=["height"]), WheelZoomTool()])
    xaxis = CategoricalAxis(plot=plot, location="bottom")
    yaxis = LinearAxis(plot=plot, location="left")
    plot.below.append(xaxis)
    plot.above.append(yaxis)

    xgrid = Grid(plot=plot, dimension=0, axis=xaxis)
    ygrid = Grid(plot=plot, dimension=1, axis=yaxis)

    glyph = Rect(x="group", y="centers", width="width", height="height")
    plot.renderers.append(Glyph(data_source=ds, xdata_range=xdr, ydata_range=ydr,
        glyph=glyph))

    return plot


def _maketestplot():
    data = dict(a = np.random.randn(100)*30 + 10,
                b = np.random.randn(30)*20 + 5,
                c = np.random.randn(300)*35 + 15)
    p = make_box_violin_plot(data, 10)
    return p

def filemain():
    p = _maketestplot()
    from bokeh.plotting import output_file, show, curdoc, save
    from bokeh.document import Document
    from bokeh.resources import Resources
    doc = Document()
    doc.add(p)
    doc._current_plot = p   #TODO: Fix this!
    output_file("violin.html")
    save("violin.html", obj=doc)
    return p

#def servermain():
#    from bokeh.plotting import output_server
#    p = _maktestplot()
#    renderer = [r for r in p.renderers if isinstance(r, Glyph)][0]
#    ds = renderer.data_source
#    while True:
#        pts = np.random.randn(50)*25
#        ds.data["a"] = np.hstack((ds.data["a"], pts))

if __name__ == "__main__":
    import sys
    filemain()


