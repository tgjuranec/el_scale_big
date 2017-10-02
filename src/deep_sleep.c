

#include <chip.h>
#include <sch.h>
#include <LCD1602.h>





#define ENABLE_CLKOUT	0


uint32_t deep_sleep_tick;
/*
 * waking up through PIN0_0
 */
	 
void WAKEUP_IRQHandler(void){
	/* Clear the pending interrupt of the start logic */
	LPC_SYSCTL->STARTRSRP0CLR	|=	(1<<(0));

	return;
}




static void config_ios(void){

	/* Configure all IOs as GPIO w/o pull-up & pull-down resistors */
	LPC_IOCON->REG[IOCON_PIO2_6]			= 0xC0;
	LPC_IOCON->REG[IOCON_PIO2_0]			= 0xC0;
	LPC_IOCON->REG[IOCON_PIO0_0] 	= 0xC1;      // RST_BOOT/P0_0
#if !ENABLE_CLKOUT
	LPC_IOCON->REG[IOCON_PIO0_1]			= 0xC0;      // CLKOUT/CT32B0_MAT2/USB_SOF
#endif /* ENABLE_CLKOUT */
	LPC_IOCON->REG[IOCON_PIO1_8]			= 0xC0;
	LPC_IOCON->REG[IOCON_PIO0_2]			= 0xC0;      // SSEL/CT16B0_CAP0	//Trigger input

	LPC_IOCON->REG[IOCON_PIO2_7]			= 0xC0;
	LPC_IOCON->REG[IOCON_PIO2_8]			= 0xC0;
	LPC_IOCON->REG[IOCON_PIO2_1]			= 0xC0;
	LPC_IOCON->REG[IOCON_PIO0_3]			= 0xC0;      // USB VBUS
	LPC_IOCON->REG[IOCON_PIO0_4]			= 0xC0;      // I2C_SCL, no pull-up, inactive
	LPC_IOCON->REG[IOCON_PIO0_5]			= 0xC0;      // I2C_SDA, no pull-up, inactive
	LPC_IOCON->REG[IOCON_PIO1_9]			= 0xC0;      // CT16B1_MAT0
	LPC_IOCON->REG[IOCON_PIO3_4]			= 0xC0;

	LPC_IOCON->REG[IOCON_PIO2_4]			= 0xC0;
	LPC_IOCON->REG[IOCON_PIO2_5]			= 0xC0;
	LPC_IOCON->REG[IOCON_PIO3_5]			= 0xC0;
	LPC_IOCON->REG[IOCON_PIO0_6]			= 0xC0;      // USB SoftConnect
	LPC_IOCON->REG[IOCON_PIO0_7	]		= 0xC0;
	LPC_IOCON->REG[IOCON_PIO2_9]			= 0xC0;
	LPC_IOCON->REG[IOCON_PIO2_10]			= 0xC0;
	LPC_IOCON->REG[IOCON_PIO2_2]			= 0xC0;

	LPC_IOCON->REG[IOCON_PIO0_8]			= 0xC0;		// SSP_MISO/CT16B0_MAT0/TRACE_CLK
	LPC_IOCON->REG[IOCON_PIO0_9]			= 0xC0;		// SSP_MOSI/CT16B0_MAT1/TRACE_SWV
	LPC_IOCON->REG[IOCON_PIO0_10]	= 0xC1;		// JTAG_CLK/P0_10/SSP_CLK/CT16B0_MAT2
	LPC_IOCON->REG[IOCON_PIO1_10]			= 0xC0;		// ADCIN6/CT16B1_MAT1
	LPC_IOCON->REG[IOCON_PIO2_11]			= 0xC0;
	LPC_IOCON->REG[IOCON_PIO0_11]	= 0xC1;		// JTAG_TDI/P0_11/ADCIN0/CT32B0_MAT3
	LPC_IOCON->REG[IOCON_PIO1_0]  = 0xC1;		// JTAG_TMS/P1_0/ADCIN1/CT32B1_CAP0
	LPC_IOCON->REG[IOCON_PIO1_1]  = 0xC1;		// JTAG_TDO/P1_1/ADCIN2/CT32B1_MAT0

	LPC_IOCON->REG[IOCON_PIO1_2] = 0xC1;	// JTAG_TRST/P1_2/ADCIN3/CT32B1_MAT1
	LPC_IOCON->REG[IOCON_PIO3_0]			= 0xC0;
	LPC_IOCON->REG[IOCON_PIO3_1]			= 0xC0;
	LPC_IOCON->REG[IOCON_PIO2_3]			= 0xC0;
	LPC_IOCON->REG[IOCON_PIO1_3]	= 0xC1;		// ARM_SWD/P1_3/ADCIN4/CT32B1_MAT2
	LPC_IOCON->REG[IOCON_PIO1_4]			= 0xC0;		// ADCIN5/CT32B1_MAT3/WAKEUP
	LPC_IOCON->REG[IOCON_PIO1_11]			= 0xC0;		// ADCIN7
	LPC_IOCON->REG[IOCON_PIO3_2]			= 0xC0;

	LPC_IOCON->REG[IOCON_PIO1_5]			= 0xC0;      // UART_DIR/CT32B0_CAP0
	LPC_IOCON->REG[IOCON_PIO1_6]			= 0xC0;      // UART_RXD/CT32B0_MAT0
	LPC_IOCON->REG[IOCON_PIO1_7]			= 0xC0;      // UART_TXD/CT32B0_MAT1
	LPC_IOCON->REG[IOCON_PIO3_3]			= 0xC0;
/*
 * INPUT PIN 1_9
 *
 * OUTPUT PINS:
 * PORT0 6,8,11
 * PORT1 6,7,10
 * PORT2 7,11
 */

	/* GPIOs at outputs */
	LPC_GPIO[0].DIR = 0xFFE;	//pin 0_0 is only input
	LPC_GPIO[1].DIR = 0xDFF;	//pin 1_9 is only input
	LPC_GPIO[2].DIR = 0xFFF;
	LPC_GPIO[3].DIR = 0xFFF;

	/* GPIO outputs to HIGH */			//LCD pins to '0'
	LPC_GPIO[0].DATA[0xFFF]  = 0x22; //0xFEF; //0x6AF pin 11,8,6,4 //0x020
	LPC_GPIO[1].DATA[0xFFF]  = 0x0; //0xFFF; //0xF3F pin 6,7,10   //0x0
	LPC_GPIO[2].DATA[0xFFF] =  0x0; //0xFFF;	//0XB7F pin 7,11    //0x0
	LPC_GPIO[3].DATA[0xFFF]  = 0x0;

	return;
}


