/*
 * tests.h
 *
 *  Created on: May 31, 2024
 *      Author: bapti
 */

#ifndef TESTS_TESTS_H_
#define TESTS_TESTS_H_


#include "bc_uart.h"

bool run_test(bool (*test)(), char* test_name) {
    uart_wait();
    bool result = test();
    wait_print(test_name);
    if (result) {
        wait_print(" passed.");
    } else {
        wait_print(" failed.");
    }
    wait_print("\033[E"); // newline command
    return result;
}

void run_tests(bool (*test_funcs[])(void), char* test_names[], unsigned int num_tests) {
    unsigned int i;
    for(i = 0; i < num_tests; i++) {
        run_test(test_funcs[i], test_names[i]);
    }
}


#endif /* TESTS_TESTS_H_ */
