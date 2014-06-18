#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <jni.h> // this is found under the JVM include directory, e.g. on windows C:\Program Files\Java\jdk1.8.0_05\include

#include "td.h"

JNIEnv *persistentJNI;
const char *persistentClass;
int debug = 1;
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
	else
		return '?';
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
	//case TD_UTF8 : return '[Ljava/lang/String;';
	}
}

void setValueFromType(td_val_t *arg, jvalue * val) {
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
	case TD_UTF8:
		val->l = (*persistentJNI)->NewStringUTF(persistentJNI,(char *)((td_string_t *)arg->object)->data);
		break;
	}
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
		printf("can't find Xlang class?\n");
		return;
	}

	jmethodID returnType = (*persistentJNI)->GetStaticMethodID(persistentJNI, clsH, "getReturnType", "(Ljava/lang/String;)Ljava/lang/String;");

	if (returnType == NULL) {
		printf("can't find Xlang method %s?\n","returnType");
		return;

	}
	//printf("found return type method for %s\n",fname);

	jstring message = (*persistentJNI)->NewStringUTF(persistentJNI, fname);
	jstring string = (*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, returnType,message);

    const char* returnString = (*persistentJNI)->GetStringUTFChars(persistentJNI,string, 0);

    //printf("return type for %s is %s\n",fname,returnString);

    char buf[512];
    char rc = setReturnTdVal(returnString);
    sprintf(buf,"()%c",rc);
  	//printf("signature is %s\n",buf);

	jmethodID midMain = (*persistentJNI)->GetStaticMethodID(persistentJNI, clsH, fname, buf);

	if (midMain == NULL) {
		printf("ERROR : can't find Xlang method %s with signature %s?\n",fname,buf);
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

void getSignature(const char* returnString, char buf[512], td_val_t* arg) {
	char rc = setReturnTdVal(returnString);
	char argC = getArgFromType(arg);
	if (td_typeof(arg) == TD_UTF8) {
		sprintf(buf, "(Ljava/lang/String;)%c", rc);
	} else {
		sprintf(buf, "(%c)%c", argC, rc);
	}
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
	//const char* persistentClass = "xlang/java/Xlang";
	// 1) Find the class we want to use
	jclass clsH = (*persistentJNI)->FindClass(persistentJNI, persistentClass);
	if (clsH == NULL) {
		printf("can't find Xlang class?\n");
		return;
	}

	// 2) Find the return type for the method we want to call
	jmethodID returnType = (*persistentJNI)->GetStaticMethodID(persistentJNI, clsH, "getReturnType", "(Ljava/lang/String;)Ljava/lang/String;");

	if (returnType == NULL) {
		printf("can't find Xlang method %s?\n","returnType");
		return;

	}
	if (debug) printf("found return type method for %s\n",fname);

	jstring message = (*persistentJNI)->NewStringUTF(persistentJNI, fname);
	jstring returnTypeMethod = (*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, returnType, message);

	const char* returnString = (*persistentJNI)->GetStringUTFChars(persistentJNI, returnTypeMethod, 0);

	if (debug) printf("return type for %s is %s\n",fname,returnString);

	// 3) Get the method signature implied by the argument and the return type.
    char signature[512];
	getSignature(returnString, signature, arg);
	if (debug) printf("signature is %s\n",signature);

	// 4) Get the method given the name and signature
	jmethodID methodToCall = (*persistentJNI)->GetStaticMethodID(persistentJNI, clsH, fname, signature);

	if (methodToCall == NULL) {
		printf("ERROR : can't find Xlang method %s with signature %s?\n", fname, signature);
		out->tag = TD_UNKNOWN;
		return;
	}

    jvalue val;
	setValueFromType(arg, &val);

	// 5) Finally lets call the method with the right type, depending on the argument type and grap the return value

	if (strcmp(returnString, "boolean") == 0) {
		//out->int32_val = (*persistentJNI)->CallStaticBooleanMethod(persistentJNI, clsH, methodToCall,val);
		// if there's a better way than copying all this code I'd love to know -- macro?
		switch (td_typeof(arg)) {
			case TD_INT8:
				out->int32_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.b);
				break;
			case TD_UINT8:
				out->int32_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.b);
				break;
			case TD_INT16:
				out->int32_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.s);
				break;
			case TD_UINT16:
				out->int32_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.s);
				break;
			case TD_INT32:
				out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
				break;
			case TD_UINT32:
				out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
				break;
			case TD_INT64:
				out->int32_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.j);
				break;
			case TD_UINT64:
				out->int32_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.j);
				break;
			case TD_FLOAT:
				out->int32_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.f);
				break;
			case TD_DOUBLE:
				out->int32_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.d);
				break;
			}

		out->tag = TD_INT32;
	} else if (strcmp(returnString, "byte") == 0) {

		switch (td_typeof(arg)) {
			case TD_INT8:
				out->int32_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.b);
				break;
			case TD_UINT8:
				out->int32_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.b);
				break;
			case TD_INT16:
				out->int32_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.s);
				break;
			case TD_UINT16:
				out->int32_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.s);
				break;
			case TD_INT32:
				out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
				break;
			case TD_UINT32:
				out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
				break;
			case TD_INT64:
				out->int32_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.j);
				break;
			case TD_UINT64:
				out->int32_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.j);
				break;
			case TD_FLOAT:
				out->int32_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.f);
				break;
			case TD_DOUBLE:
				out->int32_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.d);
				break;
			}
		out->tag = TD_INT8;
	} else if (strcmp(returnString, "char") == 0) {
		switch (td_typeof(arg)) {
				case TD_INT8:
					out->int8_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.b);
					break;
				case TD_UINT8:
					out->int8_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.b);
					break;
				case TD_INT16:
					out->int8_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.s);
					break;
				case TD_UINT16:
					out->int8_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.s);
					break;
				case TD_INT32:
					out->int8_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
					break;
				case TD_UINT32:
					out->int8_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
					break;
				case TD_INT64:
					out->int8_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.j);
					break;
				case TD_UINT64:
					out->int8_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.j);
					break;
				case TD_FLOAT:
					out->int8_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.f);
					break;
				case TD_DOUBLE:
					out->int8_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.d);
					break;
				}
		out->tag = TD_INT8;
	} else if (strcmp(returnString, "double") == 0) {
		//printf("orig %f\n",arg->double_val);
		//printf("calling double with %f arg type %d\n",val.d, td_typeof(arg));
		switch (td_typeof(arg)) {
			case TD_INT8:
				out->double_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.b);
				break;
			case TD_UINT8:
				out->double_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.b);
				break;
			case TD_INT16:
				out->double_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.s);
				break;
			case TD_UINT16:
				out->double_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.s);
				break;
			case TD_INT32:
				out->double_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
				break;
			case TD_UINT32:
				out->double_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
				break;
			case TD_INT64:
				out->double_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.j);
				break;
			case TD_UINT64:
				out->double_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.j);
				break;
			case TD_FLOAT:
				out->double_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.f);
				break;
			case TD_DOUBLE:
				out->double_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.d);
				break;
			}
		//printf("called double with %f got %f\n",val.d, out->double_val);

		out->tag = TD_DOUBLE;
	} else if (strcmp(returnString, "float") == 0) {
		switch (td_typeof(arg)) {
			case TD_INT8:
				out->float_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.b);
				break;
			case TD_UINT8:
				out->float_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.b);
				break;
			case TD_INT16:
				out->float_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.s);
				break;
			case TD_UINT16:
				out->float_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.s);
				break;
			case TD_INT32:
				out->float_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
				break;
			case TD_UINT32:
				out->float_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
				break;
			case TD_INT64:
				out->float_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.j);
				break;
			case TD_UINT64:
				out->float_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.j);
				break;
			case TD_FLOAT:
				out->float_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.f);
				break;
			case TD_DOUBLE:
				out->float_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.d);
				break;
			}
		out->tag = TD_FLOAT;
	} else if (strcmp(returnString, "int") == 0) {
		//printf("orig %d\n",arg->int32_val);
		//printf("calling int with %d\n",val.i);
		//out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val);

		switch (td_typeof(arg)) {
				case TD_INT8:
					out->int32_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.b);
					break;
				case TD_UINT8:
					out->int32_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.b);
					break;
				case TD_INT16:
					out->int32_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.s);
					break;
				case TD_UINT16:
					out->int32_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.s);
					break;
				case TD_INT32:
					out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
					break;
				case TD_UINT32:
					out->int32_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
					break;
				case TD_INT64:
					out->int32_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.j);
					break;
				case TD_UINT64:
					out->int32_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.j);
					break;
				case TD_FLOAT:
					out->int32_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.f);
					break;
				case TD_DOUBLE:
					out->int32_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.d);
					break;
				case TD_UTF8:
					out->int32_val =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val.l);
					break;
				}

		out->tag = TD_INT32;
	} else if (strcmp(returnString, "long") == 0) {
		switch (td_typeof(arg)) {
			case TD_INT8:
				out->int64_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.b);
				break;
			case TD_UINT8:
				out->int64_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.b);
				break;
			case TD_INT16:
				out->int64_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.s);
				break;
			case TD_UINT16:
				out->int64_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.s);
				break;
			case TD_INT32:
				out->int64_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
				break;
			case TD_UINT32:
				out->int64_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
				break;
			case TD_INT64:
				out->int64_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.j);
				break;
			case TD_UINT64:
				out->int64_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.j);
				break;
			case TD_FLOAT:
				out->int64_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.f);
				break;
			case TD_DOUBLE:
				out->int64_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.d);
				break;
			}
		out->tag = TD_INT64;
	} else if (strcmp(returnString, "object") == 0) {
		out->object =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val);
		out->tag = TD_OBJECT;
	} else if (strcmp(returnString, "short") == 0) {
		switch (td_typeof(arg)) {
			case TD_INT8:
				out->int16_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.b);
				break;
			case TD_UINT8:
				out->int16_val =(*persistentJNI)->CallStaticByteMethod(persistentJNI, clsH, methodToCall,val.b);
				break;
			case TD_INT16:
				out->int16_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.s);
				break;
			case TD_UINT16:
				out->int16_val =(*persistentJNI)->CallStaticShortMethod(persistentJNI, clsH, methodToCall,val.s);
				break;
			case TD_INT32:
				out->int16_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
				break;
			case TD_UINT32:
				out->int16_val =(*persistentJNI)->CallStaticIntMethod(persistentJNI, clsH, methodToCall,val.i);
				break;
			case TD_INT64:
				out->int16_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.j);
				break;
			case TD_UINT64:
				out->int16_val =(*persistentJNI)->CallStaticLongMethod(persistentJNI, clsH, methodToCall,val.j);
				break;
			case TD_FLOAT:
				out->int16_val =(*persistentJNI)->CallStaticFloatMethod(persistentJNI, clsH, methodToCall,val.f);
				break;
			case TD_DOUBLE:
				out->int16_val =(*persistentJNI)->CallStaticDoubleMethod(persistentJNI, clsH, methodToCall,val.d);
				break;
			}		out->tag = TD_INT16;
	} else if (strcmp(returnString, "void") == 0) { // TODO: no good mapping for a void function
		(*persistentJNI)->CallStaticVoidMethod(persistentJNI, clsH, methodToCall,val);
		out->tag = TD_UNKNOWN;
	} else if (strcmp(returnString, "array") == 0) {
		out->ptr_val =(*persistentJNI)->CallStaticObjectMethod(persistentJNI, clsH, methodToCall,val);
		out->tag = TD_ARRAY;
	} else {
		printf("error: unknown return value %s\n",returnString);
	}

	// free the return string we got from asking for the return type
	(*persistentJNI)->ReleaseStringUTFChars(persistentJNI,returnTypeMethod, returnString);
	(*persistentJNI)->DeleteLocalRef(persistentJNI,returnTypeMethod);

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

	printf("called get java with %s and %s\n",classpath,javaClass);

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
	//env->invoke2
	//env->invoke3

	//env->retain
	//env->release

	//env->get_type = &td_java_get_type;
	//env->get_eltype = &td_java_get_eltype;
	// env->get_dataptr = &td_java_get_dataptr;
	//env->get_length = &td_java_get_length;
	//env->get_ndims = &td_java_get_ndims;

	return env;
}

