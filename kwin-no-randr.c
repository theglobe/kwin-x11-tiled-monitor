/* SPDX-License-Identifier: GPL-3.0-or-later */
/* LD_PRELOAD shim for kwin_x11: hides the RANDR extension so KWin 5.27
 * treats the whole X screen (tiled monitors driven over two cables, e.g. 5K) as ONE output.
 * Effect: maximize covers all 5120x2880, not one 2560-wide tile.
 * Only active inside a process named kwin_x11. */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

typedef struct xcb_connection_t xcb_connection_t;
typedef struct { const char *name; int global_id; } xcb_extension_t;
typedef struct {
    uint8_t response_type, pad0; uint16_t sequence; uint32_t length;
    uint8_t present, major_opcode, first_event, first_error;
} xcb_query_extension_reply_t;

static int active = -1;
static xcb_query_extension_reply_t absent;

static int is_kwin(void) {
    if (active < 0) {
        char comm[64] = {0};
        FILE *f = fopen("/proc/self/comm", "r");
        if (f) { if (fgets(comm, sizeof comm, f)) comm[strcspn(comm, "\n")] = 0; fclose(f); }
        active = strcmp(comm, "kwin_x11") == 0;
    }
    return active;
}

const xcb_query_extension_reply_t *xcb_get_extension_data(xcb_connection_t *c, xcb_extension_t *ext) {
    static const xcb_query_extension_reply_t *(*real)(xcb_connection_t *, xcb_extension_t *);
    if (!real) real = dlsym(RTLD_NEXT, "xcb_get_extension_data");
    const xcb_query_extension_reply_t *r = real(c, ext);
    if (r && ext && ext->name && is_kwin() && strcmp(ext->name, "RANDR") == 0) {
        absent = *r; absent.present = 0;
        return &absent;
    }
    return r;
}
