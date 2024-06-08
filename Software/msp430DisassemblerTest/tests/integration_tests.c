/*
 * integration_tests.c
 *
 *  Created on: Jun 7, 2024
 *      Author: bapti
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "integration_tests.h"
#include "disassembler.h"

bool test_single_instructions(void) {
    uint16_t bytes[] = {
                        0x8321,
                        0xBBBB,
                        0xAAAA,
    };
    char output[21];
    uint16_t start_addr = 0x1000;
    uint16_t next_addr = 0x0000;
    bool result;
    result = nextInstruction(output, start_addr, bytes, &next_addr);

    if (!result || strcmp(output, "SUB.W @CG2 SP") != 0 || next_addr != 0x1002) {
        return false;
    }

    return true;
}

bool test_addc(void) {
    uint16_t bytes[] = {
                        0x6795, // ADDC.W 0x1A1B(R7) 0x2424(R5)
                        0x1A1B,
                        0x2424,
    };
    char output[31]; // maximum length can be 30 characters, plus 1 null terminator
    uint16_t start_addr = 0x1000;
    uint16_t next_addr = 0;
    bool result;
    result = nextInstruction(output, start_addr, bytes, &next_addr);

    if (!result || strcmp(output, "ADDC.W 0x1A1B(R7) 0x2424(R5)") != 0 || next_addr != 0x1006) {
        return false;
    }

    return true;
}


