#ifndef GUNROCK_CLUSTERS_H
#define GUNROCK_CLUSTERS_H

#include <stdio.h>

#include <gunrock.h>
#include <thunderdome/td.h>

int gunrock_topk( graph_t* input_td_graph, int top_nodes, int* node_ids, int* in_degrees, int* out_degrees);

int td_to_gunrock(graph_t* td_graph, struct GunrockGraph* gr_graph);
#endif // GUNROCK_CLUSTERS_H
