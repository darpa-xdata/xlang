#include "Python.h"
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/ndarrayobject.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <assert.h>

#include "td.h"

static td_tag_t longlong_to_td(long long v)
{
    if (INT32_MIN <= v && v <= INT32_MAX) return TD_INT32;
    else if (0 <= v && v <= UINT32_MAX) return TD_UINT32;
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
    else if (PyArray_Check(pVal)) {
        return TD_ARRAY;
    }
    return TD_OBJECT;
}

static td_tag_t numpy_type_to_td(int type_num)
{
    switch (type_num) {
    case NPY_BOOL:
        return TD_INT8;
    case NPY_BYTE:
        return TD_INT8;
    case NPY_UBYTE:
        return TD_UINT8;
    case NPY_SHORT:
        return TD_INT16;
    case NPY_USHORT:
        return TD_UINT16;
    case NPY_INT:
        return TD_INT32;
    case NPY_UINT:
        return TD_UINT32;
    case NPY_LONG:
#if INT32_MAX==LONG_MAX
        return TD_INT32;
#else
        return TD_INT64;
#endif
    case NPY_ULONG:
#if INT32_MAX==LONG_MAX
        return TD_UINT32;
#else
        return TD_UINT64;
#endif
    case NPY_LONGLONG:
        return TD_INT64;
    case NPY_ULONGLONG:
        return TD_UINT64;
    case NPY_FLOAT:
    case NPY_CFLOAT:
        return TD_FLOAT;
    case NPY_DOUBLE:
    case NPY_CDOUBLE:
        return TD_DOUBLE;
    case NPY_STRING:
    case NPY_UNICODE:
        return TD_UTF8;
    default:
        return TD_UNKNOWN;
    }
}

static int td_type_to_numpy(td_tag_t eltype)
{
    switch (eltype) {
    case TD_INT8:
        return TD_INT8;
    case TD_UINT8:
      return NPY_UBYTE;
    case TD_INT16:
      return NPY_SHORT;
    case TD_UINT16:
      return NPY_USHORT;
    case TD_INT32:
      return NPY_INT32;
    case TD_UINT32:
      return NPY_UINT;
    case TD_INT64:
      return  NPY_INT64;
    case TD_UINT64:
      return NPY_UINT64;
    case TD_FLOAT:
        return NPY_FLOAT;
    case TD_DOUBLE:
        return NPY_DOUBLE;
    case TD_UTF8:
      return NPY_UNICODE;
    default:
      return NPY_VOID;
    }
}

static void to_td_val(td_val_t *out, PyObject *pVal)
{
    PyObject *pStr;
    td_array_t *arr;
    PyArrayObject *pArr;
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
    case TD_ARRAY:
        arr = (td_array_t *)malloc(sizeof(td_array_t));
        pArr = (PyArrayObject *) pVal;
        arr->data = PyArray_DATA(pArr);
        arr->length = PyArray_SIZE(pArr);
        arr->eltype = numpy_type_to_td(PyArray_TYPE(pArr));
        arr->ndims = PyArray_NDIM(pArr);
        break;
    default:
        out->tag = TD_OBJECT;
        out->owner = td_env_python(NULL, NULL);
        out->object = pVal;
    }
}

static PyObject *pyarray_from_td_val(td_val_t *v){
    td_array_t *arr = (td_array_t*)v->object;
    npy_intp arr_shape[arr->ndims];
    for (size_t i=0; i<arr->ndims; ++i) arr_shape[i] = arr->length;
    int np_type = td_type_to_numpy(arr->eltype);

    PyObject *py_obj = PyArray_SimpleNewFromData(arr->ndims,
                                     arr_shape,
                                     np_type,
                                     arr->data);
    return py_obj;
}

static PyObject *from_td_val(td_val_t *v)
{
    PyObject *pVal = NULL;
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
    case TD_ARRAY:
        return pyarray_from_td_val(v);
    default:
        td_error("Cannot convert td type to Python.\n");
        return pVal;
    }
}

// entry points
int func_module_name(char *fname, char *modname, char *funcname)
{
    unsigned len = strlen(fname);
    int i;

    for(i = len-1; i > 0; --i){
        if (fname[i] == '.'){
            break;
        }
    }
    if (i > 0) {
        strncpy(modname, fname, i);
        modname[i] = '\0';
        strcpy(funcname, fname+(i+1));
        return 1;
    } else {
        strcpy(modname, "__builtin__");
        strcpy(funcname, fname);
        return 1;
    }
    return 0;
}

