# Smart Lighting Energy Management in xv6

## Project Overview
This project extends xv6 with a smart-room lighting system to explore how operating system design can support environmental sustainability. The system simulates a five-room environment (Living Room, Kitchen, Bedroom, Bathroom, and Office), each with its own controllable light, occupancy state, electricity usage tracking, and an energy-saving shutoff policy.

The goal is to demonstrate how OS-managed state, timer-based accounting, and automated control logic can be used to reduce unnecessary energy use in a simulated real-world setting.

---

## Features
The project is divided into four main features:

1. **Simulated Room/Light Environment**  
   Established a core room state model in the kernel (Living Room, Kitchen, Bedroom, Bathroom, Office).

2. **Occupancy-Based Light Control**  
   Implemented logic to automatically turn lights ON when a room becomes occupied and OFF when vacated, reducing human error in energy conservation.

3. **Electricity Usage Tracking**  
   Integrated energy accounting into the OS clock interrupt (`trap.c`), tracking cumulative "usage ticks" for every room with active lighting.

4. **Energy-Saving Shutoff Policy**  
   Implemented a kernel-level policy to automatically turn off lights that have been on continuously for too long (`MAX_LIGHT_ON_TICKS`) or when the entire house is empty.

---

## Sustainability Relevance
This project connects operating system design to environmental sustainability by focusing on energy efficiency. By managing lighting at the kernel level, the system can ensure high-precision usage tracking and enforce energy-saving policies that are independent of user intervention.

Although this is a proof-of-concept in xv6, these design principles (state-driven power management and timer-based accounting) are foundational to modern green computing and smart-home embedded systems.

---

## Important Files
### Kernel Implementation
- `kernel/lighting.c`: Core logic for room management, occupancy transitions, and usage accounting.
- `kernel/roomstat.h`: Shared data structures and constants (e.g., `NUM_ROOMS`, `LIGHT_ON`).
- `kernel/syslighting.c`: System call handlers providing the interface to userspace.
- `kernel/trap.c`: Integrated with `update_usage()` to increment energy counters on every timer tick.

### User-Space Tools
- `lightctl`: View the status of all rooms and initialize the system.
- `roomctl`: Simulate room occupancy (enter/exit rooms).
- `energymon`: Monitor real-time electricity usage across the house.
- `autoshutoff`: Manually trigger or configure the energy-saving policy.
- `lighttest`: Comprehensive unit test suite for all features.

---

## Room Environment State
The kernel tracks the following fields for each room:

- `name`: The display name (e.g., "Kitchen").
- `light_on`: Current light status (`0` for OFF, `1` for ON).
- `occupied`: Occupancy status (`0` for EMPTY, `1` for OCCUPIED).
- `usage_ticks`: Total energy units consumed (active while light is on).
- `light_on_since`: The exact tick when the light was last turned on (used for the auto-shutoff policy).

---

## Core Functions (Kernel API)
The system is built upon several key kernel functions accessible via system calls:

- `lighting_init()`: Initializes the 5-room environment at boot.
- `room_status(id, buf)`: Retrieves current state for a specific room.
- `set_room_occupied(id)`: Sets a room to occupied and activates the light.
- `set_room_empty(id)`: Sets a room to empty and deactivates the light.
- `update_usage()`: Increments usage counters (called via `clockintr`).
- `auto_shutoff()`: Enforces the energy-saving rules.

## Build and Run

### Requirements
This project is designed to run on the RISC-V version of xv6. You will need the RISC-V GNU toolchain (`riscv64-unknown-elf-gcc`) and `qemu-system-riscv64` installed on a Linux environment (Ubuntu/WSL is recommended).

### Compilation
```bash
make clean
make qemu
