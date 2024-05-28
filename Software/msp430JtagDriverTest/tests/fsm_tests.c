#include <stdbool.h>
#include "tests/fsm_tests.h"
#include "bc_uart.h"
#include "jtag_fsm.h"

bool test_init_fsm(void) {
    bool result = initFSM();
    return result;
}

bool test_ir_shift(void) {
    initFSM();
    unsigned int output = IR_SHIFT(0x00);
    if (output != 0x89) { // JTAG ID
        return false;
    }

    return true;
}

bool test_dr_shift(void) {
    return true;
}

void run_tests() {
    static const unsigned int length = sizeof(test_names)/sizeof(char*);
    bool results[length];

    unsigned int i;
    for(i = 0; i < length; i++) {
        wait_print("Running ");
        wait_print(test_names[i]);
        wait_print(" test...");
        wait_print("\033[E"); // newline command
        results[i] = run_test(test_funcs[i], test_names[i]);
        wait_print("\033[2J"); // clear screen command
        wait_print("\033[H"); // home cursor command
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

