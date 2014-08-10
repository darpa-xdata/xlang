
import numpy as np
import pandas as pd

from bokeh.objects import (CategoricalAxis, ColumnDataSource, DataRange1d, 
                           FactorRange, Glyph, Grid, LinearAxis, Plot, PanTool, WheelZoomTool, LogAxis)
from bokeh.glyphs import Rect, Text
from bokeh.plotting import output_file, show, curdoc, save, text, hold, rect, figure
from bokeh.resources import Resources



def make_box_violin_plot(data, num_bins, maxwidth=0.9):
    """ 
    data: dict[Str -> List[Number]]
    maxwidth: float
        Maximum width of tornado plot within each factor/facet

    Returns the plot object 
    """


    df = pd.DataFrame(columns=["group", "centers", "width", "height", "texts"])
    bar_height = 50
    bins = [0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6]
    # Compute histograms, while keeping track of max Y values and max counts
    for i, (group, vals) in enumerate(data.iteritems()):
        hist, edges = np.histogram(vals, bins)
        df = df.append(pd.DataFrame(dict(
            group = group,
            centers = np.arange(len(hist))*bar_height,
            width = np.log10(hist),
            height = np.ones(hist.shape)*bar_height,
            texts = map(str,hist),
            )))
            
    df.replace(-np.inf, 0)

    # Normalize the widths
    df["width"] *= (maxwidth / df["width"].max())
    
    ds = ColumnDataSource(df)

    xdr = FactorRange(factors=sorted(df["group"].unique()))
    ydr = DataRange1d(sources=[ds.columns("centers")])

    plot = Plot(data_sources=[ds], x_range=xdr, y_range=ydr,
                title="Degree Distribution",
                plot_width=750, plot_height=600, 
                tools=[])
    xaxis = CategoricalAxis(plot=plot, location="bottom", axis_label="number of nodes")
    #yaxis = LogAxis(plot=plot, location="left", axis_label="degree")
    plot.below.append(xaxis)
    #plot.above.append(yaxis)

    #xgrid = Grid(plot=plot, dimension=0, axis=xaxis)
    #ygrid = Grid(plot=plot, dimension=1, axis=yaxis)

    glyph = Rect(x="group", y="centers", width="width", height="height")
    text_glyph = Text(x="group", y="centers", text="texts", text_baseline="middle", text_align="center")
    plot.renderers.append(Glyph(data_source=ds, xdata_range=xdr, ydata_range=ydr,
                                 glyph=glyph))
    plot.renderers.append(Glyph(data_source=ds, xdata_range=xdr, ydata_range=ydr, glyph=text_glyph))

    return plot


def make_box_violin_plot_2(data, maxwidth=0.9):
    """ 
    data: dict[Str -> List[Number]]
    maxwidth: float
        Maximum width of tornado plot within each factor/facet

    Returns the plot object 
    """


    df = pd.DataFrame(columns=["group", "centers", "width", "height", "texts"])
    bar_height = 50
    bins = [0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6]
    # Compute histograms, while keeping track of max Y values and max counts
    for i, (group, vals) in enumerate(data.iteritems()):
        hist, edges = np.histogram(vals, bins)
        df = df.append(pd.DataFrame(dict(
            group = group,
            centers = np.arange(len(hist))*bar_height,
            width = np.log10(hist),
            height = np.ones(hist.shape)*bar_height,
            texts = map(str,hist),
            )))
            
    df.replace(-np.inf, 0)

    # Normalize the widths
    df["width"] *= (maxwidth / df["width"].max())

    hold()
    width, height = 800, 800

    figure(plot_width=width, plot_height=height, title="Degree Distribution",
       tools="previewsave",
       x_axis_type=None, y_axis_type=None,
       x_range=[-420, 420], y_range=[-420, 420],
       x_axis_label="Number of nodes (log)", y_label="distribution of degree",
       min_border=0, outline_line_color=None,
       background_fill="#f0e1d2", border_fill="#f0e1d2")

    size = len(df)
    rect(np.zeros(size), df["centers"], height=[50 for i in range(size)], 
         width=df["width"]*width * .8)
    text(np.zeros(size), df["centers"], text=df["texts"], angle=np.zeros(size), 
         text_color= ["#000000"] + ["#FFFFFF" for i in xrange(size-1)], 
         text_align="center", text_baseline="middle")
    text(np.ones(size+1) * -width/2, 
         [(idx-0.75)*bar_height for idx in range(size+1)], 
         text=map(str, map(int, bins)), angle=0)
    show()

def plot_graph(data, bins):
    p = make_box_violin_plot(data, bins)
    doc = curdoc()
    doc.add(p)
    doc._current_plot = p   #TODO: Fix this!
    output_file("violin.html")
    show()

    make_box_violin_plot_2(data)
    #save("violin.html", obj=doc)



if __name__ == "__main__":
    import sys
    #data = dict(a = np.random.randn(100)*30 + 10,
    #            b = np.random.randn(30)*20 + 5,
    #            c = np.random.randn(300)*35 + 15)
    #plot_graph(data, 10)

    num_deg = np.load("degrees.npy")
    plot_graph({"total_degree": num_deg}, 10)


    

