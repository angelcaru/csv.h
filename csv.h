// csv.h (public domain - https://github.com/angelcaru/csv.h)
// TODO: write documentation

#ifndef CSV_H_
#define CSV_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef Csv_String_View
typedef struct {
    char *data;
    size_t count;
} Csv_String_View;
#endif // Csv_String_View

#ifndef csv_sv_data
#define csv_sv_data(sv) (sv).data
#endif // csv_sv_data

#ifndef csv_sv_count
#define csv_sv_count(sv) (sv).count
#endif // csv_sv_count

#ifndef csv_sv_from_parts
#define csv_sv_from_parts(data_, count_) ((Csv_String_View) { .data = (data_), .count = (count_) })
#endif // csv_sv_from_parts

typedef struct {
    char col_delim;
    char row_delim;
    char quote;
    char escape;
} Csv_Config;

bool csv_next_row(Csv_String_View *file, Csv_String_View *row, Csv_Config config);
bool csv_next_item(Csv_String_View *row, Csv_String_View *item, Csv_Config config);

extern Csv_Config csv_default_config;

#endif // CSV_H_

#ifdef CSV_IMPLEMENTATION

Csv_Config csv_default_config = {
    .col_delim = ',',
    .row_delim = '\n',
    .quote = '"',
    .escape = '\\',
};

// Stolen from https://github.com/tsoding/nob.h
Csv_String_View csv_sv_chop_by_delim(Csv_String_View *sv, char delim) {
    size_t i = 0;
    while (i < csv_sv_count(*sv) && sv->data[i] != delim) {
        i += 1;
    }

    Csv_String_View result = csv_sv_from_parts(csv_sv_data(*sv), i);

    if (i < csv_sv_count(*sv)) {
        csv_sv_count(*sv) -= i + 1;
        csv_sv_data(*sv)  += i + 1;
    } else {
        csv_sv_count(*sv) -= i;
        csv_sv_data(*sv)  += i;
    }

    return result;
}

bool csv_next_row(Csv_String_View *file, Csv_String_View *row, Csv_Config config) {
    if (csv_sv_count(*file) == 0) return false;

    *row = csv_sv_chop_by_delim(file, config.row_delim);
    return true;
}

bool csv_next_item(Csv_String_View *row, Csv_String_View *item, Csv_Config config) {
    if (csv_sv_count(*row) == 0) return false;
    if (csv_sv_data(*row)[0] == config.quote) {
        // Skip starting quote
        csv_sv_data(*row)  += 1;
        csv_sv_count(*row) -= 1;

        csv_sv_data(*item) = csv_sv_data(*row);

        bool escape = false;
        while (escape || *csv_sv_data(*row) != config.quote) {
            if (!escape && *csv_sv_data(*row) == config.escape) {
                escape = true;
            } else {
                escape = false;
            }

            csv_sv_data(*row)  += 1;
            csv_sv_count(*row) -= 1;
        }

        csv_sv_count(*item) = csv_sv_data(*row) - csv_sv_data(*item);

        // Skip ending quote
        csv_sv_data(*row)  += 1;
        csv_sv_count(*row) -= 1;

        if (*csv_sv_data(*row) == config.col_delim) {
            csv_sv_data(*row)  += 1;
            csv_sv_count(*row) -= 1;
        }
    } else {
        *item = csv_sv_chop_by_delim(row, config.col_delim);
    }
    return true;
}

#endif // CSV_IMPLEMENTATION
