/*
 * tests.h
 *
 *  Created on: May 31, 2024
 *      Author: bapti
 */

#ifndef TESTS_TESTS_H_
#define TESTS_TESTS_H_


#include "bc_uart.h"

bool runTest(bool (*test)(), char* test_name) {
    waitUart();
    bool result = test();
    waitPrint(test_name);
    if (result) {
        waitPrint(" passed.");
    } else {
        waitPrint(" failed.");
    }
    waitPrint("\033[E"); // newline command
    return result;
}

void runTests(bool (*test_funcs[])(void), char* test_names[], unsigned int num_tests) {
    unsigned int i;
    for(i = 0; i < num_tests; i++) {
        runTest(test_funcs[i], test_names[i]);
    }
}


#endif /* TESTS_TESTS_H_ */
