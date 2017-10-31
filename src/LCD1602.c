#include "chip.h"
#include "LCD1602.h"
#include <io.h>




//pin port configuration of lcd
typedef struct lcd_conftd{
	CHIP_IOCON_PIO_T rs;
	CHIP_IOCON_PIO_T rw;
	CHIP_IOCON_PIO_T d7;
	CHIP_IOCON_PIO_T d6;
	CHIP_IOCON_PIO_T d5;
	CHIP_IOCON_PIO_T d4;
	CHIP_IOCON_PIO_T d3;
	CHIP_IOCON_PIO_T d2;
	CHIP_IOCON_PIO_T d1;
	CHIP_IOCON_PIO_T d0;
	CHIP_IOCON_PIO_T e;
	CHIP_IOCON_PIO_T lcdpwr;
	CHIP_IOCON_PIO_T lcdled;
}lcd_conf;

lcd_conf lc;

//state of LCD_PRINTING function
volatile uint8_t ps;

#define PS_LCD_NOFLAG		0
#define PS_LCD_BUSY_FLAG	(1 << 0)
#define PS_LCD_ROTATE_FLAG	(1 << 1)
#define PS_LCD_4BIT_FLAG 	(1 << 2)

char lcd_text[2][16];
uint8_t lcd_counter = 34;



/*
 * timer counter is incremented every tick of the processor
 */
static void tmr1_init(){
	uint32_t presc = 1;					//(presc * match_reg) = pclk / fint = pclk * Tint
	Chip_TIMER_Init(LPC_TIMER32_1);
	Chip_TIMER_Disable(LPC_TIMER32_1);
	Chip_TIMER_Reset(LPC_TIMER32_1);
	LPC_TIMER32_1->CTCR = 0;
	Chip_TIMER_MatchDisableInt(LPC_TIMER32_1,0);
	Chip_TIMER_ResetOnMatchDisable(LPC_TIMER32_1,0);
	Chip_TIMER_StopOnMatchDisable(LPC_TIMER32_1,0);
	Chip_TIMER_PrescaleSet(LPC_TIMER32_1,presc-1);
	Chip_TIMER_Enable(LPC_TIMER32_1);
}



static inline void tmr1_delay_us(uint32_t us){
	Chip_TIMER_Reset(LPC_TIMER32_1);
	Chip_TIMER_Enable(LPC_TIMER32_1);
	uint32_t sclockmhz = Chip_Clock_GetSystemClockRate()/1000000;							//get system clock 48MHz
	uint32_t delta = (uint32_t)(sclockmhz*us/(LPC_TIMER32_1->PR+1));		//get delta value of timer to end this function
	Chip_TIMER_SetMatch(LPC_TIMER32_1,0,delta);
	while(Chip_TIMER_ReadCount(LPC_TIMER32_1) < LPC_TIMER32_1->MR[0]);
}

static inline void tmr1_delay_cputicks(uint32_t cputicks){
	Chip_TIMER_Reset(LPC_TIMER32_1);
	Chip_TIMER_Enable(LPC_TIMER32_1);
	Chip_TIMER_SetMatch(LPC_TIMER32_1,0,cputicks);
	while(Chip_TIMER_ReadCount(LPC_TIMER32_1) < LPC_TIMER32_1->MR[0]);
	return;
}

void tmr1_timeout_set(uint32_t us){
	Chip_TIMER_Reset(LPC_TIMER32_1);
	Chip_TIMER_Enable(LPC_TIMER32_1);
	uint32_t sclockmhz = Chip_Clock_GetSystemClockRate()/1000000;							//get system clock 48MHz
	uint32_t delta = (uint32_t)(sclockmhz*us/(LPC_TIMER32_1->PR+1));		//get delta value of timer to end this function
	Chip_TIMER_SetMatch(LPC_TIMER32_1,0,delta);
}

uint8_t tmr1_timeout_off(){
	if(Chip_TIMER_ReadCount(LPC_TIMER32_1) < LPC_TIMER32_1->MR[0]){
		return 0;
	}
	else {
		return 1;
	}
}




/*
 *MAIN FUNCTIONS
 */


/*
 * 8-bit transfer
 * Hbyte - 2 bits are RS and RW
 * Lbyte - 8 data bits (d7 to d0)
 *
 * 4-bit transfer
 * 5th bit RS, 4th bit RW, 3-0 bit - data bits
 */