PyObject* td_py_get_callable(char *fname)
{
    PyObject *pName, *pModule, *pFunc;
    char modname[128];
    char funcname[128];
    func_module_name(fname, modname, funcname);

    pName = PyString_FromString(modname);
    if (pName == NULL) {
        fprintf(stderr, "Error converting module name to PyString\n");
        return NULL;
    }

    pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    if (pModule == NULL) {
        fprintf(stderr, "Error importing module %s\n", modname);
        return NULL;
    }

    pFunc = PyObject_GetAttrString(pModule, funcname);
    Py_DECREF(pModule);
    if (pFunc == NULL || !PyCallable_Check(pFunc)){
        fprintf(stderr, "Could not get a callable %s\n", fname);
        if (pFunc != NULL) Py_DECREF(pFunc);
        return NULL;
    }

    return pFunc;
}

void td_py_add_arg(PyObject *pArgs, int pos, td_val_t *arg)
{
    PyObject* pValue;

    pValue = from_td_val(arg);
    if (pValue == NULL)  {
        fprintf(stderr, "Error in Python value conversion\n");
        return;
    }
    /* pValue reference stolen here: */
    PyTuple_SetItem(pArgs, pos, pValue);
}

void td_py_call(td_val_t *out, PyObject *pArgs, char *fname)
{
    PyObject *pFunc, *pValue;

    pFunc = td_py_get_callable(fname);
    if (pFunc == NULL) return;
   
    pValue = PyObject_CallObject(pFunc, pArgs);
    if (pValue == NULL) {
        fprintf(stderr, "Error in Python call %s\n", fname);
        return;
    }
    to_td_val(out, pValue);
    Py_DECREF(pFunc);
    Py_DECREF(pValue);
}

void td_py_invoke0(td_val_t *out, char *fname)
{
    PyObject *pArgs;

    pArgs = PyTuple_New(0);
    td_py_call(out, pArgs, fname);
    Py_DECREF(pArgs);
}

void td_py_invoke1(td_val_t *out, char *fname, td_val_t *arg)
{
    PyObject *pArgs;

    pArgs = PyTuple_New(1);
    td_py_add_arg(pArgs, 0, arg);
    td_py_call(out, pArgs, fname);
    Py_DECREF(pArgs);
}

void td_py_invoke2(td_val_t *out, char *fname, td_val_t *arg1,td_val_t *arg2)
{
    PyObject *pArgs;

    pArgs = PyTuple_New(2);
    td_py_add_arg(pArgs, 0, arg1);
    td_py_add_arg(pArgs, 1, arg2);
    td_py_call(out, pArgs, fname);
    Py_DECREF(pArgs);
}

void td_py_invoke3(td_val_t *out, char *fname, td_val_t *arg1,td_val_t *arg2,
		   td_val_t *arg3)
{
    PyObject *pArgs;

    pArgs = PyTuple_New(3);
    td_py_add_arg(pArgs, 0, arg1);
    td_py_add_arg(pArgs, 1, arg2);
    td_py_add_arg(pArgs, 2, arg3);
    td_py_call(out, pArgs, fname);
    Py_DECREF(pArgs);
}


void td_py_invoke4(td_val_t *out, char *fname, td_val_t *arg1,td_val_t *arg2,
		   td_val_t *arg3, td_val_t *arg4)
{
    PyObject *pArgs;

    pArgs = PyTuple_New(4);
    td_py_add_arg(pArgs, 0, arg1);
    td_py_add_arg(pArgs, 1, arg2);
    td_py_add_arg(pArgs, 2, arg3);
    td_py_add_arg(pArgs, 3, arg4);
    td_py_call(out, pArgs, fname);
    Py_DECREF(pArgs);
}


void td_py_invoke5(td_val_t *out, char *fname, td_val_t *arg1,td_val_t *arg2,
		   td_val_t *arg3, td_val_t *arg4, td_val_t *arg5)
{
    PyObject *pArgs;

    pArgs = PyTuple_New(5);
    td_py_add_arg(pArgs, 0, arg1);
    td_py_add_arg(pArgs, 1, arg2);
    td_py_add_arg(pArgs, 2, arg3);
    td_py_add_arg(pArgs, 3, arg4);
    td_py_add_arg(pArgs, 4, arg5);
    td_py_call(out, pArgs, fname);
    Py_DECREF(pArgs);
}

