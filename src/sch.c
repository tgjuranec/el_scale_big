/*
 * sch.c
 *
 *  Created on: Jan 13, 2017
 *      Author: tgjuranec
 */


#include "sch.h"
#include <chip.h>

uint32_t tick;
sch_task task[MAX_TASKS];


static void sch_deletetask(uint8_t i){
	task[i].taskfunc = NULL;
	task[i].cooperative = 1;
	task[i].runtask = 0;
	task[i].tics_delay = 0;
	task[i].tics_period = 0;
}


static void sch_status(){
	return;
}



/*
 * timer counter is incremented every tick of the processor
 */
static void tmr_init(){
	uint32_t presc = 1;					//(presc * match_reg) = pclk / fint = pclk * Tint
	Chip_TIMER_Init(LPC_TIMER32_0);
	Chip_TIMER_Disable(LPC_TIMER32_0);
	Chip_TIMER_Reset(LPC_TIMER32_0);
	LPC_TIMER32_0->CTCR = 0;
	Chip_TIMER_MatchDisableInt(LPC_TIMER32_0,0);
	Chip_TIMER_ResetOnMatchDisable(LPC_TIMER32_0,0);
	Chip_TIMER_StopOnMatchDisable(LPC_TIMER32_0,0);
	Chip_TIMER_PrescaleSet(LPC_TIMER32_0,presc-1);
	Chip_TIMER_Enable(LPC_TIMER32_0);
}



void tmr_delay_us(uint32_t us){
	Chip_TIMER_Reset(LPC_TIMER32_0);
	Chip_TIMER_Enable(LPC_TIMER32_0);
	uint32_t sclockmhz = Chip_Clock_GetSystemClockRate()/1000000;							//get system clock 48MHz
	uint32_t delta = (uint32_t)(sclockmhz*us/(LPC_TIMER32_0->PR+1));		//get delta value of timer to end this function
	Chip_TIMER_SetMatch(LPC_TIMER32_0,0,delta);
	while(Chip_TIMER_ReadCount(LPC_TIMER32_0) < LPC_TIMER32_0->MR[0]);
}

void tmr_delay_cputicks(uint32_t cputicks){
	Chip_TIMER_Reset(LPC_TIMER32_0);
	Chip_TIMER_Enable(LPC_TIMER32_0);
	Chip_TIMER_SetMatch(LPC_TIMER32_0,0,cputicks);
	while(Chip_TIMER_ReadCount(LPC_TIMER32_0) < LPC_TIMER32_0->MR[0]);
	return;
}

void tmr_timeout_set(uint32_t us){
	Chip_TIMER_Reset(LPC_TIMER32_0);
	Chip_TIMER_Enable(LPC_TIMER32_0);
	uint32_t sclockmhz = Chip_Clock_GetSystemClockRate()/1000000;							//get system clock 48MHz
	uint32_t delta = (uint32_t)(sclockmhz*us/(LPC_TIMER32_0->PR+1));		//get delta value of timer to end this function
	Chip_TIMER_SetMatch(LPC_TIMER32_0,0,delta);

}

uint8_t tmr_timeout_off(){
	if(Chip_TIMER_ReadCount(LPC_TIMER32_0) < LPC_TIMER32_0->MR[0]){
		return 0;
	}
	else {
		return 1;
	}
}

static void sch_update(){
	uint32_t i;
	for(i = 0; i < MAX_TASKS; i++){
		if(task[i].taskfunc){
			//task is due to run
			if(task[i].tics_delay == 0){
				//cooperative task
				if(task[i].cooperative){
					task[i].runtask++;
				}
				//preemptive task
				else{
					task[i].taskfunc();
					if(task[i].tics_period == 0){
						sch_deletetask(i);
					}


				}
				if(task[i].tics_period){
					task[i].tics_delay = task[i].tics_period;
				}
			}
			else{
				task[i].tics_delay--;
			}
		}
	}
}



void sch_init(){
	uint32_t i;
	for(i = 0; i < MAX_TASKS; i++){
		sch_deletetask(i);
	}
	tmr_init();
	tick = 0;
}

void sch_start(){
	SysTick_Config(SystemCoreClock / TICKRATE_HZ);
}

void sch_dispatch(){
	uint32_t i;
	for(i = 0; i < MAX_TASKS; i++){
		if(task[i].runtask > 0 && task[i].cooperative == 1){
			task[i].runtask--;
			task[i].taskfunc();
			if(task[i].tics_period == 0){
				sch_deletetask(i);
			}

		}
	}
	sch_status();
}


uint32_t sch_addtask(void (* f) (void), uint32_t delay, uint32_t period, uint32_t coop){
	uint32_t i = 0;
	while(task[i].taskfunc != 0 && i < MAX_TASKS){
		i++;
	}
	if(i == MAX_TASKS){
		return MAX_TASKS;
	}
	task[i].taskfunc = f;
	task[i].tics_delay = delay;
	task[i].tics_period = period;
	task[i].runtask = 0;
	task[i].cooperative = coop;
	return i;
}




void SysTick_Handler(void)
{
	tick++;
	sch_update();
}
