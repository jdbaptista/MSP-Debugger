#include <stdbool.h>
#include <stdint.h>
#include "fsm_tests.h"
#include "jtag_fsm.h"
#include "bc_uart.h"

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

    return true;
}

bool test_ir_mdb(void) {
    // case 1: IR_DATA_TO_ADDR

    // case 2: IR_DATA_16BIT

    // case 3: IR_DATA_QUICK

    return false;
}

bool test_ir_cpu(void) {
    // case 1: IR_CNTRL_SIG_16BIT

    // case 2: IR_CNTRL_SIG_CAPTURE

    // case 3: IR_CNTRL_SIG_RELEASE

    return false;
}

void run_tests() {
    static const unsigned int length = sizeof(test_names)/sizeof(char*);
    bool results[length];

    unsigned int i;
    for(i = 0; i < length; i++) {
        results[i] = run_test(test_funcs[i], test_names[i]);
    }

    for(i = 0; i < length; i++) {
        wait_print(test_names[i]);
        if (results[i]) {
            wait_print(" passed.");
        } else {
            wait_print(" failed.");
        }
        wait_print("\033[E"); // newline command
    }
}

bool run_test(bool (*test)(), char* test_name) {
    uart_wait();
    bool result = test();
    wait_print(test_name);
    if (result) {
        wait_print(" passed.");
    } else {
        wait_print(" failed.");
    }
    wait_print("\033[E"); // newline command
    return result;
}

