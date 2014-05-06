#include <set>

#include <RInside.h>

#include "td.h"

using namespace Rcpp;
using namespace std;


static RInside r_env(0, NULL);
static set<SEXP> r_obj_set;

#ifdef __cplusplus
extern "C"
{
#endif

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
    if (td_eltype(v) <= TD_UINT32) 
    {
      pv = Rf_allocVector(INTSXP, td_length(v));
    }
    else if (td_eltype(v) > TD_UINT32 && td_eltype(v) <= TD_DOUBLE) 
    {
      pv = Rf_allocVector(REALSXP, td_length(v));
    }
    else if (td_eltype(v) == TD_UTF8)
    {
      throw("Arrays of strings are not yet supported.");
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
  to_td_val(out, ans);
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

void td_r_init(char *home_dir)
{

  td_env_t *env = (td_env_t*)malloc(sizeof(td_env_t));
  env->name = "r";

  env->eval = &td_r_eval;
  env->invoke0 = &td_r_invoke0;
  env->invoke1 = &td_r_invoke1;
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
}
#ifdef __cplusplus
} //extern "C"
#endif

