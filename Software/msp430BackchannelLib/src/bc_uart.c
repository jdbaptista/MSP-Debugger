/*
 * uart.c
 *
 *  Created on: Sep 30, 2023
 *      Author: Jaden Baptista
 */
#include <msp430.h>
#include <stdint.h>
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
#ifdef __MSP430G2553__
    // use calibrated DCO for more accurate clock
    if (CALBC1_1MHZ==0xFF)                  // If calibration constant erased
    {
        while(1);                               // do not load, trap CPU!!
    }
    DCOCTL = 0;                               // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
    DCOCTL = CALDCO_1MHZ;
#endif

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
    BCCTL0 |= UCSPB; // two stop bits

    // Set baud rate to 9600 (br = 104, brs = 1, brf = 0) for smclk
    BCBR0 = 104;
    BCBR1 = 0;
    BCMCTL = 0x02; // UCBRSx = 1

    return 1;
}

inline void use_bc_uart_pins(void) {
#ifdef __MSP430F5529__
    // backchannel uart pins on
    // msp-exp430f5529lp
    P4SEL |= 0x18;
    P4DIR |= BIT4 + BIT5; //TODO: fix this
#endif

#ifdef __MSP430G2553__
    P1SEL |= BIT1 + BIT2;
    P1SEL2 |= BIT1 + BIT2;
    P1DIR |= BIT1 + BIT2; //TODO: fix this
#endif
}


inline void usci_reset(void) {
    BCCTL1 |= UCSWRST;
}


inline void usci_start(void) {
    BCCTL1 &= ~UCSWRST;
}


inline void enable_uart_tx_interrupt(void) {
#ifdef __MSP430F5529__
    BCIE |= UCTXIE;
#endif
#ifdef __MSP430G2553__
    BCIE |= UCA0TXIE;
#endif
}


inline void disable_uart_tx_interrupt(void) {
#ifdef __MSP430F5529__
    BCIE &= ~UCTXIE;
#endif
#ifdef __MSP430G2553__
    BCIE &= ~UCA0TXIE;
#endif
}


inline void enable_uart_rx_interrupt(void) {
#ifdef __MSP430F5529__
    BCIE |= UCRXIE;
#endif
#ifdef __MSP430G2553__
    BCIE |= UCA0RXIE;
#endif
}


inline void disable_uart_rx_interrupt(void) {
#ifdef __MSP430F5529__
    BCIE &= ~UCRXIE;
#endif
#ifdef __MSP430G2553__
    BCIE &= ~UCA0RXIE;
#endif
}

inline void clear_uart_tx_interrupt_flag(void) {
#ifdef __MSP430F5529__
    BCIFG &= ~UCTXIFG;
#endif
#ifdef __MSP430G2553__
    BCIFG &= ~UCA0TXIFG;
    BCIFG &= ~UCB0TXIFG; // may be lingering
#endif
}

inline void clear_uart_rx_interrupt_flag(void) {
#ifdef __MSP430F5529__
    BCIFG &= ~UCRXIFG;
#endif
#ifdef __MSP430G2553__
    BCIFG &= ~UCA0RXIFG;
    BCIFG &= ~UCB0RXIFG;
#endif
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
#ifdef __MSP430F5529__
   switch(__even_in_range(BCIV, 4)) {
       case 0: break; // no interrupt
       case 2: // data recieved
           clear_uart_rx_interrupt_flag();
           break;
       case 4: // tx buffer empty
           clear_uart_tx_interrupt_flag();
           if (curr_bc_buffer_ndx < 0) {
               break;
           } else if (bc_buffer[curr_bc_buffer_ndx + 1] == 0) {
               curr_bc_buffer_ndx = -1;
               break;
           } else {
               curr_bc_buffer_ndx++;
               BCTXBUF = bc_buffer[curr_bc_buffer_ndx];
           }
           break;
       default: break;
   }
#endif
#ifdef __MSP430G2553__
   clear_uart_tx_interrupt_flag();
   if (bc_buffer[curr_bc_buffer_ndx + 1] == 0) {
       curr_bc_buffer_ndx = -1;
   } else {
       curr_bc_buffer_ndx++;
       BCTXBUF = bc_buffer[curr_bc_buffer_ndx];
   }
#endif
}

/**
 * Sets the backchannel uart buffer and checks for conflicts.
 * Unfortunately this has no way of interrupting the processor
 * without some kind of RTOS setup, so polling should be used
 * on this function if it is currently unable to send new data.
 * If the buffer is too small for the input, transmits only a
 * portion of the input.
 * todo: software interrupt silly ^
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

/**
 * Prints the input as 0xABCD,
 * where ABCD is the hexadecimal
 * representation of the input.
 */
bool print_hex(uint16_t input) {
    const char ascii_table[] = {
                                '0', '1', '2', '3', '4', '5', '6', '7',
                                '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    }; // a hashmap of the ascii representation of lookup numbers 0-9.

    unsigned char reset_high = BCCTL1 & UCSWRST;
    if (curr_bc_buffer_ndx != -1 || reset_high) {
        return 0; // transmission already in progress or peripheral off
    }

    // write hex indicator to buffer
    bc_buffer[0] = ascii_table[0];
    bc_buffer[1] = 'x';

    // write hex values to buffer
    bc_buffer[2] = ascii_table[(input >> 12) & 0x000F];
    bc_buffer[3] = ascii_table[(input >> 8) & 0x000F];
    bc_buffer[4] = ascii_table[(input >> 4) & 0x000F];
    bc_buffer[5] = ascii_table[input & 0x000F];

    // write null terminator
    bc_buffer[6] = 0x00;

    // begin transmitting
    curr_bc_buffer_ndx = 0;
    BCTXBUF = bc_buffer[0];

    return 1;
}

bool print_binary(uint16_t input) {
    unsigned char reset_high = BCCTL1 & UCSWRST;
    if (curr_bc_buffer_ndx != -1 || reset_high) {
        return 0; // transmission already in progress or peripheral off
    }

    // write binary indicator to buffer
    bc_buffer[0] = '0';
    bc_buffer[1] = 'b';

    unsigned int i;
    for (i = 0; i < 16; i++) {
        bc_buffer[i + 2] = 48 + ((input >> (15 - i)) & 1);
    }

    // write null terminator
    bc_buffer[18] = 0x00;

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

void wait_print_hex(uint16_t input) {
    while (!print_hex(input)) {}
}

void wait_print_binary(uint16_t input) {
    while (!print_binary(input)) {}
}

/**
 * Polls the backchannel until done transmitting
 */
void uart_wait(void) {
    while (curr_bc_buffer_ndx != -1) {}
}

#endif
