/*
 * jtag_control.h
 *
 *  Created on: Sep 21, 2023
 *      Author: bapti
 */

#ifndef INCLUDE_JTAG_CONTROL_H_
#define INCLUDE_JTAG_CONTROL_H_

#include <stdint.h>

/*
 * Enables JTAG for devices with shared JTAG and GPIO pins.
 */
void getDevice();

/*
 * Sets the target CPU to the instruction-fetch state. In this
 * state the target CPU loads and executes an instruction as
 * it would in normal operation, except that the instruction is
 * transmitted through JTAG.
 *
 * Return: 0 if the target CPU was successfully set to
 *         the instruction-fetch state.
 *         1 if a JTAG access error has occurred and a JTAG
 *         reset is recommended.
 */
int setInstrFetch();

/*
 * Sets the target CPU program counter to the address provided.
 */
void setPC(uint16_t address);

/*
 * Sets the target CPU to a defined halt state. This is used to
 * access memory locations before the CPU is returned to normal
 * operation via releaseCPU().
 */
void haltCPU();

/*
 * Returns the target CPU to normal operation from a halt state
 * brought about by haltCPU() after memory accesses.
 */
void releaseCPU();

/*
 * Force a power-up reset of the target. This may be necessary
 * while the target is under JTAG control, such as before
 * programming or erasing the flash memory of the target.
 */
void executePOR();

/*
 * Releases the target CPU from JTAG control. The target MSP430
 * then starts executing the program at the address stored at
 * the reset vector 0xFFFE.
 */
void releaseDevice();

/*
 * Reads any memory address location (peripherals, RAM, or
 * flash/FRAM) from the target. The target CPU must be set
 * to a halt state through haltCPU() before memory accesses
 * can begin. When memory accesses are complete,
 * releaseCPU() should be called to return the target CPU
 * to normal operation.
 */
uint16_t readMem(uint16_t address);

/*
 * Writes to a memory location in peripherals or to RAM (but
 * not to flash or FRAM) of the target. The target CPU must
 * be set to a halt state through haltCPU() before memory
 * manipulation can begin. When memory manipulation is
 * complete, releaseCPU() should be called to return the
 * target CPU to normal operation.
 */
uint16_t writeMem(uint16_t address, uint16_t data);

#endif /* INCLUDE_JTAG_CONTROL_H_ */
