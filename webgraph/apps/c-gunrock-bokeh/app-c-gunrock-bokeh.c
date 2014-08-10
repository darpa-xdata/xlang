#include <thunderdome/td.h>
#include <webgraph/etl/c_import.h>
#include <cluster/gunrock/gunrock_clusters.h>


void _print_usage()
{
  char* usage = "app <num_top_nodes>  <graph_type> <graph_name>\n\
    \n\
    Use c etl, clusters with gunrock top-k, and produces an \n\
    bokeh visualization. \n\
\n\
Description\n\
\n\
    num_top_nodes: Maximum number of top nodes to produce\n\
    graph_format: one of WDC or SNAP\n\
    graph_names: filename to graph (or index_file arc_file for WDC graphs)\n\
\n\
Examples:\n\
    test_c_import 3 SNAP ../data/web-NotreDame.txt\n\
\n\
    test_c_import 10  WDC ../data/pld-arc-sample ../data/pld-index-sample\n\
\n\
Author(s):\n\
    Andy R. Terrel <andy.terrel@gmail.com>\n";
  printf("%s\n", usage);
}


int main(int argc, char** argv)
{
  if (argc > 5 || argc < 4) {
    _print_usage();
    return 1;
  }

  graph_format_t format = graph_format_from_str(argv[2]);
  char* arc_file = argv[3];
  char* index_file = (argc == 5) ? argv[4] : NULL;
  int top_nodes = atoi(argv[1]);
  int *node_ids = (int*)malloc(sizeof(int) * top_nodes);
  int *in_deg = (int*)malloc(sizeof(int) * top_nodes);
  int *out_deg = (int*)malloc(sizeof(int) * top_nodes);

  graph_t td_graph;

  load_graph(format, arc_file, index_file, &td_graph);

  gunrock_topk(&td_graph, top_nodes, node_ids, in_deg, out_deg);

  printf("node_ids:");
  for (int idx=0; idx<top_nodes; ++idx){
    printf("%d, ", node_ids[idx]);
  }
  printf("\n");

  printf("in_deg:");
  for (int idx=0; idx<top_nodes; ++idx){
    printf("%d, ", in_deg[idx]);
  }
  printf("\n");

  printf("out_deg:");
  for (int idx=0; idx<top_nodes; ++idx){
    printf("%d, ", out_deg[idx]);
  }
  printf("\n");

  td_val_t out_py;
  td_env_t *py = td_env_python(TD_DIR, TD_PYTHON_EXE);

  td_array_t td_csr_offsets = { .data   = td_graph.rowOffsets, 
				.length = td_graph.numNodes+1, 
				.eltype = TD_INT32, 
				.ndims  = 1 };
  td_val_t arg_py_csr_offsets = { .tag    = TD_ARRAY, 
				  .object = &td_csr_offsets };
  td_array_t td_csr_indices = { .data   = td_graph.colIndices, 
				.length = td_graph.numEdges,
				.eltype = TD_INT32, 
				.ndims  = 1 };
  td_val_t arg_py_csr_indices = { .tag    = TD_ARRAY, 
				  .object = &td_csr_indices };
  td_array_t td_top_nodes = { .data   = node_ids, 
			      .length = top_nodes, 
			      .eltype = TD_INT32, 
			      .ndims  = 1 };
  td_val_t arg_py_top_nodes = { .tag = TD_ARRAY, 
				.object = &td_top_nodes };
  td_array_t td_in_deg = { .data   = in_deg,
			   .length = top_nodes, 
			   .eltype = TD_INT32, 
			   .ndims  = 1 };
  td_val_t arg_py_in_deg = { .tag    = TD_ARRAY, 
			     .object = &td_in_deg };
  td_array_t td_out_deg = { .data   = out_deg,
			    .length = top_nodes, 
			    .eltype = TD_INT32, 
			    .ndims  = 1 };
  td_val_t arg_py_out_deg = { .tag    = TD_ARRAY, 
			      .object = &td_out_deg };

  py->invoke5(&out_py, "bokeh_wrap.visualize",
	      &arg_py_csr_offsets, &arg_py_csr_indices,
  	      &arg_py_top_nodes, &arg_py_in_deg, &arg_py_out_deg);


  if (in_deg)    free(in_deg);
  if (out_deg)   free(out_deg);
  if (node_ids)  free(node_ids);

  return 0;
}
