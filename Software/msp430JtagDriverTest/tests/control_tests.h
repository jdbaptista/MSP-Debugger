/*
 * control_tests.h
 *
 *  Created on: May 31, 2024
 *      Author: bapti
 */

#ifndef TESTS_CONTROL_TESTS_H_
#define TESTS_CONTROL_TESTS_H_


bool test_get_device();
bool test_set_instr_fetch();
bool test_set_pc();
bool test_halt_pc();
bool test_release_cpu();
bool test_execute_por();
bool test_release_device();
bool test_read_mem();
bool test_write_mem();

static bool (*test_funcs[])(void) = {
                                     test_get_device,
                                     test_set_instr_fetch,
                                     test_set_pc,
                                     test_halt_pc,
                                     test_release_cpu,
                                     test_execute_por,
                                     test_release_device,
                                     test_read_mem,
                                     test_write_mem,
};

static char* test_names[] = {
                             "test_get_device",
                             "test_set_instr_fetch",
                             "test_set_pc",
                             "test_halt_pc",
                             "test_release_cpu",
                             "test_execute_por",
                             "test_release_device",
                             "test_read_mem",
                             "test_write_mem",
};


#endif /* TESTS_CONTROL_TESTS_H_ */
