#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <jni.h> // this is found under the JVM include directory, e.g. on windows C:\Program Files\Java\jdk1.8.0_05\include

#include "td.h"

// type mapping

JNIEnv* create_vm(JavaVM ** jvm) {

    JNIEnv *env;
    JavaVMInitArgs vm_args;
    JavaVMOption options;
    options.optionString = "-Djava.class.path=."; //Path to the java source code
    vm_args.version = JNI_VERSION_1_6; //JDK version. This indicates version 1.6
    vm_args.nOptions = 1;
    vm_args.options = &options;
    vm_args.ignoreUnrecognized = 0;

    int ret = JNI_CreateJavaVM(jvm, (void**)&env, &vm_args);
    if(ret < 0)
    	printf("\nUnable to Launch JVM\n");
	return env;
}


/*static td_tag_t java_type_to_td(java_value_t *t)
{
    if (t == (java_value_t*)java_int8_type) return TD_INT8;
    if (t == (java_value_t*)java_uint8_type) return TD_UINT8;
    if (t == (java_value_t*)java_int16_type) return TD_INT16;
    if (t == (java_value_t*)java_uint16_type) return TD_UINT16;
    if (t == (java_value_t*)java_int32_type) return TD_INT32;
    if (t == (java_value_t*)java_uint32_type) return TD_UINT32;
    if (t == (java_value_t*)java_int64_type) return TD_INT64;
    if (t == (java_value_t*)java_uint64_type) return TD_UINT64;
    if (t == (java_value_t*)java_float32_type) return TD_FLOAT;
    if (t == (java_value_t*)java_float64_type) return TD_DOUBLE;
    if (java_is_array_type(t)) return TD_ARRAY;
    if (t == (java_value_t*)java_ascii_string_type ||
        t == (java_value_t*)java_utf8_string_type)
        return TD_UTF8;
    return TD_OBJECT;
}

static java_value_t *td_type_to_java(td_tag_t tag)
{
    switch (tag) {
    case TD_INT8: return (java_value_t*)java_int8_type;
    case TD_UINT8: return (java_value_t*)java_uint8_type;
    case TD_INT16: return (java_value_t*)java_int16_type;
    case TD_UINT16: return (java_value_t*)java_uint16_type;
    case TD_INT32: return (java_value_t*)java_int32_type;
    case TD_UINT32: return (java_value_t*)java_uint32_type;
    case TD_INT64: return (java_value_t*)java_int64_type;
    case TD_UINT64: return (java_value_t*)java_uint64_type;
    case TD_FLOAT: return (java_value_t*)java_float32_type;
    case TD_DOUBLE: return (java_value_t*)java_float64_type;
    case TD_UTF8: return (java_value_t*)java_utf8_string_type;
    default:
        return (java_value_t*)java_nothing->type;
    }
}

// value mapping

static void to_td_val(td_val_t *out, java_value_t *v)
{
    td_tag_t tag = java_type_to_td(java_typeof(v));
    if (tag < TD_UTF8) {
        out->tag = tag;
        memcpy(&out->object, java_data_ptr(v), td_type_size(tag));
    }
    else {
        out->owner = td_env_julia(NULL, NULL);
        out->object = v;
    }
}

static java_value_t *from_td_val(td_val_t *v)
{
    td_tag_t tag = td_typeof(v);
    switch (tag) {
    case TD_INT8: return java_box_int8(td_int8(v));
    case TD_UINT8: return java_box_uint8(td_uint8(v));
    case TD_INT16: return java_box_int16(td_int16(v));
    case TD_UINT16: return java_box_uint16(td_uint16(v));
    case TD_INT32: return java_box_int32(td_int32(v));
    case TD_UINT32: return java_box_uint32(td_uint32(v));
    case TD_INT64: return java_box_int64(td_int64(v));
    case TD_UINT64: return java_box_uint64(td_uint64(v));
    case TD_FLOAT: return java_box_float32(td_float(v));
    case TD_DOUBLE: return java_box_float64(td_double(v));
    case TD_UTF8:
        return java_pchar_to_string(td_dataptr(v), td_length(v));
    case TD_ARRAY:
        return (java_value_t*)
            java_ptr_to_array_1d((java_value_t*)java_apply_array_type((java_datatype_t*)td_type_to_java(td_eltype(v)), 1),
                               td_dataptr(v), td_length(v), 0);
    default:
        return java_nothing;
    }
}*/

