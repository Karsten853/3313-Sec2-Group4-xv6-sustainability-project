// kernel/lighting.c
// Smart Lighting Energy Management — Core Implementation
//
// Implements all four project features at the kernel level:
//   Feature 1: Simulated room/light environment  (Karsten)
//   Feature 2: Occupancy-based light control     (Mithil)
//   Feature 3: Electricity usage tracker         (Sai)
//   Feature 4: Energy-saving auto-shutoff        (Jade)
//
// System call wrappers are in kernel/syslighting.c.

#include "types.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"
#include "lighting.h"

// ticks is defined in kernel/trap.c and declared via defs.h
extern uint ticks;

// ============================================================
// Module State
// ============================================================

struct room     rooms[NUM_ROOMS];   // global room array
int             lighting_initialized = 0;
struct spinlock lighting_lock;

// ============================================================
// Feature 1: Simulated Room/Light Environment  (Karsten)
// ============================================================

// lighting_init — Set up the five-room simulated house environment.
//
// Assigns a name to each room and resets all state to the "lights off,
// room empty" baseline.  Safe to call from kernel/main.c at boot time.
// Idempotent: subsequent calls are no-ops.
void
lighting_init(void)
{
    if (lighting_initialized)
        return;

    initlock(&lighting_lock, "lighting");
    acquire(&lighting_lock);

    // Human-readable names for the five simulated rooms
    static const char *names[NUM_ROOMS] = {
        "Living Room",
        "Kitchen",
        "Bedroom",
        "Bathroom",
        "Office"
    };

    for (int i = 0; i < NUM_ROOMS; i++) {
        // Manual copy — no strncpy in kernel
        int j = 0;
        while (names[i][j] != '\0' && j < ROOM_NAME_MAX - 1) {
            rooms[i].name[j] = names[i][j];
            j++;
        }
        rooms[i].name[j]    = '\0';
        rooms[i].light_on   = LIGHT_OFF;
        rooms[i].occupied   = ROOM_EMPTY;
        rooms[i].usage_ticks = 0;
        rooms[i].last_active = 0;
    }

    lighting_initialized = 1;
    release(&lighting_lock);
}

// room_status — Copy one room's current state into a caller-supplied buffer.
//
// roomid : 0 .. NUM_ROOMS-1
// out    : kernel-address pointer to a struct roomstat to fill
// Returns 0 on success, -1 on invalid roomid or NULL pointer.
int
room_status(int roomid, struct roomstat *out)
{
    if (!lighting_initialized) lighting_init();
    if (roomid < 0 || roomid >= NUM_ROOMS || out == 0)
        return -1;

    acquire(&lighting_lock);
    for (int j = 0; j < ROOM_NAME_MAX; j++)
        out->name[j] = rooms[roomid].name[j];
    out->light_on    = rooms[roomid].light_on;
    out->occupied    = rooms[roomid].occupied;
    out->usage_ticks = rooms[roomid].usage_ticks;
    release(&lighting_lock);

    return 0;
}

// ============================================================
// Feature 2: Occupancy-Based Light Control  (Mithil)
// ============================================================

// set_room_occupied — Mark a room as occupied and turn its light ON.
//
// Called when a person enters a room.  The light turns on immediately
// and last_active is updated so the auto-shutoff timer resets.
//
// roomid : 0 .. NUM_ROOMS-1
// Returns 0 on success, -1 on invalid roomid.
int
set_room_occupied(int roomid)
{
    if (!lighting_initialized) lighting_init();
    if (roomid < 0 || roomid >= NUM_ROOMS)
        return -1;

    acquire(&lighting_lock);
    rooms[roomid].occupied    = ROOM_OCCUPIED;
    rooms[roomid].light_on    = LIGHT_ON;   // light turns on immediately
    rooms[roomid].last_active = ticks;      // reset idle timer
    release(&lighting_lock);

    return 0;
}

// set_room_empty — Mark a room as empty and turn its light OFF.
//
// Called when the last person leaves a room.  The light turns off
// immediately, saving energy.  last_active is recorded so auto-shutoff
// can detect rooms left on by mistake.
//
// roomid : 0 .. NUM_ROOMS-1
// Returns 0 on success, -1 on invalid roomid.
int
set_room_empty(int roomid)
{
    if (!lighting_initialized) lighting_init();
    if (roomid < 0 || roomid >= NUM_ROOMS)
        return -1;

    acquire(&lighting_lock);
    rooms[roomid].occupied    = ROOM_EMPTY;
    rooms[roomid].light_on    = LIGHT_OFF;  // light turns off immediately
    rooms[roomid].last_active = ticks;      // note when room became empty
    release(&lighting_lock);

    return 0;
}

// ============================================================
// Feature 3: Electricity Usage Tracker  (Sai)
// ============================================================

// update_usage — Increment usage_ticks for every room whose light is on.
//
// Designed to be called once per timer tick from clockintr() in
// kernel/trap.c.  Each tick with a light on counts as one unit of energy.
void
update_usage(void)
{
    if (!lighting_initialized)
        return;

    acquire(&lighting_lock);
    for (int i = 0; i < NUM_ROOMS; i++) {
        if (rooms[i].light_on == LIGHT_ON)
            rooms[i].usage_ticks++;
    }
    release(&lighting_lock);
}

// get_room_usage — Return the usage_ticks counter for a specific room.
//
// Returns (uint)-1 on invalid roomid.
uint
get_room_usage(int roomid)
{
    if (!lighting_initialized) lighting_init();
    if (roomid < 0 || roomid >= NUM_ROOMS)
        return (uint)-1;

    acquire(&lighting_lock);
    uint val = rooms[roomid].usage_ticks;
    release(&lighting_lock);
    return val;
}

// get_total_usage — Return the sum of usage_ticks across all rooms.
uint
get_total_usage(void)
{
    if (!lighting_initialized) lighting_init();
    uint total = 0;

    acquire(&lighting_lock);
    for (int i = 0; i < NUM_ROOMS; i++)
        total += rooms[i].usage_ticks;
    release(&lighting_lock);

    return total;
}

// ============================================================
// Feature 4: Energy-Saving Auto-Shutoff  (Jade)
// ============================================================

// auto_shutoff — Turn off lights in rooms idle longer than timeout_ticks.
//
// A room qualifies for auto-shutoff when ALL of the following are true:
//   (a) occupied == ROOM_EMPTY
//   (b) light_on == LIGHT_ON   (light left on after occupant left)
//   (c) (ticks - last_active) >= timeout_ticks
//
// Pass timeout_ticks == 0 to use the OCCUPANCY_TIMEOUT default.
// Returns the number of rooms whose lights were turned off.
int
auto_shutoff(int timeout_ticks)
{
    if (!lighting_initialized) lighting_init();
    if (timeout_ticks <= 0)
        timeout_ticks = OCCUPANCY_TIMEOUT;

    int  count = 0;
    uint now   = ticks;

    acquire(&lighting_lock);
    for (int i = 0; i < NUM_ROOMS; i++) {
        if (rooms[i].occupied == ROOM_EMPTY &&
            rooms[i].light_on == LIGHT_ON   &&
            (now - rooms[i].last_active) >= (uint)timeout_ticks) {
            rooms[i].light_on = LIGHT_OFF;
            count++;
        }
    }
    release(&lighting_lock);

    return count;
}
