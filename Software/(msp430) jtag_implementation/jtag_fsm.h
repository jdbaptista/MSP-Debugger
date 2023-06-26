/*
 * jtag_fsm.h
 *
 *  Created on: Jun 19, 2023
 *      Author: Jaden Baptista
 *
 * JTAG Targets: MSP430G2553.
 *
 * Function definitions to drive the target's JTAG finite state machine (FSM).
 * This is accomplished through the use of bit banging. Interrupts won't cause
 * failure because the JTAG clock input (TCK) has no minimum switching speed.
 *
 * JTAG Interface Reference: https://www.ti.com/lit/ug/slau320aj/slau320aj.pdf
 */

#ifndef JTAG_FSM_H_
#define JTAG_FSM_H_

#include <stdint.h>


/*
 * Initializes the JTAG FSM to the IDLE state.
 *
 * Returns: 0 if FSM is put in the IDLE state successfully,
 *          1 if FSM failed fuse check or an error occurred.
 */
inline int initFSM();

/*
 * Shifts an 8-bit JTAG instruction into the JTAG instruction register (IR).
 *
 * input_data: The JTAG instruction to be shifted into the IR.
 *
 * Returns: 8-bit JTAG ID (See pg.64 of interface reference).
 */
uint8_t IR_SHIFT(uint8_t input_data);

/*
 * Shifts a 16-bit word into a JTAG data register (DR).
 *
 * input_data: The data to be shifted into the addressed DR.
 *
 * Returns: Last captured and stored value in the addressed DR.
 */
uint16_t DR_SHIFT(uint16_t input_data);


#endif /* JTAG_FSM_H_ */
