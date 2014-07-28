#include "etl/c_import.h"
#include "../../../thunderdome/td.h"

#include <iostream>
#include <RInside.h>         
#include <Rcpp.h>

using namespace std;
using namespace Rcpp;

#include "cluster/r_fielder/r_fielder_clustering.hpp"

void _print_usage()
{
  string usage = "c-fielder-igraph <max_num_clusters>  <graph_type> <graph_name>\n\
    \n\
    Use c etl, clusters with fielder cluseter, and produces an \n\
    igraph png. \n\
\n\
Description\n\
\n\
    max_num_cluster: Maximum number of clusters produced.\n\
    graph_format: one of WDC or SNAP\n\
    graph_names: filename to graph (or index_file arc_file for WDC graphs)\n\
\n\
Examples:\n\
    test_c_import SNAP ../data/web-NotreDame.txt\n\
\n\
    test_c_import WDC ../data/pld-arc-sample ../data/pld-index-sample\n\
\n\
Author(s):\n\
    Michael Kane <kaneplusplus@gmail.com>\n\
    Andy R. Terrel <andy.terrel@gmail.com>\n";
  printf("%s\n", usage.c_str());
}


RInside init_r_env(int argc, char*argv[]) {
  RInside R(argc, argv);
  R.parseEvalQ("source(\"../../cluster/r_fielder/graph_cluster.r\")");
  return R;
}


// You can run this with ./app 32 ../../data/web-NotreDame.txt 

int main(int argc, char** argv)
{
  if (argc != 3) {
    _print_usage();
    return 1;
  }

  // Set up R
  cout << "Setting up the R environment.\n";
  RInside R = init_r_env(argc, argv); 
  string output_png_file = "out.png";
  string snap_file_name = string(argv[2]);
  int max_cluster = atoi(argv[1]);
  graph_t input_graph;
  // ga will be the derived graph with annotation
  derived_graph_and_annotation_t ga;
  cout << "Loading " << snap_file_name << ".\n"; 
  input_graph = load_snap_graph_from_r(R, snap_file_name);
  // load_graph(format, arc_file, index_file, &output_graph);
  cout << "Creating the fielder clusters.\n";
  ga = td_fielder_cluster(R, input_graph, max_cluster);
  cout << "Writing the output to " << output_png_file << ".\n";
  write_graph(R, ga.graph, output_png_file); 
  return 0;

}
