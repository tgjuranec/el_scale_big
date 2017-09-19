/*
 * sleep.c
 *
 *  Created on: Jan 19, 2015
 *      Author: tgjuranec
 */

#include <chip.h>


/*
 * wakeup handlers
 */

uint32_t sysctl_clock_active;
void TIMER16_0_IRQHandler(void){
	return;
}

void el_scale(void);

void WAKEUP_IRQHandler(void){
	NVIC_ClearPendingIRQ(PIO0_9_IRQn);
	NVIC_DisableIRQ(PIO0_9_IRQn);
	Chip_SYSCTL_ResetStartPin(9);
	Chip_SYSCTL_DisableStartPin(9);
	Chip_SYSCTL_PowerUp(SYSCTL_POWERDOWN_SYSOSC_PD | SYSCTL_POWERDOWN_SYSPLL_PD);
	Chip_Clock_SetSystemPLLSource(SYSCTL_PLLCLKSRC_MAINOSC);
	while (!Chip_Clock_IsSystemPLLLocked()) {}
	Chip_Clock_SetMainClockSource(SYSCTL_MAINCLKSRC_PLLOUT);
	while (!(LPC_SYSCTL->MAINCLKUEN & 0x01));
	//el_scale();
	return;
}





/*
 * Call this function after last active function in main loop
 * 	ms min = 1
 * 	ms max = 65535 = 65,5s
 */

void LPC_sleep_timer_start(uint16_t ms){
	if(ms == 0) ms = 1;
	Chip_TIMER_Init(LPC_TIMER16_0);
	LPC_TIMER16_0->CTCR = 0;
	Chip_TIMER_Disable(LPC_TIMER16_0);
	Chip_TIMER_Reset(LPC_TIMER16_0);		//SET TIMER TO '0'
	Chip_TIMER_MatchDisableInt(LPC_TIMER16_0,0);
	//presc = 48000000/1000 = 48000 -> T = 1ms, ftimer = 1kHz
	Chip_TIMER_PrescaleSet(LPC_TIMER16_0,48000-1);
	LPC_TIMER16_0->PC = 0;
	Chip_TIMER_ResetOnMatchDisable(LPC_TIMER16_0,0);
	Chip_TIMER_StopOnMatchDisable(LPC_TIMER16_0,0);
	Chip_TIMER_SetMatch(LPC_TIMER16_0,0,ms);
	Chip_TIMER_Enable(LPC_TIMER16_0);
	//we don't enable interrupt-> we use to poll all values
}

bool LPC_sleep_timer_finished(){
	if(Chip_TIMER_ReadCount(LPC_TIMER16_0) > LPC_TIMER16_0->MR[0]){
		return true;
	}
	return false;
}

/*
 * Call this function after last active function in main loop
 * 	ms min = 1
 * 	ms max = 65535 = 65,5s
 */


void LPC_sleep_selfawake(uint16_t ms){
	//test if timer is finished
	if(Chip_TIMER_ReadCount(LPC_TIMER16_0) < LPC_TIMER16_0->MR[0]) return;
	LPC_TIMER16_0->MR[0] = 0;	//SET match reg i prescaler reg to '0'
	LPC_TIMER16_0->PR = 0;
	//if timer is finished->
	SCB->SCR = SCB->SCR & ~(SCB_SCR_SLEEPDEEP_Msk);
	Chip_TIMER_Init(LPC_TIMER16_0);
	LPC_TIMER16_0->CTCR = 0;
	LPC_TIMER16_0->PC = 0;
	Chip_TIMER_Disable(LPC_TIMER16_0);
	Chip_TIMER_Reset(LPC_TIMER16_0);		//SET TIMER TO '0'
	Chip_TIMER_PrescaleSet(LPC_TIMER16_0,48000-1);
	Chip_TIMER_SetMatch(LPC_TIMER16_0,0,ms);
	Chip_TIMER_MatchEnableInt(LPC_TIMER16_0,0);
	NVIC_EnableIRQ(TIMER_16_0_IRQn);
	Chip_TIMER_Enable(LPC_TIMER16_0);
	Chip_PMU_SleepState(LPC_PMU);
	return;
}


