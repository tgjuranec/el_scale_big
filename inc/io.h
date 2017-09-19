/*
 * io.h
 *
 *  Created on: Oct 5, 2014
 *      Author: tgjuranec
 */

#ifndef IO_H_
#define IO_H_
void io_init();
void io_set_as_input(CHIP_IOCON_PIO_T ioconpin);
bool io_get_input_state(CHIP_IOCON_PIO_T ioconpin);
void io_set_as_output(CHIP_IOCON_PIO_T ioconpin);
void io_set_output_state(CHIP_IOCON_PIO_T ioconpin, bool value);

#endif /* IO_H_ */