static inline void LCD1602_strobe(){
	uint8_t i = 0;
	for(i = 5; i; i--);			//wait 20 ns
	io_set_output_state(lc.e,1);	//SET E
	for(i = 12; i; i--);			//wait 230 ns
	io_set_output_state(lc.e,0);	//CLEAR E
	for(i = 12; i; i--);			//wait 230 ns
}
#define STROBE (LCD1602_strobe())

static void LCD1602_senddata(uint16_t data){
	uint8_t i;
	CHIP_IOCON_PIO_T *piocon;
	if(ps & PS_LCD_4BIT_FLAG){
		piocon = &lc.d4;				//set to d4 and then decrement
		//first HIGH nibble
		//copy states from memory of high nibble + RW +RS to their pins
		for(i = 0; i < 6; i++){
			io_set_output_state(*piocon, ((data >> (i+4)) & 0x01));
			piocon--;
		}
		//transfer
		STROBE;
		//send LOW nibble
		piocon = &lc.d4;				//set to d4 and then decrement, RS and RW are already set
		for(i = 0; i < 4; i++){
			io_set_output_state(*piocon, ((data >> i) & 0x01));
			piocon--;
		}
		//transfer
		STROBE;
	}
	else{
		piocon = &lc.d0;				//set to d0 and then decrement
		for(i = 0; i < 10; i++){
			io_set_output_state(*piocon, ((data >> i) & 0x01));
			piocon--;
		}
		//transfer
		STROBE;
	}
	return;
}




static void LCD1602_printchar(uint8_t c){			//argument is 8-bit character
	while(!tmr1_timeout_off());
	uint16_t out = (LCD1602_RS) | (c & 0xFF);
	LCD1602_senddata(out);
	tmr1_timeout_set(44);
	return;
}


/*
 * c -> 8bits parameter direct command
 * 9 bit - RS bit is set to 0 , use printchar function if you want '1' here
 * 8 bit - RW bit
 * 7-0 bits - regular data bits
 */

static void LCD1602_print_ctrl(uint8_t c){					//argument is 8-bit number
	while(!tmr1_timeout_off());
	//check RS and RW data bits
	if(c & (LCD1602_RW | LCD1602_RS)) return;			// RW bit is set for read
	uint16_t out = c;
	LCD1602_senddata(out);
	if( (c == LCD_1602_CLEAR_DISPLAY) || (c == LCD_1602_RETURN_HOME )) tmr1_timeout_set(1531);
	else tmr1_timeout_set(44);
	return;
}



/*
 * STATIC INLINE functions
 */

static void LCD1602_set_DDRAM_address(uint8_t address){
	uint16_t out;
	out = LCD_1602_SET_DDRAM | address;
	LCD1602_print_ctrl(out);
	return;
}


static void LCD1602_return_home(){
	LCD1602_print_ctrl(LCD_1602_RETURN_HOME);
	return;
}


static void LCD1602_shift(uint8_t shift){
	uint16_t out;
	out = LCD_1602_CURSOR_DISPLAY_SHIFT | shift;
	LCD1602_print_ctrl(out);
	return;
}


void LCD1602_clrscr(){
	LCD1602_print_ctrl(LCD_1602_CLEAR_DISPLAY);		//clear display, SET AC to 0x00
}




/*
 * LED - opposite logic
 */

void LCD1602_led_on(){
	io_set_output_state(lc.lcdled,0);		//turn on LED
}

void LCD1602_led_off(){
	io_set_output_state(lc.lcdled,1);		//turn off LED
}

/*
 * LCD power  - opposite logic P-channel mosfet
 */
//in order to avoid problems regards voltage levels conversion
//LCD need to be powered after setting of GPIO connecting pins
//call this function AFTER all initialization on UC is finished!!!!
void LCD1602_poweron(){
	io_set_output_state(lc.lcdpwr,0);		//turn on transistor
	tmr1_delay_us(20000);		//delay until the supply voltage becomes stable
}

void LCD1602_poweroff(){
	io_set_output_state(lc.lcdpwr,1);		//turn OFF
}

void LCD1602_displayoff(){
	LCD1602_print_ctrl(LCD_1602_DISPLAY_ON_OFF);
}


void LCD1602_displayon(){
	LCD1602_print_ctrl(LCD_1602_DISPLAY_ON_OFF | LCD_1602_DISPLAY_D);
}

