#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "td.h"

#include <R.h>
#include <Rdefines.h>

#include <RInside.h>

#ifdef __cplusplus
extern "C"
{
#endif

static td_tag_t r_type_to_td(SEXP t)
{
    if (LENGTH(t) == 1)
    { 
      if (isInteger(t)) return TD_INT32;
      if (isReal(t)) return TD_DOUBLE;
      if (isString(t)) return TD_UTF8;
    }
    
    if (LENGTH(t) > 1) return TD_ARRAY;

    return TD_OBJECT;
} 
static SEXP td_type_to_r(td_tag_t tag)
{
    switch (tag) {
    case TD_INT8: return NEW_INTEGER(1);
    case TD_UINT8: return NEW_INTEGER(1);
    case TD_INT16: return NEW_INTEGER(1);
    case TD_UINT16: return NEW_INTEGER(1);
    case TD_INT32: return NEW_INTEGER(1);
    case TD_UINT32: return NEW_INTEGER(1);
    case TD_INT64: return NEW_NUMERIC(1);
    case TD_UINT64: return NEW_NUMERIC(1);
    case TD_FLOAT: return NEW_NUMERIC(1);
    case TD_DOUBLE: return NEW_NUMERIC(1);
    case TD_UTF8: return NEW_STRING(1);
    default:
        return NEW_INTEGER(0);
    }
}

// value mapping
/*
static void to_td_val(td_val_t *out, jl_value_t *v)
{
    td_tag_t tag = jl_type_to_td(jl_typeof(v));
    if (tag < TD_UTF8) {
        out->tag = tag;
        memcpy(&out->object, jl_data_ptr(v), td_type_size(tag));
    }
    else {
        out->owner = td_env_julia(NULL, NULL);
        out->object = v;
    }
}

static jl_value_t *from_td_val(td_val_t *v)
{
    td_tag_t tag = td_typeof(v);
    switch (tag) {
    case TD_INT8: return jl_box_int8(td_int8(v));
    case TD_UINT8: return jl_box_uint8(td_uint8(v));
    case TD_INT16: return jl_box_int16(td_int16(v));
    case TD_UINT16: return jl_box_uint16(td_uint16(v));
    case TD_INT32: return jl_box_int32(td_int32(v));
    case TD_UINT32: return jl_box_uint32(td_uint32(v));
    case TD_INT64: return jl_box_int64(td_int64(v));
    case TD_UINT64: return jl_box_uint64(td_uint64(v));
    case TD_FLOAT: return jl_box_float32(td_float(v));
    case TD_DOUBLE: return jl_box_float64(td_double(v));
    case TD_UTF8:
        return jl_pchar_to_string(td_dataptr(v), td_length(v));
    case TD_ARRAY:
        return (jl_value_t*)
            jl_ptr_to_array_1d((jl_value_t*)jl_apply_array_type((jl_datatype_t*)td_type_to_jl(td_eltype(v)), 1),
                               td_dataptr(v), td_length(v), 0);
    default:
        return jl_nothing;
    }
}

// entry points

void td_jl_invoke0(td_val_t *out, char *fname)
{
    jl_function_t *f = jl_get_function(jl_base_module, fname);
    jl_value_t *v = jl_call0(f);
    to_td_val(out, v);
}

void td_jl_invoke1(td_val_t *out, char *fname, td_val_t *arg)
{
    jl_function_t *f = jl_get_function(jl_base_module, fname);
    jl_value_t *v = jl_call1(f, from_td_val(arg));
    to_td_val(out, v);
}

void td_jl_eval(td_val_t *out, char *str)
{
    jl_value_t *v = jl_eval_string(str);
    to_td_val(out, v);
}

td_tag_t td_jl_get_type(void *v)
{
    return jl_type_to_td(jl_typeof((jl_value_t*)v));
}

td_tag_t td_jl_get_eltype(void *v)
{
    if (jl_is_array(v)) {
        return jl_type_to_td(jl_tparam0(jl_typeof(v)));
    }
    return TD_UNKNOWN;
}

void *td_jl_get_dataptr(void *v)
{
    if (jl_is_array(v))
        return jl_array_data(v);
    if (jl_is_byte_string(v))
        return jl_string_data(v);
    return jl_data_ptr(v);
}

size_t td_jl_get_length(void *v)
{
    if (jl_is_array(v))
        return jl_array_len(v);
    if (jl_is_byte_string(v))
        return jl_array_len(jl_fieldref(v,0));
    return 1;
}

size_t td_jl_get_ndims(void *v)
{
    if (jl_is_array(v))
        return jl_array_ndims(v);
    return 0;
}

// initialization

void td_jl_init(char *home_dir)
{
    jl_init(home_dir);

    td_env_t *env = (td_env_t*)malloc(sizeof(td_env_t));
    env->name = "julia";

    env->eval = &td_jl_eval;
    env->invoke0 = &td_jl_invoke0;
    env->invoke1 = &td_jl_invoke1;
    //env->invoke2
    //env->invoke3

    //env->retain
    //env->release

    env->get_type = &td_jl_get_type;
    env->get_eltype = &td_jl_get_eltype;
    env->get_dataptr = &td_jl_get_dataptr;
    env->get_length = &td_jl_get_length;
    env->get_ndims = &td_jl_get_ndims;

    //env->get_dims
    //env->get_strides

    td_provide_julia(env);
}

*/
#ifdef __cplusplus
} //extern "C"
#endif

