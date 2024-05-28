/*
 * fsm_tests.h
 *
 *  Created on: May 28, 2024
 *      Author: bapti
 */

#ifndef TESTS_FSM_TESTS_H_
#define TESTS_FSM_TESTS_H_

void run_tests();
bool run_test(bool (*test)(void), char* test_name);

bool test_init_fsm(void);
bool test_ir_shift(void);
bool test_dr_shift(void);

static bool (*test_funcs[])(void) = {
                                     test_init_fsm,
                                     test_ir_shift,
                                     test_dr_shift,
};

static char* test_names[] = {
                             "test_init_fsm",
                             "test_ir_shift",
                             "test_dr_shift",
};

#endif /* TESTS_FSM_TESTS_H_ */
