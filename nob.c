#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    Cmd cmd = {0};
    cmd_append(&cmd, "cc", "-Wall", "-Wextra", "-ggdb");
    cmd_append(&cmd, "-o", "example");
    cmd_append(&cmd, "example.c");
    if (!cmd_run_sync_and_reset(&cmd)) return 1;

    return 0;
}
