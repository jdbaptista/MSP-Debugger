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
    for (i = 0xC000; i <= 0xE000; i += 0x1000) {
        uint16_t j;
        for (j = i; j < i + 0x1000; j += 4) {
            if ((j) % 64 == 0) {
                wait_print("\033[E"); // newline command
                wait_print_hex(j);
                wait_print(": ");
            }
            volatile uint16_t output = readMem(j);
            wait_print_hex(output);
            wait_print(" ");
        }
    }

    releaseCPU();
    releaseDevice();
    releaseFSM();
}
