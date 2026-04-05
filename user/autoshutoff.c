// user/autoshutoff.c
// autoshutoff — Energy-Saving Auto-Shutoff Policy
//   Feature 4 — Jade
//
// Runs max-on-time auto-shutoff (MAX_LIGHT_ON_TICKS) and whole-house-empty cleanup.
// Optional numeric argument is ignored (syscall ABI); threshold is fixed in the kernel.
//
// Usage:
//   autoshutoff
//   autoshutoff <ignored>

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

    int timeout = (argc >= 2) ? myatoi(argv[1]) : 0; // ignored by kernel

    printf("autoshutoff: applying max-on-time policy (MAX_LIGHT_ON_TICKS=%d)...\n",
           MAX_LIGHT_ON_TICKS);

    int count = auto_shutoff(timeout);

    if (count == 0) {
        printf("autoshutoff: no action needed — system is already efficient!\n");
    } else {
        printf("autoshutoff: turned off lights in %d room(s) — energy saved!\n",
               count);
    }

    exit(0);
}
