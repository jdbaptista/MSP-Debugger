/*
 * ir_shift_tests.c
 *
 *  Created on: Sep 30, 2023
 *      Author: bapti
 */

#include "include/ir_shift_tests.h"

#include <stdbool.h>
#include <stdlib.h>
#include "jtag_fsm.h"

/*
 * Tests that the output of ir_shift is 0x89,
 * which is the JTAG ID of all msp430 devices
 * as described on pg.64 of the MSP430 JTAG
 * programming guide.
 *
 * Returns true if the test passed successfully.
 * Returns false if the test did not pass.
 */
bool test_ir_shift_output() {
    initFSM();
    return false;
}
