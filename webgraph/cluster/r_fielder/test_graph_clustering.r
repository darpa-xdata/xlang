#!/usr/bin/env Rscript

library(optparse, quietly=TRUE)
library(Matrix, quietly=TRUE)
library(methods, quietly=TRUE)
library(SparseM, quietly=TRUE, warn.conflicts=FALSE)
source("graph_cluster.r")

option_list <- list(
    make_option(c("-i", "--ifile"), type="character",
    default="../../data/web-NotreDame.txt",
    help="The input SNAP file that gives the matrix."),
    make_option(c("-o", "--ofile"), type="character", default=NULL,
    help=paste("The output file. If the file extension is .htm(l) the html",
               "file is written and, on a Mac, opens in a web browser. If", 
               "the file extension is .json the nodes and links are written in",
               "the d3 force directed graph format.  If the extension is .png",
               "then an image of the graph is created. If not file is",
               "specified then the json documents are written to stdout.", 
               sep="\n\t\t")),
    make_option(c("-n", "--num_clusters"), type="integer",
    default=8,
    help="The maximum number of clusters to find")
)

op <- OptionParser(option_list=option_list)
pargs <- parse_args(op, print_help_and_exit=TRUE)

cat("Importing the graph in", pargs$ifile, "\n")
m <- import_snap_graph(pargs$ifile)
cat("Reducing the graph to, at most,", pargs$num_clusters, "nodes\n")
cg <- fielder_cluster_and_graph(m, pargs$num_clusters, TRUE)
if (is.null(pargs$ofile)) {
  cat(create_fdg_json(cg$matrix), "\n")
} else if (length(grep("\\.html*$", pargs$ofile)) > 0) {
  cat("Writing the file.\n")
  write(create_fdg(cg$matrix), pargs$ofile)
  if (Sys.info()['sysname'] == "Darwin") {
    cat("Opening derived graph in web browser")
    system(paste("open", pargs$ofile))
  }
} else if (length(grep("\\.json$", pargs$ofile)) > 0) {
  cat("Writing json documents\n")
  write(create_fdg_json(cg$matrix), pargs$ofile)
} else if (length(grep("\\.png$", pargs$ofile))) {
  cat("Writing the clustered graph to", pargs$ofile, "\n")
  write_adjacency_matrix(cg$matrix, pargs$ofile)
} else {
  stop("Unknown output file extension")
}
