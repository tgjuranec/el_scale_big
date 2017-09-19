/*
 * lcd1602.h
 *
 *  Created on: 8.8.2011.
 *      Author: djuka
 */

#ifndef LCD1602_H_
#define LCD1602_H_



#define LCD_BUSY_MATCH		0
#define LCD_ROTATE_MATCH	1


#define LCD1602_PRINTMASK 0x3FF

#define LCD1602_RS 0x0200
#define LCD1602_RW 0x0100


/*Display options*/
#define LCD1602_LINE_LEN	16
#define LCD1602_LINE_MEMORY_LEN	40



/*Communication words*/
/*
 *
Clear Display - Write “20H” to DDRA and set DDRAM address to “00H” from AC
execution time - 1.53ms
*/
#define LCD_1602_CLEAR_DISPLAY 0X01


/*
Return Home - Set DDRAM address to “00H” From AC and return cursor to Its original position if shifted.
The contents of DDRAM are not changed.
execution time - 1.53ms
*/
#define LCD_1602_RETURN_HOME 0x02


/*
Entry mode set- Assign cursor moving direction And blinking of entire display
execution time - 39us
*/
#define LCD_1602_ENTRY_MODE 0x04
#define LCD_1602_ID (1 << 1)			//high-> cursor/blink moves to the right, low-> cursor/blink moves to the left
#define LCD_1602_SH (1 << 0)			//high and DDRAM write operation, shift of entire display according to I/D value

/*
Display ON/OFF control Set display (D), cursor (C), and Blinking of cursor (B) on/off Control bit.
*/
#define LCD_1602_DISPLAY_ON_OFF 0x08
#define LCD_1602_DISPLAY_D 0x4				//high - display on
#define LCD_1602_DISPLAY_C 0X2				//high - cursor on
#define LCD_1602_DISPLAY_B 0X1				//high - blink on

/*
Cursor or Display shift - Set cursor moving and display Shift control bit, and the Direction, without changing of
DDRAM data.
*/
#define LCD_1602_CURSOR_DISPLAY_SHIFT 0X10
#define LCD_1602_SHIFT_DISPLAY 0X8			//'1' - shift display, '0' - shift cursor
#define LCD_1602_SHIFT_RIGHT 0X4			//'1' - shift right, '0' - shift left

/*
Function set - Set interface data length (DL: 8-Bit/4-bit), numbers of display Line (N: =2-line/1-line) and,
Display font type (F: 5x11/5x8)
*/
#define LCD_1602_FUNCTION_SET 0x20
#define LCD_1602_DL 0x10					//'1' - 8 bit bus transfer
#define LCD_1602_N	0x08					//'1' - 2 line display mode, '0' - 1 line display
#define LCD_1602_F	0x04					//'1' - 5x11 dots format, '0' 5x8 dots format


/*
Set CGRAM Address - Set CGRAM address in address Counter.
*/
#define LCD_1602_SET_CGRAM 0x40				//

/*
Set DDRAM Address - Set DDRAM address in address Counter.
*/
#define LCD_1602_SET_DDRAM 0x80

/*
Read busy Flag and Address Whether during internal Operation or not can be known by reading BF. The contents of
Address counter can also be read.
*/
#define LCD_1602_READ_BF 0x100				//read operation
#define LCD_1602_BF	0x80					//'1' -  busy, '0' - not busy
#define LCD_1602_ADDRESS_COUNTER_MASK 0X7F	// 7 LS bits show current status of address counter


/*
Write data to Address - Write data into internal RAM
(DDRAM/CGRAM).
*/
#define LCD_1602_WRITE_DATA 0x200			//write data it must bie 'OR'-ing with 8 bits data
/*
Read data From RAM - Read data from internal RAM
(DDRAM/CGRAM).
 */
#define LCD_1602_READ_DATA 0x300			//read data from register


extern char lcd_text[2][16];
extern uint8_t lcd_counter;



void LCD1602_init(CHIP_IOCON_PIO_T rs, CHIP_IOCON_PIO_T rw, \
		CHIP_IOCON_PIO_T d7, CHIP_IOCON_PIO_T d6, CHIP_IOCON_PIO_T d5, CHIP_IOCON_PIO_T d4, \
		CHIP_IOCON_PIO_T d3, CHIP_IOCON_PIO_T d2, CHIP_IOCON_PIO_T d1, CHIP_IOCON_PIO_T d0, \
		CHIP_IOCON_PIO_T e, CHIP_IOCON_PIO_T lcdpwr, CHIP_IOCON_PIO_T lcdled);
void LCD1602_exec();
void LCD1602_print(char *line1, char *line2);
#endif /* LCD1602_H_ */
