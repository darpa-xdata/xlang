/*
  thunderdome

  Common interface for in-memory language interop.
  A glue language, without the language.

  Feature scope:
  - loading/starting and cleaning up environments
  - describing what and where data is, in common formats
  - looking up and invoking entry points
  - describing memory management policies, transferring ownership

  Requirements:
  - any environment can either run main() itself, or be loaded by others
    (the API supports this even if some environments might not)
  - data is generally kept in place, in memory, no copies
  - late binding: the amount of glue code should be O(1) in the number of
    exposed functions.
  - stub/wrapper generation strictly optional
  - willing to accept a bit of overhead for calls (but O(1) in data size)
  - don't need to link against all supported environments
  - operate on objects in place through accessors where possible
*/

#ifndef TD_H
#define TD_H

#include <stdint.h>
#include <stdlib.h>

#if defined(__FreeBSD__)
#define _OS_FREEBSD_
#elif defined(__linux__)
#define _OS_LINUX_
#elif defined(_WIN32) || defined(_WIN64)
#define _OS_WINDOWS_
#elif defined(__APPLE__) && defined(__MACH__)
#define _OS_DARWIN_
#endif

#ifndef SHLIB_EXT
#if defined(__APPLE__)
#define SHLIB_EXT ".dylib"
#elif defined(_OS_WINDOWS_)
#define SHLIB_EXT ".dll"
#else
#define SHLIB_EXT ".so"
#endif
#endif

#ifdef _OS_WINDOWS_
#define PATHSEP '\\'
#define PATHSEPSTRING "\\"
#define PATHLISTSEP ';'
#define PATHLISTSEPSTRING ";"
#else
#define PATHSEP '/'
#define PATHSEPSTRING "/"
#define PATHLISTSEP ':'
#define PATHLISTSEPSTRING ":"
#endif


typedef enum {
    // primitive types
    TD_INT8=0, TD_UINT8, TD_INT16, TD_UINT16,
    TD_INT32, TD_UINT32, TD_INT64, TD_UINT64,
    TD_FLOAT, TD_DOUBLE, TD_PTR,

    // aggregate types
    TD_UTF8, TD_ARRAY, TD_STRUCT,

    // environment-specific objects (can be queried)
    TD_OBJECT,

    TD_UNKNOWN,
    TD_MAX_TAG
} td_tag_t;

#define td_isprim(v) ((v)->tag < TD_UTF8)
#define td_isobj(v)  ((v)->tag > TD_MAX_TAG)

// TD value structs are expected to be ephemeral, e.g. stack-allocated for
// the duration of a call. you can heap-allocate them if you want, but this
// is not required.

// Each of these either stores small inline data, or a pointer to an object
// allocated and owned by some environment.

// TODO: consider moving metadata out of the structs, or put it at the end

typedef struct {
    union {
        td_tag_t tag;
        struct _td_env_t *owner;
    };
    union {
        int8_t int8_val;
        uint8_t uint8_val;
        int16_t int16_val;
        uint16_t uint16_val;
        int32_t int32_val;
        uint32_t uint32_val;
        int64_t int64_val;
        uint64_t uint64_val;
        float float_val;
        double double_val;
        void *ptr_val;

        void *object;  // if "owner" instead of "tag"
        // if tag is TD_UTF8, object points to a td_string_t
        // if tag is TD_ARRAY, object points to a td_array_t

        // NOTE: the tag in this struct describes the representation of
        // the struct, not the underlying data. e.g.
        // (td_typeof(v)==TD_INT8) != (v->tag == TD_INT8)

        // Always *read* a td_val_t through the provided functions.
        // Set up the representation directly when *writing* one.
    };
} td_val_t;

typedef struct _td_env_t {
    char *name;

    // for environments with a command language
    void (*eval)(td_val_t *out, char *str);

    // TODO: maybe a separate function lookup step

    void (*invoke0)(td_val_t *out, char *f);
    void (*invoke1)(td_val_t *out, char *f, td_val_t *);
    void (*invoke2)(td_val_t *out, char *f, td_val_t *, td_val_t *);
    void (*invoke3)(td_val_t *out, char *f, td_val_t *, td_val_t *, td_val_t *);

    // acquire and release references to owned objects
    void (*retain)(void *obj);
    void (*release)(void *obj);

    // the following accessors apply to env-specific objects.
    // this allows environments to return their own native objects, and have
    // other environments find out about them.

    // the TD datatype an object corresponds to, if any
    td_tag_t (*get_type)(void *);

    // an element type, for arrays etc.
    td_tag_t (*get_eltype)(void *);

    // get pointer to raw data
    void *(*get_dataptr)(void *);

    size_t (*get_length)(void *);
    size_t (*get_ndims)(void *);

    // array dimensions
    void (*get_dims)(void *, size_t *dims);
    void (*get_strides)(void *, size_t *strides);
} td_env_t;


// some environments do not store metadata for values (e.g. C). they can use
// these structs as their "objects" if they want.
// TODO: decide how to manage the storage for these. Possibly regions.

typedef struct {
    void *data;
    size_t length;
    td_tag_t eltype;
    size_t ndims;
    size_t size1;
    size_t stride1;
    // etc.
} td_array_t;

typedef struct {
    void *data;
    size_t length;
} td_string_t;


// accessors

size_t td_type_size(td_tag_t tag);

td_tag_t td_typeof(td_val_t *v);
td_tag_t td_eltype(td_val_t *v);
void *td_dataptr(td_val_t *v);
size_t td_length(td_val_t *v);

int8_t   td_int8(td_val_t *v);
uint8_t  td_uint8(td_val_t *v);
int16_t  td_int16(td_val_t *v);
uint16_t td_uint16(td_val_t *v);
int32_t  td_int32(td_val_t *v);
uint32_t td_uint32(td_val_t *v);
int64_t  td_int64(td_val_t *v);
uint64_t td_uint64(td_val_t *v);
float    td_float(td_val_t *v);
double   td_double(td_val_t *v);
void    *td_pointer(td_val_t *v);


// utilities

void td_error(char *msg);


// starting environments

td_env_t *td_env_julia(char *, char*);  // to get the julia env
void td_provide_julia(td_env_t *e);     // called by julia main if it runs

td_env_t *td_env_r(char *, char*);  // to get the r env
void td_provide_r(td_env_t *e);     // called by r main if it runs

td_env_t *td_env_python(char *, char*);  // to get the julia env
void td_provide_python(td_env_t *e);     // called by julia main if it runs

td_env_t *td_env_java(char *, char*, char*);  // to get the java env
void td_provide_java(td_env_t *e);     // called by java main if it runs
td_env_t *get_java(const char *classpath, const char *javaClass);  // to get the java env


#endif
