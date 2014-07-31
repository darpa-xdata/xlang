#ifndef GUNROCK_CLUSTERS_H
#define GUNROCK_CLUSTERS_H

#include <stdio.h>

#include <gunrock.h>
#include <thunderdome/td.h>

int csr_to_csc(int num_nodes, int num_edges,
	       int* csr_row_offsets, int* csr_col_indices,
	       int* csc_col_offsets, int* csc_row_indices);

int gunrock_topk( graph_t* input_td_graph, int top_nodes, int* node_ids, int* centrality_values);

int td_to_gunrock(graph_t* td_graph, struct GunrockGraph* gr_graph);
#endif // GUNROCK_CLUSTERS_H
