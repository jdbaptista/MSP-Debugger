/*
 * uart.h
 *
 *  Created on: Sep 30, 2023
 *      Author: bapti
 */

#ifndef INCLUDE_UART_H_
#define INCLUDE_UART_H_

#include <stdbool.h>

bool uart_init();
bool uart_send_char(char input);
bool uart_send_int(int input);
bool uart_send_string(char *input, int size);

#endif /* INCLUDE_UART_H_ */
