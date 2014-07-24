#include "c_import.h"


const char* getfield(char* line, int num)
{
  const char* tok;
  for (tok = strtok(line, ",");
       tok && *tok;
       tok = strtok(NULL, ",\n"))
    {
      if (!--num)
	return tok;
    }
  return NULL;
}

int _parse_graph(FILE* stream, graph_t* output_graph)
{
  char line[1024];

  return 0;
}

int _snap_parse_head(FILE* stream, int* num_nodes_ptr, int* num_edges_ptr)
{
  char line[1024];
  int num_nodes=0, num_edges=0, new_bytes=0, ierr;

  while (fgets(line, 1024, stream)) {
    if (line[0] == '#') {
      if (num_nodes == 0) {
	ierr = sscanf(line, "# Nodes: %d Edges: %d\n", &num_nodes, &num_edges);
	if (ierr < 0 || num_nodes == 0) {
	  num_nodes = 0; 
	  num_edges = 0;
	} else {
	  *num_nodes_ptr = num_nodes;
	  *num_edges_ptr = num_edges;
	}
      }
    } else {
      new_bytes = strlen(line);
      fseek(stream, -new_bytes, SEEK_CUR);
      break;
    }
  }
  if (num_nodes != 0) {
    return 0;
  } else {
    return 1;
  }
}


int _snap_parse(char* filename, graph_t* output_graph)
{
  FILE* stream = fopen(filename, "r");

  int num_nodes=0, num_edges=0, ierr=0, i;

  printf("---> Reading header\n");
  ierr = _snap_parse_head(stream, &num_nodes, &num_edges);
  if (ierr) {
    printf("----> Error reading snap header\n");
    return 1;
  }

  printf("---> Creating graph num_nodes: %d, num_edges: %d\n", num_nodes, num_edges);

  printf("-------> creating name nodes\n");
  output_graph->numNodes = num_nodes;
  output_graph->nodeNames = (char**) malloc(sizeof(char*) * num_nodes);
  for (i = 0; i < num_nodes; ++i) {
    output_graph->nodeNames[i] = (char*) malloc(sizeof(char) * 32);
    sprintf(output_graph->nodeNames[i],"%d", i);
  }
  
  printf("-------> creating values\n");
  output_graph->numValues = num_nodes;
  output_graph->values = (double*) malloc(sizeof(double) * num_nodes);
  for (i = 0; i < num_nodes; ++i) {
    output_graph->values[i] = 0.0;
  }

  printf("-------> creating edges\n");
  output_graph->numRowPtrs = num_nodes;
  output_graph->rowValueOffsets = (int*) malloc(sizeof(int) * num_nodes);
  output_graph->colOffsets = (int*) malloc(sizeof(int) * num_edges);

  printf("-------> parsing graph\n");
  _parse_graph(stream, output_graph);
  printf("-------> finished building graph\n");

  return 0;
}


graph_format_t graph_format_from_str(char* graph_format_str)
{
  graph_format_t ret;
  if (! strcmp(graph_format_str, "WDC")) {
    ret = WDC;
  } else if (! strcmp(graph_format_str, "SNAP")) {
    ret = SNAP;
  }
  return ret;
}


int load_graph(char* filename, graph_format_t graph_format, graph_t* output_graph)
{
  printf("Load graph\n");
  if (graph_format == SNAP) {
    _snap_parse(filename, output_graph);
  } else {
    printf("Unable to read graph format.\n");
    return 1;
  }
  return 0;
}
