
#ifndef __TIMER_H__
#define __TIMER_H__

#include "relay.h"

void timer_task(void *pvParameters);
int add_timer(void (*callback)(), int seconds, int id = 0);

#endif
