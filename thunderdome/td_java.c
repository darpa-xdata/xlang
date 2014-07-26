#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <jni.h> // this is found under the JVM include directory, e.g. on windows C:\Program Files\Java\jdk1.8.0_05\include

#include "td.h"

static const char* XLANG_JAVA_GRAPH = "xlang/java/Graph";
JNIEnv *persistentJNI;
const char *persistentClass;
int debug = 0;
// type mapping

JNIEnv* create_vm(JavaVM ** jvm, const char *classpath) {

    JNIEnv *env;
    JavaVMInitArgs vm_args;
    JavaVMOption options;
    char buf[1024];
    sprintf(buf,"-Djava.class.path=%s",classpath);
   //  char *bptr = &buf;
    options.optionString = &buf; //Path to the java source code
    vm_args.version = JNI_VERSION_1_6; //JDK version. This indicates version 1.6
    vm_args.nOptions = 1;
    vm_args.options = &options;
    vm_args.ignoreUnrecognized = 0;

    int ret = JNI_CreateJavaVM(jvm, (void**)&env, &vm_args);
    if(ret < 0)
    	printf("\nERROR: Unable to Launch JVM\n");
	return env;
}

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
	else {
		return '?';
	}
}

/**
 *     // primitive types
    TD_INT8=0, TD_UINT8, TD_INT16, TD_UINT16,
    TD_INT32, TD_UINT32, TD_INT64, TD_UINT64,
    TD_FLOAT, TD_DOUBLE, TD_PTR,

    // aggregate types
    TD_UTF8, TD_ARRAY, TD_STRUCT,

    // environment-specific objects (can be queried)
    TD_OBJECT,

    TD_UNKNOWN,
    TD_MAX_TAG
 */
char getArgFromType(td_val_t *arg) {
	switch(td_typeof(arg)) {
	case TD_INT8 : return 'B';
	case TD_UINT8 : return 'B';
	case TD_INT16 : return 'S';
	case TD_UINT16 : return 'S';
	case TD_INT32 : return 'I';
	case TD_UINT32 : return 'I';
	case TD_INT64 : return 'J';
	case TD_UINT64 : return 'J';
	case TD_FLOAT : return 'F';
	case TD_DOUBLE : return 'D';
	case TD_OBJECT : return 'L';
	case TD_ARRAY : return '[';
	//case TD_UTF8 : return 'Ljava/lang/String;';
	}
}

jintArray makeIntArray( int length, int * values) {
	JNIEnv *env = persistentJNI;
	jintArray outJNIArray = (*env)->NewIntArray(env, length);
	(*env)->SetIntArrayRegion(env, outJNIArray, 0, length, values);  // copy
	return outJNIArray;
}

jdoubleArray makeDoubleArray( int length, double * values) {
	JNIEnv *env = persistentJNI;
	jintArray outJNIArray = (*env)->NewDoubleArray(env, length);
	(*env)->SetDoubleArrayRegion(env, outJNIArray, 0, length, values);  // copy
	return outJNIArray;
}

jobjectArray makeStringArray( int length, td_string_t ** values) {
	JNIEnv *env = persistentJNI;
	jclass classString = (*env)->FindClass(env, "java/lang/String");
	jobjectArray outJNIArray = (*env)->NewObjectArray(env, length,classString,NULL);
	int i = 0;
	for (; i < length; i++) {
		jstring val = (*env)->NewStringUTF(env, (char *) values[i]->data);
		(*env)->SetObjectArrayElement(env, outJNIArray, i, val);
	}

	return outJNIArray;
}


// TODO: if TD_UTF8 - release the string we pass in...
void setValueFromArgument(td_val_t *arg, jvalue * val) {
	switch (td_typeof(arg)) {
	case TD_INT8:
		val->b = arg->int8_val;
		break;
	case TD_UINT8:
		val->b = arg->int8_val;
		break;
	case TD_INT16:
		val->s = arg->int16_val;
		break;
	case TD_UINT16:
		val->s = arg->uint16_val;
		break;
	case TD_INT32:
		val->i = arg->int32_val;
		break;
	case TD_UINT32:
		val->i = arg->uint32_val;
		break;
	case TD_INT64:
		val->j = arg->int64_val;
		break;
	case TD_UINT64:
		val->j = arg->uint64_val;
		break;
	case TD_FLOAT:
		val->f = arg->float_val;
		break;
	case TD_DOUBLE:
		val->d = arg->double_val;
		break;
	case TD_OBJECT:
		val->l = arg->object;
		break;
	case TD_ARRAY:
		val->l = arg->object;

		td_array_t* arr = (td_array_t *) arg->object;
		//char * argType= "Ljava/lang/Integer;";
		switch(arr->eltype) {
		case TD_INT32:
			val->l = makeIntArray(arr->length,(int*)arr->data);
			break;
		case TD_DOUBLE:
			val->l = makeDoubleArray(arr->length,(double*)arr->data);
			break;
		case TD_UTF8:
			val->l = makeStringArray(arr->length,(td_string_t**)arr->data);
			break;
			// TODO : fill in all the rest
		}

		break;
	case TD_UTF8:
		val->l = (*persistentJNI)->NewStringUTF(persistentJNI,(char *)((td_string_t *)arg->object)->data);
		break;
	}
}

