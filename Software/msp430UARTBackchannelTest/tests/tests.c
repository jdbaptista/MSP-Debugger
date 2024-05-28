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

bool test_config(void) {
    // case 1: check that registers are unchanged
    // upon usci software reset low with smclk usci
    // clock source and returns false.
    BCCTL1 &= ~UCSWRST;

    int prev_BCCTL0 = BCCTL0;
    int prev_BCCTL1 = BCCTL1;
    int prev_BCBR0 = BCBR0;
    int prev_BCBR1 = BCBR1;
    int prev_BCMCTL = BCMCTL;
    int prev_BCSTAT = BCSTAT;
    int prev_BCABCTL = BCABCTL;
    int prev_BCIE = BCIE;
    int prev_BCIFG = BCIFG;

    bool ret = uart_config();
    if (ret) return false;
    if (prev_BCCTL0 != BCCTL0 ||
            prev_BCCTL1 != BCCTL1 ||
            prev_BCBR0 != BCBR0 ||
            prev_BCBR1 != BCBR1 ||
            prev_BCMCTL != BCMCTL ||
            prev_BCSTAT != BCSTAT ||
            prev_BCABCTL != BCABCTL ||
            prev_BCIE != BCIE ||
            prev_BCIFG != BCIFG) {
        return false;
    }

    // case 2: check that non-smclk usci clock source
    // and software reset high leaves registers unchanged
    // and returns false.
    BCCTL1 |= UCSWRST;
    BCCTL1 &= ~(BIT7 + BIT6); // aclk
    BCCTL1 |= BIT6; // aclk

    prev_BCCTL0 = BCCTL0;
    prev_BCCTL1 = BCCTL1;
    prev_BCBR0 = BCBR0;
    prev_BCBR1 = BCBR1;
    prev_BCMCTL = BCMCTL;
    prev_BCSTAT = BCSTAT;
    prev_BCABCTL = BCABCTL;
    prev_BCIE = BCIE;
    prev_BCIFG = BCIFG;

    ret = uart_config();
    if (ret) return false;
    if (prev_BCCTL0 != BCCTL0 ||
            prev_BCCTL1 != BCCTL1 ||
            prev_BCBR0 != BCBR0 ||
            prev_BCBR1 != BCBR1 ||
            prev_BCMCTL != BCMCTL ||
            prev_BCSTAT != BCSTAT ||
            prev_BCABCTL != BCABCTL ||
            prev_BCIE != BCIE ||
            prev_BCIFG != BCIFG) {
        return false;
    }

    // case 3: correct operation.
    // reset peripheral
    BCCTL1 |= BIT7; // smclk
    BCCTL1 &= ~BIT6; // smclk
    BCCTL1 |= UCSWRST;

    volatile bool ret2 = uart_config();
    if (!ret2) {
        return false;
    }

    BCCTL1 &= ~UCSWRST;

    return true;
}

bool test_use_pins(void) {
    return true;
}

/***
 * THIS TEST SHOWS THAT RESETTING
 * THE UART PERIPHERAL BREAKS IT
 * EVEN AFTER RESTARTING. DONT DO
 * THIS.
 */
bool test_usci_reset(void) {
    usci_reset();
    if (BCCTL1 & UCSWRST != UCSWRST) {
        return false;
    }
    usci_start();
    if (BCCTL1 & UCSWRST == UCSWRST) {
        BCCTL1 &= ~UCSWRST; // start peripheral
        return false;
    }
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
