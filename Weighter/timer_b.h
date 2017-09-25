#ifndef TIMER_B_H_
#define TIMER_B_H_

#include "msp430f2272.h"

ulong local_ticktime(void);
bool timeout(ulong last_time, ulong ms);
#endif /*TIMER_B_H_*/