// ask the Xlang class to get the return type for function fname in class clsH
jstring getReturnType(char** fname, jclass* clsH) {
	jclass clsXlang = (*persistentJNI)->FindClass(persistentJNI, "xlang/java/Xlang");
	if (clsXlang == NULL) {
		printf("getReturnType can't find %s class?\n", "xlang/java/Xlang");
		//return;
	}
	jmethodID returnType = (*persistentJNI)->GetStaticMethodID(persistentJNI, clsXlang, "getReturnTypeInClass",
			"(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
	if (returnType == NULL) {
		printf("getReturnType can't find Xlang method %s?\n", "getReturnTypeInClass");
		return NULL;
	}
	//printf("found return type method for %s\n",fname);
	jstring methodName = (*persistentJNI)->NewStringUTF(persistentJNI, *fname);
	jstring className  = (*persistentJNI)->NewStringUTF(persistentJNI, persistentClass);
	jstring string = (*persistentJNI)->CallStaticObjectMethod(persistentJNI,*clsH, returnType, className, methodName);
	return string;
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
	jclass clsH = (*persistentJNI)->FindClass(persistentJNI, persistentClass);
	if (clsH == NULL) {
		printf("td_java_invoke0 can't find %s class?\n", persistentClass);
		return;
	}

	jstring string = getReturnType(&fname, &clsH);
    const char* returnString = (*persistentJNI)->GetStringUTFChars(persistentJNI,string, 0);

    //printf("return type for %s is %s\n",fname,returnString);

    char buf[512];
    char rc = setReturnTdVal(returnString);
    if (rc == '?') {
    	sprintf(buf,"()L%s",returnString);
    }
    else {
    	sprintf(buf,"()%c",rc);
    }
  	//printf("signature is %s\n",buf);

	jmethodID midMain = (*persistentJNI)->GetStaticMethodID(persistentJNI, clsH, fname, buf);

	if (midMain == NULL) {
		printf("ERROR : can't find method %s with signature %s in %s?\n",fname,buf,persistentClass);
		out->tag = TD_UNKNOWN;
		return;
	}
	//printf ("found Xlang method %s\n",fname);

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
	} else {
		printf("error: unknown return value %s\n",returnString);
	}

	(*persistentJNI)->ReleaseStringUTFChars(persistentJNI,string, returnString);
	(*persistentJNI)->DeleteLocalRef(persistentJNI,string);

	//printf ("called Xlang method %s\n",fname);
}

void getArgPrefix(char *prefix, td_val_t* arg) {
	//char prefix[512];
	char argC = getArgFromType(arg);
	if (td_typeof(arg) == TD_UTF8) {
		strcpy(prefix,"Ljava/lang/String;");
	} else {
		if (argC == '[') {
			td_array_t* arr = (td_array_t *) arg->object;
			char * argType = "Ljava/lang/Integer;";
			switch (arr->eltype) {
			case TD_INT32:
				argType = "I";
				break;
			case TD_DOUBLE:
				argType = "D";
				break;
			case TD_UTF8:
				argType = "Ljava/lang/String;";
				break;
				// TODO : fill in all the rest
			}
			sprintf(prefix, "[%s", argType);

			/*	if (rc == '?') {
			 sprintf(buf, "([%s)L%s;", argType, returnString);
			 }
			 else {
			 sprintf(buf, "([%s)%c", argType, rc);
			 }*/
		} else {
			sprintf(prefix, "%c", argC);
		}
	}
	//return prefix;
}

void getSignature(const char* returnString, char *buf,  td_val_t* arg) {
	char rc = setReturnTdVal(returnString);
	char prefix[1024] = "";

	int i = 0;
	getArgPrefix(prefix, arg);

	if (rc == '?') {
		sprintf(buf, "(%s)L%s;", prefix, returnString);
	}
	else {
		sprintf(buf, "(%s)%c", prefix, rc);
	}
}

void getSignatureMany(const char* returnString, char *buf, int len, td_val_t** arg) {
	char rc = setReturnTdVal(returnString);
	char prefix[1024] = "";

	int i = 0;
	for (;i < len; i++) {
		char prefix2[512];

		getArgPrefix(prefix2, arg[i]);
		 strcat(prefix, prefix2);
	}
	if (rc == '?') {
		sprintf(buf, "(%s)L%s;", prefix, returnString);
	}
	else {
		sprintf(buf, "(%s)%c", prefix, rc);
	}
}

// OK make the return value and the string on the stack
// copy the value from jvm side to the C side and discard the java string
void getString(jstring returnValue, JNIEnv* persistentJNI, td_val_t* out) {
	td_string_t *str = (td_string_t*)malloc(sizeof(td_string_t));
	const char* returnString = (*persistentJNI)->GetStringUTFChars(persistentJNI, returnValue, 0);
	str->length = strlen(returnString);

	str->data = malloc(sizeof(char)*str->length+1); // add space for trailing 0
	strcpy((char *)str->data, returnString);
	out->ptr_val = str;
	out->tag = TD_UTF8;
	(*persistentJNI)->ReleaseStringUTFChars(persistentJNI, returnValue,	returnString);
	(*persistentJNI)->DeleteLocalRef(persistentJNI, returnValue); // not sure if this is required

}

