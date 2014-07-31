#ifndef R_FIELDER_CLUSTERING_HPP
#define R_FIELDER_CLUSTERING_HPP

#include <string>
#include <sstream>
#include <vector>
#include <Rcpp.h>

using namespace std;
using namespace Rcpp;

#include "../../../thunderdome/td.h"

typedef struct {
  char *nodes;
  char *links;
} force_directed_graph_json_t;

SEXP graph_to_dgRMatrix(graph_t *g) {
  Language sp_mat_call("new", "dgRMatrix");
  S4 sp_mat(sp_mat_call.eval());
  IntegerVector dims(2);
  dims[0] = dims[1] = g->numNodes;
  IntegerVector p(g->numNodes+1);
  copy(g->rowOffsets, g->rowOffsets+g->numNodes+1, p.begin());
  IntegerVector j(g->numEdges);
  copy(g->colIndices, g->colIndices+g->numEdges, j.begin());
  NumericVector x(g->numEdges);
  copy(g->edgeValues, g->edgeValues+g->numEdges, x.begin());
  sp_mat.slot("Dim") = dims;
  sp_mat.slot("p") = p;
  sp_mat.slot("j") = j;
  sp_mat.slot("x") = x;
  if (g->nodeNames) 
  {
    CharacterVector row_names(g->numNodes);
    CharacterVector col_names(g->numNodes);
    for (int i=0; i < g->numNodes; ++i)
    {
      row_names[i] = col_names[i] = g->nodeNames[i];
    }
    List dim_names(2);
    dim_names[0] = row_names;
    dim_names[1] = col_names;
    sp_mat.slot("Dimnames") = dim_names;
  }
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

graph_t* SEXP_to_graph(const List &s, bool with_dimnames=false) {
  graph_t *g = (graph_t*)malloc(sizeof(graph_t));
  g->numNodes = get_first<IntegerVector>(s["numNodes"]);
  if (with_dimnames) {
    vector<string> strs;
    stringstream ss;
    for (int i=0; i < g->numNodes; ++i) {
      ss << i;
      strs.push_back(ss.str());
    }
    g->nodeNames = strings_to_cstrings(strs);
  } else {
    g->nodeNames = NULL;
  }
  g->numEdges = get_first<IntegerVector>(s["numEdges"]);
  g->edgeValues = copy_to_c_type<NumericVector>(s["edgeValues"]);
  g->rowOffsets = copy_to_c_type<IntegerVector>(s["rowOffsets"]);
  g->colIndices = copy_to_c_type<IntegerVector>(s["colIndices"]);
  return g;
}

extern "C" {

graph_t* load_snap_graph_from_r(RInside &R, string file_name) {
  SEXP ans;
  graph_t *g;
  R["fn"] = file_name;
  R.parseEval("dgRMatrix_to_list(import_snap_graph(fn))", ans);
  g = SEXP_to_graph(ans);
  R.parseEvalQ("rm(\"fn\")");
  return g;
}

derived_graph_t td_fielder_cluster(RInside &R, graph_t *g, int k) {
  derived_graph_t ret;
  SEXP ans;
  R["m"] = graph_to_dgRMatrix(g);
//  R["m"] = graph_to_matrix_csr(g);
  R["k"] = k;
  R.parseEval("fielder_cluster_and_graph(m, k)", ans);
  R.parseEvalQ("rm(\"m\", \"k\")");
  List l(ans);
  ret.orig_graph = g; 
  ret.derived_graph = SEXP_to_graph(l["graph"]);
  ret.node_mapping = copy_to_c_type<IntegerVector>(l["clusters"]);
  return ret;
}

void write_graph(RInside &R, graph_t *g, string png_file_name) {
  R["m"] = graph_to_dgRMatrix(g);
  R["fn"] = png_file_name;
  R.parseEvalQ("write_adjacency_matrix(m, fn)");
  R.parseEvalQ("rm(\"m\", \"fn\")");
}

force_directed_graph_json_t get_fdg_json(RInside &R, graph_t *g) {
  R["m"] = graph_to_dgRMatrix(g);
  SEXP ans;
  R.parseEval("create_fdg_json(m)", ans);
  R.parseEvalQ("rm(\"m\")");
  CharacterVector cv(ans);
  char *nodes, *links;
  string node_string = String(cv[0]);
  string link_string = String(cv[1]);
  nodes = (char*)malloc(sizeof(char)*(node_string.size()+1));
  links = (char*)malloc(sizeof(char)*(link_string.size()+1));
  strncpy(nodes, node_string.c_str(), node_string.size()+1);
  strncpy(links, link_string.c_str(), link_string.size()+1);

  force_directed_graph_json_t ret;
  ret.nodes = nodes;
  ret.links = links;
  return ret;
}

}

#endif //R_FIELDER_CLUSTERING_HPP
