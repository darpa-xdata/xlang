#include "Python.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <assert.h>

#include "td.h"

// value mapping

static td_tag_t longlong_to_td(long long v)
{
	if (INT32_MIN <= v <= INT32_MAX) return TD_INT32;
	else if (0 <= v <= UINT32_MAX) return TD_UINT32;
	else return TD_INT64;
}

static td_tag_t py_type_to_td(PyObject *pVal)
{
    if (PyInt_Check(pVal)){
#if INT32_MAX==LONG_MAX
        return TD_INT32;
#else
        return longlong_to_td(PyInt_AS_LONG(pVal));
#endif
    }
    else if (PyLong_Check(pVal)){
        PyObject *pErr;
        long long v;
        v = PyLong_AsLongLong(pVal);
        if ( v == -1 && PyErr_Occurred()) {
            // could not eval as long long try unsigned long long later
            PyErr_Clear();
        }
        else {
        	return longlong_to_td(PyInt_AS_LONG(v));
        }

        unsigned long long vv;
        vv = PyLong_AsUnsignedLongLong(pVal);
        if ( v == (unsigned long long) -1 && PyErr_Occurred() ) {
            PyErr_Clear();
            return TD_OBJECT;
        }
        else {
            return TD_UINT64;
        }
    }
    else if (PyFloat_Check(pVal)) {
        return TD_DOUBLE;
    }
    else if (PyUnicode_Check(pVal)){
        PyObject *pStr;
        pStr = PyUnicode_AsUTF8String(pVal);
        if ( pStr && PyErr_Occurred() ){
            PyErr_Clear();
            return TD_OBJECT;
        }
        else {
            return TD_UTF8;
        }
    }
    else if (PyString_Check(pVal)) {
        return TD_UTF8;
    }
    return TD_OBJECT;
}



static void to_td_val(td_val_t *out, PyObject *pVal)
{
    PyObject *pStr;
    td_tag_t tag = py_type_to_td(pVal);
    switch (tag) {
    case TD_INT32:
        out->tag = TD_INT32;
        if (PyInt_Check(pVal))
            out->int32_val = PyInt_AS_LONG(pVal);
        else
            out->int32_val = PyLong_AsLong(pVal);
        break;
    case TD_UINT32:
        out->tag = TD_UINT32;
        out->uint32_val = PyLong_AsUnsignedLong(pVal);
        break;
    case TD_INT64:
        out->tag = TD_INT64;
        out->int64_val = PyLong_AsLongLong(pVal);
        break;
    case TD_UINT64:
        out->tag = TD_UINT64;
        out->uint64_val = PyLong_AsUnsignedLongLong(pVal);
        break;
    case TD_DOUBLE:
        out->tag = TD_DOUBLE;
        out->double_val = PyFloat_AsDouble(pVal);
        break;
    case TD_UTF8:
        pStr = pVal;
        if (PyUnicode_Check(pStr)) pStr = PyUnicode_AsUTF8String(pStr);
        size_t len = PyString_Size(pStr);
        char* data = malloc((len+1)*sizeof(char));
        strcpy(PyString_AsString(pStr), data);
        td_string_t *obj = malloc(sizeof(td_string_t));
        obj->length = len;
        obj->data = (void*) data;
        out->tag = TD_UTF8;
        out->object = (void*) obj;
        break;
//    case TD_ARRAY:
//        return
//
//        return (PyObject*)
//
//
//            py_ptr_to_array_1d((PyObject*)py_apply_array_type((py_datatype_t*)td_type_to_jl(td_eltype(v)), 1),
//                               td_dataptr(v), td_length(v), 0);
    default:
        out->tag = TD_OBJECT;
        out->owner = td_env_python(NULL, NULL);
        out->object = pVal;
    }
}

static PyObject *from_td_val(td_val_t *v)
{
    PyObject *pVal;
    td_tag_t tag = td_typeof(v);
    switch (tag) {
    case TD_INT8:
        return PyInt_FromLong(td_int8(v));
    case TD_UINT8:
        return PyInt_FromLong(td_uint8(v));
    case TD_INT16:
        return PyInt_FromLong(td_int16(v));
    case TD_UINT16:
        return PyInt_FromLong(td_uint16(v));
    case TD_INT32:
        return PyInt_FromLong(td_int32(v));
    case TD_UINT32:
        return PyLong_FromUnsignedLong(td_uint32(v));
    case TD_INT64:
        return PyLong_FromLongLong(td_int64(v));
    case TD_UINT64:
        return PyLong_FromUnsignedLongLong(td_uint64(v));
    case TD_FLOAT:
        return PyFloat_FromDouble(td_float(v));
    case TD_DOUBLE:
        return PyFloat_FromDouble(td_double(v));
    case TD_UTF8:
        return PyUnicode_DecodeUTF8(td_dataptr(v), td_length(v), "strict");
//    case TD_ARRAY:
//        return
//
//        return (PyObject*)
//
//
//            py_ptr_to_array_1d((PyObject*)py_apply_array_type((py_datatype_t*)td_type_to_jl(td_eltype(v)), 1),
//                               td_dataptr(v), td_length(v), 0);
    default:
        td_error("Cannot convert td type to Python.\n");
        return pVal;
    }
}