char * getStringSimple(jstring returnValue) {
	//td_string_t *str = (td_string_t*)malloc(sizeof(td_string_t));
	const char* returnString = (*persistentJNI)->GetStringUTFChars(persistentJNI, returnValue, 0);
	//str->length = strlen(returnString);
	int len = strlen(returnString);
	char * str = malloc(sizeof(char)*(len+1)); // add space for trailing 0
	strcpy(str, returnString);

	(*persistentJNI)->ReleaseStringUTFChars(persistentJNI, returnValue,	returnString);
	(*persistentJNI)->DeleteLocalRef(persistentJNI, returnValue); // not sure if this is required

	return str;

}

//! call a static java function with one parameter -- determine the method signature from the arguments here
// this can fail in a number of ways -- if the function doesn't exist in the class, or the function is there
// but it doesn't have the requested signature.
// Does introspection on method to figure out the return type for out
// Not really sure what we do to support passing in or returning objects, unless they're strings.
/*!
\param out stores the returned value - type determined by asking java what the return type is
\param fname name of the function
\param arg look at the type to determine method signature
*/
void td_java_invoke1(td_val_t *out, char *fname, td_val_t *arg)
{
	// 1) Find the class we want to use
	jclass clsH = (*persistentJNI)->FindClass(persistentJNI, persistentClass);
	if (clsH == NULL) {
		printf("can't find Xlang class?\n");
		return;
	}

	// 2) Find the return type for the method we want to call
	jstring returnTypeMethod = getReturnType(&fname, &clsH);
	const char* returnString = (*persistentJNI)->GetStringUTFChars(persistentJNI, returnTypeMethod, 0);

	if (debug) printf("return type for %s is %s\n",fname,returnString);

	// 3) Get the method signature implied by the argument and the return type.
    char signature[512];
	getSignature(returnString, signature, arg);
	if (debug) printf("signature is %s\n",signature);

	// 4) Get the method given the name and signature
	jmethodID methodToCall = (*persistentJNI)->GetStaticMethodID(persistentJNI, clsH, fname, signature);

	if (methodToCall == NULL) {
		printf("ERROR : can't find method %s with signature %s in %s?\n", fname, signature, persistentClass);
		out->tag = TD_UNKNOWN;
		return;
	}

    jvalue val;
	setValueFromArgument(arg, &val);

	// 5) Finally lets call the method with the right type, depending on the argument type and grap the return value

	if (strcmp(returnString, "boolean") == 0) {
		// if there's a better way than copying all this code I'd love to know -- macro?
		switch (td_typeof(arg)) {
		case TD_INT8:
		case TD_UINT8:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.b);
			break;
		case TD_INT16:
		case TD_UINT16:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.s);
			break;
		case TD_INT32:
		case TD_UINT32:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
			break;
		case TD_INT64:
		case TD_UINT64:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.j);
			break;
		case TD_FLOAT:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.f);
			break;
		case TD_DOUBLE:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.d);
			break;
		case TD_UTF8:
		case TD_ARRAY:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.l);
			break;
		}

		out->tag = TD_INT32;
	} else if (strcmp(returnString, "byte") == 0) {
		switch (td_typeof(arg)) {
		case TD_INT8:
		case TD_UINT8:
			out->int8_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.b);
			break;
		case TD_INT16:
		case TD_UINT16:
			out->int8_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.s);
			break;
		case TD_INT32:
		case TD_UINT32:
			out->int8_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.i);
			break;
		case TD_INT64:
		case TD_UINT64:
			out->int8_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.j);
			break;
		case TD_FLOAT:
			out->int8_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.f);
			break;
		case TD_DOUBLE:
			out->int8_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.d);
			break;
		case TD_UTF8:
		case TD_ARRAY:
			out->int8_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.l);
			break;
		}
		out->tag = TD_INT8;
	} else if (strcmp(returnString, "char") == 0) {
		switch (td_typeof(arg)) {
		case TD_INT8:
		case TD_UINT8:
			out->int8_val =(*persistentJNI)->CallStaticCharMethod(persistentJNI, clsH, methodToCall,val.b);
			break;
		case TD_INT16:
		case TD_UINT16:
			out->int8_val =(*persistentJNI)->CallStaticCharMethod(persistentJNI, clsH, methodToCall,val.s);
			break;
		case TD_INT32:
		case TD_UINT32:
			out->int8_val =(*persistentJNI)->CallStaticCharMethod(persistentJNI, clsH, methodToCall,val.i);
			break;
		case TD_INT64:
		case TD_UINT64:
			out->int8_val =(*persistentJNI)->CallStaticCharMethod(persistentJNI, clsH, methodToCall,val.j);
			break;
		case TD_FLOAT:
			out->int8_val =(*persistentJNI)->CallStaticCharMethod(persistentJNI, clsH, methodToCall,val.f);
			break;
		case TD_DOUBLE:
			out->int8_val =(*persistentJNI)->CallStaticCharMethod(persistentJNI, clsH, methodToCall,val.d);
			break;
		case TD_UTF8:
		case TD_ARRAY:
			out->int8_val =(*persistentJNI)->CallStaticCharMethod(persistentJNI, clsH, methodToCall,val.l);
			break;
		}
		out->tag = TD_INT8;
	} else if (strcmp(returnString, "double") == 0) {
		switch (td_typeof(arg)) {
		case TD_INT8:
		case TD_UINT8:
			out->double_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.b);
			break;
		case TD_INT16:
		case TD_UINT16:
			out->double_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.s);
			break;
		case TD_INT32:
		case TD_UINT32:
			out->double_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.i);
			break;
		case TD_INT64:
		case TD_UINT64:
			out->double_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.j);
			break;
		case TD_FLOAT:
			out->double_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.f);
			break;
		case TD_DOUBLE:
			out->double_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.d);
			break;
		case TD_UTF8:
		case TD_ARRAY:
			out->double_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.l);
			break;
		}

		out->tag = TD_DOUBLE;
	} else if (strcmp(returnString, "float") == 0) {
		switch (td_typeof(arg)) {
		case TD_INT8:
		case TD_UINT8:
			out->float_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.b);
			break;
		case TD_INT16:
		case TD_UINT16:
			out->float_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.s);
			break;
		case TD_INT32:
		case TD_UINT32:
			out->float_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.i);
			break;
		case TD_INT64:
		case TD_UINT64:
			out->float_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.j);
			break;
		case TD_FLOAT:
			out->float_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.f);
			break;
		case TD_DOUBLE:
			out->float_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.d);
			break;
		case TD_UTF8:
		case TD_ARRAY:
			out->float_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.l);
			break;
		}
		out->tag = TD_FLOAT;
	} else if (strcmp(returnString, "int") == 0) {
		switch (td_typeof(arg)) {
		case TD_INT8:
		case TD_UINT8:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.b);
			break;
		case TD_INT16:
		case TD_UINT16:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.s);
			break;
		case TD_INT32:
		case TD_UINT32:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
			break;
		case TD_INT64:
		case TD_UINT64:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.j);
			break;
		case TD_FLOAT:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.f);
			break;
		case TD_DOUBLE:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.d);
			break;
		case TD_UTF8:
		case TD_ARRAY:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.l);
			break;
		}

		out->tag = TD_INT32;
	} else if (strcmp(returnString, "long") == 0) {
		switch (td_typeof(arg)) {
		case TD_INT8:
		case TD_UINT8:
			out->int64_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.b);
			break;
		case TD_INT16:
		case TD_UINT16:
			out->int64_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.s);
			break;
		case TD_INT32:
		case TD_UINT32:
			out->int64_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.i);
			break;
		case TD_INT64:
		case TD_UINT64:
			out->int64_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.j);
			break;
		case TD_FLOAT:
			out->int64_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.f);
			break;
		case TD_DOUBLE:
			out->int64_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.d);
			break;
		case TD_UTF8:
		case TD_ARRAY:
			out->int64_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.l);
			break;
		}
		out->tag = TD_INT64;
	} else if (strcmp(returnString, "object") == 0) {
		out->object =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val);
		out->tag = TD_OBJECT;
	} else if (strcmp(returnString, "short") == 0) {
		switch (td_typeof(arg)) {
		case TD_INT8:
		case TD_UINT8:
			out->int16_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.b);
			break;
		case TD_INT16:
		case TD_UINT16:
			out->int16_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.s);
			break;
		case TD_INT32:
		case TD_UINT32:
			out->int16_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.i);
			break;
		case TD_INT64:
		case TD_UINT64:
			out->int16_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.j);
			break;
		case TD_FLOAT:
			out->int16_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.f);
			break;
		case TD_DOUBLE:
			out->int16_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.d);
			break;
		case TD_UTF8:
		case TD_ARRAY:
			out->int16_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.l);
			break;
		}
		out->tag = TD_INT16;
	} else if (strcmp(returnString, "void") == 0) { // TODO: no good mapping for a void function
		(*persistentJNI)->CallStaticVoidMethod(persistentJNI, clsH, methodToCall,val);
		out->tag = TD_UNKNOWN;
	} else if (strcmp(returnString, "array") == 0) {
		out->ptr_val =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val);
		out->tag = TD_ARRAY;
	} else if (strcmp(returnString, "java/lang/String") == 0) {
		jstring returnValue;
		out->tag = TD_UNKNOWN;
		switch (td_typeof(arg)) {
		case TD_INT8:
		case TD_UINT8:
			returnValue = (*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val.b);
			getString(returnValue, persistentJNI, out);
			break;
		case TD_INT16:
		case TD_UINT16:
			returnValue =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val.s);
			getString(returnValue, persistentJNI, out);
			break;
		case TD_INT32:
		case TD_UINT32:
			returnValue =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val.i);
			getString(returnValue, persistentJNI, out);
			break;
		case TD_INT64:
		case TD_UINT64:
			returnValue =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val.j);
			getString(returnValue, persistentJNI, out);
			break;
		case TD_FLOAT:
			returnValue =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val.f);
			getString(returnValue, persistentJNI, out);
			break;
		case TD_DOUBLE:
			returnValue =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val.d);
			getString(returnValue, persistentJNI, out);
			break;
		case TD_UTF8:
			returnValue =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val.l);
			getString(returnValue, persistentJNI, out);
			break;
		}
	} else {
		printf("error: unknown return value %s\n",returnString);
	}

	// free the return string we got from asking for the return type
	(*persistentJNI)->ReleaseStringUTFChars(persistentJNI,returnTypeMethod, returnString);
	(*persistentJNI)->DeleteLocalRef(persistentJNI,returnTypeMethod);
}

