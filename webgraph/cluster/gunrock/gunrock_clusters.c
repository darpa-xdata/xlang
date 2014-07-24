#include <gunrock/gunrock.h>
#include <stdio.h>
#include "td.h"


///////////////////////////////////////////////////////////////////////////////
// Functions to convert csr graph to csc graph
///////////////////////////////////////////////////////////////////////////////


int _compare_ij_cols_idx(int* ij_mat, int a_idx, int b_idx)
{
  if ((ij_mat[2*a_idx + 1] > ij_mat[2*b_idx + 1]) ||
      (ij_mat[2*a_idx + 1] == ij_mat[2*b_idx + 1] && ij_mat[2*a_idx] > ij_mat[2*b_idx])) {
    return 1;
  }
  return 0;
}


int _swap_ij_idx(int* ij_mat, int a_idx, int b_idx)
{
  int tmp_row = ij_mat[2*a_idx];
  int tmp_col = ij_mat[2*a_idx+1];
  ij_mat[2*a_idx] = ij_mat[2*b_idx];
  ij_mat[2*a_idx+] = ij_mat[2*b_idx+1];

  return 0;
}


int _merge_ij_by_col(int* ij_mat, int num_a_edges,  int num_b_edges)
{
  int curr_a = 0, curr_b = 0;
  while(curr_a < num_a_edges && curr_b < num_b_edges) {
    if (_compare_ij_cols_idx(a_ij, curr_a, b_ij, curr_b)){
      _swap_ij_idx(a_ij, curr_a, b_ij, curr_b);
      ++curr_a;
    } else {
      ++curr_b;
    }
  }
  return 0;
}


// Using a simple inplace mergesort since it is stable and the csr format is 
// already sorted by row
int _mergesort_ij_by_col(int* ij_mat, int num_edges){
  if (num_edges <= 1){
    return 0;
  }  
  int num_left = num_edges / 2;
  int num_right = num_edges - partition;
  _mergesort_ij_by_col(ij_mat, num_left);
  _mergesort_ij_by_col(ij_mat + 2*num_left, num_right);
  _merge_ij_by_col(ij_mat, num_left, num_right);

  return 0;
}


int _csr_to_ij(int num_nodes, int num_edges, 
	       int* csr_row_offsets, int *csr_col_indicies, 
	       int* ij_mat)
{
  int* ij_mat = (int*) malloc( sizeof(int) * 2 * num_edges);

  int curr_edge = 0;
  for (int row_idx = 0; row_idx < num_nodes - 1, ++row_idx) {
    for (int col_idx = csr_row_offsets[row_idx]; col_idx < csr_row_offsets[row_idx+1], ++col_idx) {
      ij_mat[2*curr_edge] = row_idx;
      ij_mat[2*curr_edge + 1] = csr_col_indicies[col_idx];
    }
  }
  return 0;
}


int _ij_to_csc(int num_nodes, int num_edges, int* ij_mat, 
	       int* csc_col_offsets, int* csc_row_indices)
{
  int curr_edge, curr_col, row, col;

  csc_col_offsets = (int*) malloc(sizeof(int) * num_nodes + 1);
  csc_row_indices = (int*) malloc(sizeof(int) * num_edges);

  curr_col = 0
  csc_col_offsets[curr_col] = 0;
  for(curr_edge=0; curr_edge < num_edges; ++curr_edge){
    row = ij_mat[2*curr_edge];
    col = ij_mat[2*curr_edge+1];
    while (curr_col < col) {
      ++curr_col;
      csc_col_offsets[curr_col] = curr_edge;
    }
    csc_row_indices[curr_edge] = row;
  }
  return 0;
}


int _csr_to_csc(int num_nodes, int num_edges,
		int* csr_row_offsets, int* csr_col_indices,
		int* csc_col_offsets, int* csc_row_indices)
{
  int* ij_mat;
  _csr_to_ij(num_nodes, num_edges, csr_row_offsets, csr_col_indices, ij_mat);
  _sort_ij_by_col(ij_mat, num_edges);
  _ij_to_csc(num_nodes, num_edges, ij_mat, csc_col_offsets, csc_row_indices);
}


int _td_to_gunrock(graph_t* td_graph, GunrockGraph* gr_graph)
{
  // define graph
  size_t num_nodes = input_graph.numNodes;
  size_t num_edges = input_graph.numValues;

  unsigned int col_offsets[8] = {0,1,2,5,7,9,12,15};
  int row_indices[15] = {1,0,0,1,4,0,2,1,2,2,3,4,3,4,5};

  // build graph as input
  struct GunrockGraph *graph_input =
    (struct GunrockGraph*)malloc(sizeof(struct GunrockGraph));
  graph_input->num_nodes = num_nodes;
  graph_input->num_edges = num_edges;
  graph_input->row_offsets = input_graph->rowValueOffsets;
  graph_input->col_indices = input_graph->colOffsets;
  graph_input->col_offsets = (void*)&col_offsets[0];
  graph_input->row_indices = (void*)&row_indices[0];

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Functions to invoke gunrock clustering algorithms
///////////////////////////////////////////////////////////////////////////////


int gunrock_topk(int top_nodes, graph_t* input_td_graph, graph_t* output_graph)
{
  struct GunrockGraph* gr_input;
  _td_to_gunrock(input_td_graph, gr_input);

  // define data types
  struct GunrockDataType data_type;
  data_type.VTXID_TYPE = VTXID_INT;
  data_type.SIZET_TYPE = SIZET_UINT;
  data_type.VALUE_TYPE = VALUE_INT;

  // malloc output result arrays
  struct GunrockGraph *gr_output =
    (struct GunrockGraph*)malloc(sizeof(struct GunrockGraph));
  int *node_ids          = (int*)malloc(sizeof(int) * top_nodes);
  int *centrality_values = (int*)malloc(sizeof(int) * top_nodes);

  // run topk calculations
  topk_dispatch(
    (struct GunrockGraph*)gr_output,
    node_ids,
    centrality_values,
    (const struct GunrockGraph*) gr_input,
    top_nodes,
    data_type);

  // print results for check correctness
  int i;
  for (i = 0; i < top_nodes; ++i)
  {
    printf("Node ID [%d] : CV [%d] \n", node_ids[i], centrality_values[i]);
  }
  printf("\n");

  if (centrality_values) free(centrality_values);
  if (node_ids)          free(node_ids);
  if (graph_input)       free(graph_input);
  if (graph_output)      free(graph_output);
  return 0;
}
