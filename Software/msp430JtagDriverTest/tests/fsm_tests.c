#include <stdbool.h>
#include <stdint.h>
#include "tests/fsm_tests.h"
#include "jtag_fsm.h"
#include "bc_uart.h"

bool test_init_fsm(void) {
    // case 1: standard operation
    bool result = initFSM();
    return result;
}

bool test_ir_shift(void) {
    // run prerequisite tests
    if (!test_init_fsm()) {
        return false;
    }
    // case 1: standard operation
    initFSM();
    unsigned int output = IR_SHIFT(0x00);
    if (output != 0x89) { // JTAG ID
        return false;
    }

    return true;
}

bool test_dr_shift(void) {
    // run prerequisite tests
    if (!test_init_fsm() || !test_ir_shift()) {
        return false;
    }
    // case 1: IR_BYPASS
    initFSM();
    IR_SHIFT(IR_BYPASS);
    DR_SHIFT(0); // set first bit
    volatile uint16_t output = DR_SHIFT(0x4411);
    if (output != 0x2208) {
        return false;
    }
    // case 2: MAB manipulation
    IR_SHIFT(IR_ADDR_16BIT);
    DR_SHIFT(0);
    output = DR_SHIFT(0x4411);
    if (output != 0) {
        return false;
    }

    return true;
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