void td_java_invoke2(td_val_t *out, char *fname, td_val_t *arg, td_val_t *second)
{
	// 1) Find the class we want to use
	jclass clsH = (*persistentJNI)->FindClass(persistentJNI, persistentClass);
	if (clsH == NULL) {
		printf("can't find Xlang class?\n");
		return;
	}

	// 2) Find the return type for the method we want to call
	jmethodID returnType = (*persistentJNI)->GetStaticMethodID(persistentJNI, clsH, "getReturnType", "(Ljava/lang/String;)Ljava/lang/String;");

	if (returnType == NULL) {
		printf("td_java_invoke2 : can't find Xlang method %s?\n","returnType");
		return;

	}
	if (debug) printf("found return type method for %s\n",fname);

	jstring message = (*persistentJNI)->NewStringUTF(persistentJNI, fname);
	jstring returnTypeMethod = (*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, returnType, message);

	const char* returnString = (*persistentJNI)->GetStringUTFChars(persistentJNI, returnTypeMethod, 0);

	if (debug) printf("return type for %s is %s\n",fname,returnString);

	// 3) Get the method signature implied by the argument and the return type.
    char signature[1024];
    td_val_t* vals[2] = {arg,second};
	getSignatureMany(returnString, signature,2, vals);
	if (debug) printf("signature is %s\n",signature);

	// 4) Get the method given the name and signature
	jmethodID methodToCall = (*persistentJNI)->GetStaticMethodID(persistentJNI, clsH, fname, signature);

	if (methodToCall == NULL) {
		printf("ERROR : can't find Xlang method %s with signature %s?\n", fname, signature);
		out->tag = TD_UNKNOWN;
		return;
	}

	// TODO : make an array of jvalues to pass in
    jvalue val;
	setValueFromArgument(arg, &val);

	// 5) Finally lets call the method with the right type, depending on the argument type and grap the return value

	if (strcmp(returnString, "boolean") == 0) {
		// if there's a better way than copying all this code I'd love to know -- macro?
		switch (td_typeof(arg)) {
		case TD_INT8:
		case TD_UINT8:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.b);
			break;
		case TD_INT16:
		case TD_UINT16:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.s);
			break;
		case TD_INT32:
		case TD_UINT32:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
			break;
		case TD_INT64:
		case TD_UINT64:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.j);
			break;
		case TD_FLOAT:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.f);
			break;
		case TD_DOUBLE:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.d);
			break;
		case TD_UTF8:
		case TD_ARRAY:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.l);
			break;
		}

		out->tag = TD_INT32;
	} else if (strcmp(returnString, "byte") == 0) {
		switch (td_typeof(arg)) {
		case TD_INT8:
		case TD_UINT8:
			out->int8_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.b);
			break;
		case TD_INT16:
		case TD_UINT16:
			out->int8_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.s);
			break;
		case TD_INT32:
		case TD_UINT32:
			out->int8_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.i);
			break;
		case TD_INT64:
		case TD_UINT64:
			out->int8_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.j);
			break;
		case TD_FLOAT:
			out->int8_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.f);
			break;
		case TD_DOUBLE:
			out->int8_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.d);
			break;
		case TD_UTF8:
		case TD_ARRAY:
			out->int8_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.l);
			break;
		}
		out->tag = TD_INT8;
	} else if (strcmp(returnString, "char") == 0) {
		switch (td_typeof(arg)) {
		case TD_INT8:
		case TD_UINT8:
			out->int8_val =(*persistentJNI)->CallStaticCharMethod(persistentJNI, clsH, methodToCall,val.b);
			break;
		case TD_INT16:
		case TD_UINT16:
			out->int8_val =(*persistentJNI)->CallStaticCharMethod(persistentJNI, clsH, methodToCall,val.s);
			break;
		case TD_INT32:
		case TD_UINT32:
			out->int8_val =(*persistentJNI)->CallStaticCharMethod(persistentJNI, clsH, methodToCall,val.i);
			break;
		case TD_INT64:
		case TD_UINT64:
			out->int8_val =(*persistentJNI)->CallStaticCharMethod(persistentJNI, clsH, methodToCall,val.j);
			break;
		case TD_FLOAT:
			out->int8_val =(*persistentJNI)->CallStaticCharMethod(persistentJNI, clsH, methodToCall,val.f);
			break;
		case TD_DOUBLE:
			out->int8_val =(*persistentJNI)->CallStaticCharMethod(persistentJNI, clsH, methodToCall,val.d);
			break;
		case TD_UTF8:
		case TD_ARRAY:
			out->int8_val =(*persistentJNI)->CallStaticCharMethod(persistentJNI, clsH, methodToCall,val.l);
			break;
		}
		out->tag = TD_INT8;
	} else if (strcmp(returnString, "double") == 0) {
		switch (td_typeof(arg)) {
		case TD_INT8:
		case TD_UINT8:
			out->double_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.b);
			break;
		case TD_INT16:
		case TD_UINT16:
			out->double_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.s);
			break;
		case TD_INT32:
		case TD_UINT32:
			out->double_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.i);
			break;
		case TD_INT64:
		case TD_UINT64:
			out->double_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.j);
			break;
		case TD_FLOAT:
			out->double_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.f);
			break;
		case TD_DOUBLE:
			out->double_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.d);
			break;
		case TD_UTF8:
		case TD_ARRAY:
			out->double_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.l);
			break;
		}

		out->tag = TD_DOUBLE;
	} else if (strcmp(returnString, "float") == 0) {
		switch (td_typeof(arg)) {
		case TD_INT8:
		case TD_UINT8:
			out->float_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.b);
			break;
		case TD_INT16:
		case TD_UINT16:
			out->float_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.s);
			break;
		case TD_INT32:
		case TD_UINT32:
			out->float_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.i);
			break;
		case TD_INT64:
		case TD_UINT64:
			out->float_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.j);
			break;
		case TD_FLOAT:
			out->float_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.f);
			break;
		case TD_DOUBLE:
			out->float_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.d);
			break;
		case TD_UTF8:
		case TD_ARRAY:
			out->float_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.l);
			break;
		}
		out->tag = TD_FLOAT;
	} else if (strcmp(returnString, "int") == 0) {
		switch (td_typeof(arg)) {
		case TD_INT8:
		case TD_UINT8:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.b);
			break;
		case TD_INT16:
		case TD_UINT16:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.s);
			break;
		case TD_INT32:
		case TD_UINT32:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
			break;
		case TD_INT64:
		case TD_UINT64:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.j);
			break;
		case TD_FLOAT:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.f);
			break;
		case TD_DOUBLE:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.d);
			break;
		case TD_UTF8:
		case TD_ARRAY:
			out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.l);
			break;
		}

		out->tag = TD_INT32;
	} else if (strcmp(returnString, "long") == 0) {
		switch (td_typeof(arg)) {
		case TD_INT8:
		case TD_UINT8:
			out->int64_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.b);
			break;
		case TD_INT16:
		case TD_UINT16:
			out->int64_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.s);
			break;
		case TD_INT32:
		case TD_UINT32:
			out->int64_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.i);
			break;
		case TD_INT64:
		case TD_UINT64:
			out->int64_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.j);
			break;
		case TD_FLOAT:
			out->int64_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.f);
			break;
		case TD_DOUBLE:
			out->int64_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.d);
			break;
		case TD_UTF8:
		case TD_ARRAY:
			out->int64_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.l);
			break;
		}
		out->tag = TD_INT64;
	} else if (strcmp(returnString, "object") == 0) {
		out->object =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val);
		out->tag = TD_OBJECT;
	} else if (strcmp(returnString, "short") == 0) {
		switch (td_typeof(arg)) {
		case TD_INT8:
		case TD_UINT8:
			out->int16_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.b);
			break;
		case TD_INT16:
		case TD_UINT16:
			out->int16_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.s);
			break;
		case TD_INT32:
		case TD_UINT32:
			out->int16_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.i);
			break;
		case TD_INT64:
		case TD_UINT64:
			out->int16_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.j);
			break;
		case TD_FLOAT:
			out->int16_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.f);
			break;
		case TD_DOUBLE:
			out->int16_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.d);
			break;
		case TD_UTF8:
		case TD_ARRAY:
			out->int16_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.l);
			break;
		}
		out->tag = TD_INT16;
	} else if (strcmp(returnString, "void") == 0) { // TODO: no good mapping for a void function
		(*persistentJNI)->CallStaticVoidMethod(persistentJNI, clsH, methodToCall,val);
		out->tag = TD_UNKNOWN;
	} else if (strcmp(returnString, "array") == 0) {
		out->ptr_val =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val);
		out->tag = TD_ARRAY;
	} else if (strcmp(returnString, "java/lang/String") == 0) {
		jstring returnValue;
		out->tag = TD_UNKNOWN;
		switch (td_typeof(arg)) {
		case TD_INT8:
		case TD_UINT8:
			returnValue = (*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val.b);
			getString(returnValue, persistentJNI, out);
			break;
		case TD_INT16:
		case TD_UINT16:
			returnValue =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val.s);
			getString(returnValue, persistentJNI, out);
			break;
		case TD_INT32:
		case TD_UINT32:
			returnValue =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val.i);
			getString(returnValue, persistentJNI, out);
			break;
		case TD_INT64:
		case TD_UINT64:
			returnValue =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val.j);
			getString(returnValue, persistentJNI, out);
			break;
		case TD_FLOAT:
			returnValue =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val.f);
			getString(returnValue, persistentJNI, out);
			break;
		case TD_DOUBLE:
			returnValue =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val.d);
			getString(returnValue, persistentJNI, out);
			break;
		case TD_UTF8:
			returnValue =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val.l);
			getString(returnValue, persistentJNI, out);
			break;
		}
	} else {
		printf("error: unknown return value %s\n",returnString);
	}

	// free the return string we got from asking for the return type
	(*persistentJNI)->ReleaseStringUTFChars(persistentJNI,returnTypeMethod, returnString);
	(*persistentJNI)->DeleteLocalRef(persistentJNI,returnTypeMethod);
}

