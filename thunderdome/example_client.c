#include <stdlib.h>
#include <stdio.h>

#include "td.h"

// this is the class that's called below... change it to use something else...
#define MAIN_CLASS "xlang/java/Xlang"

// for java, first argument can be a classpath
int main(int argc, char *argv[])
{
#ifdef TD_HAS_JULIA
    // start julia

    td_env_t *jl = td_env_julia(".", TD_JULIA_BIN);



    // call "sin" with one scalar argument
    td_val_t arg_jl = { .tag = TD_DOUBLE, .double_val = 3.14 };
    td_val_t out;
    jl->invoke1(&out, "sin", &arg_jl);

    printf("sin(3.14) = %g\n", td_double(&out));


    // call "norm" on a Float64 3-vector
    double v[] = { 1.0, 2.0, 3.0 };
    td_array_t a = { .data=v, .length=3, .eltype=TD_DOUBLE, .ndims=1 };
    td_val_t av = { .tag = TD_ARRAY, .object = &a };
    jl->invoke1(&out, "norm", &av);

    printf("norm([1.0,2.0,3.0]) = %g\n", td_double(&out));
#endif

#ifdef TD_HAS_PYTHON
    td_val_t out_py;
    td_env_t *py = td_env_python(".", TD_PYTHON_EXE);
    py->invoke0(&out_py, "int");
    printf("int() = %d\n", td_int32(&out_py));

    td_val_t arg_py = { .tag = TD_INT32, .int32_val = 2 };

    py->invoke1(&out_py, "int", &arg_py);
    printf("int(2) = %d\n", td_int32(&out_py));
#endif

#ifdef TD_HAS_JAVA

    td_val_t out_java;

    char *classpath = "out";
    if (argc == 2) {
    	classpath = argv[1];
    }
    td_env_t *java_env = td_env_java(".",classpath,MAIN_CLASS);

    // tests!
    java_env->invoke0(&out_java, "nextInt");
    printf("nextInt() = %d tag %d\n", td_int32(&out_java), td_typeof(&out_java));

    java_env->invoke0(&out_java, "nextBool");
    printf("nextBool() = %d\n", td_int32(&out_java));

    java_env->invoke0(&out_java, "nextDouble");
    printf("nextDouble() = %f\n", td_double(&out_java));


   // td_val_t arg = { .tag = TD_INT32, .int32_val = 2 };

    td_val_t arg_java = { TD_INT32, 4 };
    java_env->invoke1(&out_java, "sqr", &arg_java);
    printf("sqr(2) = %d\n", td_int32(&out_java));

    arg_java.tag = TD_DOUBLE; arg_java.double_val = 3.14159/2;
    java_env->invoke1(&out_java, "sin", &arg_java);
    printf("sin(%f) = %f\n", arg.double_val, td_double(&out_java));

    arg_java.tag = TD_INT32; arg.int32_val = 4;
    java_env->invoke1(&out_java, "isEven", &arg_java);
    printf("isEven(%d) = %d\n", arg.int32_val, td_uint32(&out_java));

    arg_java.tag = TD_INT32; arg_java.int32_val = 3;
    java_env->invoke1(&out_java, "isEven", &arg_java);
    printf("isEven(%d) = %d\n", arg_java.int32_val, td_uint32(&out_java));

    // error test cases -------------------------------------------

    // bad method name
    java_env->invoke0(&out_java, "unknownMethod");
    printf("unknownMethod() = %f\n", td_double(&out_java));

    // another error case - no sqr that takes a double
    arg_java.tag = TD_DOUBLE; arg_java.double_val = 3.14159/2;
    java_env->invoke1(&out_java, "sqr", &arg_java);
    printf("sqr(%f) = %f\n", arg_java.double_val, td_double(&out_java));
#endif

    return 0;
}
