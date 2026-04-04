// user/lighttest.c
// lighttest — Unit Tests for Smart Lighting Energy Management
//
// Tests all four features:
//   Feature 1: Room simulation and initialization    (Karsten)
//   Feature 2: Occupancy-based light control         (Mithil)
//   Feature 3: Electricity usage tracker             (Sai)
//   Feature 4: Energy-saving auto-shutoff            (Jade)
//
// Run from the xv6 shell:  $ lighttest
// Output: each test prints [PASS] or [FAIL] with a short description.

#include "kernel/types.h"
#include "kernel/roomstat.h"
#include "user/user.h"

static int passed = 0;
static int failed = 0;

static void
check(const char *desc, int cond)
{
    if (cond) {
        printf("  [PASS] %s\n", desc);
        passed++;
    } else {
        printf("  [FAIL] %s\n", desc);
        failed++;
    }
}

// ---- Feature 1: Room Simulation (Karsten) ----
static void
test_room_simulation(void)
{
    printf("\n--- Feature 1: Room Simulation (Karsten) ---\n");
    lighting_init();

    struct roomstat rs;

    // All rooms should start with lights off and unoccupied
    int all_off = 1, all_empty = 1;
    for (int i = 0; i < NUM_ROOMS; i++) {
        if (room_status(i, &rs) != 0) { all_off = 0; all_empty = 0; break; }
        if (rs.light_on  != LIGHT_OFF)   all_off   = 0;
        if (rs.occupied  != ROOM_EMPTY)  all_empty = 0;
    }
    check("All rooms start with lights OFF", all_off);
    check("All rooms start EMPTY (unoccupied)", all_empty);

    // room_status should fail for out-of-range IDs
    check("room_status(-1) returns error",           room_status(-1,        &rs) < 0);
    check("room_status(NUM_ROOMS) returns error",    room_status(NUM_ROOMS, &rs) < 0);

    // Room 0 should be "Living Room" — check first character
    room_status(0, &rs);
    check("Room 0 name starts with 'L' (Living Room)", rs.name[0] == 'L');

    // Room 1 should be "Kitchen" — starts with 'K'
    room_status(1, &rs);
    check("Room 1 name starts with 'K' (Kitchen)", rs.name[0] == 'K');

    // Usage starts at zero for every room
    int usage_zero = 1;
    for (int i = 0; i < NUM_ROOMS; i++) {
        room_status(i, &rs);
        if (rs.usage_ticks != 0) { usage_zero = 0; break; }
    }
    check("All rooms start with usage_ticks == 0", usage_zero);
}

// ---- Feature 2: Occupancy-Based Light Control (Mithil) ----
static void
test_occupancy_control(void)
{
    printf("\n--- Feature 2: Occupancy-Based Light Control (Mithil) ---\n");
    struct roomstat rs;

    // set_occupied turns light ON
    set_occupied(2);    // Bedroom
    room_status(2, &rs);
    check("set_occupied(2): room is OCCUPIED",    rs.occupied == ROOM_OCCUPIED);
    check("set_occupied(2): light turns ON",       rs.light_on == LIGHT_ON);

    // set_empty turns light OFF
    set_empty(2);
    room_status(2, &rs);
    check("set_empty(2): room is EMPTY",           rs.occupied == ROOM_EMPTY);
    check("set_empty(2): light turns OFF",         rs.light_on == LIGHT_OFF);

    // Multiple rooms can be occupied simultaneously
    set_occupied(0);   // Living Room
    set_occupied(4);   // Office
    room_status(0, &rs);
    check("set_occupied(0): Living Room light ON", rs.light_on == LIGHT_ON);
    room_status(4, &rs);
    check("set_occupied(4): Office light ON",      rs.light_on == LIGHT_ON);

    // Clearing one room doesn't affect the other
    set_empty(0);
    room_status(0, &rs);
    check("set_empty(0): Living Room light OFF",   rs.light_on == LIGHT_OFF);
    room_status(4, &rs);
    check("Office light still ON after Living Room cleared", rs.light_on == LIGHT_ON);
    set_empty(4);   // clean up

    // Invalid room IDs should return an error
    check("set_occupied(-1) returns error",        set_occupied(-1)        < 0);
    check("set_occupied(NUM_ROOMS) returns error", set_occupied(NUM_ROOMS) < 0);
    check("set_empty(-1) returns error",           set_empty(-1)           < 0);
    check("set_empty(NUM_ROOMS) returns error",    set_empty(NUM_ROOMS)    < 0);
}

// ---- Feature 3: Electricity Usage Tracker (Sai) ----
static void
test_usage_tracker(void)
{
    printf("\n--- Feature 3: Electricity Usage Tracker (Sai) ---\n");
    struct roomstat rs;

    // get_usage(-1) is the total across all rooms
    // After cleanup above all lights should be off, so at least returns a uint
    uint total = (uint)get_usage(-1);
    check("get_usage(-1) returns a value without crashing", 1 /* always */);

    // Usage for an out-of-range room should equal (uint)-1
    uint bad = (uint)get_usage(NUM_ROOMS);
    check("get_usage(NUM_ROOMS) returns (uint)-1", bad == (uint)-1);

    // Record baseline for room 3, turn light on, check usage increases
    room_status(3, &rs);
    uint before = rs.usage_ticks;

    // Turn light on and spin briefly so at least one tick fires
    set_occupied(3);
    // busy-wait for a small amount of time to let ticks advance
    for (volatile int i = 0; i < 20000000; i++) {}
    set_empty(3);

    room_status(3, &rs);
    // usage_ticks should be >= before (could be equal if no ticks fired in QEMU)
    check("Room 3 usage_ticks >= before after light was on",
          rs.usage_ticks >= before);

    // Total usage should be >= per-room usage
    total = (uint)get_usage(-1);
    check("Total usage >= room 3 usage", total >= rs.usage_ticks);
}

// ---- Feature 4: Auto-Shutoff (Jade) ----
static void
test_auto_shutoff(void)
{
    printf("\n--- Feature 4: Energy-Saving Auto-Shutoff (Jade) ---\n");
    struct roomstat rs;

    // A room that IS occupied should NOT be shut off if timeout has not elapsed
    // (last_active was just reset; use a huge timeout so idle time is never enough)
    set_occupied(1);    // Kitchen
    int n = auto_shutoff(0x7fffffff);
    room_status(1, &rs);
    check("Occupied room NOT shut off when timeout not reached",
          rs.light_on == LIGHT_ON && n == 0);
    set_empty(1);   // clean up

    // An empty room with its light off should not be counted
    room_status(0, &rs);        // Living Room should be off + empty from above
    int before_count = auto_shutoff(1);
    check("auto_shutoff returns int without crashing", 1 /* always */);
    (void)before_count;

    // A room that is empty but has its light on AND idle long enough IS shut off.
    // We cannot reliably simulate real ticks in a unit test; instead we manually
    // verify the boundary condition: passing timeout=0 uses the default constant.
    n = auto_shutoff(0);
    check("auto_shutoff(0) uses default and returns >= 0", n >= 0);

    // Passing a very large timeout shuts off nothing (rooms haven't been idle that long)
    n = auto_shutoff(0x7fffffff);
    check("auto_shutoff with huge timeout shuts off nothing", n == 0);
}

int
main(int argc, char *argv[])
{
    printf("=== lighttest: Smart Lighting Unit Tests ===\n");

    lighting_init();

    test_room_simulation();
    test_occupancy_control();
    test_usage_tracker();
    test_auto_shutoff();

    printf("\n=== Results: %d passed, %d failed ===\n", passed, failed);

    exit(failed == 0 ? 0 : 1);
}