void copyGraph(jobject graph, JNIEnv* persistentJNI, graph_t* out) {
	// Get the class
	jclass mvclass = (*persistentJNI)->GetObjectClass(persistentJNI, graph);
	// Get method ID for method getSomeDoubleArray that returns a double array
	// copy the node names
	jmethodID mid = (*persistentJNI)->GetMethodID(persistentJNI, mvclass,
			"getNodeNames", "()[Ljava/lang/String;");
	// Call the method, returns JObject (because Array is instance of Object)
	jobjectArray arr = (*persistentJNI)->CallObjectMethod(persistentJNI, graph,
			mid);
	// Get the elements (you probably have to fetch the length of the array as well
	int i = 0;
	jsize arrLength = (*persistentJNI)->GetArrayLength(persistentJNI, arr);
	//		printf("got node name array length %d\n",arrLength);
	out->numNodes = arrLength;
	out->nodeNames = malloc(arrLength * sizeof(char*));
	for (; i < arrLength; i++) {
		jobject data = (*persistentJNI)->GetObjectArrayElement(persistentJNI,
				arr, i);
		char* nodeName = getStringSimple((jstring) data);
		//printf("got node name %s\n",nodeName);
		out->nodeNames[i] = nodeName;
	}
	// Don't forget to release it?
	//(*persistentJNI)->ReleaseDoubleArrayElements(*arr, data, 0);
	// copy the values array
	jmethodID values = (*persistentJNI)->GetMethodID(persistentJNI, mvclass,
			"getValues", "()[D");
	jdoubleArray valuesArr = (*persistentJNI)->CallObjectMethod(persistentJNI,
			graph, values);
	i = 0;
	arrLength = (*persistentJNI)->GetArrayLength(persistentJNI, valuesArr);
	out->numEdges = arrLength;
	out->edgeValues = malloc(arrLength * sizeof(double));
	jboolean isCopy1;
	jdouble* srcArrayElems = (*persistentJNI)->GetDoubleArrayElements(
			persistentJNI, valuesArr, &isCopy1);
	for (; i < arrLength; i++) {
		out->edgeValues[i] = srcArrayElems[i];
	}
	if (isCopy1 == JNI_TRUE) {
		(*persistentJNI)->ReleaseDoubleArrayElements(persistentJNI, valuesArr,
				srcArrayElems, JNI_ABORT);
	}
	// copy the row offsets array
	jmethodID rowIndex = (*persistentJNI)->GetMethodID(persistentJNI, mvclass,
			"getRowIndex", "()[I");
	jintArray rowIndexArr = (*persistentJNI)->CallObjectMethod(persistentJNI,
			graph, rowIndex);
	i = 0;
	arrLength = (*persistentJNI)->GetArrayLength(persistentJNI, rowIndexArr);
	out->numNodes = arrLength;
	out->rowOffsets = malloc(arrLength * sizeof(int));
	jint* srcArrayElemsInt = (*persistentJNI)->GetIntArrayElements(
			persistentJNI, rowIndexArr, &isCopy1);
	for (; i < arrLength; i++) {
          out->rowOffsets[i] = srcArrayElemsInt[i];
	}
	if (isCopy1 == JNI_TRUE) {
		(*persistentJNI)->ReleaseIntArrayElements(persistentJNI, rowIndexArr,
				srcArrayElemsInt, JNI_ABORT);
	}
	// copy the col offsets array
	jmethodID colIndex = (*persistentJNI)->GetMethodID(persistentJNI, mvclass,
			"getColIndex", "()[I");
	jintArray colIndexArr = (*persistentJNI)->CallObjectMethod(persistentJNI,
			graph, colIndex);
	i = 0;
	arrLength = (*persistentJNI)->GetArrayLength(persistentJNI, colIndexArr);
	printf("col index len %d\n",arrLength);
	printf("numValues %d\n",out->numEdges);
	out->colIndices = malloc(out->numEdges * sizeof(int));
	srcArrayElemsInt = (*persistentJNI)->GetIntArrayElements(persistentJNI,
			colIndexArr, &isCopy1);
	for (; i < arrLength; i++) {
		out->colIndices[i] = srcArrayElemsInt[i];
	}
	if (isCopy1 == JNI_TRUE) {
		(*persistentJNI)->ReleaseIntArrayElements(persistentJNI, colIndexArr,
				srcArrayElemsInt, JNI_ABORT);
	}
}

