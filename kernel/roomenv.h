#ifndef ROOMENV_H
#define ROOMENV_H

struct roomenv {
  char name[16];
  int light_on;
  int occupied;
  int last_motion_tick;
  int total_light_on_ticks;
};

void roomenv_init(void);
void roomenv_reset(void);
void roomenv_set_light(int on);
void roomenv_set_occupied(int occupied);
void roomenv_print_status(void);

#endif