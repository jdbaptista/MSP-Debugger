#include <msp430.h>
#include <jtag_fsm.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "bc_uart.h"
#include "jtag_fsm.h"
#include "jtag_control.h"
#include "disassembler.h"

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
    wait_print("\033[2J"); // clear screen command
    wait_print("\033[H"); // home cursor command
}

/**
 * main.c
 */
int main(void)
{
    uint16_t bytes[3];
    uint16_t curr_addr = 0xC000;
    char assembly[31];
    initBackchannel();
    initFSM();
    getDevice();
    haltCPU();

    bytes[0] = readMem(curr_addr);
    bytes[1] = readMem(curr_addr + 2);
    bytes[2] = readMem(curr_addr + 4);

    while (curr_addr >= 0xC000) { // continue until overflow
        uart_wait(); // for easy debugging
        int numBytes = nextInstruction(assembly, curr_addr, bytes, &curr_addr);
        if (numBytes > 0) {
            wait_print_hex(curr_addr);
            wait_print(": ");
            unsigned int i;
            for (i = 0; i < numBytes; i++) {
                wait_print_hex(bytes[i]);
                wait_print(" ");
            }
            wait_print("\033[E");
            wait_print(assembly);
            wait_print("\033[E");
            wait_print("\033[E");
        }

        bytes[0] = readMem(curr_addr);
        bytes[1] = readMem(curr_addr + 2);
        bytes[2] = readMem(curr_addr + 4);
    }

    releaseCPU();
    releaseDevice();
    releaseFSM();
}
