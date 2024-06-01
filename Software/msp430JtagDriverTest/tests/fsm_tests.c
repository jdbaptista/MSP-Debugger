#include <stdbool.h>
#include <stdint.h>
#include "fsm_tests.h"
#include "jtag_fsm.h"
#include "jtag_control.h"

bool test_ir_shift(void) {
    // case 1: standard operation
    initFSM();
    unsigned int output = IR_SHIFT(0x00);
    if (output != 0x89) { // JTAG ID
        return false;
    }

    return true;
}

bool test_dr_shift(void) {
    // case 1: IR_BYPASS
    initFSM(); // should default to IR_BYPASS
    DR_SHIFT(0); // set first bit
    volatile uint16_t output = DR_SHIFT(0x4411);
    if (output != 0x2208) {
        return false;
    }

    return true;
}

bool test_ir_mab(void) {
    // case 1: set and read MAB
    initFSM();
    IR_SHIFT(IR_ADDR_16BIT);
    volatile uint16_t output = DR_SHIFT(0xBEEF);
    output = DR_SHIFT(0);
    if (output != 0xBEEF) {
        return false;
    }

    // I'm not sure what the expected
    // behavior of IR_ADDR_CAPTURE is,
    // but it isn't doing what I think
    // it should be doing so I will not
    // write a test for it. It's usually
    // something around 0xC008, but I
    // can't seem to control the output.

    return true;
}
