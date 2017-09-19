/*
 * sleep.h
 *
 *  Created on: Jan 19, 2015
 *      Author: tgjuranec
 */

#ifndef SLEEP_H_
#define SLEEP_H_

/*
 * function for starting countdown to sleep
 */
void LPC_sleep_timer_start(uint16_t ms);
bool LPC_sleep_timer_finished();
void LPC_sleep_selfawake(uint16_t ms);
void LPC_dsleep_selfawake(uint32_t ms);
void LPC_dsleep_extawake();


#endif /* SLEEP_H_ */
