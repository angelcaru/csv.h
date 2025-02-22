// csv.h (public domain - https://github.com/angelcaru/csv.h)
// TODO: write documentation

#ifndef CSV_H_
#define CSV_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

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

// Macros for internal use by the macro magic below
#define CSV_SUPPORTED_TYPES 3
static_assert(CSV_SUPPORTED_TYPES == 3, "Exhaustive handling of supported types in CSV__TYPE");
#define CSV__TYPE(val) _Generic((val), \
        int: "int",     \
        float: "float",   \
        Csv_String_View: "string_view"   \
    )

#define CSV__ARRAY_LEN(array) (sizeof(array)/sizeof(*array))

typedef struct {
    size_t offset;
    const char *type;
} Csv__Field;

void csv__fill_struct(void *out, Csv_String_View *row, Csv_Config, const Csv__Field *fields, size_t fields_count);

// CSVF is short for CSV_FIELD
#define CSVF(field) { .offset = (size_t)&((typeof(_out))NULL)->field, .type = CSV__TYPE(_out->field) }
#define CSV_FILL_STRUCT(row, config, out, ...) do { \
        typeof(out) *_out = &out;           \
        Csv__Field fields[] = {__VA_ARGS__};    \
        csv__fill_struct(_out, &(row), (config), fields, CSV__ARRAY_LEN(fields)); \
    } while(0)

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

void csv__fill_struct(void *out_v, Csv_String_View *row, Csv_Config config, const Csv__Field *fields, size_t fields_count) {
    char *out = out_v;
    Csv_String_View item = {0};
    for (size_t i = 0; i < fields_count && csv_next_item(row, &item, config); i++) {
        Csv__Field field = fields[i];
        void *field_loc = out + field.offset;

        static_assert(CSV_SUPPORTED_TYPES == 3, "Exhaustive handling of supported types in csv__fill_struct()");
        if (strcmp(field.type, "int") == 0) {
            int val = atoi(csv_sv_data(item));
            memcpy(field_loc, &val, sizeof(int));
        } else if (strcmp(field.type, "float") == 0) {
            float val = (float)atof(csv_sv_data(item));
            memcpy(field_loc, &val, sizeof(float));
        } else if (strcmp(field.type, "string_view") == 0) {
            memcpy(field_loc, &item, sizeof(Csv_String_View));
        } else {
            assert(0 && "unreachable");
        }
    }
}

#endif // CSV_IMPLEMENTATION
