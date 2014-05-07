/*
 * TD Datashape Parser
 *
 * This parses a subset of the grammar defined in datashape
 * http://datashape.pydata.org/grammar.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "td_ds.h"

// recursive descent parsing functions
static td_ds_t *parse_datashape(const char **pbegin, const char *end);
static td_ds_t *parse_dim_datashape(const char **pbegin, const char *end);
static td_ds_t *parse_dtype(const char **pbegin, const char *end);

static td_ds_t *create_primitive_ds(td_tag_t tag)
{
    td_ds_t *res = malloc(sizeof(td_ds_t));
    if (res == NULL) {
        td_error("out of memory");
        return NULL;
    }
    res->tag = tag;
    return res;
}

static td_ds_t *create_pointer_ds(td_ds_t *tgt)
{
    td_ds_ptr_t *res = malloc(sizeof(td_ds_ptr_t));
    if (res == NULL) {
        td_error("out of memory");
        return NULL;
    }
    res->ds.tag = TD_PTR;
    res->tgt = tgt;
    return &res->ds;
}

static td_ds_t *create_dim_ds(td_ds_t *el, size_t dim_size, intptr_t stride)
{
    td_ds_dim_t *res = malloc(sizeof(td_ds_dim_t));
    if (res == NULL) {
        td_free_datashape(el);
        td_error("out of memory");
        return NULL;
    }
    // Indicate the output is an array
    res->ds.tag = TD_ARRAY;
    res->el = el;
    res->dim_size = dim_size;
    res->stride = stride;

    return &res->ds;
}

static td_ds_struct_t *create_struct_ds()
{
    td_ds_struct_t *res = malloc(sizeof(td_ds_struct_t));
    if (res == NULL) {
        td_error("out of memory");
        return NULL;
    }
    res->ds.tag = TD_STRUCT;
    res->size = 0;
    return res;
}

static td_ds_struct_t *append_struct_ds(td_ds_struct_t *dss,
                                        const char *namebegin,
                                        const char *nameend,
                                        td_ds_t *type,
                                        intptr_t offset)
{
    // Grow the number of fields by 1
    if (dss->size > 0) {
        td_ds_struct_t *newdss = realloc(dss, sizeof(td_ds_struct_t) +
                                         dss->size * sizeof(td_struct_field_t));
        if (newdss == NULL) {
            td_free_datashape((td_ds_t *)dss);
            td_free_datashape(type);
            td_error("out of memory");
            return NULL;
        }
        dss = newdss;
    }
    td_struct_field_t *field = &dss->fields[0] + dss->size;
    char *fieldname = malloc(nameend - namebegin + 1);
    if (fieldname == NULL) {
        td_free_datashape((td_ds_t *)dss);
        td_free_datashape(type);
        td_error("out of memory");
        return NULL;
    }
    memcpy(fieldname, namebegin, nameend - namebegin + 1);
    fieldname[nameend - namebegin] = 0;
    field->name = fieldname;
    field->type = type;
    field->offset = offset;
    dss->size += 1;
    return dss;
}

static void skip_whitespace(const char **pbegin, const char *end)
{
    const char *begin = *pbegin;
    while (begin < end && isspace(*begin)) {
        ++begin;
    }

    // Comments
    if (begin < end && *begin == '#') {
        const char *line_end = (const char *)memchr(begin, '\n', end - begin);
        if (line_end == NULL) {
            begin = end;
        } else {
            begin = line_end + 1;
            skip_whitespace(&begin, end);
        }
    }

    *pbegin = begin;
}

static int parse_token(const char **pbegin, const char *end, const char *token)
{
    const char *begin = *pbegin;
    size_t tokenlen = strlen(token);
    if (tokenlen <= end - begin && memcmp(begin, token, tokenlen) == 0) {
        *pbegin = begin + tokenlen;
        return 1;
    } else {
        return 0;
    }
}

static int parse_char_token(const char **pbegin, const char *end, char token)
{
    const char *begin = *pbegin;
    if (1 <= end - begin && *begin == token) {
        *pbegin = begin + 1;
        return 1;
    } else {
        return 0;
    }
}

// [a-zA-Z_][a-zA-Z0-9_]*
static int parse_name(const char **pbegin, const char *end,
                      const char **out_strbegin, const char **out_strend)
{
    const char *begin = *pbegin;
    if (begin == end) {
        return 0;
    }
    if (('a' <= *begin && *begin <= 'z') || ('A' <= *begin && *begin <= 'Z') ||
            *begin == '_') {
        ++begin;
    } else {
        return 0;
    }
    while (begin < end && (('a' <= *begin && *begin <= 'z') ||
                           ('A' <= *begin && *begin <= 'Z') ||
                           ('0' <= *begin && *begin <= '9') || *begin == '_')) {
        ++begin;
    }
    *out_strbegin = *pbegin;
    *out_strend = begin;
    *pbegin = begin;
    return 1;
}

// # Dimension Type (from the dimension type symbol table)
// dim_datashape : dim ASTERISK datashape
// dim : "strided"
//     | INTEGER
static td_ds_t *parse_dim_datashape(const char **pbegin, const char *end)
{
    const char *begin = *pbegin;
    size_t dim_size;
    if (parse_token(&begin, end, "strided")) {
        dim_size = SIZE_MAX;
    } else if (!parse_size_t(&begin, end, &dim_size)) {
        return NULL;
    }

    skip_whitespace(&begin, end);
    if (!parse_char_token(&begin, end, '*')) {
        return NULL;
    }

    td_ds_t *el = parse_datashape(&begin, end);
    if (el == NULL) {
        return NULL;
    }

    return create_dim_ds(el, dim_size, INTPTR_MAX);
}

// pointer : "pointer"[datashape]
static td_ds_t *parse_pointer(const char **pbegin, const char *end)
{
    const char *begin = *pbegin;
    if (parse_token(&begin, end, "pointer")) {
        if (!parse_char_token(&begin, end, '[')) {
            return NULL;
        }
        skip_whitespace(&begin, end);
        td_ds_t *tgt = parse_datashape(&begin, end);
        if (tgt == NULL) {
            return NULL;
        }
        if (!parse_char_token(&begin, end, ']')) {
            td_free_datashape(tgt);
            td_error("expected closing ] in datashape");
            return NULL;
        }
        *pbegin = begin;
        return create_pointer_ds(tgt);
    } else {
        return NULL;
    }
}

// struct_field_name : NAME COLON datashape
static int parse_struct_field(const char **pbegin, const char *end,
                              const char **pnamebegin, const char **pnameend,
                              td_ds_t **type)
{
    const char *begin = *pbegin;
    if (!parse_name(&begin, end, pnamebegin, pnameend)) {
        return 0;
    }
    skip_whitespace(&begin, end);
    if (!parse_char_token(&begin, end, ':')) {
        return 0;
    }
    *type = parse_datashape(&begin, end);
    if (*type == NULL) {
        return 0;
    }
    *pbegin = begin;
    return 1;
}

// struct_type : LBRACE struct_field_list RBRACE
//             | LBRACE struct_field_list COMMA RBRACE
// struct_field_list : struct_field COMMA struct_field_list
//                   | struct_field
static td_ds_t *parse_struct(const char **pbegin, const char *end)
{
    const char *begin = *pbegin;
    if (!parse_char_token(&begin, end, '{')) {
        return NULL;
    }
    td_ds_struct_t *res = create_struct_ds();
    if (res == NULL) {
        return NULL;
    }
    const char *namebegin, *nameend;
    td_ds_t *type;
    skip_whitespace(&begin, end);
    while (parse_struct_field(&begin, end, &namebegin, &nameend,
                              &type)) {
        res = append_struct_ds(res, namebegin, nameend, type, INTPTR_MAX);
        if (res == NULL) {
            return NULL;
        }
        skip_whitespace(&begin, end);
        if (parse_char_token(&begin, end, ',')) {
            skip_whitespace(&begin, end);
            if (parse_char_token(&begin, end, '}')) {
                *pbegin = begin;
                return &res->ds;
            }
            continue;
        } else if (parse_char_token(&begin, end, '}')) {
            *pbegin = begin;
            return &res->ds;
        } else {
            td_free_datashape((td_ds_t *)res);
            td_error("expected a ',' or '}' in struct");
            return NULL;
        }
    }

    td_free_datashape((td_ds_t *)res);
    td_error("expected a struct field");
    return NULL;
}

// dtype : "int8" | "int16" } "int32" | "int64"
//       | "uint8" | "uint16" | "uint32" | "uint64"
//       | "float32" | "float64"
//       | "int" | "real" |
//       | "string" |
//       | "pointer"[datashape]
//       | struct
static td_ds_t *parse_dtype(const char **pbegin, const char *end)
{
    if (parse_token(pbegin, end, "int8")) {
        return create_primitive_ds(TD_INT8);
    } else if (parse_token(pbegin, end, "int16")) {
        return create_primitive_ds(TD_INT16);
    } else if (parse_token(pbegin, end, "int32")) {
        return create_primitive_ds(TD_INT32);
    } else if (parse_token(pbegin, end, "int64")) {
        return create_primitive_ds(TD_INT64);
    } else if (parse_token(pbegin, end, "uint8")) {
        return create_primitive_ds(TD_UINT8);
    } else if (parse_token(pbegin, end, "uint16")) {
        return create_primitive_ds(TD_UINT16);
    } else if (parse_token(pbegin, end, "uint32")) {
        return create_primitive_ds(TD_UINT32);
    } else if (parse_token(pbegin, end, "uint64")) {
        return create_primitive_ds(TD_UINT64);
    } else if (parse_token(pbegin, end, "float32")) {
        return create_primitive_ds(TD_FLOAT);
    } else if (parse_token(pbegin, end, "float64")) {
        return create_primitive_ds(TD_DOUBLE);
    } else if (parse_token(pbegin, end, "int")) {
        return create_primitive_ds(TD_INT32);
    } else if (parse_token(pbegin, end, "real")) {
        return create_primitive_ds(TD_DOUBLE);
    } else if (parse_token(pbegin, end, "string")) {
        return create_primitive_ds(TD_UTF8);
    }

    td_ds_t *res;
    res = parse_pointer(pbegin, end);
    if (res != NULL) {
        return res;
    }
    
    res = parse_struct(pbegin, end);
    if (res != NULL) {
        return res;
    }

    return NULL;
}

// # Asterisk-separated list of dimensions, followed by data type
// datashape : dim ASTERISK datashape
//           | dtype
static td_ds_t *parse_datashape(const char **pbegin, const char *end)
{
    td_ds_t *res;
    skip_whitespace(pbegin, end);
    res = parse_dim_datashape(pbegin, end);
    if (res == NULL) {
        res = parse_dtype(pbegin, end);
    }
    return res;
}

td_ds_t *td_parse_datashape(const char *strbegin,
                            const char *strend)
{
    td_ds_t *res = parse_datashape(&strbegin, strend);
    if (res != NULL) {
        skip_whitespace(&strbegin, strend);
        if (strbegin == strend) {
            return res;
        } else {
            td_free_datashape(res);
        }
    }

    td_error("unrecognized datashape");
    return 0;
}

void td_free_datashape(td_ds_t *ds)
{
    if (ds == NULL) {
        return;
    } else if (ds->tag == TD_ARRAY) {
        td_ds_dim_t *dsd = (td_ds_dim_t *)ds;
        td_free_datashape(dsd->el);
    } else if (ds->tag == TD_PTR) {
        td_ds_ptr_t *dsp = (td_ds_ptr_t *)ds;
        td_free_dstashape(dsp->tgt);
    } else if (ds->tag == TD_STRUCT) {
        td_ds_struct_t *dss = (td_ds_struct_t *)ds;
        int i, size = dss->size;
        td_struct_field_t *fields = &dss->fields[0];
        for (i = 0; i < size; ++i) {
            free((void *)fields[i].name);
            td_free_datashape(fields[i].type);
        }
    }

    free(ds);
}

