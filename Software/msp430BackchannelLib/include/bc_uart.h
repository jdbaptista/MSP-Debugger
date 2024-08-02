/*
 * uart.h
 *
 *  Created on: Sep 30, 2023
 *      Author: Jaden Baptista
 */

#ifndef INCLUDE_UART_H_
#define INCLUDE_UART_H_

#include <stdbool.h>
#include <stdint.h>
#include "bc_config.h"

#define BC_BUFFER_SIZE 128


bool uartConfig(void);
inline void useBCUartPins(void);
inline void usciReset(void);
inline void usciStart(void);

inline void enableUartTXInterrupt(void);
inline void disableUartTXInterrupt(void);
inline void enableUartRXInterrupt(void);
inline void disableUartRXInterrupt(void);
inline void clearUartTXInterruptFlag(void);
inline void clearUartRXInterruptFlag(void);

bool uartSendChar(char input);

#ifdef USE_BC_IRQ
bool print(char *input);
bool printHex(uint16_t input);
bool printBinary(uint16_t input);
void waitPrint(char *input);
void waitPrintHex(uint16_t input);
void waitPrintBinary(uint16_t input);
void waitUart(void);
#endif


#endif /* INCLUDE_UART_H_ */
