/*
 * jtag_fsm.c
 *
 *  Created on: Jun 19, 2023
 *      Author: Jaden Baptista
 */

#include <msp430.h>
#include <stdlib.h>
#include <stdint.h>
#include "jtag_fsm.h"
#include "jtag_config.h"

volatile void delay() {
    volatile uint16_t timer = 0xF;
    while (timer != 0) {
        timer--;
    }
}

/*
 * Clocks port.pin.
 */
volatile inline void clock(volatile uint8_t* port, int pin) {
    *port &= ~pin;
    *port |= pin;
}


/*
 * Has a total period of 2 * 7.62microseconds, which satisfies
 * the fuse check time of 5microseconds.
 */
volatile inline void slowClock(volatile uint8_t* port, int pin) {
    *port &= ~pin;
    *port &= ~pin;
    *port |= pin;
    *port |= pin;

}

/*
 * Sets port.pin high if value is true (1) and
 * low if value is false (0).
 */
volatile void setLevel(volatile uint8_t* port, int pin, uint16_t value) {
    if (value) {
        *port |= pin;
    } else {
        *port &= ~pin;
    }
}

/*
 * Initializes the JTAG FSM to the IDLE state.
 *
 * Returns: 1 if FSM is put in the IDLE state successfully,
 *          0 if FSM failed fuse check or an error occurred.
 */
int initFSM() {
    // configure JTAG GPIO pins
    JTAGDIR = 0xFF;  // Begin with all pins set to output
    JTAGDIR &= ~TDO; // Set TDO to input
    JTAGOUT = 0x00;  // Set output pins low and resistors down
    JTAGREN = 0x00;
    JTAGREN |= TDO;  // Turn on TDO pulldown resistor

    // JTAG entry sequence: case 2b, Fig.2-13
    JTAGOUT |= TEST;
    clock(&JTAGOUT, TEST); // low->high
    JTAGOUT |= RST;

    // Reset FSM to IDLE
    JTAGOUT |= TMS;
    int i;
    for (i = 0; i < 7; i++) {
        clock(&JTAGOUT, TCK);    // FSM: TLR
    }
    JTAGOUT &= ~TCK;
    JTAGOUT &= ~TMS;
    JTAGOUT |= TDI;              // FSM: IDLE
    JTAGOUT |= TCK;

    // Perform fuse check
    // this needs a low phase of 5microseconds
    slowClock(&JTAGOUT, TMS);
    slowClock(&JTAGOUT, TMS);
    slowClock(&JTAGOUT, TMS);
    JTAGOUT &= ~TMS;

    return 1;
}

/*
 * Shifts an 8-bit JTAG instruction into the JTAG instruction register (IR).
 *
 * input_data: The JTAG instruction to be shifted into the IR.
 *
 * Returns: 8-bit JTAG ID (See pg.64 of interface reference).
 */
uint8_t IR_SHIFT(uint8_t input_data) {
    uint8_t output_data = 0;
    int prev_TDI = (JTAGOUT & TDI);

    // Set FSM to Shift-IR state
    // FSM set through falling and rising edge
    JTAGOUT |= TMS;     // TMS high
    clock(&JTAGOUT, TCK);    // (1) FSM: Select-DR
    clock(&JTAGOUT, TCK);    // (1) FSM: Select-IR
    JTAGOUT &= ~TMS;    // TMS low
    clock(&JTAGOUT, TCK);     // (0) FSM: Capture-IR
    clock(&JTAGOUT, TCK);     // (0) FSM: Shift-IR

    // Shift data into IR LSB first
    volatile uint8_t bit;
    volatile uint8_t sent = 0;
    int i;
    for (i = 0; i < 7; i++) {
        bit = (input_data >> i) & 1;
        setLevel(&JTAGOUT, TDI, bit);
        clock(&JTAGOUT, TCK);
        uint8_t level = 1;
        if ((JTAGIN & TDO) == 0) {
            level = 0;
        }
        output_data |= level << (7 - i);
        sent |= bit << i;
    }

    // Send MSB and return to IDLE state
    bit = (input_data >> 7) & 1;
    if (bit) {
        sent |= 0x80; // the compiler made me do it...
    }
    JTAGOUT |= TMS;
    setLevel(&JTAGOUT, TDI, bit);
    clock(&JTAGOUT, TCK);    // (1) FSM: Exit-IR
    if ((JTAGIN & TDO) == 0) {
        output_data &= ~1;
    } else {
        output_data |= 1;
    }
    setLevel(&JTAGOUT, TDI, prev_TDI);
    clock(&JTAGOUT, TCK);    // (1) FSM: Update-IR
    JTAGOUT &= ~TMS;
    clock(&JTAGOUT, TCK);    // (0) FSM: IDLE

    return output_data;
}

/*
 * Shifts a 16-bit word into a JTAG data register (DR).
 *
 * input_data: The data to be shifted into the addressed DR.
 *
 * Returns: Last captured and stored value in the addressed DR.
 */
uint16_t DR_SHIFT(uint16_t input_data) {
    uint16_t output_data = 0;
    int prev_TDI = JTAGOUT & TDI;

    // Set FSM to Shift-IR state
    // FSM set through falling and rising edge
    JTAGOUT |= TMS;     // TMS high
    clock(&JTAGOUT, TCK);    // (1) FSM: Select-DR
    JTAGOUT &= ~TMS;    // TMS low
    clock(&JTAGOUT, TCK);     // (0) FSM: Capture-DR
    clock(&JTAGOUT, TCK);     // (0) FSM: Shift-DR

    // Shift data into DR MSB first
    uint16_t bit;
    int i;
    for (i = 15; i > 0; i--) {
        bit = (input_data >> i) & 1;
        setLevel(&JTAGOUT, TDI, bit);
        clock(&JTAGOUT, TCK);
        volatile uint16_t level = 1;
          if ((JTAGIN & TDO) == 0) {
            level = 0;
        } else {
            level = 1;
        }
        output_data |= level << i;
    }

    // Send LSB and return to IDLE state
    bit = input_data & 1;
    JTAGOUT |= TMS;
    setLevel(&JTAGOUT, TDI, bit);
    clock(&JTAGOUT, TCK);    // (1) FSM: Exit-DR

    if ((JTAGIN & TDO) == 0) {
        output_data &= (uint16_t) ~1;
    } else {
        output_data |= (uint16_t) 1;
    }
    setLevel(&JTAGOUT, TDI, prev_TDI);
    clock(&JTAGOUT, TCK);    // (1) FSM: Update-DR
    JTAGOUT &= ~TMS;
    clock(&JTAGOUT, TCK);    // (0) FSM: IDLE

    return output_data;
}

/*
 * Sets TCLK to 1.
 */
inline void setTCLK() {
    JTAGOUT |= TCK;
}

/*
 * Sets TCLK to 0.
 */
inline void clrTCLK() {
    JTAGOUT &= ~TCK;
}
