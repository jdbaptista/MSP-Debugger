/*
 * fsm_tests.h
 *
 *  Created on: May 28, 2024
 *      Author: bapti
 */

#ifndef TESTS_FSM_TESTS_H_
#define TESTS_FSM_TESTS_H_


bool test_ir_shift(void);
bool test_dr_shift(void);
bool test_ir_mab(void);

static bool (*test_funcs[])(void) = {
                                     test_ir_shift,
                                     test_dr_shift,
                                     test_ir_mab,
};

static char* test_names[] = {
                             "test_ir_shift",
                             "test_dr_shift",
                             "test_ir_mab",
};


#endif /* TESTS_FSM_TESTS_H_ */
