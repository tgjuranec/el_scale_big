/*
===============================================================================
 Name        : el_scale.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/


#include "chip.h"
#include <LCD1602.h>
#include <HX711.h>
#include <string.h>
#include <stdio.h>

#include <io.h>
#include <cr_section_macros.h>
#include <data_process.h>
#include <deep_sleep.h>
#include <sch.h>









int main(void) {
	Chip_SYSCTL_PowerUp(SYSCTL_POWERDOWN_SYSOSC_PD);
	Chip_Clock_SetSystemPLLSource(SYSCTL_PLLCLKSRC_MAINOSC);
	while (!Chip_Clock_IsSystemPLLLocked()) {}

	SystemCoreClockUpdate();
	//enable clockout PIN 0_1
	Chip_Clock_SetCLKOUTSource(SYSCTL_CLKOUTSRC_MAINSYSCLK,100);
	Chip_IOCON_PinMux(LPC_IOCON,IOCON_PIO0_1,0,IOCON_FUNC1);

	sch_init();
	tmr_delay_us(50000);


	// LCD PIN CONFIGURATION
	//RS
	//RW - dummy - NC
	//D7
	//D6
	//D5
	//D4
	//D3 - dummy - NC
	//D2 - dummy - NC
	//D1 - dummy - NC
	//D0 - dummy
	//E
	//LCD_PWR - dummy - NC
	//LCD_LED - dummy - NC

    LCD1602_init(IOCON_PIO1_7, /*RS*/ \
			IOCON_PIO2_7, /*RW*/ \
			IOCON_PIO0_8, /*D7*/ \
			IOCON_PIO1_10, /*D6*/ \
			IOCON_PIO2_11, /*D5*/ \
			IOCON_PIO0_11, /*D4*/ \
			IOCON_PIO0_11, \
 			IOCON_PIO0_11, \
			IOCON_PIO0_11, \
			IOCON_PIO0_11, \
			IOCON_PIO1_6, /*E*/ \
			IOCON_PIO0_4, \
			IOCON_PIO0_5);

	// HX711 PIN CONFIGURATION
	// clock
	// data
    //calibration button
	hx711_init(IOCON_PIO0_6, IOCON_PIO1_9,IOCON_PIO0_2);
    sleep_timer_init(60);


	sch_addtask(io_debounce_exec,0,1,1);
	sch_addtask(hx711_get_weight,99,100,1);
	sch_addtask(LCD1602_exec,1,10,1);
	sch_addtask(sleep_timer_check,19,20,1);
	sch_start();
	while(1){
		sch_dispatch();

	}

    return 0 ;
}
