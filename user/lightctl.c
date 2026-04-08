// user/lightctl.c
// lightctl — Room and Light Status Viewer
//   Feature 1 — Karsten
//
// Initializes and displays the simulated room/light environment.
//
// Usage:
//   lightctl               initialize and show all rooms
//   lightctl init          initialize the lighting system
//   lightctl status        show full status table
//   lightctl status <id>   show status for one room

#include "kernel/types.h"
#include "kernel/roomstat.h"
#include "user/user.h"

static int
streq(const char *a, const char *b)
{
    while (*a && *b && *a == *b) { a++; b++; }
    return (*a == '\0' && *b == '\0');
}

static int
myatoi(const char *s)
{
    int n = 0;
    while (*s >= '0' && *s <= '9')
        n = n * 10 + (*s++ - '0');
    return n;
}

static void
print_header(void)
{
    printf("  ID  Room            Light  Occupancy   Usage (ticks)\n");
    printf("  -------------------------------------------------------\n");
}

static void
print_room(int id, struct roomstat *rs)
{
    printf("  %d   %s: %s, %s, %u ticks\n",
           id,
           rs->name,
           rs->light_on == LIGHT_ON      ? "ON"  : "OFF",
           rs->occupied == ROOM_OCCUPIED ? "OCCUPIED" : "EMPTY",
           rs->usage_ticks);
}

int
main(int argc, char *argv[])
{
    lighting_init();

    if (argc < 2 || streq(argv[1], "status")) {
        int single = (argc >= 3);
        struct roomstat rs;

        printf("\n=== Smart Lighting System — Room Overview ===\n");
        print_header();

        if (single) {
            int id = myatoi(argv[2]);
            if (room_status(id, &rs) < 0) {
                printf("lightctl: invalid room id %d\n", id);
                exit(1);
            }
            print_room(id, &rs);
        } else {
            for (int i = 0; i < NUM_ROOMS; i++)
                if (room_status(i, &rs) == 0)
                    print_room(i, &rs);
        }

        printf("  =====================================================\n\n");

    } else if (streq(argv[1], "init")) {
        printf("lightctl: lighting system initialized (%d rooms).\n", NUM_ROOMS);
    } else {
        printf("lightctl: unknown command '%s'\n", argv[1]);
        printf("Usage: lightctl [init | status [roomid]]\n");
        exit(1);
    }

    exit(0);
}
