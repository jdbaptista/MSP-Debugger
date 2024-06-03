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
    // setup backchannel at 9600 baud
    usci_reset();
    // source UCSI from SMCLK
    BCCTL1 |= UCSSEL__SMCLK;
//    BCCTL1 &= ~(BIT7 | BIT6);
//    BCCTL1 |= UCSSEL__ACLK;
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
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    initBackchannel();
    initFSM();
    getDevice();
    setInstrFetch();
    haltCPU();

    uint16_t i;
    for (i = 0xBFF0; i >= 0xB000; i -= 16) {
        wait_print_hex(i + 0x100F);
        wait_print(": ");

        uint16_t j;
        for (j = 0x100F; j >= 0x1000; j--) {
            wait_print_hex(readMem(i + j));
            wait_print(" ");
        }

//        wait_print("\033[E"); // newline command
        wait_print("\n");
    }

    releaseCPU();
    releaseDevice();
}
