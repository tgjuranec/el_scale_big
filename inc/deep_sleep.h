/*
 * deep_sleep.h
 *
 *  Created on: Sep 22, 2017
 *      Author: tgjuranec
 */

#ifndef DEEP_SLEEP_H_
#define DEEP_SLEEP_H_


void sleep_timer_init(uint32_t s);

void sleep_timer_reset(uint32_t s);

void sleep_timer_check();


#endif /* DEEP_SLEEP_H_ */
