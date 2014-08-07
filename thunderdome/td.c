/*
  td

  Routines for starting thunderdome environments and accessing data.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "td.h"
#ifdef _OS_WINDOWS_
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#include <dlfcn.h>
#endif

// TODO: consider including (part of?) libuv for uv_dlopen, uv_exepath

// Standard functions

void td_error(char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

td_tag_t td_typeof(td_val_t *v)
{
    if (td_isobj(v))
        return v->owner->get_type(v->object);
    return v->tag;
}

td_tag_t td_eltype(td_val_t *v)
{
    if (v->tag == TD_ARRAY)
        return ((td_array_t*)v->object)->eltype;
    return v->owner->get_eltype(v->object);
}

void *td_dataptr(td_val_t *v)
{
    if (td_isobj(v))
        return v->owner->get_dataptr(v->object);
    if (v->tag == TD_ARRAY)
        return ((td_array_t*)v->object)->data;
    if (v->tag == TD_UTF8)
        return ((td_string_t*)v->object)->data;
    return (void*)&(v->object);
}

size_t td_length(td_val_t *v)
{
    if (td_isobj(v))
        return v->owner->get_length(v->object);
    if (v->tag == TD_ARRAY)
        return ((td_array_t*)v->object)->length;
    if (v->tag == TD_UTF8)
        return ((td_string_t*)v->object)->length;
    return 1;
}

int8_t   td_int8(td_val_t *v)   { return *(int8_t*)td_dataptr(v); }
uint8_t  td_uint8(td_val_t *v)  { return *(uint8_t*)td_dataptr(v); }
int16_t  td_int16(td_val_t *v)  { return *(int16_t*)td_dataptr(v); }
uint16_t td_uint16(td_val_t *v) { return *(uint16_t*)td_dataptr(v); }
int32_t  td_int32(td_val_t *v)  { return *(int32_t*)td_dataptr(v); }
uint32_t td_uint32(td_val_t *v) { return *(uint32_t*)td_dataptr(v); }
int64_t  td_int64(td_val_t *v)  { return *(int64_t*)td_dataptr(v); }
uint64_t td_uint64(td_val_t *v) { return *(uint64_t*)td_dataptr(v); }
float    td_float(td_val_t *v)  { return *(float*)td_dataptr(v); }
double   td_double(td_val_t *v) { return *(double*)td_dataptr(v); }
void    *td_pointer(td_val_t *v) { return *(void**)td_dataptr(v); }

static size_t type_sizes[] = { 1, 1, 2, 2, 4, 4, 8, 8, 4, 8, sizeof(void*) };

size_t td_type_size(td_tag_t tag)
{
    if (tag < TD_UTF8) return type_sizes[tag];
    return sizeof(void*);
}


// Starting environments

static td_env_t *cached_julia_env=NULL;

td_env_t *td_env_julia(char *plugin_path, char *julia_path)
{
    char path[512];

    if (cached_julia_env == NULL) {
        snprintf(path, sizeof(path), "%s%s%s%s",
                 plugin_path, PATHSEPSTRING, "libtd_julia", SHLIB_EXT);
        void *h = dlopen(path, RTLD_GLOBAL | RTLD_NOW);
        if (h == NULL) {
            fprintf(stderr, "%s\n", dlerror());
            td_error("could not load libjulia");
        }
        void (*init)(char*) = dlsym(h, "td_jl_init");
        init(julia_path);
        assert(cached_julia_env);
    }

    return cached_julia_env;
}

void td_provide_julia(td_env_t *e)
{
    cached_julia_env = e;
}

static td_env_t *cached_python_env=NULL;

td_env_t *td_env_python(char *plugin_path, char *python_path)
{
    char path[512];

    if (cached_python_env == NULL) {
        snprintf(path, sizeof(path), "%s%s%s%s",
                 plugin_path, PATHSEPSTRING, "libtd_python", SHLIB_EXT);//".so"); //SHLIB_EXT);
        void *h = dlopen(path, RTLD_GLOBAL | RTLD_NOW);
        if (h == NULL) {
            fprintf(stderr, "%s\n", dlerror());
            td_error("could not load libpython");
        }
        void (*init)(char*) = dlsym(h, "td_py_init");
        init(python_path);
        assert(cached_python_env);
    }

    return cached_python_env;
}

void td_provide_python(td_env_t *e)
{
    cached_python_env = e;
}

static td_env_t *cached_java_env=NULL;

// looks for libtd_java .so or .dll or .dylib - could be smarter about that.
td_env_t *td_env_java(char *plugin_path, char *classpath, char *java_path)
{
    char path[512];

    if (cached_java_env == NULL) {
        snprintf(path, sizeof(path), "%s%s%s%s",
                 plugin_path, PATHSEPSTRING, "libtd_java", ".so"); //SHLIB_EXT);
        printf("looking for lib at %s\n", path);
        void *h = dlopen(path, RTLD_GLOBAL | RTLD_NOW);
        if (h == NULL) {
        	fprintf(stderr, "%s\n", dlerror());
        	h = dlopen("libtd_java.dll", RTLD_GLOBAL | RTLD_NOW);
        }
        if (h == NULL) {
        	fprintf(stderr, "%s\n", dlerror());
        	h = dlopen("libtd_java.dylib", RTLD_GLOBAL | RTLD_NOW);
        }
        if (h == NULL) {
        	fprintf(stderr, "%s\n", dlerror());
        	td_error("could not load libjava - did you set LD_LIBRARY_PATH to include the libjvm location?\n");
        	td_error("for instance something like : .:/usr/lib/jvm/java-7-openjdk-amd64/jre/lib/amd64/server/\n");
        }
        void (*init)(char*,char*) = dlsym(h, "td_java_init");
        init(classpath, java_path);

        assert(cached_java_env);
    }

    return cached_java_env;
}

void td_provide_java(td_env_t *e)
{
    cached_java_env = e;
}

// WS: added for R
static td_env_t *cached_r_env = NULL;

td_env_t *td_env_r(char *homedir) {
  if (cached_r_env == NULL) {
    void *h = dlopen("libtd_r.so", RTLD_GLOBAL | RTLD_NOW);
    if (h == NULL) {
      fprintf(stderr, "%s\n", dlerror());
      h = dlopen("libtd_r.dll", RTLD_GLOBAL | RTLD_NOW);
    }
    if (h == NULL) {
      fprintf(stderr, "%s\n", dlerror());
      h = dlopen("libtd_r.dylib", RTLD_GLOBAL | RTLD_NOW);
    }
    if (h == NULL) {
      fprintf(stderr, "%s\n", dlerror());
    }
    fprintf(stderr, "h: 0x%px\n",  h);
    td_env_t *(*init)(char*) = dlsym(h, "td_r_init");
    //cached_r_env = init(homedir);
    fprintf(stderr, "got here: 0x%px\n", init);
    init(homedir);
    fprintf(stderr, "after init\n");
    
    assert(cached_r_env);
  }
  return cached_r_env;
}

void td_provide_r(td_env_t *e) {
  cached_r_env = e;
}

int td_create_simple_graph(graph_t *graph)
{
  size_t num_nodes = 7;
  size_t num_edges = 15;
  size_t top_nodes = 3;

  static char* names[7] = {"a", "b", "c", "d", "e", "f", "g"};
  static unsigned int row_offsets[8] = {0,3,6,9,11,14,15,15};
  static int col_indices[15] = {1,2,3,0,2,4,3,4,5,5,6,2,5,6,6};

  graph->numNodes = num_nodes;
  graph->numEdges = num_edges;
  graph->nodeNames = names;
  graph->rowOffsets = row_offsets;
  graph->colIndices = col_indices;

  return 0;
}
