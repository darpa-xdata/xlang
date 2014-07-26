#include <set>

#include <RInside.h>

#include "td.h"

using namespace Rcpp;
using namespace std;


static RInside r_env(0, NULL);
static set<SEXP> r_obj_set;

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
  g.numEdges = get_first<IntegerVector>(s["numValues"]);
  g.edgeValues = copy_to_c_type<NumericVector>(s["values"]);
  g.numNodes = get_first<IntegerVector>(s["numRowPtrs"]);
  g.rowOffsets = copy_to_c_type<IntegerVector>(s["rowValueOffsets"]);
  g.colIndices = copy_to_c_type<IntegerVector>(s["colOffsets"]);
  return g;
}

// WS: This is a horrible hack on my part that must eventually die...  Someone please kill it.
graph_t SEXP_to_graph2(const List &s, graph_t &g) {
  g.numNodes = get_first<IntegerVector>(s["numNodes"]);
  vector<string> strs;
  stringstream ss;
  for (int i=0; i < g.numNodes; ++i) {
    ss << i;
    strs.push_back(ss.str());
  }
  g.nodeNames = strings_to_cstrings(strs);
  g.numEdges = get_first<IntegerVector>(s["numValues"]);
  g.edgeValues = copy_to_c_type<NumericVector>(s["values"]);
  g.numNodes = get_first<IntegerVector>(s["numRowPtrs"]);
  g.rowOffsets = copy_to_c_type<IntegerVector>(s["rowValueOffsets"]);
  g.colIndices = copy_to_c_type<IntegerVector>(s["colOffsets"]);
  return g;
}

