/*
 * tests.h
 *
 *  Created on: May 27, 2024
 *      Author: bapti
 */

#ifndef TESTS_TESTS_H_
#define TESTS_TESTS_H_

void run_tests();

bool run_test(bool (*test)(void), char* test_name);

bool test_config(void);
bool test_use_pins(void);
bool test_usci_reset(void);
bool test_usci_start(void);
bool test_enable_tx_interrupt(void);
bool test_disable_tx_interrupt(void);
bool test_enable_rx_interrupt(void);
bool test_disable_rx_interrupt(void);
bool test_clear_tx_interrupt(void);
bool test_clear_rx_interrupt(void);
bool test_send_char(void);
bool test_print(void);

static bool (*test_funcs[])(void) = {
                                     test_use_pins,
                                     test_enable_tx_interrupt,
                                     test_disable_tx_interrupt,
                                     test_enable_rx_interrupt,
                                     test_disable_rx_interrupt,
                                     test_clear_tx_interrupt,
                                     test_clear_rx_interrupt,
                                     test_send_char,
                                     test_print,
};

static char* test_names[] = {
                             "test_use_pins",
                             "test_enable_tx_interrupt",
                             "test_disable_tx_interrupt",
                             "test_enable_rx_interrupt",
                             "test_disable_rx_interrupt",
                             "test_clear_tx_interrupt",
                             "test_clear_rx_interrupt",
                             "test_send_char",
                             "test_print",
};

#endif /* TESTS_TESTS_H_ */
