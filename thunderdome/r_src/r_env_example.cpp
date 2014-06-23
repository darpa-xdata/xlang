#include <RInside.h>         

typedef struct {
  int numNodes;
  char **nodeNames;
  // CSR encoding
  int numValues;
  double *values;
  int numRowPtrs;
  int *rowValueOffsets;
  int *colOffsets;
} graph_t;

int main(int argc, char *argv[]) {
  RInside R(argc, argv); 
  R["txt"] = "Hello, world!\n";
  R.parseEvalQ("cat(txt)");   
  exit(0);
}
