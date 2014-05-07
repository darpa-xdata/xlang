/*
 * TD Datashape
 *
 * This header defines a structure and parsing functionality
 * for creating datashape type objects to be used by TD.
 */

#pragma once

#include "td.h"

/**
 * This is how every td_ds_*_t starts,
 * with a tag indicating the type.
 *
 * TD_INT*, TD_UINT*, TD_FLOAT, TD_DOUBLE,
 * and TD_UTF8 use this directly.
 */
typedef struct {
    td_tag_t tag;
} td_ds_t;

/**
 * TD_ARRAY for one dimension
 */
typedef struct {
    td_ds_t ds;
    /** Element type */
    td_ds_t *el;
    /** The dimension size (SIZE_MAX uninitialized) */
    size_t dim_size;
    /** The stride, in bytes */
    intptr_t stride;
} td_ds_dim_t;

typedef struct {
    /** The name of the field in UTF8 */
    const char *name;
    /** The type of the field */
    td_ds_t *type;
    /** The offset of the field within the struct (INTPTR_MAX uninitialized) */
    intptr_t offset;
} td_struct_field_t;

/**
 * TD_STRUCT
 */
typedef struct {
    td_ds_t ds;
    int size;
    td_struct_field_t fields[1];
} td_ds_struct_t;

/**
 * TD_PTR
 */
typedef struct {
    td_ds_t ds;
    /** Type of the pointer target */
    td_ds_t *tgt;
} td_ds_ptr_t;

/*
 * This function parses a datashape, building a
 * skeleton td_val_t which has the type filled in
 * but all data pointers set as NULL.
 *
 * \param strbegin  The start of the UTF8 string to parse as
 *                  a datashape.
 * \param strend  One past the end of the UTF8 string to parse as
 *                a datashape.
 *
 * \returns  A pointer to a td_ds_t, or NULL on error.
 */
td_ds_t *td_parse_datashape(const char *strbegin, const char *strend);

void td_free_datashape(td_ds_t *ds);

