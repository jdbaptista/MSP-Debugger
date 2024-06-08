#include <msp430.h>
#include <jtag_fsm.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "bc_uart.h"
#include "jtag_fsm.h"
#include "jtag_control.h"

inline void initBackchannel() {
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
    // setup backchannel at 9600 baud, 2 stop bits
    usci_reset();
    BCCTL1 |= UCSSEL_3; // set uart to use SMCLK
    use_bc_uart_pins();
    uart_config();
    usci_start();
    enable_uart_tx_interrupt();
    clear_uart_tx_interrupt_flag();
    __bis_SR_register(GIE);
//    wait_print("\033[2J"); // clear screen command
//    wait_print("\033[H"); // home cursor command
}

/**
 * main.c
 */
int main(void)
{
    initBackchannel();
    initFSM();
    getDevice();
    haltCPU();

    uint16_t i;
    for (i = 0xC000; i <= 0xFFFF; i += 2) {
        wait_print_hex(i);
        wait_print(": ");
        wait_print_hex(readMem(i));
        wait_print("\033[E"); // newline command
    }

    releaseCPU();
    releaseDevice();
    releaseFSM();
}
