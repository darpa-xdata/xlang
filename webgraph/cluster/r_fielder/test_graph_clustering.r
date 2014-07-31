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
    make_option(c("-o", "--ofile"), type="character",
    default="out.png",
    help="The name of the output png file showing the clustered matrix."),
    make_option(c("-n", "--num_clusters"), type="integer",
    default=8,
    help="The maximum number of clusters to find")
)

op <- OptionParser(option_list=option_list)
pargs <- parse_args(op, print_help_and_exit=TRUE)

cat("Importing the graph in", pargs$ifile, "\n")
m <- import_snap_graph(pargs$ifile)
cat("Reducing the graph to, at most,", pargs$num_clusters, "clusters\n")
cg <- fielder_cluster_and_graph(m, pargs$num_clusters, TRUE)
cat("Opening derived graph in web browser")
write(create_fdg(cg$matrix), pargs$ofile)
system(paste("open", pargs$ofile))
#cat("Writing the clustered graph to", pargs$ofile, "\n")
#write_adjacency_matrix(cg$matrix, pargs$ofile)
