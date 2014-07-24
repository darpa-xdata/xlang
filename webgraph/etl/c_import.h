/*
  c_import

  Simple loader for web graphs into a thunderdome graph struct that
  doesn't have any dependicies. Intended for very simple testing.

*/

#ifndef C_IMPORT_H
#define C_IMPORT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../thunderdome/td.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef enum {WDC, SNAP} graph_format_t;


graph_format_t graph_format_from_str(char* graph_format_str);
int load_graph(graph_format_t graph_format, char* edge_filename, 
	       char* index_filename, graph_t* output_graph);

#ifdef __cplusplus
}
#endif

#endif // C_IMPORT_H
