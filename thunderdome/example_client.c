#include <stdlib.h>
#include <stdio.h>

#include "td.h"

int main(int argc, char *argv[])
{
#ifdef TD_HAS_JULIA
    // start julia
    td_env_t *jl = td_env_julia(".",
                                "/Applications/Julia-0.2.1.app/Contents/Resources/julia/bin");


    // call "sin" with one scalar argument
    td_val_t arg = { .tag = TD_DOUBLE, .double_val = 3.14 };
    td_val_t out;
    jl->invoke1(&out, "sin", &arg);

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
    td_env_t *py = td_env_python(".",
                                 "/Users/aterrel/workspace/opt/apps/anaconda/anaconda-1.9.1/anaconda/bin/python");
    py->invoke0(&out_py, "int");
    printf("int() = %d\n", td_int32(&out_py));

    td_val_t arg = { .tag = TD_INT32, .int32_val = 2 };

    py->invoke1(&out_py, "int", &arg);
    printf("int(2) = %d\n", td_int32(&out_py));
#endif



    td_val_t out_java;

    printf("got here 1 ");
    //td_env_t *java_env = td_env_java(".", "");
    td_env_t *java_env = get_java();
    printf("got here 2 ");

    java_env->invoke0(&out_java, "int");
    printf("int() = %d\n", td_int32(&out_java));

    td_val_t arg = { .tag = TD_INT32, .int32_val = 2 };

    java_env->invoke1(&out_java, "int", &arg);
    printf("int(2) = %d\n", td_int32(&out_java));

   // printf("good morning!\n");
    return 0;
}
