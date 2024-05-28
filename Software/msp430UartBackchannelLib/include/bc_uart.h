/*
 * uart.h
 *
 *  Created on: Sep 30, 2023
 *      Author: Jaden Baptista
 */

#ifndef INCLUDE_UART_H_
#define INCLUDE_UART_H_

#include <stdbool.h>
#include "bc_config.h"

#define USE_BC_IRQ // sets the interrupt handler
#define BC_BUFFER_SIZE 128


bool uart_config(void);
inline void use_bc_uart_pins(void);
inline void usci_reset(void);
inline void usci_start(void);

inline void enable_uart_tx_interrupt(void);
inline void disable_uart_tx_interrupt(void);
inline void enable_uart_rx_interrupt(void);
inline void disable_uart_rx_interrupt(void);
inline void clear_uart_tx_interrupt_flag(void);
inline void clear_uart_rx_interrupt_flag(void);

bool uart_send_char(char input);

#ifdef USE_BC_IRQ
bool print(char *input);
void wait_print(char *input);
#endif


#endif /* INCLUDE_UART_H_ */
