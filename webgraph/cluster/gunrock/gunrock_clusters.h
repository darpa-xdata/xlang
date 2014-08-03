#ifndef GUNROCK_CLUSTERS_H
#define GUNROCK_CLUSTERS_H

#include <stdio.h>

#include <gunrock.h>
#include <thunderdome/td.h>

int gunrock_topk(
    graph_t* input_td_graph, int top_nodes, int* node_ids, int* in_degrees, int* out_degrees);

int gunrock_pr(
    graph_t* intput_td_graph, int top_nodes, int* node_ids, float* page_rank);

int td_to_gunrock(graph_t* td_graph, struct GunrockGraph* gr_graph, bool csc_convert);
#endif // GUNROCK_CLUSTERS_H
