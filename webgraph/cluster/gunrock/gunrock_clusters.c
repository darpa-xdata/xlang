#include "gunrock_clusters.h"

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
  ij_mat[2*a_idx+1] = ij_mat[2*b_idx+1];
  ij_mat[2*b_idx] = tmp_row;
  ij_mat[2*b_idx+1] = tmp_col;

  return 0;
}


int _merge_ij_by_col(int* ij_mat, int num_a_edges,  int num_b_edges)
{
  int curr_a = 0, curr_b = num_a_edges, total_edges = num_a_edges + num_b_edges;
  while(curr_a < num_a_edges && curr_b < total_edges) {
    if (_compare_ij_cols_idx(ij_mat, curr_a, curr_b)){
      _swap_ij_idx(ij_mat, curr_a, curr_b);
      ++curr_a;
      for (int idx=curr_b; idx<total_edges; ++idx) {
	if (_compare_ij_cols_idx(ij_mat, idx, idx+1))
	  _swap_ij_idx(ij_mat, idx, idx+1);
	else
	  break;
      }
    } else {
      ++curr_a;
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
  int num_right = num_edges - num_left;

  _mergesort_ij_by_col(ij_mat, num_left);
  _mergesort_ij_by_col(ij_mat + 2*num_left, num_right);
  _merge_ij_by_col(ij_mat, num_left, num_right);

  return 0;
}


int _csr_to_ij(int num_nodes, int num_edges, 
	       int* csr_row_offsets, int *csr_col_indicies, 
	       int* ij_mat)
{

  int curr_edge = 0;
  for (int row = 0; row < num_nodes; ++row) {
    for (int col_idx = csr_row_offsets[row]; col_idx < csr_row_offsets[row+1]; 
	 ++col_idx, ++curr_edge) {
      int col = csr_col_indicies[col_idx];
      ij_mat[2*curr_edge] = row;
      ij_mat[2*curr_edge + 1] = col;
    }
  }
  return 0;
}


int _ij_to_csc(int num_nodes, int num_edges, int* ij_mat, 
	       int* csc_col_offsets, int* csc_row_indices)
{
  int curr_edge, curr_col, row, col;

  _mergesort_ij_by_col(ij_mat, num_edges);

  curr_col = 0;
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
  csc_col_offsets[num_nodes] = num_edges;
  return 0;
}


int _csr_to_csc(int num_nodes, int num_edges,
		int* csr_row_offsets, int* csr_col_indices,
		int* csc_col_offsets, int* csc_row_indices)
{
  int* ij_mat = (int*) malloc( sizeof(int) * 2 * num_edges);
  _csr_to_ij(num_nodes, num_edges, csr_row_offsets, csr_col_indices, ij_mat);
  _ij_to_csc(num_nodes, num_edges, ij_mat, csc_col_offsets, csc_row_indices);
  
  return 0;
}


int td_to_gunrock(graph_t* td_graph, struct GunrockGraph* gr_graph)
{
  // define graph
  size_t num_nodes = td_graph->numNodes;
  size_t num_edges = td_graph->numValues;

  int* csc_col_offsets = (int*) malloc(sizeof(int) * num_nodes + 1);
  int* csc_row_indices = (int*) malloc(sizeof(int) * num_edges);
  _csr_to_csc(num_nodes, num_edges, td_graph->rowValueOffsets, td_graph->colOffsets,
	      csc_col_offsets, csc_row_indices);

  gr_graph->num_nodes = num_nodes;
  gr_graph->num_edges = num_edges;
  gr_graph->row_offsets = td_graph->rowValueOffsets;
  gr_graph->col_indices = td_graph->colOffsets;
  gr_graph->col_offsets = (void*)csc_col_offsets;
  gr_graph->row_indices = (void*)csc_row_indices;

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Functions to invoke gunrock clustering algorithms
///////////////////////////////////////////////////////////////////////////////


int gunrock_topk(int top_nodes, graph_t* input_td_graph, graph_t* output_graph)
{
  struct GunrockGraph* gr_input;
  td_to_gunrock(input_td_graph, gr_input);

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
  if (gr_output)      free(gr_output);
  return 0;
}
