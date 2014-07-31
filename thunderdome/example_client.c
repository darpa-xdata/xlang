#include <stdlib.h>
#include <stdio.h>

#include "td.h"

// this is the class that's called below... change it to use something else...
#define MAIN_CLASS "xlang/java/GraphAlgorithms"

void printGraph(graph_t* out_graph) {
	printf("getExampleGraph() = nodes %d \n", out_graph->numNodes);
	int i = 0;
	printf("names [");
	for (i = 0; i < out_graph->numNodes; i++)
		printf("%s ", out_graph->nodeNames[i]);
	printf("]\nvalues [");
	for (i = 0; i < out_graph->numEdges; i++)
		printf("%f ", out_graph->edgeValues[i]);
	printf("]\nrow offset [");
	for (i = 0; i < out_graph->numNodes; i++)
		printf("%d ", out_graph->rowOffsets[i]);
	printf("]\ncol offset [");
	for (i = 0; i < out_graph->numEdges; i++)
		printf("%d ", out_graph->colIndices[i]);
	printf("]\n");

}

// for java, first argument can be a classpath
int main(int argc, char *argv[])
{
    double v[] = { 1.0, 2.0, 3.0 };
    // call "norm" on a Float64 3-vector
    td_array_t a = { .data=v, .length=3, .eltype=TD_DOUBLE, .ndims=1 };
    td_val_t av = { .tag = TD_ARRAY, .object = &a };

#ifdef TD_HAS_JULIA
    // start julia

    td_env_t *jl = td_env_julia(".", TD_JULIA_BIN);



    // call "sin" with one scalar argument
    td_val_t arg_jl = { .tag = TD_DOUBLE, .double_val = 3.14 };
    td_val_t out;
    jl->invoke1(&out, "sin", &arg_jl);

    printf("sin(3.14) = %g\n", td_double(&out));


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

    py->invoke1(&out_py, "numpy.linalg.norm", &av);
    printf("numpy.linalg.norm([1.0, 2.0, 3.0]) = %g\n", td_double(&out_py));

#endif

#ifdef TD_HAS_JAVA

    td_val_t out_java;

    // modify as you pull in more jars
    char *classpath = "out:lib/la4j-0.4.9.jar:lib/commons-lang3-3.3.2.jar";
    //   printf("num %d\n",argc);
    if (argc >= 2) {
    	classpath = argv[1];

    }
    char *classToUse = MAIN_CLASS;
    if (argc >= 3) {
    	classToUse = argv[2];
    }
    td_env_t *java_env = td_env_java(".",classpath, classToUse);

    // tests!

    if (0) {
    	java_env->invoke0(&out_java, "nextInt");
    	printf("nextInt() = %d tag %d\n", td_int32(&out_java), td_typeof(&out_java));

    	// compare these with GraphAlgorithms.main output
    	graph_t out_graph;
    	java_env->invokeGraph0(&out_graph, "getExampleGraph");
    	printGraph(&out_graph);

    	java_env->invokeGraph0(&out_graph, "getExampleGraph2");
    	printGraph(&out_graph);
    }

    // get back a graph, no arguments
    graph_t out_graph;
    java_env->invokeGraph0(&out_graph, "testCD");
    printGraph(&out_graph);

    // graph in, graph out - community detection
    graph_t in_graph;
    int rowPtrs[4] = {0, 2, 4, 6};
    int colOffsets[6] = {1, 2, 0, 2, 0, 1};

    in_graph.numEdges = 6;
    in_graph.numNodes = 4;
    in_graph.rowOffsets = rowPtrs;
    in_graph.colIndices = colOffsets;
    java_env->invokeGraph1(&out_graph, "communityDetection", &in_graph);
    printGraph(&out_graph);


    if (1) return 0;

    td_string_t str;
    str.length = 7;
    str.data = "Bueller";

    td_val_t arg;
    arg.tag = TD_UTF8; arg.object = &str;
    java_env->invoke1(&out_java, "strLen", &arg);
    printf("strLen(%s) = %d\n", (char *)((td_string_t *)arg.object)->data, td_int32(&out_java));

    arg.tag = TD_UTF8; arg.object = &str;
    java_env->invoke1(&out_java, "toUpper", &arg);
    printf("toUpper(%s) = %s\n", (char *)((td_string_t *)arg.object)->data, (char *)((td_string_t *)out_java.object)->data);

    td_array_t arr;
    arr.eltype = TD_UTF8;
    arr.length = 3;

    td_string_t str1 = { "one",3};
    td_string_t str2 = { "two",3};
    td_string_t str3 = { "three",5};
    //char * words[3] = {"one","two","three"};
    td_string_t * words[3] = {&str1,&str2,&str3};
    arr.data = words;
    arg.tag = TD_ARRAY; arg.object = &arr;
    java_env->invoke1(&out_java, "howManyArr", &arg);
    printf("howManyArr = %d\n",  td_int32(&out_java));


    td_array_t arr2;
    arr2.eltype = TD_UTF8;
    arr2.length = 0;

//    arr.data = words;
    td_val_t arg2;

    arg2.tag = TD_ARRAY; arg2.object = &arr;
    java_env->invoke2(&out_java, "toUpperInOut", &arg, &arg2);
    printf("toUpperInOut \n");

    if (1) return 0;

    // sum ints
   // td_array_t arr;
    arr.eltype = TD_INT32;
    arr.length = 3;
    int nums[3] = {1,2,3};
    arr.data = &nums;
    arg.tag = TD_ARRAY; arg.object = &arr;
    java_env->invoke1(&out_java, "sumArr", &arg);
    printf("sum = %d\n",  td_int32(&out_java));

    // sum doubles
    double dnums[3] = {3.14,3.14,3.14};
    arr.eltype = TD_DOUBLE;
    arr.data = &dnums;
    arg.tag = TD_ARRAY; arg.object = &arr;
    java_env->invoke1(&out_java, "sumDoubleArr", &arg);
    printf("sum = %f\n",  td_double(&out_java));

    java_env->invoke0(&out_java, "nextInt");
    printf("nextInt() = %d tag %d\n", td_int32(&out_java), td_typeof(&out_java));

    java_env->invoke0(&out_java, "nextBool");
    printf("nextBool() = %d\n", td_int32(&out_java));

    java_env->invoke0(&out_java, "nextDouble");
    printf("nextDouble() = %f\n", td_double(&out_java));

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
