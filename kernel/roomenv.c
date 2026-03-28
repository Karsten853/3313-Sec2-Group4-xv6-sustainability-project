#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "roomenv.h"

struct spinlock roomenv_lock;
struct roomenv room;

void
roomenv_init(void)
{
  initlock(&roomenv_lock, "roomenv");

  safestrcpy(room.name, "MainRoom", sizeof(room.name));
  room.light_on = 0;
  room.occupied = 0;
  room.last_motion_tick = 0;
  room.total_light_on_ticks = 0;
}

void
roomenv_reset(void)
{
  acquire(&roomenv_lock);

  safestrcpy(room.name, "MainRoom", sizeof(room.name));
  room.light_on = 0;
  room.occupied = 0;
  room.last_motion_tick = 0;
  room.total_light_on_ticks = 0;

  release(&roomenv_lock);
}

void
roomenv_set_light(int on)
{
  acquire(&roomenv_lock);
  room.light_on = on;
  release(&roomenv_lock);
}

void
roomenv_set_occupied(int occupied)
{
  acquire(&roomenv_lock);
  room.occupied = occupied;
  room.last_motion_tick = ticks;
  release(&roomenv_lock);
}

void
roomenv_print_status(void)
{
  acquire(&roomenv_lock);

  printf("Room: %s\n", room.name);
  printf("Light: %s\n", room.light_on ? "ON" : "OFF");
  printf("Occupied: %s\n", room.occupied ? "YES" : "NO");
  printf("Last motion tick: %d\n", room.last_motion_tick);
  printf("Total light-on ticks: %d\n", room.total_light_on_ticks);

  release(&roomenv_lock);
}