/*
 *  ms -> duration of sleeping
 *  ms min = 100
 *  ms max = (6553500/1000)s = 6553500 ms = 6553,5 s
 *
 */

void LPC_dsleep_selfawake(uint32_t ms){
	//test if timer is finished
	if(Chip_TIMER_ReadCount(LPC_TIMER16_0) < LPC_TIMER16_0->MR[0]) return;
	LPC_TIMER16_0->MR[0] = 0;	//SET match reg i prescaler reg to '0'
	LPC_TIMER16_0->PR = 0;
	//if timer is finished->
	//select power configuration PDSLEEPCFG register
	Chip_SYSCTL_SetDeepSleepPD(SYSCTL_DEEPSLP_BOD_PD);

	//In the SYSAHBCLKCTRL register disable all except timer and wtd osc
	sysctl_clock_active = LPC_SYSCTL->SYSAHBCLKCTRL;
	uint32_t sysctl_clock_sleep = 	(1 << SYSCTL_CLOCK_SYS) |
									(1 << SYSCTL_CLOCK_ROM) |
									(1 << SYSCTL_CLOCK_RAM) |
									(1 << SYSCTL_CLOCK_FLASHREG) |
									(1 << SYSCTL_CLOCK_FLASHARRAY) |
									(1 << SYSCTL_CLOCK_GPIO) |
									(1 << SYSCTL_CLOCK_CT16B0) |
									(1 << SYSCTL_CLOCK_IOCON);
	LPC_SYSCTL->SYSAHBCLKCTRL = sysctl_clock_sleep;
	//enable start logic
	Chip_SYSCTL_SetStartPin(9,0);
	Chip_SYSCTL_ResetStartPin(9);
	Chip_SYSCTL_EnableStartPin(9);
	//SET MATCH OUTPUT PIN
	Chip_IOCON_PinMux(LPC_IOCON,IOCON_PIO0_9,0,IOCON_FUNC2);
	//TIMER 16_0MAT1
	Chip_TIMER_Init(LPC_TIMER16_0);
	Chip_TIMER_Disable(LPC_TIMER16_0);
	Chip_TIMER_Reset(LPC_TIMER16_0);		//SET TIMER TO '0'
	LPC_TIMER16_0->CTCR = 0;
	LPC_TIMER16_0->PC = 0;
	Chip_TIMER_ResetOnMatchEnable(LPC_TIMER16_0,1);
	Chip_TIMER_StopOnMatchEnable(LPC_TIMER16_0,1);
	Chip_TIMER_PrescaleSet(LPC_TIMER16_0,940-1);	//inc every 100ms
	Chip_TIMER_SetMatch(LPC_TIMER16_0,1,(uint16_t)(ms/100));
	Chip_TIMER_MatchEnableInt(LPC_TIMER16_0,1);
	//SET BEHAVIOUR ON THE PIN
	Chip_TIMER_ExtMatchControlSet(LPC_TIMER16_0,1,TIMER_EXTMATCH_CLEAR,1);


	//SET watchdog oscilator and switch control of main osc to wd osc
	Chip_SYSCTL_PowerUp(SYSCTL_POWERDOWN_IRCOUT_PD | SYSCTL_POWERDOWN_IRC_PD | SYSCTL_POWERDOWN_FLASH_PD | SYSCTL_POWERDOWN_WDTOSC_PD | SYSCTL_POWERDOWN_SYSOSC_PD | SYSCTL_POWERDOWN_SYSPLL_PD);
	LPC_SYSCTL->WDTOSCCTRL = (1 << 5) | (0x1F);

	//select the power conf in the PDAWAKECFG register
	LPC_SYSCTL->PDWAKECFG = LPC_SYSCTL->PDRUNCFG;
	Chip_Clock_SetMainClockSource(SYSCTL_MAINCLKSRC_WDTOSC);

	//Write '1' into the SCB register
	SCB->SCR = SCB->SCR | (SCB_SCR_SLEEPDEEP_Msk);
	//clear deep powerdown flag
	LPC_PMU->PCON |= (1<<8);

	//enable IRQ
	//NVIC_EnableIRQ(TIMER_16_0_IRQn);
	Chip_TIMER_Enable(LPC_TIMER16_0);
	NVIC_ClearPendingIRQ(PIO0_9_IRQn);
	NVIC_EnableIRQ(PIO0_9_IRQn);
	Chip_PMU_SleepState(LPC_PMU);
	return;
}


