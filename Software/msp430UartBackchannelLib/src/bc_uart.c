/*
 * uart.c
 *
 *  Created on: Sep 30, 2023
 *      Author: Jaden Baptista
 */
#include <msp430.h>
#include "bc_uart.h"
#include "bc_config.h"

/**
 * Sets the USCI UART module defined
 * by backchannel_config.h to UART mode at
 * 9600 baud, configures the protocol, and
 * initializes appropriate interrupts. User
 * must hold USCI in reset mode before
 * calling, and release USCI when necessary.
 * The system must be configured to meet the
 * following specifications, otherwise this
 * function does nothing and returns false:
 *      - USCI is in reset mode
 *      - USCI clock source is SMCLK
 */
bool uart_config(void) {
    // check that USCI is in reset mode
    unsigned char reset_high = BCCTL1 & UCSWRST;
    if (!reset_high) {
        return 0;
    }

    // check USCI clock source is SMCLK
    unsigned char clk_sel_bits = BCCTL1 & (UCSSEL0 | UCSSEL1);
    if (clk_sel_bits != 0x80 && clk_sel_bits != 0xC0) {
        return 0;
    }

    // set USCI module to UART mode
    BCCTL0 &= ~UCSYNC;
    // configure UART
    BCCTL0 &= ~UCPEN; // no parity bit
    BCCTL0 &= ~UC7BIT; // 8-bit data
    BCCTL0 &= ~UCMSB; // LSB first
    BCCTL0 &= ~UCMODE0; // UART mode (00)
    BCCTL0 &= ~UCMODE1;

    // Set baud rate to 9600 (br = 104, brs = 1, brf = 0)
    BCBR0 = 104;
    BCBR1 = 0;
    BCMCTL = 0x02; // UCBRSx = 1

    return 1;
}

inline void use_bc_uart_pins(void) {
    // backchannel uart pins on
    // msp-exp430f5529lp
    P4SEL |= 0x18;
    P4DIR |= BIT4 + BIT5;
//    P3SEL |= 0x18;
//    P3DIR |= BIT3 + BIT4;
}


inline void usci_reset(void) {
    BCCTL1 |= UCSWRST;
}


inline void usci_start(void) {
    BCCTL1 &= ~UCSWRST;
}


inline void enable_uart_tx_interrupt(void) {
    BCIE |= UCTXIE;
}


inline void disable_uart_tx_interrupt(void) {
    BCIE &= ~UCTXIE;
}


inline void enable_uart_rx_interrupt(void) {
    BCIE |= UCRXIE;
}


inline void disable_uart_rx_interrupt(void) {
    BCIE &= ~UCRXIE;
}

inline void clear_uart_tx_interrupt_flag(void) {
    BCIFG &= ~UCTXIFG;
}

inline void clear_uart_rx_interrupt_flag(void) {
    BCIFG &= ~UCRXIFG;
}


bool uart_send_char(char input) {
    unsigned char reset_high = BCCTL1 & UCSWRST;
    if (reset_high) {
        return 0;
    }

    BCTXBUF = input; // clears UCTXIFG
    return 1;
}


#ifdef USE_BC_IRQ  // strings need a specific buffer and irq to behave properly
static char bc_buffer[BC_BUFFER_SIZE]; // should always be null terminated!
static int curr_bc_buffer_ndx = -1; // -1 indicates no current string transmission,
                                    // otherwise indicates the index of the
                                    // previously transmitted character.

#pragma vector=BC_INT_VEC
interrupt void bc_uart_irq(void) {
   switch(__even_in_range(BCIV, 4)) {
               case 0: break; // no interrupt
               case 2: // data recieved
                   BCIFG &= ~UCRXIFG;
                   break;
               case 4: // tx buffer empty
                   BCIFG &= ~UCTXIFG;
                   if (curr_bc_buffer_ndx < 0) {
                       break;
                   } else if (bc_buffer[curr_bc_buffer_ndx] == 0x00) {
                       curr_bc_buffer_ndx = -1;
                       break;
                   } else {
                       curr_bc_buffer_ndx++;
                       BCTXBUF = bc_buffer[curr_bc_buffer_ndx];
                   }
                   break;
               default: break;
           }
}

/**
 * Sets the backchannel uart buffer and checks for conflicts.
 * Unfortunately this has no way of interrupting the processor
 * without some kind of RTOS setup, so polling should be used
 * on this function if it is currently unable to send new data.
 * If the buffer is too small for the input, transmits only a
 * portion of the input.
 */
bool print(char *input) {
    unsigned char reset_high = BCCTL1 & UCSWRST;
    if (curr_bc_buffer_ndx != -1 || reset_high) {
        return 0; // transmission already in progress or peripheral off.
    }

    // write input to buffer
    unsigned int curr_ndx = 0;
    while (curr_ndx < BC_BUFFER_SIZE - 1 && input[curr_ndx] != 0x00) {
        bc_buffer[curr_ndx] = input[curr_ndx];
        curr_ndx++;
    }
    bc_buffer[curr_ndx] = 0x00; // add null terminator

    // begin transmitting
    curr_bc_buffer_ndx = 0;
    BCTXBUF = bc_buffer[0];

    return 1;
}

/***
 * Polls the backchannel until done transmitting.
 */
inline void wait_print(char *input) {
    while (!print(input)) {}
}

#endif
