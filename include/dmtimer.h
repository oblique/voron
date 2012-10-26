#ifndef __DMTIMER_H
#define __DMTIMER_H

#include <kernel.h>

typedef void (*dmtimer_callback_func)(int timer_id, struct regs *regs);

int dmtimer_register(int id, dmtimer_callback_func func, u32 ms);
int dmtimer_trigger(int id);

#endif	/* __DMTIMER_H */
