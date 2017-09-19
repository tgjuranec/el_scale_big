/*
 * HX711.h
 *
 *  Created on: Jul 24, 2014
 *      Author: djuka
 */

#ifndef HX711_H_
#define HX711_H_

extern int32_t hx711_w[16], hx711_sw[16];
extern uint32_t hx711_count;

#define ABS(x)           (((x) < 0) ? -(x) : (x))

void hx711_init(CHIP_IOCON_PIO_T sck_port, \
		CHIP_IOCON_PIO_T dt_port);
void hx711_get_weight();



#endif /* HX711_H_ */
