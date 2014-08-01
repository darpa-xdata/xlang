#include "thunderdome/td.h"
#include "etl/c_import.h"
#include "cluster/gunrock/gunrock_clusters.h"

void _print_usage()
{
  char* usage = "c-loader <graph_type> <graph_name>\n\
    \n\
    Use c etl, call webgraph to serve\n\
\n\
Description\n\
\n\
    graph_format: one of WDC or SNAP\n\
    graph_names: filename to graph (or index_file arc_file for WDC graphs)\n\
\n\
Examples:\n\
    test_c_import SNAP ../data/web-NotreDame.txt\n\
\n\
    test_c_import  WDC ../data/pld-arc-sample ../data/pld-index-sample\n\
\n\
Author(s):\n\
    Andy R. Terrel <andy.terrel@gmail.com>\n";
  printf("%s\n", usage);
}


int main(int argc, char** argv)
{
  if (argc > 4 || argc < 3) {
    _print_usage();
    return 1;
  }

  graph_format_t format = graph_format_from_str(argv[1]);
  char* arc_file = argv[2];
  char* index_file = (argc == 4) ? argv[3] : NULL;

  graph_t td_graph;
  load_graph(format, arc_file, index_file, &td_graph);
  int *csc_offsets = (int*)malloc(sizeof(int) * td_graph.numNodes+1);
  int *csc_indices = (int*)malloc(sizeof(int) * td_graph.numEdges);
  csr_to_csc(td_graph.numNodes, td_graph.numEdges,
	     td_graph.rowOffsets, td_graph.colIndices,
	     csc_offsets, csc_indices);

  td_val_t out_py;
  td_env_t *py = td_env_python(TD_DIR, TD_PYTHON_EXE);
  py->invokeGraphAndCSC(&out_py, "webgraph_app.run_app", &td_graph, 
			csc_offsets, csc_indices);
  
  if (csc_offsets) free(csc_offsets);
  if (csc_indices) free(csc_indices);
  if (td_graph.rowOffsets) free(td_graph.rowOffsets);
  if (td_graph.colIndices) free(td_graph.colIndices);
  

  return 0;

}
