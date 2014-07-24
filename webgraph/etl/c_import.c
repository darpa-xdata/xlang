#include "c_import.h"


graph_format_t graph_format_from_str(char* graph_format_str)
{
  graph_format_t ret;
  if (! strcmp(graph_format_str, "WDC")) {
    ret = WDC;
  } else if (! strcmp(graph_format_str, "SNAP")) {
    ret = SNAP;
  }
  return ret;
}


int load_graph(char* filename, graph_format_t graph_format, graph_t* output_graph)
{
  printf("Inside load_graph\n");
  return 0;
}
