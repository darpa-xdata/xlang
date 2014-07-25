Webgraph Data
=============

Data in this folder was obtained from the following places:

pld-arc-sample.gz and pld-index-sample.gz

    file obtained from: http://webdatacommons.org/hyperlinkgraph/
    citation: Oliver Lehmberg, Robert Meusel, Christian Bizer: The Graph Structure of the Web aggregated by Pay-Level Domain. Accepted paper at the ACM Web Science 2014 Conference (WebSci2014), Bloomington, USA, June 2014.
    license: http://commoncrawl.org/about/terms-of-use/full-terms-of-use/
    modifications: subsampled of the larger pld-arc.gz and pld-index.gz files

web-NotreDame.txt.gz

    file obtained from: https://snap.stanford.edu/data/web-NotreDame.html
    citation: R. Albert, H. Jeong, A.-L. Barabasi. Diameter of the World-Wide Web. Nature, 1999.

louvain.txt.gz

    clustering using Louvain modularity from DeLite on web-NotreDame dataset
    Format:
      <number of nodes>
      <node name>…
      <number of rowptrs>
      <rowptr>…
      <number of colidxs>
      <colidx>…
