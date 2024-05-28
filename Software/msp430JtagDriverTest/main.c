#include <bc_uart.h>
#include <msp430.h>
#include <stdbool.h>
#include "tests/fsm_tests.h" // CHANGE THIS LINE TO RUN DIFFERENT TESTS

/***
 * This test file is special because it tests
 * the backchannel UART library. That is, it
 * will not state anything to the serial monitor
 * if it is not working! If so, debug the wait_print
 * function inside setup().
 ***/

/***
 * Things may not work here, but there is no
 * way to tell the user if they don't other
 * than figuring it out through debugging.
 */
inline void setup() {
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
    // setup backchannel at 9600 baud
    usci_reset();
    // source UCSI from SMCLK
    BCCTL1 |= UCSSEL__SMCLK;
    use_bc_uart_pins();
    uart_config();
    usci_start();
    enable_uart_tx_interrupt();
    clear_uart_tx_interrupt_flag();
    __bis_SR_register(GIE);
    wait_print("\033[2J"); // clear screen command
    wait_print("\033[H"); // home cursor command
}

/**
 * main.c
 */
int main(void)
{
    setup();
    run_tests();
    return 0;
}
