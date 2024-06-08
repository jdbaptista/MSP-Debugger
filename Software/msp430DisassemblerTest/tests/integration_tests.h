/*
 * integration_tests.h
 *
 *  Created on: Jun 7, 2024
 *      Author: bapti
 */

#ifndef TESTS_INTEGRATION_TESTS_H_
#define TESTS_INTEGRATION_TESTS_H_


bool test_single_instructions(void);
bool test_addc(void);

static bool (*test_funcs[])(void) = {
                                     test_single_instructions,
                                     test_addc,
};

static char* test_names[] = {
                             "test_single_instructions",
                             "test_addc",
};


#endif /* TESTS_INTEGRATION_TESTS_H_ */
