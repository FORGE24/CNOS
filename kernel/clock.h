/* Monotonic tick counter（PIT IRQ0 每次调度前递增，供用户态 SYS_UPTIME_TICKS） */

#ifndef KERNEL_CLOCK_H
#define KERNEL_CLOCK_H

#include <stdint.h>

extern volatile uint64_t chaseros_kernel_ticks;

#endif
