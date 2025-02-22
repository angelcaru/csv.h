#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

bool build_example(const char *src_path, const char *dst_path) {
    bool result = true;

    Cmd cmd = {0};
    cmd_append(&cmd, "cc", "-Wall", "-Wextra", "-ggdb");
    cmd_append(&cmd, "-I.");
    cmd_append(&cmd, "-o", dst_path);
    cmd_append(&cmd, src_path);
    if (!cmd_run_sync_and_reset(&cmd)) return_defer(false);

defer:
    free(cmd.items);
    return result;
}

const char *examples[] = {
    "example_basic",
};

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!mkdir_if_not_exists("build")) return 1;

    for (size_t i = 0; i < ARRAY_LEN(examples); i++) {
        const char *example = examples[i];
        if (!build_example(temp_sprintf("examples/%s.c", example), temp_sprintf("build/%s", example))) return 1;
    }

    UNUSED(shift_args(&argc, &argv));

    if (argc == 0) return 0;

    const char *arg = shift_args(&argc, &argv);
    for (size_t i = 0; i < ARRAY_LEN(examples); i++) {
        const char *example = examples[i];
        if (strcmp(example, arg) == 0) {
            Cmd cmd = {0};
            cmd_append(&cmd, temp_sprintf("build/%s", example));
            da_append_many(&cmd, argv, argc);
            if (!cmd_run_sync_and_reset(&cmd)) return 1;
            return 0;
        }
    }

    nob_log(ERROR, "Unknown example: %s", arg);
    return 0;
}
