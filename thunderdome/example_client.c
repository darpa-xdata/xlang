#include <stdlib.h>
#include <stdio.h>

#include "td.h"

int main(int argc, char *argv[])
{
    // start julia
    td_env_t *jl = td_env_julia(".",
                                "/home/jeff/src/julia2/julia/usr/bin");


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

    return 0;
}
