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
#include <IAP.h>
#include <io.h>
#include <sch.h>
#include <stdio.h>
#include <stdarg.h>
#include <deep_sleep.h>





typedef enum hx711_statetd{
	HX_INIT,
	HX_READY,
	HX_CAL_START_PRESS,
	HX_CAL_START_RELEASE,
	HX_CAL_PROC,
	HX_CAL_END,
	HX_ERROR
} hx711_state;

typedef struct hx711_conftd{
	CHIP_IOCON_PIO_T sck;
	CHIP_IOCON_PIO_T dt;
	CHIP_IOCON_PIO_T calibrate_button;
	CHIP_IOCON_PIO_T pwron;
	hx711_state state;
	int32_t tara;
} hx711_conf;

hx711_conf hc;



/*
 *
 * constant 791 -> results in gram
 * constant 79 -> results in tenth of gram
 *
 * 1959 FOR BIG SCALE
 */
static uint32_t hx711_val_to_10g(uint32_t val){
	uint32_t *flashdivider = (uint32_t *) 0x7000;
	if((*flashdivider == 0) || (*flashdivider == 0xFFFFFFFF)){
		return val/1959;
	}
	else{
		return val/(*flashdivider);
	}
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
	int32_t hx711_w[16], hx711_sw[16], read;
	uint32_t i;
	while(hc.state == HX_INIT){
		q31_t mw,stdw;
		for(i = 0;i < 16; i++){
			read= (int32_t) hx711_val_to_10g(hx711_read());
			hx711_w[i] = read;
			hx711_sw[i] = read << 8;
		}
		arm_mean_q31 (&hx711_w[0], 16, &mw);
		arm_std_q31(&hx711_sw[0], 16, &stdw);
		if(stdw == 0){
			hc.state = HX_READY;
			LCD1602_print("Weight: 0       ","Ready          ");
			hc.tara = mw;
			break;
		}
	}
	return;
}

static uint32_t sprintf_int(char *str,char *format,...){
	va_list l;
	va_start(l,format);
	char *count = &format[0];
	uint32_t nchar = 0;
	while(*count){
		//we found a marker for number
		if(*count == '%'){
			if(*(count+1) == 'd' || *(count+1) == 'u'){
				int num = va_arg(l,int);
				uint32_t div = 1000000000; //init billion
				//cnumber conteins digits in ascii format
				uint8_t written = 0;
				for(div = 1000000000; div > 0; div=div/10){
					if(num/div > 0){
						written = 1;
					}
					if(written == 1){
						str[nchar] = num/div + 0x30;
						nchar++;
					}
					num = num%div;
				}
				if(written == 0){
					str[nchar] = '0';
					nchar++;
				}
				count++;
			}
			else{
				va_end(l);
				return 0;
			}
		}
		//copy char form format into a output string
		else{
			str[nchar] = *count;
			nchar++;
		}
		count++;
	}
	str[nchar] = 0;
	va_end(l);
	return nchar;
}


/*
 * function for storing calibrated value
 * into flash memory on location 0x7800 (7th sector)
 */
static void hx711_calibration_store(uint32_t divider){
	uint32_t write[32];
	write[0] = divider;
	u32IAP_PrepareSectors(7,7);
	u32IAP_EraseSectors(7,7);
	u32IAP_PrepareSectors(7,7);
	u32IAP_CopyRAMToFlash(0x7000,&write[0],256);
	return;
}


void hx711_init(CHIP_IOCON_PIO_T sck_port, \
				CHIP_IOCON_PIO_T dt_port,
				CHIP_IOCON_PIO_T bt_callibration){
	//SET pins to appropriate direction
	//'sck' CLOCK to output, open drain
	//'dt' - DATA to input
	hc.state = HX_INIT;
	hc.sck = sck_port;
	hc.dt = dt_port;
	hc.calibrate_button = bt_callibration;
	io_set_as_input(hc.dt);
	io_set_as_output(hc.sck);
	io_set_output_state(hc.sck, 0);
	//set calibration button
	io_debounce_init(&(hc.calibrate_button),1);
	//set tara
	hx711_tara_set();
}


int32_t hx711_get_weight_oneshot(){
	int32_t res;
	res = hx711_read();
	return hx711_val_to_10g(res) - hc.tara;
}


void hx711_get_weight(){		//returns weight in tenths of kilograms
	static uint32_t hx711_count;
	static int32_t hx711_w[16], hx711_sw[16];			//list of weights
	char line1[17] = "                ", line2[17]= "                ";
	int32_t read = (int32_t) hx711_val_to_10g(hx711_read()) - hc.tara;	//read and convert
	hx711_w[hx711_count & 0xF] = read;				//assign value
	hx711_sw[hx711_count & 0xF] = read << 8;
	hx711_count++;									//increment counter

	switch(hc.state){
	case HX_READY:
		if((hx711_count & 0xF) == 0){
			q31_t mw,stdw;
			arm_mean_q31 (&hx711_w[0], 16, &mw);
			arm_std_q31(&hx711_sw[0], 16, &stdw);
			if(stdw == 0){		//lcd will be updated only if there is change in weight
					if(mw < 0){
						sprintf_int(line1,"-%d.%d kg",ABS(mw/10),ABS(mw%10));
					}
					else{
						sprintf_int(line1,"%d.%d kg",mw/10,mw%10);
					}
					sprintf_int(line2,"Ready");
					LCD1602_print(line1,line2);
			}
			else{
				sprintf_int(line2,"Measure...");
				LCD1602_print(line1,line2);
				sleep_timer_reset(60);
			}
		}
		//check button
		if(io_button_check(hc.calibrate_button)==BT_ON){
			hc.state = HX_CAL_START_PRESS;
		}
		break;
	case HX_CAL_START_PRESS:
		if(io_button_check(hc.calibrate_button)==BT_OFF){
			hc.state = HX_CAL_START_RELEASE;
			sprintf_int(line1,"Place 1 kg load,");
			sprintf_int(line2,"Press CAL");
			LCD1602_print(line1,line2);
		}
		break;

	case HX_CAL_START_RELEASE:
		if(io_button_check(hc.calibrate_button)==BT_ON){
			hc.state = HX_CAL_PROC;
			sprintf_int(line1,"Calibration");
			sprintf_int(line2,"Measure...");
			LCD1602_print(line1,line2);
			hx711_count = 0;
		}
		break;
	case HX_CAL_PROC:
		if((hx711_count & 0xF) == 0){
			q31_t mw,stdw;
			arm_mean_q31 (&hx711_w[0], 16, &mw);
			arm_std_q31(&hx711_sw[0], 16, &stdw);
			if(stdw == 0){		//lcd will be updated only if there is change in weight
				//ERROR during process of calibration
				if(mw < 0){
					hc.state = HX_ERROR;
					sprintf_int(line1,"0 kg");
					sprintf_int(line2,"Error");
					LCD1602_print(line1,line2);
				}
				//CALIBRATION OK
				else{
					hc.state = HX_CAL_END;
					uint32_t *flashdivider = (uint32_t *)0x7000;
					hx711_calibration_store((*flashdivider)*mw/1000);
					sprintf_int(line1,"1000 g");
					sprintf_int(line2,"Cal. OK");
					LCD1602_print(line1,line2);
				}
			}
			else{

			}
		}
	case HX_INIT:
		break;
	case HX_CAL_END:
		break;
	case HX_ERROR:
		break;
	default:
		break;
	}

}