// Partially implemented - grabs the node names from the graph object.
// doesn't yet grab the row values or the col offsets of the CSR format
// this assumes the graph begins on the java side -- if we it begins in julia, for instance,
// we'll need to pass that in, without copying
//
// TODO : for now it copies values, soon we will need to avoid copying using GetDirectBufferAddress or NewDirectByteBuffer
void td_java_getgraph0(graph_t *out, char *fname)
{
	jclass clsH = (*persistentJNI)->FindClass(persistentJNI, persistentClass);
	if (clsH == NULL) {
		printf("td_java_getgraph0 : can't find %s class?\n",persistentClass);
		return;
	}

	printf("td_java_getgraph0 class  is %s\n",persistentClass);
	printf("td_java_getgraph0 method is %s\n",fname);

	jstring string = getReturnType(&fname, &clsH);
    const char* returnString = (*persistentJNI)->GetStringUTFChars(persistentJNI,string, 0);

    printf("td_java_getgraph0 return type for %s is %s\n",fname,returnString);

	if (strcmp(returnString, XLANG_JAVA_GRAPH) == 0) {
    	jmethodID getGraph = (*persistentJNI)->GetStaticMethodID(persistentJNI, clsH, fname, "()Lxlang/java/Graph;");
    	jobject graph = (*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, getGraph);

    	// Get the graph
		copyGraph(graph, persistentJNI, out);
    }
    else {
    	printf("ERROR expecting a graph as a return value instead of %s\n",  returnString);
    }

	(*persistentJNI)->ReleaseStringUTFChars(persistentJNI,string, returnString);
	(*persistentJNI)->DeleteLocalRef(persistentJNI,string);
}

