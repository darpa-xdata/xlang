/*
 * TD Datashape Parser
 *
 * This function parses a datashape, building a
 * skeleton td_val_t which has the type filled in
 * but all data pointers set as NULL.
 */

#pragma once

#include "td.h"

/*
 * This function parses a datashape, building a
 * skeleton td_val_t which has the type filled in
 * but all data pointers set as NULL.
 *
 * \param out  An uninitialized td_val_t which is populated
 * \param str  The string, assumed to be UTF8, which is parsed
 *             as a datashape.
 *
 * \returns  0 on success, -1 on error.
 */
int td_parse_datashape(td_val_t *out, const char *str);

