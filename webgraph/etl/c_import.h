/*
  c_import

  Simple loader for web graphs into a thunderdome graph struct that
  doesn't have any dependicies. Intended for very simple testing.

*/

#ifndef C_IMPORT_H
#define C_IMPORT_H

#include <stdio.h>
#include <string.h>

#include "thunderdome/td.h"

typedef enum {WDC, SNAP} graph_format_t;

graph_format_t graph_format_from_str(char* graph_format_str);
int load_graph(char* filename, graph_format_t graph_format, graph_t* output_graph);

#endif // C_IMPORT_H
