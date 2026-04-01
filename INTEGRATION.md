# Integration Guide — Smart Lighting System

This file explains exactly which lines to add to the **stock xv6-riscv** source
so that the new `.c` / `.h` files in this repo compile and run correctly.

Clone the base OS alongside the project files, or copy these project files
into your xv6-riscv tree.

---

## 1. `kernel/syscall.h` — add 6 syscall numbers

Append after the last existing `#define SYS_*` line (usually `SYS_close 21`):

```c
#define SYS_lighting_init  22
#define SYS_room_status    23
#define SYS_set_occupied   24
#define SYS_set_empty      25
#define SYS_get_usage      26
#define SYS_auto_shutoff   27
```

---

## 2. `kernel/syscall.c` — add 6 entries to the dispatch table

Near the top, add `extern` declarations alongside the other `extern uint64 sys_*`:

```c
extern uint64 sys_lighting_init(void);
extern uint64 sys_room_status(void);
extern uint64 sys_set_occupied(void);
extern uint64 sys_set_empty(void);
extern uint64 sys_get_usage(void);
extern uint64 sys_auto_shutoff(void);
```

In the `syscalls[]` array, add these entries:

```c
[SYS_lighting_init]  sys_lighting_init,
[SYS_room_status]    sys_room_status,
[SYS_set_occupied]   sys_set_occupied,
[SYS_set_empty]      sys_set_empty,
[SYS_get_usage]      sys_get_usage,
[SYS_auto_shutoff]   sys_auto_shutoff,
```

---

## 3. `kernel/defs.h` — add function prototypes

Add a new section (e.g., before `// bio.c`):

```c
// lighting.c
void  lighting_init(void);
int   room_status(int, struct roomstat*);
int   set_room_occupied(int);
int   set_room_empty(int);
void  update_usage(void);
uint  get_room_usage(int);
uint  get_total_usage(void);
int   auto_shutoff(int);
```

You will also need to add a forward declaration of `struct roomstat` near the
top of `defs.h` (before the lighting prototypes):

```c
struct roomstat;   // defined in kernel/roomstat.h
```

---

## 4. `kernel/main.c` — initialize at boot

In `main()`, after `consoleinit()` and before `userinit()`, add:

```c
lighting_init();   // Smart Lighting Energy Management
```

---

## 5. `kernel/trap.c` — tick-level energy accounting

In the `clockintr()` function (called every timer tick), add one line:

```c
update_usage();    // increment usage_ticks for rooms with lights on
```

---

## 6. `user/user.h` — user-facing syscall prototypes

Add after the existing `struct stat;` forward declaration:

```c
struct roomstat;
```

Add the following prototypes alongside the other syscall stubs:

```c
int   lighting_init(void);
int   room_status(int, struct roomstat*);
int   set_occupied(int);
int   set_empty(int);
uint  get_usage(int);
int   auto_shutoff(int);
```

---

## 7. `user/usys.pl` — syscall stubs (RISC-V assembly generation)

Add six `entry` lines at the end of the file:

```perl
entry("lighting_init");
entry("room_status");
entry("set_occupied");
entry("set_empty");
entry("get_usage");
entry("auto_shutoff");
```

---

## 8. `Makefile` — add new files to the build

**Kernel objects** — in the `OBJS` list add:

```make
$K/lighting.o \
$K/syslighting.o \
```

**User programs** — in the `UPROGS` list add:

```make
$U/_lightctl \
$U/_roomctl \
$U/_energymon \
$U/_autoshutoff \
$U/_lighttest \
```

---

## Quick Demo Sequence (in the xv6 shell)

```sh
$ lightctl status          # all rooms: lights OFF, EMPTY, 0 ticks
$ roomctl occupied 0       # Living Room occupied  → light ON
$ roomctl occupied 2       # Bedroom occupied      → light ON
$ lightctl status          # two lights ON
$ energymon                # some usage_ticks accumulated
$ roomctl empty 0          # Living Room vacated   → light OFF immediately
$ autoshutoff 10           # shut off any remaining idle lights
$ energymon                # updated totals
$ lighttest                # run unit tests → all PASS
```
