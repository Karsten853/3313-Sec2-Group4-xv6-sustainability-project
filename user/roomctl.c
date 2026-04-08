// user/roomctl.c
// roomctl — Occupancy-Based Light Control
//   Feature 2 — Mithil
//
// Controls room occupancy in the Smart Lighting Energy Management system.
// Marking a room occupied turns its light ON; marking it empty turns it OFF.
//
// Usage:
//   roomctl                      display status of all rooms
//   roomctl list                 display status of all rooms
//   roomctl occupied <roomid>    mark room occupied  (light ON)
//   roomctl empty    <roomid>    mark room empty     (light OFF)
//
// Room IDs:  0=Living Room  1=Kitchen  2=Bedroom  3=Bathroom  4=Office

#include "kernel/types.h"
#include "kernel/roomstat.h"
#include "user/user.h"

// ---- tiny helpers (no stdlib in xv6 user space) ----

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

// ---- display helpers ----

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

// ---- commands ----

static void
cmd_list(void)
{
    struct roomstat rs;
    printf("\n=== Smart Lighting — Room Status ===\n");
    print_header();
    for (int i = 0; i < NUM_ROOMS; i++) {
        if (room_status(i, &rs) == 0)
            print_room(i, &rs);
    }
    printf("  =====================================================\n\n");
}

static void
cmd_occupied(int roomid)
{
    if (set_occupied(roomid) < 0) {
        printf("roomctl: invalid room id %d  (valid: 0-%d)\n",
               roomid, NUM_ROOMS - 1);
        exit(1);
    }
    struct roomstat rs;
    room_status(roomid, &rs);
    printf("roomctl: room '%s' is now OCCUPIED — light turned ON.\n", rs.name);
}

static void
cmd_empty(int roomid)
{
    if (set_empty(roomid) < 0) {
        printf("roomctl: invalid room id %d  (valid: 0-%d)\n",
               roomid, NUM_ROOMS - 1);
        exit(1);
    }
    struct roomstat rs;
    room_status(roomid, &rs);
    printf("roomctl: room '%s' is now EMPTY — light turned OFF.\n", rs.name);
}

static void
usage(void)
{
    printf("Usage:\n");
    printf("  roomctl                    show all room statuses\n");
    printf("  roomctl list               show all room statuses\n");
    printf("  roomctl occupied <id>      mark room occupied  (light ON)\n");
    printf("  roomctl empty    <id>      mark room empty     (light OFF)\n");
    printf("\nRoom IDs: 0=Living Room  1=Kitchen  2=Bedroom  3=Bathroom  4=Office\n");
}

int
main(int argc, char *argv[])
{
    lighting_init();   // safe to call even if already initialized at boot

    if (argc < 2 || streq(argv[1], "list")) {
        cmd_list();
    } else if (streq(argv[1], "occupied")) {
        if (argc < 3) {
            printf("roomctl: 'occupied' requires a room id\n");
            usage();
            exit(1);
        }
        cmd_occupied(myatoi(argv[2]));
    } else if (streq(argv[1], "empty")) {
        if (argc < 3) {
            printf("roomctl: 'empty' requires a room id\n");
            usage();
            exit(1);
        }
        cmd_empty(myatoi(argv[2]));
    } else if (streq(argv[1], "help") || streq(argv[1], "-h")) {
        usage();
    } else {
        printf("roomctl: unknown command '%s'\n", argv[1]);
        usage();
        exit(1);
    }

    exit(0);
}
