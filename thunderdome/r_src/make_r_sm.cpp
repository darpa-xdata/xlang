#include "td_graph.h"
#include <Rcpp.h>
#include <iostream>

using namespace Rcpp;
using namespace std;

typedef struct {
  int numNodes;
  char **nodeNames;
  // CSR encoding
  int numValues;
  double *values;
  int numRowPtrs;
  int *rowValueOffsets;
  int *colOffsets;
} graph_t;

SEXP graph_to_dgRMatrix(graph_t g) {
  Language sp_mat_call("new", "dgRMatrix");
  S4 sp_mat(sp_mat_call.eval());
  IntegerVector dims(2);
  dims[0] = dims[1] = g.numNodes;
  IntegerVector p(g.numRowPtrs);
  copy(g.rowValueOffsets, g.rowValueOffsets+g.numRowPtrs, p.begin());
  IntegerVector j(g.numValues);
  copy(g.colOffsets, g.colOffsets+g.numValues, j.begin());
  NumericVector x(g.numValues);
  copy(g.values, g.values+g.numValues, x.begin());
  sp_mat.slot("Dim") = dims;
  sp_mat.slot("p") = p;
  sp_mat.slot("j") = j;
  sp_mat.slot("x") = x;
  CharacterVector row_names(g.numNodes);
  CharacterVector col_names(g.numNodes);
  for (size_t i=0; i < g.numNodes; ++i)
  {
    row_names[i] = col_names[i] = g.nodeNames[i];
  }
  List dim_names(2);
  dim_names[0] = row_names;
  dim_names[1] = col_names;
  sp_mat.slot("Dimnames") = dim_names;
  return sp_mat;
}

//[[Rcpp::export]]
SEXP make_csr_matrix() 
{
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
  return graph_to_dgRMatrix(g);
}