void td_py_invoke_graph1(graph_t *out_graph, char *fname, graph_t *in_graph)
{
    PyObject *pFunc, *pArgs, *pValue;

    pFunc = td_py_get_callable(fname);
    if (pFunc == NULL) {
      printf("Error getting pyhton function: %s\n", fname);
      return;
    }

    pArgs = PyTuple_New(2);
    pValue = PyLong_FromSize_t((size_t) out_graph);
    if (pValue == NULL) {
      printf("Error getting python args: in_graph\n");
      return;
    }
    PyTuple_SetItem(pArgs, 0, pValue);

    
    pValue = PyLong_FromSize_t((size_t) in_graph);
    if (pValue == NULL) {
      printf("Error getting python args: in_graph\n");
      return;
    }
    PyTuple_SetItem(pArgs, 1, pValue);

  
    pValue = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pFunc);
    Py_DECREF(pArgs);
    if (pValue == NULL) {
        fprintf(stderr, "Error in Python call %s\n", fname);
        return;
    }
    Py_DECREF(pValue);

}

void td_py_invoke_graph_and_csc(td_val_t *out, char *fname, graph_t *in_graph, 
				int *in_csc_offsets, int *in_csc_indices)
			       
{
    PyObject *pArgs;

    td_array_t node_array = { .data=in_graph->nodeNames, .length=in_graph->numNodes,
			      .eltype=TD_INT64, .ndims=1 };
    td_val_t nodes = { .tag = TD_ARRAY, .object = &node_array };

    td_array_t csr_offset_array = { .data=in_graph->rowOffsets, .length=in_graph->numNodes+1,
				    .eltype=TD_INT32, .ndims=1 };
    td_val_t csr_offsets = { .tag = TD_ARRAY, .object = &csr_offset_array };
 
    td_array_t csr_indices_array = { .data=in_graph->colIndices, .length=in_graph->numEdges,
				     .eltype=TD_INT32, .ndims=1 };
    td_val_t csr_indices = { .tag = TD_ARRAY, .object = &csr_indices_array };
 
    td_array_t csc_offset_array = { .data=in_csc_offsets, .length=in_graph->numNodes+1,
				    .eltype=TD_INT32, .ndims=1 };
    td_val_t csc_offsets = { .tag = TD_ARRAY, .object = &csc_offset_array };

    td_array_t csc_indices_array = { .data=in_csc_indices, .length=in_graph->numEdges,
				     .eltype=TD_INT32, .ndims=1 };
    td_val_t csc_indices = { .tag = TD_ARRAY, .object = &csc_indices_array };

    pArgs = PyTuple_New(5);
    td_py_add_arg(pArgs, 0, &nodes);
    td_py_add_arg(pArgs, 1, &csr_offsets);
    td_py_add_arg(pArgs, 2, &csr_indices);
    td_py_add_arg(pArgs, 3, &csc_offsets);
    td_py_add_arg(pArgs, 4, &csc_indices);
    td_py_call(out, pArgs, fname);
    Py_DECREF(pArgs);
}

void td_py_eval(td_val_t *out, char *str)
{
    PyObject *v = PyRun_String(str, 0, NULL, NULL);
    to_td_val(out, v);
}

td_tag_t td_py_get_type(void *v)
{
    return py_type_to_td((PyObject*) v);
}

td_tag_t td_py_get_eltype(void *v)
{
    if (PyArray_Check( (PyObject *) v)){
        PyArrayObject *arr = (PyArrayObject *) v;
        return numpy_type_to_td(PyArray_TYPE(arr));
    }
    return TD_UNKNOWN;
}

void *td_py_get_dataptr(void *v)
{
    if (PyArray_Check( (PyObject *) v))
        return PyArray_DATA( (PyArrayObject *) v);
    return NULL;
}

size_t td_py_get_length(void *v)
{
    if (PyArray_Check( (PyObject *) v))
        return PyArray_SIZE( (PyArrayObject *)v);
    return 1;
}

size_t td_py_get_ndims(void *v)
{
    if (PyArray_Check( (PyObject *) v))
        return PyArray_NDIM( (PyArrayObject *)v);
    return 0;
}

// initialization

void td_py_init(char *homedir)
{
    Py_Initialize();
    import_array();

    td_env_t *env = (td_env_t*)malloc(sizeof(td_env_t));
    env->name = "python";

    env->eval = &td_py_eval;
    env->invoke0 = &td_py_invoke0;
    env->invoke1 = &td_py_invoke1;
    env->invoke2 = &td_py_invoke2;
    env->invoke3 = &td_py_invoke3;
    env->invoke4 = &td_py_invoke4;
    env->invoke5 = &td_py_invoke5;
    env->invokeGraph1 = &td_py_invoke_graph1;
    env->invokeGraphAndCSC = &td_py_invoke_graph_and_csc;
    //env->retain
    //env->release

    env->get_type = &td_py_get_type;
    env->get_eltype = &td_py_get_eltype;
    env->get_dataptr = &td_py_get_dataptr;
    env->get_length = &td_py_get_length;
    env->get_ndims = &td_py_get_ndims;

    //env->get_dims
    //env->get_strides

    td_provide_python(env);
}