void td_java_getgraph1(graph_t *out, char *fname, graph_t *in)
{
	jclass clsH = (*persistentJNI)->FindClass(persistentJNI, persistentClass);
	if (clsH == NULL) {
		printf("td_java_getgraph0 : can't find %s class?\n",persistentClass);
		return;
	}

	printf("td_java_getgraph1 class  is %s\n",persistentClass);
	printf("td_java_getgraph1 method is %s\n",fname);

	jstring string = getReturnType(&fname, &clsH);
	const char* returnString = (*persistentJNI)->GetStringUTFChars(persistentJNI,string, 0);

	printf("td_java_getgraph1 return type for %s is %s\n",fname,returnString);

	if (strcmp(returnString, XLANG_JAVA_GRAPH) == 0) {
		jmethodID getGraph = (*persistentJNI)->GetStaticMethodID(persistentJNI, clsH, fname, "(Lxlang/java/Graph;)Lxlang/java/Graph;");
		jobject cls = (*persistentJNI)->FindClass(persistentJNI, "xlang/java/Graph");

		jmethodID constructor = (*persistentJNI)->GetMethodID(persistentJNI, cls, "<init>", "([I[I)V");

		// TODO : add ability to read from a byte buffer on java side, so we don't copy the values
		// jobject buffer = (*persistentJNI)->NewDirectByteBuffer(persistentJNI,in->rowValueOffsets,in->numRowPtrs);

		jintArray rowOffsets = makeIntArray(in->numNodes, in->rowOffsets);
		printf("made   rowOffsets...\n");
		jintArray colOffets  = makeIntArray(in->numEdges, in->colIndices);

		printf("making in graph...\n");

		jobject inGraph = (*persistentJNI)->NewObject(persistentJNI, cls, constructor, rowOffsets, colOffets);
		printf("made   in graph...\n");

		// create a graph object, set values from in graph

		jobject graph = (*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, getGraph, inGraph);
		printf("made   out graph...\n");

		// Get the graph
		copyGraph(graph, persistentJNI, out);
	}
	else {
		printf("ERROR expecting a graph as a return value instead of %s\n",  returnString);
	}

	(*persistentJNI)->ReleaseStringUTFChars(persistentJNI,string, returnString);
	(*persistentJNI)->DeleteLocalRef(persistentJNI,string);
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

void td_java_init(const char *classpath, const char *javaClass)
{
    td_env_t *env = get_java(classpath, javaClass);
   td_provide_java(env);
}

//! get or make the java environment
/*!
\param javaClass class with static methods you would like to call
\return the callable
*/
td_env_t *get_java(const char *classpath, const char *javaClass) {
	JNIEnv *jniEnv;
	JavaVM * jvm;

	printf("get_java : called get java with %s and %s\n",classpath,javaClass);

	jniEnv = create_vm(&jvm, classpath);
	persistentJNI = jniEnv;
	persistentClass = javaClass;

	if (jniEnv == NULL) {
		printf("ERROR : couldn't make the java vm!\n");
		return NULL;
	}
	else {
		//printf(" made java vm!\n");
	}

	td_env_t *env = (td_env_t*)malloc(sizeof(td_env_t));
	env->name = "java";

	//env->eval = &td_java_eval;
	env->invoke0 = &td_java_invoke0;
	env->invoke1 = &td_java_invoke1;
	env->invoke2 = &td_java_invoke2;
	env->invokeGraph0 = &td_java_getgraph0;
	env->invokeGraph1 = &td_java_getgraph1;
	//env->getGraph1 = &td_java_graph1;
	//env->invoke3

	//env->retain
	//env->release

	//env->get_type = &td_java_get_type;
	//env->get_eltype = &td_java_get_eltype;
	// env->get_dataptr = &td_java_get_dataptr;
	//env->get_length = &td_java_get_length;
	//env->get_ndims = &td_java_get_ndims;
	//printf(" returning env!\n");

	return env;
}