static inline void LCD1602_4bit_init(){
	uint8_t i;
	ps |= PS_LCD_4BIT_FLAG;

	//send 3x 0x03 to LCD
	uint8_t data = 0x03;
	CHIP_IOCON_PIO_T *piocon;
	piocon = &lc.d4;				//set to d4 and then decrement
	for(i = 0; i < 6; i++){
		io_set_output_state(*piocon, ((data >> i) & 0x01));
		piocon--;
	}
	for(i = 0; i < 3; i++){
		//transfer
		STROBE;
		tmr1_delay_us(100000);
	}

	//send 0x02
	data = 0x02;
	piocon = &lc.d4;				//set to d4 and then decrement
	for(i = 0; i < 6; i++){
		io_set_output_state(*piocon, ((data >> i) & 0x01));
		piocon--;
	}
	STROBE;
	tmr1_delay_us(10000);

	LCD1602_print_ctrl(LCD_1602_FUNCTION_SET | LCD_1602_N);
}


static inline void LCD1602_8bit_init(){

}


/*
 * init ports for LCD
 * 8bits - fullfill everything from d7 up to d0
 * 4bits - fullfill from d7 and d4 with their own IOCON values, and d3 to d0 with IOcon value of d4
 *
 * we need minimum 6 outputs: D7-D4, E, RS (RW connected to '0')
 * and maximum 13 outputs D7-D0, E, RS, RW, LED, PWR
 */

void LCD1602_init(CHIP_IOCON_PIO_T rs, CHIP_IOCON_PIO_T rw,  \
					CHIP_IOCON_PIO_T d7, CHIP_IOCON_PIO_T d6, CHIP_IOCON_PIO_T d5, CHIP_IOCON_PIO_T d4, \
					CHIP_IOCON_PIO_T d3, CHIP_IOCON_PIO_T d2, CHIP_IOCON_PIO_T d1, CHIP_IOCON_PIO_T d0, \
					CHIP_IOCON_PIO_T e, CHIP_IOCON_PIO_T lcdpwr, CHIP_IOCON_PIO_T lcdled)
{
	lc.rs = rs;
	lc.rw = rw;
	lc.e = e;
	lc.d7 = d7;
	lc.d6 = d6;
	lc.d5 = d5;
	lc.d4 = d4;
	lc.d3 = d3;
	lc.d2 = d2;
	lc.d1 = d1;
	lc.d0 = d0;
	lc.lcdled = lcdled;
	lc.lcdpwr = lcdpwr;
	CHIP_IOCON_PIO_T *piocon = &(lc.rs);
	uint8_t i;
	for(i = 0; i < sizeof(lcd_conf)/sizeof(CHIP_IOCON_PIO_T); i++){
		io_set_as_output(*piocon);
		io_set_output_state(*piocon,0);
		piocon++;
	}
	tmr1_init();
	LCD1602_poweron();
	ps = PS_LCD_NOFLAG;
	if(lc.d4 == lc.d0){			//4-bits transfer
		LCD1602_4bit_init();
	}
	else {	//8-bits transfer
		LCD1602_8bit_init();
	}
	//start LCD
	LCD1602_print_ctrl(LCD_1602_DISPLAY_ON_OFF);
	LCD1602_clrscr();
	LCD1602_print_ctrl(LCD_1602_ENTRY_MODE | LCD_1602_ID);
	LCD1602_set_DDRAM_address(0);
	LCD1602_print_ctrl(LCD_1602_DISPLAY_ON_OFF | LCD_1602_DISPLAY_D);
	return;
}



void LCD1602_exec(){
	//check whether we need to update

	if(lcd_counter < 34){
		if(lcd_counter == 0){
			LCD1602_set_DDRAM_address(0x0);
		}
		else if(lcd_counter < 17){
			LCD1602_printchar(lcd_text[0][(lcd_counter-1) & 0x0F]);
		}
		else if(lcd_counter == 17){
			LCD1602_set_DDRAM_address(0x40);
		}
		else if(lcd_counter > 17){
			LCD1602_printchar(lcd_text[1][(lcd_counter-2) & 0x0F]);

		}
		lcd_counter++;
	}
}

/*
 * function prints strings on LCD
 */
void LCD1602_print(char *line1, char *line2){
	//checking whether it is the last transmission is closed
	if(lcd_counter >= 34){
		//copy first line
		uint8_t i=0;
		while(line1[i] != 0 && i < 16){
			lcd_text[0][i] = line1[i];
			i++;
		}
		//fill the rest of the line with spaces '0x20'
		while(i < 16){
			lcd_text[0][i] = 0x20;
			i++;
		}
		i = 0;
		//copy the second line line
		while(line2[i] != 0 && i < 16){
			lcd_text[1][i] = line2[i];
			i++;
		}
		//fill the rest of the line with spaces '0x20'
		while(i < 16){
			lcd_text[1][i] = 0x20;
			i++;
		}
		lcd_counter = 0;
	}
}
