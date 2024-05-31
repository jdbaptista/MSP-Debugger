/*
 * jtag_control.c
 *
 *  Created on: May 31, 2024
 *      Author: bapti
 */
#include <msp430.h>
#include <stdlib.h>
#include <stdint.h>
#include "jtag_fsm.h"

/*
 * Takes the target CPU under JTAG control by setting TCE1 of
 * the JTAG control register to 1. Checks TCE to test if sync
 * with JTAG control was successful.
 */
void getDevice() {
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT(0x2401);
    IR_SHIFT(IR_CNTRL_SIG_CAPTURE);
    volatile uint16_t output = DR_SHIFT(0);
    while (!(DR_SHIFT(0) & BIT9)) {} // wait for sync
}

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
int setInstrFetch() {
    IR_SHIFT(IR_CNTRL_SIG_CAPTURE);
    int i;
    for (i = 0; i < 8; i++) {
        if (!(DR_SHIFT(0) & BIT7)) {
            return true;
        }
        ClrTCLK();
        SetTCLK();
    }
    return false;
}

/*
 * Sets the target CPU program counter to the address provided.
 */
void setPC(uint16_t address) {
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT(0x3401); // release low byte
    IR_SHIFT(IR_DATA_16BIT);
    DR_SHIFT(0x4030); // instruction to load PC
    ClrTCLK();
    SetTCLK();
    DR_SHIFT(address);
    ClrTCLK();
    SetTCLK();
    IR_SHIFT(IR_ADDR_CAPTURE); // disable IR_DATA_16BIT
    ClrTCLK();
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT(0x2401); // low byte controlled by JTAG
}

/*
 * Sets the target CPU to a defined halt state. This is used to
 * access memory locations before the CPU is returned to normal
 * operation via releaseCPU().
 */
void haltCPU() {
    IR_SHIFT(IR_DATA_16BIT);
    DR_SHIFT(0x3FFF); // JMP $ instruction to keep
                      // CPU from changing state
    ClrTCLK();
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT(0x2409); // set HALT_JTAG bit
    SetTCLK();
}

/*
 * Returns the target CPU to normal operation from a halt state
 * brought about by haltCPU() after memory accesses.
 */
void releaseCPU() {
    ClrTCLK();
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT(0x2401); // clear HALT_JTAG bit
    IR_SHIFT(IR_ADDR_CAPTURE);
    SetTCLK();
}

/*
 * Force a power-up reset of the target. This may be necessary
 * while the target is under JTAG control, such as before
 * programming or erasing the flash memory of the target.
 */
void executePOR() {
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT(0x2C01); // apply reset
    DR_SHIFT(0x2401); // remove reset
    ClrTCLK();
    SetTCLK();
    ClrTCLK();
    SetTCLK();
    ClrTCLK();
    IR_SHIFT(IR_ADDR_CAPTURE);
    SetTCLK();
}

/*
 * Releases the target CPU from JTAG control. The target MSP430
 * then starts executing the program at the address stored at
 * the reset vector 0xFFFE.
 */
void releaseDevice() {
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT(0x2C01); // apply reset
    DR_SHIFT(0x2401); // remove reset
    IR_SHIFT(IR_CNTRL_SIG_RELEASE);
}

/*
 * Reads any memory address location (peripherals, RAM, or
 * flash/FRAM) from the target. The target CPU must be set
 * to a halt state through haltCPU() before memory accesses
 * can begin. When memory accesses are complete,
 * releaseCPU() should be called to return the target CPU
 * to normal operation.
 */
uint16_t readMem(uint16_t address) {
    ClrTCLK();
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT(0x2409); // read one word from memory. To read
                      // a byte, the value shifted is 0x2419.
    IR_SHIFT(IR_ADDR_16BIT);
    DR_SHIFT(address);
    IR_SHIFT(IR_DATA_TO_ADDR);
    SetTCLK();
    ClrTCLK();
    return DR_SHIFT(0);
}

/*
 * Writes to a memory location in peripherals or to RAM (but
 * not to flash or FRAM) of the target. The target CPU must
 * be set to a halt state through haltCPU() before memory
 * manipulation can begin. When memory manipulation is
 * complete, releaseCPU() should be called to return the
 * target CPU to normal operation.
 */
uint16_t writeMem(uint16_t address, uint16_t data) {
    ClrTCLK();
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT(0x2408); // write one word to memory. For a
                      // byte, the value shifted is 0x2418.
    IR_SHIFT(IR_ADDR_16BIT);
    DR_SHIFT(address);
    IR_SHIFT(IR_DATA_TO_ADDR);
    DR_SHIFT(data);
    SetTCLK();
}


