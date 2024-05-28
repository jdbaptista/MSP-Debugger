#include <stdbool.h>
#include "tests.h"
#include "bc_uart.h"


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

bool test_config(void) {
    return true;
}

bool test_use_pins(void) {
    return true;
}

bool test_usci_reset(void) {
    return true;
}

bool test_usci_start(void) {
    return true;
}

bool test_enable_tx_interrupt(void) {
    return true;
}

bool test_disable_tx_interrupt(void) {
    return true;
}

bool test_enable_rx_interrupt(void) {
    return true;
}

bool test_disable_rx_interrupt(void) {
    return true;
}

bool test_clear_tx_interrupt(void) {
    return true;
}

bool test_clear_rx_interrupt(void) {
    return true;
}

bool test_send_char(void) {
    return true;
}

bool test_print(void) {
    return true;
}
