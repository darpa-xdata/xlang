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
  int src_node, dst_node, ierr;
  int curr_edge = 0, curr_row = 0;
  int num_nodes = output_graph->numNodes;
  int num_edges = output_graph->numValues;
  int* row_offsets = output_graph->rowValueOffsets;
  int* col_offsets = output_graph->colOffsets;

  row_offsets[0] = 0;
  while (fgets(line, 1024, stream)) {
    if (line[0] == '#') {
      continue;
    }
    ierr = sscanf(line, "%d\t%d\n", &src_node, &dst_node);
    if (ierr < 0) {
      printf("Error reading graph line: %s\n", line);
      return 1;
    } else if (src_node >= num_nodes || dst_node >= num_nodes){
      printf("Error reading graph line: %s\n----> read a node that is too large, num nodes is %d\n",
	     line, num_nodes);
      return 1;
    } else if (src_node < 0 || dst_node < 0) {
      printf("Error reading graph line: %s\n----> recieved negative node\n", line);
      return 1;
    }

    while (src_node != curr_row) {
      if (src_node < curr_row) {
	printf("Error src nodes not monotonoically increasing, line %s\n", line);
	return 1;
      }
      curr_row += 1;
      if (curr_row > num_nodes) {
	printf("Error row out of bounds curr_row:%d, line: %s\n", curr_row, line);
	return 1;
      }
      row_offsets[curr_row] = curr_edge;
    }

    col_offsets[curr_edge] = dst_node;
    
    curr_edge += 1;
    if (curr_edge > num_edges) {
	printf("Error edges out of bounds curr_edge:%d, last line read: %s\n", curr_edge, line);
	return 1;
    }
  }

  curr_row += 1;
  row_offsets[curr_row] = num_edges;


  if (curr_edge != num_edges) {
    printf("Error not enough edges read only found %d\n", curr_edge);
    return 1;
  }

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
  output_graph->numValues = num_edges;
  output_graph->values = (double*) malloc(sizeof(double) * num_edges);
  for (i = 0; i < num_edges; ++i) {
    output_graph->values[i] = 0.0;
  }

  printf("-------> creating edges\n");
  output_graph->numRowPtrs = num_nodes;
  output_graph->rowValueOffsets = (int*) malloc(sizeof(int) * (num_nodes + 1));
  output_graph->colOffsets = (int*) malloc(sizeof(int) * num_edges);

  printf("-------> parsing graph\n");
  ierr = _parse_graph(stream, output_graph);
  if (ierr) {
    printf("------> Error parsing graph\n");
    return 1;
  }
  printf("-------> finished building graph\n");

  return 0;
}


int _count_lines(FILE* stream){
  int count = 0;
  char line[1024];

  while(fgets(line, 1024, stream)) {
    if (line[0] == '#' || strlen(line) == 0) continue;
    count +=1;
  }
  fseek(stream, 0, SEEK_SET);
  return count;
}

int _wdc_parse_index(FILE* stream, int num_nodes, char** node_names){
  char line[1024], url[1024];
  int curr_node = 0, node_id, url_size, ierr;

  while(fgets(line, 1024, stream)) {
    ierr = sscanf(line, "%s\t%d\n", url, &node_id);
    if (ierr < 0) {
      printf("Error reading index line: %s\n", line);
      return 1;
    }
    if (node_id < curr_node) {
      printf("-------> Error index nodes not monotonically increasing, line: %s", line);
      return 1;
    }

    while (curr_node < node_id) {
      curr_node += 1;
    }

    if (curr_node >= num_nodes) {
      printf("-------> Error hit node bigger than max_node");
      return 1;
    }
    url_size = strlen(url);
    node_names[curr_node] = (char*) malloc(sizeof(char) * url_size);
    strcpy(node_names[curr_node], url);
  }

  return 0;
}

int _wdc_parse(char* arc_filename, char* index_filename, graph_t* output_graph)
{
  FILE* arc_stream = fopen(arc_filename, "r");
  FILE* index_stream = fopen(index_filename, "r");
  int num_nodes=0, num_edges=0, ierr=0, i;

  printf("---> Getting number of nodes and edges\n");
  num_nodes = _count_lines(index_stream);
  num_edges = _count_lines(arc_stream);
  printf("-------> Found %d nodes, %d edges\n", num_nodes, num_edges);

  printf("-------> creating name nodes\n");
  output_graph->numNodes = num_nodes;
  output_graph->nodeNames = (char**) malloc(sizeof(char*) * num_nodes);
  _wdc_parse_index(index_stream, num_nodes, output_graph->nodeNames);

  printf("-------> creating values\n");
  output_graph->numValues = num_edges;
  output_graph->values = (double*) malloc(sizeof(double) * num_edges);
  for (i = 0; i < num_edges; ++i) {
    output_graph->values[i] = 0.0;
  }

  printf("-------> creating edges\n");
  output_graph->numRowPtrs = num_nodes;
  output_graph->rowValueOffsets = (int*) malloc(sizeof(int) * (num_nodes + 1));
  output_graph->colOffsets = (int*) malloc(sizeof(int) * num_edges);

  printf("-------> parsing graph\n");
  ierr = _parse_graph(arc_stream, output_graph);
  if (ierr) {
    printf("------> Error parsing graph\n");
    return 1;
  }
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


int load_graph(graph_format_t graph_format, char* edge_filename, 
	       char* index_filename,  graph_t* output_graph)
{
  printf("Load graph\n");
  switch(graph_format) {
  case SNAP:
    if (edge_filename == NULL) {
      printf("---> Error not given an edge_filename\n");
      return 1;
    }
    _snap_parse(edge_filename, output_graph);
    break;
  case WDC:
    if (edge_filename == NULL || index_filename == NULL) {
      printf("---> Error not given an edge_filename or index_filename\n");
      return 1;
    }
    _wdc_parse(edge_filename, index_filename, output_graph);
    break;
  default:
    printf("Unable to read graph format.\n");
    return 1;
  }
  return 0;
}
