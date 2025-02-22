#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define Csv_String_View Nob_String_View
#define CSV_IMPLEMENTATION
#include "csv.h"

typedef struct {
    float a;
    float b;
    float c;
    String_View name;
} Entry;

typedef struct {
    Entry *items;
    size_t count, capacity;
} Entries;

int main(void) {
    String_Builder sb = {0};
    if (!read_entire_file("examples/test_floats.csv", &sb)) return 1;
    String_View file = sb_to_sv(sb);

    Entries entries = {0};

    String_View row = {0};
    while (csv_next_row(&file, &row, csv_default_config)) {
        Entry entry;
        CSV_FILL_STRUCT(row, csv_default_config, entry, CSVF(a), CSVF(b), CSVF(c), CSVF(name));

        da_append(&entries, entry);
    }

    for (size_t i = 0; i < entries.count; i++) {
        Entry entry = entries.items[i];
        nob_log(INFO, "%.2f * %.2f = %.2f (discovered by "SV_Fmt")", entry.a, entry.b, entry.c, SV_Arg(entry.name));
    }

    return 0;
}
