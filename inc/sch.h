/*
 * sch.h
 *
 *  Created on: Jan 14, 2017
 *      Author: tgjuranec
 */

#ifndef SCH_H_
#define SCH_H_

#include <chip.h>

#define TICKRATE_HZ 100
#define MAX_TASKS 16

extern uint32_t tick;

typedef struct stask{
	void (*taskfunc) (void);
	uint32_t tics_delay;
	uint32_t tics_period;
	uint8_t runtask;
	uint8_t cooperative;
} sch_task;


void sch_init();
void sch_start();
void sch_dispatch();
uint32_t sch_addtask(void (* f) (void), uint32_t delay, uint32_t period, uint32_t coop);
void tmr_delay_us(uint32_t us);
void tmr_delay_cputicks(uint32_t cputicks);
void tmr_timeout_set(uint32_t us);
uint8_t tmr_timeout_off();

#endif /* SCH_H_ */
