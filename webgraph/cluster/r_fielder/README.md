# Contents of this directory

1. graph_cluster.r 
  - R code used to perform fielder clustering.
  - R code to output a matrix to a png file, an html page, or a json document.
2. test_graph_clustering.r
  - A hashbang executable R script for testing R functionality in graph_cluster.r
3. r_fielder_clustering.hpp
  - C code to make use of (1) from a C program 
  - C code to convert from R SEXP's to thunderdome's graph_t and derived_graph_t
4. r_env_example.cpp
  - A toy application written in C that makes use of 1 and 2 to clusters a small graph.
5. Makefile
  - A make file to build 4

# Running the R script

The test_graph_clustering.r script imports a SNAP file; peforms fielder
clustering to create a derived, smaller graph; and writes the output to
stdout, a .json file, a .png file, or a .html file. It can be run in a 
terminal window and provides help documentation.
```
./test_graph_clustering.r --help
```

To reduce the Notre Dame data (325729 nodes and 2180216 edges), which is 
included in the repository, to at most 32 clusters and put the output to 
an html file the following command is sufficient.

```
./test_graph_clustering.r -i ../../data/web-NotreDame.txt -o example.html -n 32
```

It should be noted that on a Mac the browser will be open automatically
after the html file has been written.

# Running the C program

The r_env_example.cpp is a minimal toy example that shows how to call
R functions from C.  The r_env_example application can be created with the 
following command in the terminal
```
make example
```
and it can be run with the following command
```
./r_env_example
```

The example itself initializes the R environment; sources the R code in 
graph_cluster.r; creates a small graph_t struct in C; performs fielder
clustering on the graph returning a derived graph and mapping from a
set of nodes in the original to nodes in the derived graph; and outputs
the node assignments.  The program is intended as a starting point for users 
who want to create their own C-callable R functions. A more complete 
application for fielder clustering is available in 
xlang/webgraph/apps/c-fielder-igraph.
