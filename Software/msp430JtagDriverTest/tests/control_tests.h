/*
 * control_tests.h
 *
 *  Created on: May 31, 2024
 *      Author: bapti
 */

#ifndef TESTS_CONTROL_TESTS_H_
#define TESTS_CONTROL_TESTS_H_


bool test_read_write();

static bool (*test_funcs[])(void) = {
                                     test_read_write,
};

static char* test_names[] = {
                             "test_read_write",
};


#endif /* TESTS_CONTROL_TESTS_H_ */
