#include <msp430.h>
#include <stdbool.h>
#include "tests.h"
#include "bc_uart.h"


#include "integration_tests.h" // CHANGE THIS LINE TO
                       // RUN DIFFERENT TESTS



/***
 * Things may not work here, but there is no
 * way to tell the user if they don't other
 * than figuring it out through debugging.
 */
inline void setup() {
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
    // setup backchannel at 9600 baud
    usciReset();
    // source UCSI from SMCLK
    BCCTL1 |= UCSSEL_3;
    useBCUartPins();
    uartConfig();
    usciStart();
    enableUartTXInterrupt();
    clearUartTXInterruptFlag();
    __bis_SR_register(GIE);
}

/**
 * main.c
 */
int main(void)
{
    setup();
    runTests(test_funcs, test_names, sizeof(test_names)/sizeof(char*));
    return 0;
}
