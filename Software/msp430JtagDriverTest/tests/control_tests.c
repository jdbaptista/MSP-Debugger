/*
 * control_tests.c
 *
 *  Created on: May 31, 2024
 *      Author: bapti
 */

#include <msp430.h>
#include <stdlib.h>
#include <stdbool.h>
#include "control_tests.h"
#include "jtag_control.h"
#include "jtag_fsm.h"

bool test_read_write() {
    const uint16_t addr1 = 0x0332; // non-flash memory
    const uint16_t addr2 = 0x0200; // which is writeable

    initFSM();
    getDevice();
    haltCPU();
    writeMem(addr1, 0xB0BA);
    writeMem(addr2, 0xCAFE);
    releaseCPU();

    haltCPU();
    volatile uint16_t output = readMem(addr1);
    if (output != 0xB0BA) {
        return false;
    }
    output = readMem(addr2);
    if (output != 0xCAFE) {
        return false;
    }

    output = readMem(0xFFFE);
    output = readMem(0xC555);

    releaseCPU();

    return true;
}
