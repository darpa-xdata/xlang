#include "gunrock_clusters.h"
#include <string.h>

// Forward declaration for testing.
int _ij_to_csc(int num_nodes, int num_edges, int* ij_mat, 
	       int* csc_col_offsets, int* csc_row_indices);
int _csr_to_ij(int num_nodes, int num_edges, 
	       int* csr_row_offsets, int *csr_col_indicies, 
	       int* ij_mat);
int _mergesort_ij_by_col(int* ij_mat, int num_edges);
int _parallel_mergesort_ij_by_col(int* ij_mat, int num_edges);


int _create_simple_td_graph(graph_t *graph)
{
  size_t num_nodes = 7;
  size_t num_edges = 15;
  size_t top_nodes = 3;

  unsigned int row_offsets[8] = {0,3,6,9,11,14,15,15};
  int col_indices[15] = {1,2,3,0,2,4,3,4,5,5,6,2,5,6,6};
  
  graph->numNodes = num_nodes;
  graph->numEdges = num_edges;
  graph->rowOffsets = (int*) malloc(sizeof(int) * num_nodes+1);
  graph->colIndices = (int*) malloc(sizeof(int) * num_edges);

  memcpy(graph->rowOffsets, (int*)row_offsets, (num_nodes+1) * sizeof(int));
  memcpy(graph->colIndices, (int*)col_indices, num_edges * sizeof(int));

  return 0;  
}

int _destroy_simple_td_graph(graph_t *graph)
{
  if(graph->rowOffsets) free(graph->rowOffsets);
  if(graph->colIndices) free(graph->colIndices);
  return 0;
}

int _print_small_array(int* arr, int len){
  printf("{ ");
  for(int i=0; i < len; ++i) {
    printf("%d", arr[i]);
    if (i < len - 1)
      printf(", ");
    else
      printf("}");
  }

  return 0;
}

int _compare_arrays(int* a, int* b, int len){
  int ret = 1;
  for (int idx=0; idx<len; ++idx) 
    ret &= (a[idx] == b[idx]);
  return !ret;
}

int test_csr_to_ij()
{
  printf("Test CSR to IJ\n");
  int num_nodes = 7;
  int num_edges = 15;

  unsigned int row_offsets[8] = {0,3,6,9,11,14,15,15};
  int col_indices[15] = {1,2,3,0,2,4,3,4,5,5,6,2,5,6,6};
  int ij_mat[30];
  int ij_expected[30] = { 0, 1, 0, 2, 0, 3, 1, 0, 1, 2, 1, 4, 2, 3, 2, 4, 2, 5, 3,
		          5, 3, 6, 4, 2, 4, 5, 4, 6, 5, 6};

  _csr_to_ij(num_nodes, num_edges, (int*) row_offsets, (int*) col_indices, ij_mat);
  if (! _compare_arrays(ij_mat, ij_expected, 30) ){
    printf("---> success\n");
    return 0;
  }

  printf("---> Error printing diagnostics\n");
  printf("---> row_offsets\n");
  printf("-------> ");
  _print_small_array((int*)row_offsets, num_nodes+1);
  printf("\n");

  printf("---> col_indices\n");
  printf("-------> ");
  _print_small_array((int*)col_indices, num_edges);
  printf("\n");

  printf("---> ij_mat\n");
  printf("------> ");
  _print_small_array(ij_mat, 30);
  printf("\n");

  printf("---> ij_expected\n");
  printf("------> ");
  _print_small_array(ij_expected, 30);
  printf("\n");
  
  return 1;
}