void LPC_dsleep_extawake_check(){
	/* SYSAHBCLKCTRL while running in FLASH */
	LPC_SYSCTL->SYSAHBCLKCTRL	=
								 (1<<0) 	//SYS Clock
								|(1<<1)		//ROM
								|(1<<2) 	//RAM
								|(1<<3) 	//FLASHREG
								|(1<<4) 	//FLASHARRAY
								|(1<<6) 	//GPIO
								|(1<<12)	//UART
								|(1<<16)	//IOCON
								;
	/* Specify the start logic to allow the chip to be waken up */
	LPC_SYSCTL->STARTAPRP0 		&= ~(1<<(2));		// Falling edge
	LPC_SYSCTL->STARTRSRP0CLR	|=	(1<<(2));		// Clear pending bit
	LPC_SYSCTL->STARTERP0		|=	(1<<(2));		// Enable Start Logic
	/* Turn on all the IRC & Flash  */
	LPC_SYSCTL->PDRUNCFG &= ~((1<<0) | (1<<1) | (1<<2));

	/* Switch MAINCLKSEL to IRC */
	LPC_SYSCTL->MAINCLKSEL	= 0;
	LPC_SYSCTL->MAINCLKUEN  = 0;
	LPC_SYSCTL->MAINCLKUEN  = 1;
	while (!(LPC_SYSCTL->MAINCLKUEN & 0x01));

	/* Make sure only the IRC is running */
	LPC_SYSCTL->PDRUNCFG = ~((1<<0) | (1<<1) | (1<<2) | (1<<9)) ;

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

	LPC_SYSCTL->STARTRSRP0CLR |= (1<<(2));
	NVIC_ClearPendingIRQ(PIO0_9_IRQn);
	NVIC_EnableIRQ(PIO0_9_IRQn);



	/* Enter Deep Sleep mode */
	__WFI();

}

void LPC_dsleep_extawake(){
	//test if timer is finished
	if(Chip_TIMER_ReadCount(LPC_TIMER16_0) < LPC_TIMER16_0->MR[0]) return;
	LPC_TIMER16_0->MR[0] = 0;	//SET match reg i prescaler reg to '0'
	LPC_TIMER16_0->PR = 0;
	//if timer is finished->
	//select power configuration PDSLEEPCFG register
	Chip_SYSCTL_SetDeepSleepPD(SYSCTL_DEEPSLP_BOD_PD | SYSCTL_DEEPSLP_WDTOSC_PD);
	//control IRC
	Chip_SYSCTL_PowerUp(SYSCTL_POWERDOWN_IRCOUT_PD | SYSCTL_POWERDOWN_IRC_PD);
	Chip_Clock_SetMainClockSource(SYSCTL_MAINCLKSRC_IRC);
	//select the power conf in the PDAWAKECFG register
	Chip_SYSCTL_SetWakeup(SYSCTL_SLPWAKE_IRCOUT_PD | SYSCTL_SLPWAKE_IRC_PD | SYSCTL_SLPWAKE_SYSOSC_PD | SYSCTL_SLPWAKE_ADC_PD);
	//if ext pin is used to wake up -> enable start logic
	Chip_SYSCTL_SetStartPin(9,0);
	Chip_SYSCTL_ResetStartPin(9);
	Chip_SYSCTL_EnableStartPin(9);
	//In the SYSAHBCLKCTRL register disable all except timer and wtd osc
	Chip_Clock_DisablePeriphClock(LPC_SYSCTL->SYSAHBCLKCTRL);
	//set interrupt
	NVIC_ClearPendingIRQ(PIO0_9_IRQn);
	NVIC_EnableIRQ(PIO0_9_IRQn);
	//Write '1' into the SCB register
	SCB->SCR = SCB->SCR | (SCB_SCR_SLEEPDEEP_Msk);
	Chip_PMU_SleepState(LPC_PMU);
	return;
}
