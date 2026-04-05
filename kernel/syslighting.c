// kernel/syslighting.c
// System call handlers for the Smart Lighting Energy Management system.
//
// Each sys_* function is invoked by the kernel's syscall dispatcher
// (kernel/syscall.c) when a user process triggers the corresponding syscall.
// Arguments are read with argint() / argaddr(); results are copied to
// userspace with copyout() where needed.

#include "types.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"
#include "lighting.h"

// sys_lighting_init — Initialize the lighting system (syscall 22).
// Userspace signature: int lighting_init(void)
uint64
sys_lighting_init(void)
{
    lighting_init();
    return 0;
}

// sys_room_status — Get status of one room (syscall 23).
// Userspace signature: int room_status(int roomid, struct roomstat *buf)
//
// The kernel fills a local roomstat, then copies it out to the user pointer.
uint64
sys_room_status(void)
{
    int    roomid;
    uint64 uaddr;          // user-space pointer to struct roomstat
    struct roomstat rs;

    argint(0, &roomid);
    argaddr(1, &uaddr);

    if (room_status(roomid, &rs) < 0)
        return -1;

    if (copyout(myproc()->pagetable, uaddr, (char *)&rs, sizeof(rs)) < 0)
        return -1;

    return 0;
}

// sys_set_occupied — Mark a room occupied, light ON (syscall 24).
// Userspace signature: int set_occupied(int roomid)
uint64
sys_set_occupied(void)
{
    int roomid;
    argint(0, &roomid);
    return set_room_occupied(roomid);
}

// sys_set_empty — Mark a room empty, light OFF (syscall 25).
// Userspace signature: int set_empty(int roomid)
uint64
sys_set_empty(void)
{
    int roomid;
    argint(0, &roomid);
    return set_room_empty(roomid);
}

// sys_get_usage — Return energy usage ticks (syscall 26).
// Userspace signature: uint get_usage(int roomid)
//
// roomid == -1  →  total usage across all rooms
// roomid >= 0   →  usage for that specific room
uint64
sys_get_usage(void)
{
    int roomid;
    argint(0, &roomid);
    if (roomid == -1)
        return get_total_usage();
    return get_room_usage(roomid);
}

// sys_auto_shutoff — Run the auto-shutoff policy (syscall 27).
// Userspace signature: int auto_shutoff(int timeout_ticks)  (argument ignored)
//
// Returns the number of lights turned off.
uint64
sys_auto_shutoff(void)
{
    int timeout;
    argint(0, &timeout);
    return auto_shutoff(timeout);
}
