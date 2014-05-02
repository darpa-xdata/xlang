/*
 * TD Datashape Parser
 *
 * This parses a subset of the grammar defined in datashape
 * http://datashape.pydata.org/grammar.html
 */

#include <td_ds.h>

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
    if (tokenlen <= end - begin && memcmp(begin_skipws, token, tokenlen) == 0) {
        *pbegin = begin + tokenlen;
        return 1;
    } else {
        return 0;
    }
}

static int parse_token(const char *&begin, const char *end, char token)
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
    rbegin = begin;
    return 1;
}

// # Dimension Type (from the dimension type symbol table)
// dim : "strided"
//     | INTEGER
int parse_dim(td_val_t *out, const char **pbegin, const char *end)
{
    size_t dim_size;
    if (parse_token(pbegin, end, "strided")) {
        dim_size = SIZE_MAX;
    } else if (!parse_size_t(pbegin, end, &dim_size)) {
        return 0;
    }

    // Indicate the output is an array
    out->tag = TD_ARRAY;
    out->object = malloc(sizeof(td_array_t));

    // Allocate a td_array_t and populate it,
    // except for the data
    td_array_t *arr = (td_array_t *)out->object;
    arr->data = NULL;
    arr->length = dim_size;
    arr->ndims = 1;
}

// # Comma-separated list of dimensions, followed by data type
// datashape : dim ASTERISK datashape
//           | dtype
int parse_datashape(td_val_t *out, const char **pbegin, const char *end)
{
    skip_whitespace(pbegin, end);
    if (parse_dim(out, pbegin, end)) {
        TODO
    } else if (parse_dtype(out, pbegin, end)) {
        return 1;
    } else {
        return 0;
    }
}

int td_parse_datashape(td_val_t *out, const char *str_begin,
                       const char *str_end)
{
    if (parse_datashape(out, str_begin, str_end)) {
        skip_whitespace(str_begin, str_end);
        if (str_begin == str_end) {
            return 1;
        }
    }

    td_error("unrecognized datashape");
    return 0;
}

