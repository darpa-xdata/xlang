#ifndef GUNROCK_CLUSTERS_H
#define GUNROCK_CLUSTERS_H

#include <stdio.h>

#include <gunrock.h>
#include <thunderdome/td.h>

int gunrock_topk(int top_nodes, graph_t* input_td_graph, graph_t* output_graph);

int td_to_gunrock(graph_t* td_graph, struct GunrockGraph* gr_graph);


#endif // GUNROCK_CLUSTERS_H
