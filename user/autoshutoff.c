// user/autoshutoff.c
// autoshutoff — Energy-Saving Auto-Shutoff Policy
//   Feature 4 — Jade
//
// Applies the auto-shutoff rule: any room that has been unoccupied with
// its light on for longer than the timeout gets its light turned off.
//
// Usage:
//   autoshutoff              run with default timeout (OCCUPANCY_TIMEOUT, ~10 s)
//   autoshutoff <ticks>      run with a custom timeout in ticks

#include "kernel/types.h"
#include "kernel/roomstat.h"
#include "user/user.h"

static int
myatoi(const char *s)
{
    int n = 0;
    while (*s >= '0' && *s <= '9')
        n = n * 10 + (*s++ - '0');
    return n;
}

int
main(int argc, char *argv[])
{
    lighting_init();

    int timeout = (argc >= 2) ? myatoi(argv[1]) : 0; // 0 → use kernel default

    printf("autoshutoff: checking rooms (timeout = %d ticks)...\n",
           timeout > 0 ? timeout : OCCUPANCY_TIMEOUT);

    int count = auto_shutoff(timeout);

    if (count == 0) {
        printf("autoshutoff: no action needed — system is already efficient!\n");
    } else {
        printf("autoshutoff: turned off lights in %d room(s) — energy saved!\n",
               count);
    }

    exit(0);
}