static void deep_sleep_start(void) {


	/* Turn off all other peripheral dividers */
	LPC_SYSCTL->SSP0CLKDIV = 0;
	LPC_SYSCTL->SSP1CLKDIV = 0;
	LPC_SYSCTL->WDTCLKDIV = 0;


	/* SYSAHBCLKCTRL while running in FLASH */
	LPC_SYSCTL->SYSAHBCLKCTRL	=
								 (1<<0) 	//SYS Clock
								|(1<<1)		//ROM
								|(1<<2) 	//RAM
								|(1<<3) 	//FLASHREG
								|(1<<4) 	//FLASHARRAY
								|(1<<6) 	//GPIO
								|(1<<16)	//IOCON
								;  

	#if ENABLE_CLKOUT
	/* Output the Clk onto the CLKOUT Pin PIO0_1 to monitor the freq on a scope */
	LPC_IOCON->REG[IOCON_PIO0_1	= (1<<0);
	/* Select the MAIN clock as the clock out selection since it's driving the core */
	LPC_SYSCON->CLKOUTCLKSEL = 3;
	/* Set CLKOUTDIV to 6 */
	LPC_SYSCON->CLKOUTDIV = 1;		//	CLKOUT Divider = 1
	/* Enable CLKOUT */
	LPC_SYSCON->CLKOUTUEN = 0;
	LPC_SYSCON->CLKOUTUEN = 1;
	while (!(LPC_SYSCON->CLKOUTUEN & 0x01));
	#endif
		  						 
	/* Specify the start logic to allow the chip to be waken up */
	LPC_SYSCTL->STARTAPRP0 		&= ~(1<<(0));		// Falling edge
	LPC_SYSCTL->STARTRSRP0CLR	|=	(1<<(0));		// Clear pending bit
	LPC_SYSCTL->STARTERP0		|=	(1<<(0));		// Enable Start Logic





			/* Deep Sleep Mode Options */

				/* Turn on all the IRC & Flash  */
	LPC_SYSCTL->PDRUNCFG &= ~((1<<0) | (1<<1) | (1<<2));
				
				/* Switch MAINCLKSEL to IRC */
	LPC_SYSCTL->MAINCLKSEL	= 0;
	LPC_SYSCTL->MAINCLKUEN  = 0;
	LPC_SYSCTL->MAINCLKUEN  = 1;
	while (!(LPC_SYSCTL->MAINCLKUEN & 0x01));
	
	/* Make sure only the IRC is running */
	LPC_SYSCTL->PDRUNCFG = (~((1<<0) | (1<<1) | (1<<2) | (1<<9) | (1 << 12))) & 0xffff ;

	/* Clear the Deep Sleep Flag */
	LPC_PMU->PCON |= (1<<8);

	/* All OFF */
	LPC_SYSCTL->PDSLEEPCFG |= 0x000018FF;

	/* Specify peripherals to be powered up again when returning from deep sleep mode */
	LPC_SYSCTL->PDWAKECFG = LPC_SYSCTL->PDRUNCFG;
	/* Ensure DPDEN is disabled in the power control register */
	LPC_PMU->PCON	= (1<<11); //Clear DPDFLAG if it was set
	/* Specify Deep Sleep mode before entering mode */
	SCB->SCR	|=	(1<<2);		//Set SLEEPDEEP bit
				
	LPC_SYSCTL->STARTRSRP0CLR |= (1<<(0));
	NVIC_ClearPendingIRQ(PIO0_0_IRQn);
	NVIC_EnableIRQ(PIO0_0_IRQn);


	/* Reconfigure the IOs */
	config_ios();
				/* Enter Deep Sleep mode */
	__WFI();
	NVIC_SystemReset();
}


/*
 * input parameter number of seconds to activate
 */
void sleep_timer_init(uint32_t s){
	deep_sleep_tick = s*TICKRATE_HZ + tick;
}

void sleep_timer_reset(uint32_t s){
	deep_sleep_tick = s*TICKRATE_HZ + tick;
}

void sleep_timer_check(){
	if(tick > deep_sleep_tick){
		LCD1602_clrscr();
		LCD1602_displayoff();
		LCD1602_led_off();
		LCD1602_poweroff();
		deep_sleep_start();
	}
}
