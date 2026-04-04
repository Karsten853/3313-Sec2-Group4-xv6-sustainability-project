// kernel/lighting.h
// Kernel-internal data structures and function prototypes for the
// Smart Lighting Energy Management system.
//
// User-space programs should include "kernel/roomstat.h" instead;
// this header is for kernel files only.

#ifndef LIGHTING_H
#define LIGHTING_H

#include "roomstat.h"   // shared constants + struct roomstat
#include "spinlock.h"

// Internal room structure — extends the public roomstat with a kernel-only
// field (last_active) used by the auto-shutoff timer comparison.
struct room {
    char name[ROOM_NAME_MAX]; // room display name
    int  light_on;            // LIGHT_ON or LIGHT_OFF
    int  occupied;            // ROOM_OCCUPIED or ROOM_EMPTY
    uint usage_ticks;         // energy-usage counter (ticks with light on)
    uint last_active;         // tick timestamp of last occupancy change
};

// ---- Module state (defined in lighting.c) ----
extern struct room      rooms[NUM_ROOMS];
extern int              lighting_initialized;
extern struct spinlock  lighting_lock;

// ---- Function prototypes ----

// Feature 1 – Simulated room/light environment (Karsten)
void lighting_init(void);
int  room_status(int roomid, struct roomstat *out);

// Feature 2 – Occupancy-based light control (Mithil)
int  set_room_occupied(int roomid);
int  set_room_empty(int roomid);

// Feature 3 – Electricity usage tracker (Sai)
void update_usage(void);       // call once per tick from clockintr()
uint get_room_usage(int roomid);
uint get_total_usage(void);

// Feature 4 – Energy-saving auto-shutoff (Jade)
void lighting_tick(void);      // call once per tick from clockintr() after update_usage()
int  auto_shutoff(int timeout_ticks);

#endif // LIGHTING_H
