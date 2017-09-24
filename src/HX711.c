/*
 * HX711.c
 *
 *  Created on: Jul 24, 2014
 *      Author: djuka
 */


#include "chip.h"
#include "HX711.h"
#include "LCD1602.h"
#include "arm_math.h"
#include "math_helper.h"
#include <io.h>
#include <sch.h>
#include <stdio.h>
#include <deep_sleep.h>





typedef struct hx711_conftd{
	CHIP_IOCON_PIO_T sck;
	CHIP_IOCON_PIO_T dt;
	CHIP_IOCON_PIO_T pwron;
	int32_t tara;
} hx711_conf;

hx711_conf hc;



/*
 *
 * constant 791 -> results in gram
 * constant 79 -> results in tenth of gram
 */
static uint32_t hx711_val_to_10g(uint32_t val){
	return val/1959;
}






static int32_t hx711_read(){
	//checking data ready -> DT should be '0' -> out of loop
	while(io_get_input_state(hc.dt));	//we are waiting for '0'

	//data are ready -> start clock
	io_set_output_state(hc.sck, 0);
	tmr_delay_us(1);
	uint32_t count = 0, i=0;
	__disable_irq();
	for(i = 0; i < 24; i++){
		io_set_output_state(hc.sck, 1);
		tmr_delay_us(1);
		count = count << 1;
		if(io_get_input_state(hc.dt)){
			count++;
		}
		io_set_output_state(hc.sck, 0);
		tmr_delay_us(1);
	}
	io_set_output_state(hc.sck, 1);
	tmr_delay_us(1);
	io_set_output_state(hc.sck, 0);
	__enable_irq();

	tmr_delay_us(1);
	return count;
}




/*
 * function for setting tara
 * scale need to be without burden
 */

static void hx711_tara_set(){
	int32_t res;
	res = hx711_read();
	hc.tara = hx711_val_to_10g(res);
	return;
}




void hx711_init(CHIP_IOCON_PIO_T sck_port, \
				CHIP_IOCON_PIO_T dt_port){
	//SET pins to appropriate direction
	//'sck' CLOCK to output, open drain
	//'dt' - DATA to input
	hc.sck = sck_port;
	hc.dt = dt_port;
	io_set_as_input(hc.dt);
	io_set_as_output(hc.sck);
	io_set_output_state(hc.sck, 0);
	//set tara
	hx711_tara_set();
	LCD1602_print("Weight: 0       ","W max: 0       ");
}


int32_t hx711_get_weight_oneshot(){
	int32_t res;
	res = hx711_read();
	return hx711_val_to_10g(res) - hc.tara;
}


void hx711_get_weight(){		//returns weight in tenths of kilograms
	static uint32_t hx711_count;
	static int32_t hx711_w[16], hx711_sw[16];			//list of weights
	static int32_t mw_old,mw_max;
	int32_t read = (int32_t) hx711_val_to_10g(hx711_read()) - hc.tara;	//read and convert
	hx711_w[hx711_count & 0xF] = read;				//assign value
	hx711_sw[hx711_count & 0xF] = read << 8;
	hx711_count++;									//increment counter
	if((hx711_count & 0xf) == 0){
		q31_t mw,stdw;
		arm_mean_q31 (&hx711_w[0], 16, &mw);
		arm_std_q31(&hx711_sw[0], 16, &stdw);
		if((stdw == 0) && (mw_old != mw)){		//lcd will be updated only if there is change in weight
			char line1[17], line2[17];
			if(mw < 0){
				sprintf(line1,"Weight: -%d.%d",ABS(mw/10),ABS(mw%10));
			}
			else{
				sprintf(line1,"Weight: %d.%d",mw/10,mw%10);
			}
			if(mw > mw_max) mw_max = mw;
			sprintf(line2,"W max: %d.%d",mw_max/10,mw_max%10);
			LCD1602_print(line1,line2);
			sleep_timer_reset(60);
		}
		mw_old = mw;
	}
}









