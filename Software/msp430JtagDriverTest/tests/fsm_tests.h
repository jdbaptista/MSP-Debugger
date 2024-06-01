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
bool test_ir_mdb(void);
bool test_ir_cpu(void);

static bool (*test_funcs[])(void) = {
                                     test_ir_shift,
                                     test_dr_shift,
                                     test_ir_mab,
                                     test_ir_mdb,
                                     test_ir_cpu,
};

static char* test_names[] = {
                             "test_ir_shift",
                             "test_dr_shift",
                             "test_ir_mab",
                             "test_ir_mdb",
                             "test_ir_cpu",
};


#endif /* TESTS_FSM_TESTS_H_ */
