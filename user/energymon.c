// user/energymon.c
// energymon — Electricity Usage Monitor
//   Feature 3 — Sai
//
// Displays per-room energy usage (ticks with light on) and total house usage.
//
// Usage:
//   energymon              show usage for all rooms + total
//   energymon <roomid>     show usage for one room
//   energymon total        show only the total house usage

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

int
main(int argc, char *argv[])
{
    if (argc >= 2 && streq(argv[1], "total")) {
        // Print only the total house usage
        uint total = (uint)get_usage(-1);
        printf("Total house energy usage: %u ticks\n", total);

    } else if (argc >= 2 && !(argv[1][0] < '0' || argv[1][0] > '9')) {
        // Single room by id
        int roomid = myatoi(argv[1]);
        struct roomstat rs;
        if (room_status(roomid, &rs) < 0) {
            printf("energymon: invalid room id %d\n", roomid);
            exit(1);
        }
        printf("Room '%s' energy usage: %u ticks (light is %s)\n",
               rs.name, rs.usage_ticks,
               rs.light_on == LIGHT_ON ? "ON" : "OFF");

    } else {
        // All rooms + total
        struct roomstat rs;
        uint total = 0;

        printf("\n=== Energy Usage Report ===\n");
        printf("  ID  Room            Usage (ticks)  Light\n");
        printf("  ------------------------------------------\n");

        for (int i = 0; i < NUM_ROOMS; i++) {
            if (room_status(i, &rs) == 0) {
                printf("  %d   %s: %u ticks (%s)\n",
                       i, rs.name, rs.usage_ticks,
                       rs.light_on == LIGHT_ON ? "ON" : "OFF");
                total += rs.usage_ticks;
            }
        }

        printf("  ------------------------------------------\n");
        printf("  TOTAL HOUSE USAGE: %u ticks\n", total);
        printf("  ==========================================\n\n");
    }

    exit(0);
}