//#ifdef __cplusplus
extern "C"
{
  //#endif

typedef struct {
  graph_t graph;
  int *cluster_assignments;
} derived_graph_and_annotation_t;

static td_tag_t r_type_to_td(SEXPTYPE t)
{
  if (t == INTSXP) return TD_INT32;
  if (t == REALSXP) return TD_DOUBLE;
  if (t == CHARSXP) return TD_UTF8;
  return TD_UNKNOWN;
} 

static SEXPTYPE td_type_to_r(td_tag_t tag)
{
  switch (tag) {
    case TD_INT8: return INTSXP;
    case TD_UINT8: return INTSXP;
    case TD_INT16: return INTSXP;
    case TD_UINT16: return INTSXP;
    case TD_INT32: return INTSXP;
    case TD_UINT32: return INTSXP;
    case TD_INT64: return REALSXP;
    case TD_UINT64: return REALSXP;
    case TD_FLOAT: return REALSXP;
    case TD_DOUBLE: return REALSXP;
    case TD_UTF8: return STRSXP;
    default:
      return NILSXP;
    }
}

// value mapping
static void to_td_val(td_val_t *out, SEXP v)
{
  td_tag_t tag = r_type_to_td(TYPEOF(v));
  out->tag = tag;
  if (tag < TD_UTF8)
  {
    memcpy(&out->object, INTEGER(v), td_type_size(tag));
  }
  else if (tag == TD_ARRAY)
  {
    out->tag = TD_OBJECT;
    out->owner = td_env_r(NULL);
    out->object = v;
  }
  else 
  {
    throw("td_val_t not supported for specified types");
  }
}

static SEXP from_td_val(td_val_t *v)
{
  td_tag_t tag = td_typeof(v);
  SEXP pv;
  if (tag < TD_INT64) 
  {
    pv = Rf_allocVector(INTSXP, 1);
  }
  else if (tag >= TD_INT64 && tag < TD_DOUBLE)
  {
    pv = Rf_allocVector(REALSXP, 1);
  }
  else if ( tag == TD_UTF8 )
  {
    pv = Rf_allocVector(STRSXP, 1);
  }
  else if ( tag == TD_ARRAY) 
  {
    td_array_t *arr;
    arr = (td_array_t *) td_dataptr(v);
    if (arr->ndims > 2)
    {
      throw("Unsupported array dimension.");
    }
    if (td_eltype(v) <= TD_UINT32) 
    {
      pv = Rf_allocVector(INTSXP, td_length(v));
    }
    else if (td_eltype(v) > TD_UINT32 && td_eltype(v) <= TD_DOUBLE) 
    {
      pv = Rf_allocVector(REALSXP, td_length(v));
    }
    else 
    {
      throw("Unsupported type.");
    }
  }
  else 
  {
    pv = Rf_allocVector(REALSXP, 0);
  }
  R_PreserveObject(pv);
  return pv;
}

// entry points

void td_r_eval(td_val_t *out, char *str)
{
  SEXP ans;
  r_env.parseEval(str, ans);
  // WS: I decree by general hack, for now this will be void
  //to_td_val(out, ans);
}

void td_r_invoke0(td_val_t *out, char *fname)
{
  SEXP ans;
  r_env.parseEval((string(fname) + "()").c_str(), ans);
  to_td_val(out, ans);
}

void td_r_invoke1(td_val_t *out, char *fname, td_val_t *arg)
{
  SEXP ans;
  string arg_name = 
    r_env.parseEval("paste(sample(letters, 10, replace=TRUE), collapse='')");
  r_env[arg_name] = from_td_val(arg);
  r_env.parseEval( (string(fname) + "(" + arg_name + ")").c_str(), ans);
  r_env.parseEvalQ(string("rm(") + arg_name + ")");
  R_PreserveObject(ans);
  to_td_val(out, ans);
}


SEXP graph_to_dgRMatrix(graph_t g) {
  Language sp_mat_call("new", "dgRMatrix");
  S4 sp_mat(sp_mat_call.eval());
  IntegerVector dims(2);
  dims[0] = dims[1] = g.numNodes;
  IntegerVector p(g.numNodes);
  copy(g.rowOffsets, g.rowOffsets+g.numNodes, p.begin());
  IntegerVector j(g.numEdges);
  copy(g.colIndices, g.colIndices+g.numEdges, j.begin());
  NumericVector x(g.numEdges);
  copy(g.edgeValues, g.edgeValues+g.numEdges, x.begin());
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

void td_r_invokeGraph2(graph_t *out, char *fname, graph_t *arg, int k)
{
  char str[255];
  derived_graph_and_annotation_t ret;
  SEXP ans;
  r_env["m"] = graph_to_dgRMatrix(*arg);
  r_env["k"] = k;
  sprintf(str, "%s(m, k)", fname);
  r_env.parseEval(str, ans);
  r_env.parseEvalQ("rm(\"m\", \"k\")");
  List l(ans);
  ret.graph = SEXP_to_graph2(l["graph"], *out);
  ret.cluster_assignments = copy_to_c_type<IntegerVector>(l["clusters"]);
}

td_tag_t td_r_get_type(void *v)
{
  return r_type_to_td(TYPEOF((SEXP)v));
}

td_tag_t td_r_get_eltype(void *v)
{
  return r_type_to_td(TYPEOF((SEXP)v));
}

void *td_r_get_dataptr(void *v)
{
  return INTEGER( (SEXP)v );
}

size_t td_r_get_length(void *v)
{
  return LENGTH( SEXP(v) );
}

size_t td_r_get_ndims(void *v)
{
  RObject r_obj( (SEXP)v );
  if (r_obj.hasAttribute("dim")) 
  {
    SEXP da = r_obj.attr("dim");
    return LENGTH(da);
  }
  return 0;
}

// initialization

td_env_t *td_r_init(char *home_dir)
{
  td_env_t *env = (td_env_t*)malloc(sizeof(td_env_t));
  env->name = "r";

  env->eval = &td_r_eval;
  env->invoke0 = &td_r_invoke0;
  env->invoke1 = &td_r_invoke1;

  env->invokeGraph2 = &td_r_invokeGraph2;
  //env->invoke2
  //env->invoke3

  //env->retain
  //env->release

  env->get_type = &td_r_get_type;
  env->get_eltype = &td_r_get_eltype;
  env->get_dataptr = &td_r_get_dataptr;
  env->get_length = &td_r_get_length;
  env->get_ndims = &td_r_get_ndims;

  //env->get_dims
  //env->get_strides

  td_provide_r(env);
  return env;
}
  //#ifdef __cplusplus
} //extern "C"
//#endif

