#ifndef R_FIELDER_CLUSTERING_HPP
#define R_FIELDER_CLUSTERING_HPP

#include <string>
#include <sstream>
#include <vector>
#include <Rcpp.h>

using namespace std;
using namespace Rcpp;

// typedef struct {
//   int numNodes;
//   char **nodeNames;
//   // CSR encoding
//   int numValues;
//   double *values;
//   int numRowPtrs;
//   int *rowValueOffsets;
//   int *colOffsets;
// } graph_t;

typedef struct {
  graph_t graph;
  int *cluster_assignments;
} derived_graph_and_annotation_t;


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
  for (int i=0; i < g.numNodes; ++i)
  {
    row_names[i] = col_names[i] = g.nodeNames[i];
  }
  List dim_names(2);
  dim_names[0] = row_names;
  dim_names[1] = col_names;
  sp_mat.slot("Dimnames") = dim_names;
  return sp_mat;
}

char** strings_to_cstrings(const vector<string> &cv) {
  char** ret = (char**)malloc(sizeof(char*) * cv.size());
  for (size_t i=0; i < cv.size(); ++i) {
    ret[i] = (char*)malloc(sizeof(char*) * (cv[i].size()+1));
    strncpy(ret[i], cv[i].c_str(), cv[i].size());
  }
  return ret;
}

// Let g++ deduce the type of SEXPType.
template <typename RcppVecType, typename SEXPType>
typename RcppVecType::init_type* copy_to_c_type(SEXPType s) {
  typedef typename RcppVecType::init_type c_type;
  RcppVecType r_vec = s;
  c_type* vec = (c_type*)malloc(sizeof(c_type) * r_vec.size());
  copy(r_vec.begin(), r_vec.end(), vec);
  return vec;
}

template <typename RcppVecType, typename SEXPType>
typename RcppVecType::init_type get_first(SEXPType s) {
  RcppVecType r_vec = s;
  return r_vec[0];
}

graph_t SEXP_to_graph(const List &s) {
  graph_t g;
  g.numNodes = get_first<IntegerVector>(s["numNodes"]);
  vector<string> strs;
  stringstream ss;
  for (int i=0; i < g.numNodes; ++i) {
    ss << i;
    strs.push_back(ss.str());
  }
  g.nodeNames = strings_to_cstrings(strs);
  g.numValues = get_first<IntegerVector>(s["numValues"]);
  g.values = copy_to_c_type<NumericVector>(s["values"]);
  g.numRowPtrs = get_first<IntegerVector>(s["numRowPtrs"]);
  g.rowValueOffsets = copy_to_c_type<IntegerVector>(s["rowValueOffsets"]);
  g.colOffsets = copy_to_c_type<IntegerVector>(s["colOffsets"]);
  return g;
}

extern "C" {
derived_graph_and_annotation_t td_fielder_cluster(RInside &R,graph_t &g,int k) {
  derived_graph_and_annotation_t ret;
  SEXP ans;
  R["m"] = graph_to_dgRMatrix(g);
  R["k"] = k;
  R.parseEval("fielder_cluster_and_graph(m, k)", ans);
  R.parseEvalQ("rm(\"m\", \"k\")");
  List l(ans);
  ret.graph = SEXP_to_graph(l["graph"]);
  ret.cluster_assignments = copy_to_c_type<IntegerVector>(l["clusters"]);
  return ret;
}
}

#endif //R_FIELDER_CLUSTERING_HPP
