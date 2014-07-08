#include <iostream>
#include <RInside.h>         
#include <Rcpp.h>

using namespace std;
using namespace Rcpp;

#include "r_fielder_clustering.hpp"

RInside init_r_env(int argc, char*argv[]) {
  RInside R(argc, argv);
  R.parseEvalQ("source(\"graph_cluster.r\")");
  return R;
}

int main(int argc, char *argv[]) {
  // ga will be the derived graph with annotation
  derived_graph_and_annotation_t ga;

  // Set up R
  cout << "setting up the R environment\n";
  RInside R = init_r_env(argc, argv); 

  // Create the graph.
  cout << "creating the csr matrix\n";
  double values[]={10, -2, 3, 9, 3, 7, 8, 7, 3, 8, 7, 5, 8, 9, 9, 13, 4, 2, -1};
  int ci[]  = { 0,  4, 0, 1, 5, 1, 2, 3, 0, 2, 3, 4, 1, 3, 4, 5, 1, 4, 5};
  int rp[] =  {0, 2, 5, 8, 12, 16, 19};
  char c1[]={'u','n', '\0'};
  char c2[]={'d','e','u','x', '\0'};
  char c3[]={'t','r','o','i','s', '\0'};
  char c4[]={'q','u','a','t','r','e', '\0'};
  char c5[]={'c','i','n','q', '\0'};
  char c6[]={'s','i','x', '\0'};
  char* cs[] = {c1, c2, c3, c4, c5, c6};
  graph_t g;
  g.numNodes = 6;
  g.numValues = 19;
  g.values = values;
  g.numRowPtrs = 7;
  g.rowValueOffsets = rp;
  g.colOffsets=ci;
  g.nodeNames = cs;

  // Do the graph clustering. 
  cout << "creating the fielder clusters\n";
  ga = td_fielder_cluster(R, g, 2);

  // Output the results.
  cout << "Number of nodes in derived graph is " << ga.graph.numNodes << endl;
  cout << "Cluster assignments for original graph are:\n";
  for (int i=0; i < ga.graph.numNodes; ++i) {
    cout << ga.cluster_assignments[i] <<  " ";
  }
  cout << endl;
  
  cout << "Thank you! Goodnight!\n";
  exit(0);
}
