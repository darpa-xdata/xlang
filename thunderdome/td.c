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