// entry points

void td_java_invoke0(td_val_t *out, char *fname)
{
   // java_function_t *f = java_get_function(java_base_module, fname);
   // java_value_t *v = java_call0(f);
  //  to_td_val(out, v);
}

void td_java_invoke1(td_val_t *out, char *fname, td_val_t *arg)
{
   // java_function_t *f = java_get_function(java_base_module, fname);
   // java_value_t *v = java_call1(f, from_td_val(arg));
  //  to_td_val(out, v);
}

void td_java_eval(td_val_t *out, char *str)
{
   // java_value_t *v = java_eval_string(str);
    //to_td_val(out, v);
}

td_tag_t td_java_get_type(void *v)
{
    return TD_UNKNOWN;
}

td_tag_t td_java_get_eltype(void *v)
{
   // if (java_is_array(v)) {
   //     return java_type_to_td(java_tparam0(java_typeof(v)));
   // }
    return TD_UNKNOWN;
}

/*void *td_java_get_dataptr(void *v)
{
   // if (java_is_array(v))
  //      return java_array_data(v);
   // if (java_is_byte_string(v))
  //      return java_string_data(v);
    return java_data_ptr(v);
}*/

size_t td_java_get_length(void *v)
{
   // if (java_is_array(v))
   //     return java_array_len(v);
   // if (java_is_byte_string(v))
   //     return java_array_len(java_fieldref(v,0));
    return 1;
}

size_t td_java_get_ndims(void *v)
{
   // if (java_is_array(v))
   //     return java_array_ndims(v);
    return 0;
}

// initialization

void td_java_init(char *home_dir)
{
  //  java_init(home_dir);

	JNIEnv *jniEnv;
	JavaVM * jvm;
	jniEnv = create_vm(&jvm);
	if (jniEnv == NULL)
		return;
	else {
		printf("got here -- made java vm!\n");
	}

    td_env_t *env = (td_env_t*)malloc(sizeof(td_env_t));
    env->name = "java";

    env->eval = &td_java_eval;
    env->invoke0 = &td_java_invoke0;
    env->invoke1 = &td_java_invoke1;
    //env->invoke2
    //env->invoke3

    //env->retain
    //env->release

    env->get_type = &td_java_get_type;
    env->get_eltype = &td_java_get_eltype;
   // env->get_dataptr = &td_java_get_dataptr;
    env->get_length = &td_java_get_length;
    env->get_ndims = &td_java_get_ndims;

    //env->get_dims
    //env->get_strides

    td_provide_java(env);
}

td_env_t *get_java() {
	JNIEnv *jniEnv;
	JavaVM * jvm;
	jniEnv = create_vm(&jvm);
	if (jniEnv == NULL)
		return NULL;
	else {
		printf("got here -- made java vm!\n");
	}

    td_env_t *env = (td_env_t*)malloc(sizeof(td_env_t));
    env->name = "java";

    env->eval = &td_java_eval;
    env->invoke0 = &td_java_invoke0;
    env->invoke1 = &td_java_invoke1;
    //env->invoke2
    //env->invoke3

    //env->retain
    //env->release

    env->get_type = &td_java_get_type;
    env->get_eltype = &td_java_get_eltype;
   // env->get_dataptr = &td_java_get_dataptr;
    env->get_length = &td_java_get_length;
    env->get_ndims = &td_java_get_ndims;

    return env;
}
