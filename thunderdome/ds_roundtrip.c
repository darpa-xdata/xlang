#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "td.h"
#include "td_ds.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("example:");
        printf("$ ./ds_roundtrip \"3 * 5 * {x : int, y : int}\"\n");
        printf("3 * 5 * { x : int32, y : int32 }\n");
        return 1;
    }

    td_ds_t *ds = td_parse_datashape(argv[1], argv[1] + strlen(argv[1]));
    if (ds == NULL) {
        printf("invalid datashape");
        return 1;
    }
    td_print_datashape(ds);
    printf("\n");
    td_free_datashape(ds);

    return 0;
}
