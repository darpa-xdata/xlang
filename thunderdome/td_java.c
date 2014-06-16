#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <jni.h> // this is found under the JVM include directory, e.g. on windows C:\Program Files\Java\jdk1.8.0_05\include

#include "td.h"

JNIEnv *persistentJNI;
// type mapping

JNIEnv* create_vm(JavaVM ** jvm) {

    JNIEnv *env;
    JavaVMInitArgs vm_args;
    JavaVMOption options;
    options.optionString = "-Djava.class.path=out"; //Path to the java source code
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


char setReturnTdVal(const char* ret) {
	if (strcmp(ret, "boolean") == 0) {
		return 'Z'; // how do we represent a null?
	} else if (strcmp(ret, "byte") == 0) {
		return 'B';
	} else if (strcmp(ret, "char") == 0)
		return 'C';
	else if (strcmp(ret, "double") == 0)
		return 'D';
	else if (strcmp(ret, "float") == 0)
		return 'F';
	else if (strcmp(ret, "int") == 0)
		return 'I';
	else if (strcmp(ret, "long") == 0)
		return 'J';
	else if (strcmp(ret, "object") == 0)
		return 'L';
	else if (strcmp(ret, "short") == 0)
		return 'S';
	else if (strcmp(ret, "void") == 0)
		return 'V';
	else if (strcmp(ret, "array") == 0)
		return '[';
	else
		return '?';
}
/**
 * boolean      Z
 byte         B
 char         C
 double       D
 float        F
 int          I
 long         J
 object       L
 short        S
 void         V
 array        [

 * @param method
 * @return
 */
void td_java_invoke0(td_val_t *out, char *fname)
{
	const char* mainClass = "xlang/java/Xlang";
	jclass clsH = (*persistentJNI)->FindClass(persistentJNI, mainClass);
	if (clsH == NULL) {
		printf("can't find Xlang class?\n");
		return;

	}
	//printf ("found Xlang class %s \n", mainClass);


	jmethodID returnType = (*persistentJNI)->GetStaticMethodID(persistentJNI, clsH, "getReturnType", "(Ljava/lang/String;)Ljava/lang/String;");

	if (returnType == NULL) {
		printf("can't find Xlang method %s?\n","returnType");
		return;

	}
	printf("found return type method for %s\n",fname);

	jstring message = (*persistentJNI)->NewStringUTF(persistentJNI, fname);
	jstring string = (*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, returnType,message);

//	printf("got response");

    const char* returnString = (*persistentJNI)->GetStringUTFChars(persistentJNI,string, 0);

    printf("return type for %s is %s\n",fname,returnString);

    (*persistentJNI)->ReleaseStringUTFChars(persistentJNI,string, returnString);
    (*persistentJNI)->DeleteLocalRef(persistentJNI,string);

//	printf("can't find Xlang method %s?\n","returnType");
    char buf[512];
    char rc = setReturnTdVal(returnString);
  //  sprintf(buf,"([Ljava/lang/String;)%c",rc);
    sprintf(buf,"()%c",rc);
    	printf("signature is %s\n",buf);

	jmethodID midMain = (*persistentJNI)->GetStaticMethodID(persistentJNI, clsH, fname, buf);

	if (midMain == NULL) {
		printf("can't find Xlang method %s?\n",fname);
		return;

	}
	printf ("found Xlang method %s\n",fname);


	if (strcmp(returnString, "boolean") == 0) {
		out->int32_val = (*persistentJNI)->CallStaticBooleanMethod(persistentJNI, clsH, midMain);
		out->tag = TD_INT32;
	} else if (strcmp(returnString, "byte") == 0) {
		out->int8_val = (*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, midMain);
		out->tag = TD_INT8;

	} else if (strcmp(returnString, "char") == 0) {
		out->int8_val =(*persistentJNI)->CallStaticCharMethod(persistentJNI, clsH, midMain);
		out->tag = TD_INT8;

	} else if (strcmp(returnString, "double") == 0) {
		out->double_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, midMain);
		out->tag = TD_DOUBLE;

	} else if (strcmp(returnString, "float") == 0) {
		out->float_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, midMain);
		out->tag = TD_FLOAT;
	} else if (strcmp(returnString, "int") == 0) {
		out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, midMain);
		out->tag = TD_INT32;
	} else if (strcmp(returnString, "long") == 0) {
		out->int64_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, midMain);
		out->tag = TD_UINT64;
	} else if (strcmp(returnString, "object") == 0) {
		out->object =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, midMain);
		out->tag = TD_OBJECT;
	} else if (strcmp(returnString, "short") == 0) {
		out->int16_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, midMain);
		out->tag = TD_INT16;

	} else if (strcmp(returnString, "void") == 0) {
		(*persistentJNI)->CallStaticVoidMethod(persistentJNI, clsH, midMain);
		out->tag = TD_UNKNOWN;

	} else if (strcmp(returnString, "array") == 0) {
		out->ptr_val =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, midMain);

		out->tag = TD_ARRAY;

	}	else
		printf("error: unknown return value %s\n",returnString);

	printf ("called Xlang method %s\n",fname);

	// java_function_t *f = java_get_function(java_base_module, fname);
   // java_value_t *v = java_call0(f);
  //  to_td_val(out, v);
}

void td_java_invoke1(td_val_t *out, char *fname, td_val_t *arg)
{
	const char* mainClass = "xlang/java/Xlang";
	jclass clsH = (*persistentJNI)->FindClass(persistentJNI, mainClass);
	if (clsH == NULL) {
		printf("can't find Xlang class?\n");
		return;

	}
	//printf ("found Xlang class %s \n", mainClass);
	jmethodID midMain = NULL;
	// for now static int that takes an int and returns one
	midMain       = (*persistentJNI)->GetStaticMethodID(persistentJNI, clsH, fname, "(I)I");

	if (midMain == NULL) {
		printf("can't find Xlang method %s?\n",fname);
		return;

	}
	printf ("found Xlang method %s\n",fname);

	jint val = (*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, midMain,arg->int32_val);
	   printf("In C, the int is %d\n", val);
	printf ("called Xlang method %s\n",fname);
    out->int32_val = val;
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
    td_env_t *env = get_java();

    td_provide_java(env);
}

td_env_t *get_java() {

	JNIEnv *jniEnv;

	JavaVM * jvm;

	if (persistentJNI != NULL) {
		jniEnv = persistentJNI;
	}
	else {
		jniEnv = create_vm(&jvm);
		persistentJNI = jniEnv;
	}
	if (jniEnv == NULL)
		return NULL;
	else {
		printf(" made java vm!\n");
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

	persistentJNI = jniEnv;
	return env;

}