int test_sort_ij_by_col()
{
  printf("Test Sort IJ by Col\n");
  int num_nodes = 7;
  int num_edges = 15;

  int ij_mat[30] = {0, 1, 0, 2, 0, 3, 1, 0, 1, 2, 1, 4, 2, 3, 2, 4, 2, 5, 3, 
		     5, 3, 6, 4, 2, 4, 5, 4, 6, 5, 6};
  int ij_mat_orig[30] = {0, 1, 0, 2, 0, 3, 1, 0, 1, 2, 1, 4, 2, 3, 2, 4, 2, 5, 3, 
		     5, 3, 6, 4, 2, 4, 5, 4, 6, 5, 6};

  int ij_expected[30] = {1, 0, 0, 1, 0, 2, 1, 2,  4, 2, 0, 3,  2, 3, 1, 4, 2, 4, 2, 5, 3, 
			 5, 4, 5, 3, 6, 4, 6, 5, 6};
  
  _mergesort_ij_by_col(ij_mat, num_edges);

  if (! _compare_arrays(ij_mat, ij_expected, 30) ){
    printf("---> success\n");
    return 0;
  }
  printf("---> Error printing diagnostics\n");
  printf("---> ij_mat_orig\n");
  printf("------> ");
  _print_small_array(ij_mat_orig, 30);
  printf("\n");
  printf("---> ij_mat_sorted_by_col\n");
  printf("------> ");
  _print_small_array(ij_mat, 30);
  printf("\n");
  printf("---> ij_expected\n");
  printf("------> ");
  _print_small_array(ij_expected, 30);
  printf("\n");


  return 1;
}


int test_gunrock_graph_convert()
{
  graph_t td_graph;
  struct GunrockGraph gr_graph;

  printf("Test Gunrock Graph Convert\n");

  _create_simple_td_graph(&td_graph);
  td_to_gunrock(&td_graph, &gr_graph);

  int csc_col_expected[8] =  { 0, 1, 2, 5, 7, 9, 12, 15};
  int csc_row_expected[15] = { 1, 0, 0, 1, 4, 0, 2, 1, 2, 2, 3, 4, 3, 4, 5};

  if (! _compare_arrays(gr_graph.col_offsets, csc_col_expected, 8) &&
      ! _compare_arrays(gr_graph.row_indices, csc_row_expected, 15)  ){
    printf("---> success\n");
    return 0;
  }

  printf("---> gunrock row_offsets\n");
  printf("-----------> ");
  _print_small_array((int*)gr_graph.row_offsets, gr_graph.num_nodes+1);
  printf("\n");

  printf("---> gunrock col_indices\n");
  printf("-----------> ");
  _print_small_array((int*)gr_graph.col_indices, gr_graph.num_edges);
  printf("\n");


  printf("---> gunrock col_offsets\n");
  printf("-----------> ");
  _print_small_array((int*)gr_graph.col_offsets, gr_graph.num_nodes+1);
  printf("\n");

  printf("---> gunrock row_indices\n");
  printf("-----------> ");
  _print_small_array((int*)gr_graph.row_indices, gr_graph.num_edges);
  printf("\n");


  return 1;
}


int test_gunrock_topk()
{
  graph_t td_graph;
  int top_nodes          = 3;
  int *node_ids          = (int*)malloc(sizeof(int) * top_nodes);
  int *centrality_values = (int*)malloc(sizeof(int) * top_nodes);

  printf("Test Gunrock TopK Cluster\n");

  _create_simple_td_graph(&td_graph);
  gunrock_topk(&td_graph, top_nodes, node_ids, centrality_values);

  // print results for check correctness
  int i;
  for (i = 0; i < top_nodes; ++i)
  {
    printf("Node ID [%d] : CV [%d] \n", node_ids[i], centrality_values[i]);
  }
  printf("\n");

  if (centrality_values) free(centrality_values);
  if (node_ids)          free(node_ids);

  return 0;
}


int main(int argc, char** argv)
{
  int ret = 0;
  ret |= test_csr_to_ij();
  ret |= test_sort_ij_by_col();
  ret |= test_parallel_sort_ij_by_col();
  ret |= test_gunrock_graph_convert();
  ret |= test_gunrock_topk();
  return ret;
}
