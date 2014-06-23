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
  double val[] = {10, -2, 3, 9, 3, 7, 8, 7, 3, 8, 7, 5, 8, 9, 9, 13, 4, 2, -1};
  double ci[]  = { 1,  5, 1, 2, 6, 2, 3, 4, 1, 3, 4, 5, 2, 4, 5,  6, 2, 5,  6};
  double rp[] =  {1, 3, 6, 9, 13, 17, 20};
  RInside R(argc, argv); 
  R["txt"] = "Hello, world!\n";
  R.parseEvalQ("cat(txt)");   
  exit(0);
}
