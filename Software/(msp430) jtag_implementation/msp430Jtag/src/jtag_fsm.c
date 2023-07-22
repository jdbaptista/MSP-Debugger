/*
 * jtag_fsm.c
 *
 *  Created on: Jun 19, 2023
 *      Author: Jaden Baptista
 */

#include <jtag_config.h>
#include <msp430.h>
#include <stdlib.h>
#include <stdint.h>
#include "jtag_fsm.h"
#include "jtag_config.h"

inline int initFSM() {
    // configure JTAG GPIO pins
    JTAGDIR = 0xFF;  // Begin with all pins set to output
    JTAGDIR &= ~TDO; // Set TDO to input
    JTAGOUT = 0x00;

    // JTAG entry sequence: case 2a, Fig.2-13
    JTAGOUT |= RST;
    JTAGOUT |= TEST;
    JTAGOUT &= ~RST;
    clock(JTAGOUT, TEST);
    JTAGOUT |= RST;

    // Reset FSM to IDLE in case of faulty fuse check
    JTAGOUT |= TMS;
    int i;
    for (i = 0; i < 6; i++) {
        clock(JTAGOUT, TCK);    // FSM: TLR
    }
    JTAGOUT &= ~TMS;
    for (i = 0; i < 3; i++) {
        clock(JTAGOUT, TCK);     // FSM: IDLE
    }

    // TODO: Perform fuse check
    JTAGOUT |= TMS;
    slowClock(JTAGOUT, TMS);
    slowClock(JTAGOUT, TMS);
    JTAGOUT &= ~TMS;

    return 0;
}

uint8_t IR_SHIFT(uint8_t input_data) {
    uint8_t output_data = 0;
    int prev_TDI = JTAGOUT & TDI;

    // Set FSM to Shift-IR state
    // FSM set through falling and rising edge
    JTAGOUT |= TMS;     // TMS high
    clock(JTAGOUT, TCK);    // (1) FSM: Select-DR
    clock(JTAGOUT, TCK);    // (1) FSM: Select-IR
    JTAGOUT &= ~TMS;    // TMS low
    clock(JTAGOUT, TCK);     // (0) FSM: Capture-IR
    clock(JTAGOUT, TCK);     // (0) FSM: Shift-IR

    // Shift data into IR LSB first
    uint8_t bit;
    int i;
    for (i = 0; i < 7; i++) {
        bit = input_data >> i;
        bit &= 0x01; // send the selected bit
        setLevel(JTAGOUT, TDI, bit);
        clock(JTAGOUT, TCK);
        uint8_t level = 1;
        if (JTAGOUT & TDO == 0) {
            level = 0;
        }
        output_data |= level << (7 - i);
    }

    // Send MSB and return to IDLE state
    bit = input_data >> 7;
    bit &= 0x01;
    JTAGOUT |= TMS;
    setLevel(JTAGOUT, TDI, bit);
    clock(JTAGOUT, TCK);    // (1) FSM: Exit-IR

    if (JTAGOUT & TDO == 0) {
        output_data &= ~TDO;
    } else {
        output_data |= TDO;
    }
    setLevel(JTAGOUT, TDI, prev_TDI);
    clock(JTAGOUT, TCK);    // (1) FSM: Update-IR
    JTAGOUT &= ~TMS;
    clock(JTAGOUT, TCK);    // (0) FSM: IDLE

    // Confirm FSM is in IDLE state
    for (i = 0; i < 4; i++) {
        clock(JTAGOUT, TCK);
    }

    return output_data;
}

uint16_t DR_SHIFT(uint16_t input_data) {
    uint16_t output_data = 0;
    int prev_TDI = JTAGOUT & TDI;

    // Set FSM to Shift-IR state
    // FSM set through falling and rising edge
    JTAGOUT |= TMS;     // TMS high
    clock(JTAGOUT, TCK);    // (1) FSM: Select-DR
    JTAGOUT &= ~TMS;    // TMS low
    clock(JTAGOUT, TCK);     // (0) FSM: Capture-DR
    clock(JTAGOUT, TCK);     // (0) FSM: Shift-DR

    // Shift data into DR MSB first
    uint16_t bit;
    int i;
    for (i = 15; i > 0; i--) {
        bit = input_data >> i;
        bit &= 0x0001; // send the selected bit
        setLevel(JTAGOUT, TDI, bit);
        clock(JTAGOUT, TCK);
        uint8_t level = 1;
        if (JTAGOUT & TDO == 0) {
            level = 0;
        }
        output_data |= level << i;
    }

    // Send LSB and return to IDLE state
    bit = input_data;
    bit &= 0x0001;
    JTAGOUT |= TMS;
    setLevel(JTAGOUT, TDI, bit);
    clock(JTAGOUT, TCK);    // (1) FSM: Exit-DR

    if (JTAGOUT & TDO == 0) {
            output_data &= ~TDO;
        } else {
            output_data |= TDO;
        }

    setLevel(JTAGOUT, TDI, prev_TDI);
    clock(JTAGOUT, TCK);    // (1) FSM: Update-DR
    JTAGOUT &= ~TMS;
    clock(JTAGOUT, TCK);    // (0) FSM: IDLE

    // Confirm FSM is in IDLE state
    for (i = 0; i < 4; i++) {
        clock(JTAGOUT, TCK);
    }

    return output_data;
}
