#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

#define CSV_IMPLEMENTATION
#include "csv.h"

#define return_defer(value) do { result = (value); goto defer; } while(0)

bool read_entire_file(const char *path, Csv_String_View *sv) {
    bool result = true;

    FILE *f = fopen(path, "rb");
    if (f == NULL)                 return_defer(false);
    if (fseek(f, 0, SEEK_END) < 0) return_defer(false);
    long m = ftell(f);
    if (m < 0)                     return_defer(false);
    if (fseek(f, 0, SEEK_SET) < 0) return_defer(false);

    char *data = malloc(m);
    assert(data != NULL && "Buy MORE RAM!! lol");

    fread(data, m, 1, f);
    if ((errno = ferror(f))) return_defer(false);

    sv->data = data;
    sv->count = m;

defer:
    if (!result) fprintf(stderr, "ERROR: Could not read file %s: %s\n", path, strerror(errno));
    if (f) fclose(f);
    return result;
}

int main(void) {
    Csv_String_View file = {0};
    if (!read_entire_file("examples/test.csv", &file)) return 1;

    Csv_String_View row = {0};
    while (csv_next_row(&file, &row, csv_default_config)) {
        printf("New Row\n");
        Csv_String_View item = {0};
        while (csv_next_item(&row, &item, csv_default_config)) {
            printf("Item: %.*s\n", (int)item.count, item.data);
        }
    }

    return 0;
}
