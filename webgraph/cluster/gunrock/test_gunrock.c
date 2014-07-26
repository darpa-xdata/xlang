#include "gunrock_clusters.h"
#include <string.h>

int _create_simple_td_graph(graph_t *graph)
{
  size_t num_nodes = 7;
  size_t num_edges = 15;
  size_t top_nodes = 3;

  unsigned int row_offsets[8] = {0,3,6,9,11,14,15,15};
  int col_indices[15] = {1,2,3,0,2,4,3,4,5,5,6,2,5,6,6};
  
  graph->numNodes = num_nodes;
  graph->numValues = num_edges;
  graph->rowValueOffsets = (int*) malloc(sizeof(int) * num_nodes+1);
  graph->colOffsets = (int*) malloc(sizeof(int) * num_edges);

  memcpy(graph->rowValueOffsets, row_offsets, num_nodes+1);
  memcpy(graph->colOffsets, col_indices, num_edges);

  return 0;  
}

int _destroy_simple_td_graph(graph_t *graph)
{
  if(graph->rowValueOffsets) free(graph->rowValueOffsets);
  if(graph->colOffsets) free(graph->colOffsets);
  return 0;
}

int test_gunrock_graph_convert()
{
  graph_t td_graph;
  struct GunrockGraph gr_graph;

  _create_simple_td_graph(&td_graph);
  _td_to_gunrock(&td_graph, &gr_graph);

  printf("gunrock col_offsets\n");
  printf("------> {");

  for(int i=0; i <= gr_graph.num_nodes; ++i) {
    printf("%d", ((int*)gr_graph.col_offsets)[i]);
    if (i <= gr_graph.num_nodes)
      printf(", ");
    else
      printf("}");      
  }

  printf("gunrock col_offsets\n");
  printf("------> {");

  for(int i=0; i < gr_graph.num_edges; ++i) {
    printf("%d", ((int*) gr_graph.row_indices)[i]);
    if (i < gr_graph.num_edges)
      printf(", ");
    else
      printf("}");      
  }

  return 0;
}


int test_gunrock_topk()
{
  int top_nodes = 10;

  // malloc output result arrays
  struct GunrockGraph *graph_output =
    (struct GunrockGraph*)malloc(sizeof(struct GunrockGraph));
  int *node_ids          = (int*)malloc(sizeof(int) * top_nodes);
  int *centrality_values = (int*)malloc(sizeof(int) * top_nodes);

  return 0;
}


int main(int argc, char** argv)
{
  int ret = 0;
  ret |= test_gunrock_graph_convert();
  ret |= test_gunrock_topk();
  return ret;
}
