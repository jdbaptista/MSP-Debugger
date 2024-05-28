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

// JTAG Instructions: (pg. 14)

/***
 * This instruction enables setting of the MAB to a specific
 * value, which is shifted in with the next JTAG 16-bit data
 * access using the DR_SHIFT macro. The MAB of the MSP430 CPU
 * is set to the value written to the JTAG MAB register. The
 * previous value stored in the JTAG MAB register is
 * simultaneously shifted out on TDO while the new address is
 * shifted in through TDI.
 */
#define IR_ADDR_16BIT (0x83)

/***
 * This instruction enables readout of the data on the MAB with
 * the next data access. The MAB value is not changed during the
 * data access; that is, the data sent on TDI with this command
 * is ignored.
 */
#define IR_ADDR_CAPTURE (0x84)

/***
 * This instruction enables setting of the MSP430 MDB to a specific
 * value shifted in with the next JTAG data access. The MDB of the
 * MSP430 CPU is set to the value written to the JTAG MDB register.
 * As the new value is written to the MDB register, the prior value
 * in the MSP430 MDB is captured and shifted out on TDO. The MSP430
 * MAB is set by the value in the JTAG MAB register during execution
 * of the IR_DATA_TO_ADDR instruction. This instruction is used to
 * write to all memory locations of the MSP430.
 */
#define IR_DATA_TO_ADDR (0x85)

/***
 * This instruction enables setting of the MSP430 MDB to the
 * specified 16-bit value shifted in with the next 16-bit JTAG data
 * access. The complete MSP430 MDB is set to the value of the JTAG
 * MDB register. At the same time, the last value of the MSP430 MDB
 * is captured and shifted out on TDO. In this situation, the MAB is
 * still controlled by the CPU. The PC of the CPU sets the MAB value.
 */
#define IR_DATA_16BIT (0x41)

/***
 * This instruction enables setting of the MSP430 MDB to a specified
 * value shifted in with the next JTAG data access. The MSp430 MDB
 * is set to the value written to the JTAG MDB register. During the
 * data transfer, the previous MDB value is captured and shifted out
 * on TDO. The MAB value is set by the PC of the CPU. This instruction
 * auto-increments the program counter by two on every falling edge
 * of TCLK to automatically point to the next memory location. The
 * PC of the target CPU must be loaded with the starting memory
 * address before execution of this instruction, which can be used to
 * quickly read or write to a memory array. This instruction cannot
 * be used to write flash memory, nor read or write USB RAM as this
 * is dual ported RAM.
 */
#define IR_DATA_QUICK (0x43)

/***
 * This instruction delivers the input to TDI as an output
 * on TDO delayed by one TCK clock. When this instruction is loaded,
 * the IR_CNTRL_SIG_RELEASE instruction is performed simultaneously.
 * After execution of the bypass instruction, the data shifted out
 * on TDI does not affect any register of the JTAG control module of
 * the target MSP430 device.
 */
#define IR_BYPASS (0xFF)

/***
 * This instruction enables setting of the complete JTAG control
 * signal register with the next data access. Simultaneously, the
 * last value stored in the register is shifted out on TDO. The new
 * value takes effect when the TAP controller enters the UPDATE-DR
 * state.
 */
#define IR_CNTRL_SIG_16BIT (0x13)

/***
 * This instruction enables readout of the JTAG control signal
 * register with the next data access instruction.
 */
#define IR_CNTRL_SIG_CAPTURE (0x14)

/***
 * This instruction completely releases the CPU from JTAG control.
 * Once executed, the JTAG control signal register and other JTAG
 * data registers no longer have any effect on the target MSP430 CPU.
 */
#define IR_CNTRL_SIG_RELEASE (0x15)

/***
 * This instruction switches the JTAG_DATA_REG into the PSA mode. In
 * this mode, the program counter of the MSP430 is incremented by
 * every two system clocks provided on TCLK. The CPU program counter
 * must be loaded with the start address before execution of this
 * instruction. The number of TCLK clocks determines how many
 * memory locations are included in the PSA calculation.
 */
#define IR_DATA_PSA (0x44)

/***
 * This instruction should be used in conjunction with the IR_DATA_PSA
 * instruction. This instruction shifts out the PSA pattern generated
 * by the IR_DATA_PSA command. During the SHIFT-DR state of the TAP
 * controller, the content of the JTAG_DATA_REG is shifted out through
 * the TDO pin. While this JTAG instruction is executed, the capture
 * and update functions of the JTAG_DATA_REG are disabled.
 */
#define IR_SHIFT_OUT_PSA (0x46)

/***
 * This instruction sets the MSP430 into program-fuse mode.
 */
#define IR_PREPARE_BLOW (0x22)

/***
 * This instruction programs (blows) the access-protection fuse. To
 * execute properly, it must be loaded after the IR_PREPARE_BLOW
 * instruction is given.
 */
#define IR_EX_BLOW (0x24)

/***
 * No description provided.
 */
#define IR_JMB_EXCHANGE (0x61)

/*
 * Initializes the JTAG FSM to the IDLE state.
 *
 * Returns: 1 if FSM is put in the IDLE state successfully,
 *          0 if FSM failed fuse check or an error occurred.
 */
int initFSM();

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

/*
 * Sets TCLK to 1
 */
inline void setTCLK();

/*
 * Sets TCLK to 0
 */
inline void clrTCLK();

#endif /* JTAG_FSM_H_ */
