#include "c_import.h"


void _print_usage()
{
  char* usage = "test_c_import <graph_name> <graph_type> \n\
\n\
    graph_name: filename to graph\n\
    graph_format: one of WDC or SNAP\n";
  printf("%s\n", usage);
}

int main(int argc, char** argv)
{
  for (int i=0; i < argc; ++i) {
    printf("argv[%d]: %s\n", i, argv[i]); 
  }
  printf("\n");

  if (argc != 3) {
    _print_usage();
    return 1;
  }

  graph_t output_graph;
  load_graph(argv[1], graph_format_from_str(argv[2]), &output_graph);
  

  return 0;

}
