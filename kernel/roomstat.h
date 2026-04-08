// kernel/roomstat.h
// Shared room status structure for the Smart Lighting Energy Management system.
// Included by both kernel modules and user-space programs — mirrors the role
// of kernel/stat.h in the standard xv6 distribution.

#ifndef ROOMSTAT_H
#define ROOMSTAT_H

// Number of simulated rooms in the house
#define NUM_ROOMS      5
// Maximum length of a room name string (including null terminator)
#define ROOM_NAME_MAX  32

// Light state constants
#define LIGHT_OFF      0
#define LIGHT_ON       1

// Occupancy state constants
#define ROOM_EMPTY     0
#define ROOM_OCCUPIED  1

// Max continuous time a single light may stay ON (even if occupied).
// Resets when the light turns OFF or when set_room_occupied starts a new ON period.
#define MAX_LIGHT_ON_TICKS  100

// Room status structure — copied from kernel to userspace via copyout().
// Only contains the fields that user programs need to read.
struct roomstat {
    char name[ROOM_NAME_MAX]; // human-readable room name
    int  light_on;            // LIGHT_ON or LIGHT_OFF
    int  occupied;            // ROOM_OCCUPIED or ROOM_EMPTY
    uint usage_ticks;         // total ticks the light has been on (energy proxy)
};

#endif // ROOMSTAT_H