// entry points
int module_name(char *fname, char *modname)
{
    unsigned len = strlen(fname);
    int i;

    for(i = 0; i > 0; --i){
        if (fname[len] == '.'){
            break;
        }
    }
    if (i > 0) {
        strncpy(fname, modname, len);
    } else {
        strcpy(modname, "__builtin__");
    }
}


PyObject* td_py_get_callable(char *fname)
{
    PyObject *pName, *pModule, *pFunc;
    char modname[32] = "__builtin__";

    pName = PyString_FromString(modname);
    if (pName == NULL) {
        fprintf(stderr, "Error converting module name to PyString");
        return NULL;
    }

    pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    if (pModule == NULL) {
        fprintf(stderr, "Error importing module %s", modname);
        return NULL;
    }

    pFunc = PyObject_GetAttrString(pModule, fname);
    Py_DECREF(pModule);
    if (pFunc == NULL || !PyCallable_Check(pFunc)){
        fprintf(stderr, "Could not get a callable %s\n", fname);
        if (pFunc != NULL) Py_DECREF(pFunc);
        return NULL;
    }

    return pFunc;
}

void td_py_invoke0(td_val_t *out, char *fname)
{
    printf("Here td_py_invoke0 10\n");
    PyObject *pFunc, *pArgs, *pValue;
    printf("Here td_py_invoke0 20\n");

    pFunc = td_py_get_callable(fname);
    if (pFunc == NULL) return;
    printf("Here td_py_invoke0 30\n");

    pArgs = PyTuple_New(0);
    pValue = PyObject_CallObject(pFunc, pArgs);
    printf("Here td_py_invoke0 40\n");
    if (pValue == NULL) {
        fprintf(stderr, "Error in Python call %s\n", fname);
        return;
    }
    to_td_val(out, pValue);
    printf("Here td_py_invoke0 50\n");
}

void td_py_invoke1(td_val_t *out, char *fname, td_val_t *arg)
{
    printf("Here td_py_invoke1 10\n");
    PyObject *pFunc, *pArgs, *pValue;
    printf("Here td_py_invoke1 20\n");

    pFunc = td_py_get_callable(fname);
    if (pFunc == NULL) return;
    printf("Here td_py_invoke1 30\n");

    pArgs = PyTuple_New(1);
    pValue = from_td_val(arg);
    if (pValue == NULL) return;
    PyTuple_SetItem(pArgs, 0, pValue);
    Py_DECREF(pValue);

    pValue = PyObject_CallObject(pFunc, pArgs);
    printf("Here td_py_invoke1 40\n");
    if (pValue == NULL) {
        fprintf(stderr, "Error in Python call %s\n", fname);
        return;
    }
    to_td_val(out, pValue);
    printf("Here td_py_invoke1 50\n");
}

void td_py_eval(td_val_t *out, char *str)
{
//    PyObject *v = py_eval_string(str);
//    to_td_val(out, v);
}

td_tag_t td_py_get_type(void *v)
{
    return py_type_to_td((PyObject*) v);
}

td_tag_t td_py_get_eltype(void *v)
{
//    if (py_is_array(v)) {
//        return py_type_to_td(py_tparam0(py_typeof(v)));
//    }
    return TD_UNKNOWN;
}

void *td_py_get_dataptr(void *v)
{
//    if (py_is_array(v))
//        return py_array_data(v);
//    if (py_is_byte_string(v))
//        return py_string_data(v);
//    return py_data_ptr(v);
}

size_t td_py_get_length(void *v)
{
//    if (py_is_array(v))
//        return py_array_len(v);
//    if (py_is_byte_string(v))
//        return py_array_len(py_fieldref(v,0));
//    return 1;
}

size_t td_py_get_ndims(void *v)
{
//    if (py_is_array(v))
//        return py_array_ndims(v);
//    return 0;
}

// initialization

void td_py_init(char *homedir)
{
    printf("Here td_py_init 10\n");
    Py_Initialize();
    printf("Here td_py_init 20\n");

    td_env_t *env = (td_env_t*)malloc(sizeof(td_env_t));
    env->name = "python";
    printf("Here td_py_init 30\n");

    env->eval = &td_py_eval;
    env->invoke0 = &td_py_invoke0;
    env->invoke1 = &td_py_invoke1;
    printf("Here td_py_init 40\n");
    //env->invoke2
    //env->invoke3

    //env->retain
    //env->release

    env->get_type = &td_py_get_type;
    env->get_eltype = &td_py_get_eltype;
    env->get_dataptr = &td_py_get_dataptr;
    env->get_length = &td_py_get_length;
    env->get_ndims = &td_py_get_ndims;
    printf("Here td_py_init 50\n");

    //env->get_dims
    //env->get_strides

    td_provide_python(env);
    printf("Here td_py_init 60\n");
}

