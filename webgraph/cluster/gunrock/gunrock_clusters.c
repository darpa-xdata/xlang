#include "gunrock_clusters.h"
#ifdef USE_PTHREADS
  #include <pthread.h>
#endif // USE_PTHREADS


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
  int curr_a = 0;
  int curr_b = num_a_edges;
  int total_edges = num_a_edges + num_b_edges;

  while(curr_a < num_a_edges && curr_b < total_edges) {
    if (_compare_ij_cols_idx(ij_mat, curr_a, curr_b)){
      _swap_ij_idx(ij_mat, curr_a, curr_b);

      // Put the value swaped into in the correct spot in b
      for (; curr_b<total_edges; ++curr_b) {
	if (_compare_ij_cols_idx(ij_mat, curr_b, curr_b+1))
	  _swap_ij_idx(ij_mat, curr_b, curr_b+1);
	else
	  break;
      }
    }
    ++curr_a;
  }
  return 0;
}


// Using a simple inplace mergesort since it is stable and the csr format is 
// already sorted by row
int _mergesort_ij_by_col(int* ij_mat, int num_edges)
{
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


#ifdef USE_PTHREADS

typedef struct {
  int* ij_mat;
  int num_edges;
  int num_left;
  int num_right;
} arg_t;

void* _parallel_mergesort_entry(void* arg)
{
  arg_t *arg_cast = (arg_t*)arg;
  _mergesort_ij_by_col(arg_cast->ij_mat, arg_cast->num_edges);
  return NULL;
}

void* _parallel_merge_entry(void* arg)
{
  arg_t *arg_cast = (arg_t*)arg;
  _merge_ij_by_col(arg_cast->ij_mat, arg_cast->num_left, arg_cast->num_right);
  return NULL;
}

int _parallel_mergesort_ij_by_col(int* ij_mat, int num_edges)
{
  int num_threads = 8;
  int edges_per_thread = num_edges / num_threads;
  void* end;
  int rc;

  pthread_t threads[num_threads];
  arg_t args[num_threads];

  int edges[num_threads];
  for (int idx=0; idx < num_threads; ++idx)
    edges[idx] = (idx < num_edges % num_threads) ? edges_per_thread + 1 
                                                 : edges_per_thread; 

  for (int idx=0; idx < num_threads; ++idx){
    args[idx].num_edges = edges[idx];
    int offset = 0;
    for (int e_idx=0; e_idx < idx; ++e_idx) offset += 2*edges[e_idx];
    args[idx].ij_mat = ij_mat + offset;

    rc = pthread_create(&threads[idx], NULL, _parallel_mergesort_entry, &args[idx]);
    if ( rc ){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

  for (int idx=0; idx < num_threads; ++idx){
    rc = pthread_join(threads[idx], end);
    if( rc ) {
      printf("ERROR; return code from pthread_join() is %d\n", rc);
      exit(-1);
    }
  }


  for ( num_threads >>= 1; num_threads > 0; num_threads >>= 1){
    edges_per_thread = num_edges / num_threads;
    
    int lefts[num_threads];
    int rights[num_threads];
    int offsets[num_threads];
    for (int idx=0, cum=0; idx < num_threads; ++idx){
      edges[idx] = (idx < num_edges % num_threads) ? edges_per_thread + 1 
	                                           : edges_per_thread; 
      rights[idx] = edges[idx] / 2;
      lefts[idx] = edges[idx] - rights[idx];
      offsets[idx] = cum;
      cum += edges[idx];
    }

    for (int idx=0; idx < num_threads; ++idx){
      args[idx].num_left = lefts[idx];
      args[idx].num_right = rights[idx];

      args[idx].ij_mat = ij_mat + 2*offsets[idx];

      rc = pthread_create(&threads[idx], NULL, _parallel_merge_entry, &args[idx]);
      if ( rc ){
	printf("ERROR; return code from pthread_create() is %d\n", rc);
	exit(-1);
      }
    }

    for (int idx=0; idx < num_threads; ++idx){
      rc = pthread_join(threads[idx], end);
      if(rc){
	  printf("ERROR; return code from pthread_join() is %d\n", rc);
	  exit(-1);
      } 
    }
  }

  return 0;  
}

#endif // USE_PTHREADS

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

  printf("------> Sorting ij_mat (edgelist)\n");
#ifdef USE_PTHREADS
  _parallel_mergesort_ij_by_col(ij_mat, num_edges);
#else
  _mergesort_ij_by_col(ij_mat, num_edges);
#endif // USE_PTHREADS


  printf("------> Building csr\n");
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
  printf("------> Converting csr to ij_mat (edgelist)\n");
  _csr_to_ij(num_nodes, num_edges, csr_row_offsets, csr_col_indices, ij_mat);
  printf("------> Converting ij_mat (edgelist) to csr\n");
  _ij_to_csc(num_nodes, num_edges, ij_mat, csc_col_offsets, csc_row_indices);
  if(ij_mat) free(ij_mat);
  return 0;
}


int td_to_gunrock(graph_t* td_graph, struct GunrockGraph* gr_graph)
{
  printf("------> Converting Thunderdome graph to GunRock graph\n");

  // define graph
  size_t num_nodes = td_graph->numNodes;
  size_t num_edges = td_graph->numEdges;

  int* csc_col_offsets = (int*) malloc(sizeof(int) * num_nodes + 1);
  int* csc_row_indices = (int*) malloc(sizeof(int) * num_edges);
  _csr_to_csc(num_nodes, num_edges, td_graph->rowOffsets, td_graph->colIndices,
	      csc_col_offsets, csc_row_indices);

  gr_graph->num_nodes = num_nodes;
  gr_graph->num_edges = num_edges;
  gr_graph->row_offsets = td_graph->rowOffsets;
  gr_graph->col_indices = td_graph->colIndices;
  gr_graph->col_offsets = (void*)csc_col_offsets;
  gr_graph->row_indices = (void*)csc_row_indices;

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Functions to invoke gunrock clustering algorithms
///////////////////////////////////////////////////////////////////////////////


int gunrock_topk( graph_t* input_td_graph, int top_nodes, int* node_ids, int* centrality_values)
{
  printf("GunRock TopK\n");

  struct GunrockGraph gr_input;
  struct GunrockGraph gr_output;
  struct GunrockDataType data_type;
  data_type.VTXID_TYPE = VTXID_INT;
  data_type.SIZET_TYPE = SIZET_UINT;
  data_type.VALUE_TYPE = VALUE_INT;


  td_to_gunrock(input_td_graph, &gr_input);

  // run topk calculations
  printf("------> Running topk dispatch\n");
  topk_dispatch(
    (struct GunrockGraph*) &gr_output,
    node_ids,
    centrality_values,
    (const struct GunrockGraph*) &gr_input,
    top_nodes,
    data_type);

  if (gr_input.col_offsets) free(gr_input.col_offsets);
  if (gr_input.row_indices) free(gr_input.row_indices);

  printf("-----> Finished topk dispatch\n");
  return 0;
}
