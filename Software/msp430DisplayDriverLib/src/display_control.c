/*
 * display_control.c
 *
 * A hardware abstraction layer for interaction with the Newhaven
 * NHD-0440AZ-FL-YBW LCD Module, designed for use by a Texas
 * Instruments MSP430G2553.
 *
 * Definitions:
 *  - "Device Controller"
 *      The external application MPU sending signals to the
 *      LCD Module, distinct from the internal ST7066U controller.
 *  - "Display Datasheet"
 *      The Newhaven NHD-0440AZ-FL-YBW datasheet, Rev 8,
 *      https://newhavendisplay.com/content/specs/NHD-0440AZ-FL-YBW.pdf.
 *  - "Display Controller Datasheet"
 *      The Sitronix ST7066U datasheet, V2.2,
 *      https://newhavendisplay.com/content/app_notes/ST7066U.pdf.
 *
 *  Created on: Jun 20, 2024
 *      Author: Jaden Baptista
 */

#include "display_control.h"

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include "display_config.h"

/***
 * Switches device controller pins to the correct
 * configuration for the display driver.
 */
void use_display_pinout() {
    const uint8_t data_pins = DB4 + DB5 + DB6 + DB7;
    const uint8_t cntl_pins = RW + RS + EN1 + EN2;
    /* set data pins to output and clear data lines */
    DISPLAYDATAOUT &= ~data_pins;
    DISPLAYDATADIR |= data_pins;
    /* set data pins to GPIO function */
    DISPLAYDATASEL &= ~data_pins;
    DISPLAYDATASEL2 &= ~data_pins;
    /* set control pins to output and clear signals */
    DISPLAYCNTLOUT &= ~cntl_pins;
    DISPLAYCNTLDIR |= cntl_pins;
    DISPLAYCNTLSEL &= ~cntl_pins;
    DISPLAYCNTLSEL2 &= ~cntl_pins;
}

/***
 * Initialize the display to the following settings:
 *  - 4-bit interface
 *  - 2-line display mode (per enable pin)
 *  - display ON
 *  - cursor ON (for debugging)
 *  - cursor blink OFF
 *  - 5x8 dot format
 *
 *  See: Display Controller Datasheet, pg 25.
 */
void initDisplay(enum DisplayLine line) {
    /* set instruction code to function set with
     * DL = 1, necessary for startup */
    DISPLAYCNTLOUT &= ~(RW + RS);
    DISPLAYDATAOUT &= ~(DB7 + DB6);
    DISPLAYDATAOUT |= DB5 + DB4;
    clockEN(line);
    /* delay at least 37us (at 1MHz: 37 cycles) */
    volatile uint8_t cycles = 50;
    for (cycles; cycles != 0; cycles--) {}
    /* set instruction code to function set with
     * DL = 0 (4-bit interface), N = 1, F = 0 */
    DISPLAYCNTLOUT &= ~(RW + RS);
    DISPLAYDATAOUT &= ~(DB7 + DB6 + DB4);
    DISPLAYDATAOUT |= DB5;
    clockEN(line);
    DISPLAYDATAOUT &= ~(DB6 + DB5 + DB4);
    DISPLAYDATAOUT |= DB7;
    clockEN(line);
    /* delay at least 37us (at 1MHz: 37 cycles) */
    cycles = 50;
    for (cycles; cycles != 0; cycles--) {}
    /* confirm the same instruction,
     * following the diagram provided. */
    DISPLAYCNTLOUT &= ~(RW + RS);
    DISPLAYDATAOUT &= ~(DB7 + DB6 + DB4);
    DISPLAYDATAOUT |= DB5;
    clockEN(line);
    DISPLAYDATAOUT &= ~(DB6 + DB5 + DB4);
    DISPLAYDATAOUT |= DB7;
    clockEN(line);
    /* busy flag can now be checked and waited on */
    waitBusyFlagOff(line);
    /* display on/off control */
    DISPLAYDATAOUT &= ~(DB7 + DB6 + DB5 + DB4);
    clockEN(line);
    DISPLAYDATAOUT |= (DB7 + DB6 + DB5 + DB4); // D = 1, C = 1, B = 1
    clockEN(line);
    waitBusyFlagOff(line);
}



/***
 * Reads the busy flag. Sets data pins to input
 * with pulldown resistors during operation, then
 * back to output with low signals and disabled
 * internal resistors when complete.
 *
 * A device controller clock speed of maximum 1MHz
 * is required to satisfy delay requirements.
 *
 * See: Display Controller Datasheet, pgs 9,17,21,27.
 */
bool isDisplayBusy(enum DisplayLine line) {
    const uint8_t data_pins = DB4 + DB5 + DB6 + DB7;
    /* set data pins to input */
    DISPLAYDATAOUT &= ~data_pins; // use pulldown resistors
    DISPLAYDATAREN |= data_pins;
    DISPLAYDATADIR &= ~data_pins; // use gpio input
    /* set instruction code to read busy flag */
    DISPLAYCNTLOUT &= ~RS; // select command
    DISPLAYCNTLOUT |= RW; // select read
    /* delay at least 80us (at 1MHz: 80 cycles) */
    volatile uint8_t cycles = 100;
    for (cycles; cycles != 0; cycles--) {}
    /* read and return busy flag */
    bool busy = true;
    switch (line) {
    case TOP:
        DISPLAYCNTLOUT |= EN1;
        busy = DISPLAYDATAIN & DB7;
        DISPLAYCNTLOUT &= ~EN1;
        break;
    case BOTTOM:
        DISPLAYCNTLOUT |= EN2;
        busy = DISPLAYDATAIN & DB7;
        DISPLAYCNTLOUT &= ~EN2;
        break;
    default:
        break;
    }
    clockEN(line);
    /* return data pins to output */
    DISPLAYDATAOUT &= ~data_pins;
    DISPLAYDATAREN &= ~data_pins;
    DISPLAYDATADIR |= data_pins; // use gpio output
    return busy;
}

/***
 * Wait for the busy flag to turn off.
 */
inline void waitBusyFlagOff(enum DisplayLine line) {
    while (isDisplayBusy(line)) {}
}

/***
 * Reads the address counter (AC), which stores
 * DDRAM/CGRAM addresses.
 *
 * See: Display Controller Datasheet, pgs 9,17,21.
 */
uint8_t displayAddressCounter(enum DisplayLine line) {
    /* TODO: read and return address counter */
    return 0;
}

/***
 * Sends a full instruction to the display, assuming
 * it is set to the 4-bit interface mode.
 */
void sendInstruction(enum DisplayLine line, uint8_t data_lines) {
    setDataLines(data_lines >> 4);
    clockEN(line);
    setDataLines(data_lines);
    clockEN(line);
}

/***
 * Sets the data lines to the value specified
 * by the lower 4 bits of the input, where the
 * 0th bit corresponds to DB4 and the 3rd bit
 * corresponds to DB7.
 */
void setDataLines(uint8_t data) {
    if (data & BIT3) {
        DISPLAYDATAOUT |= DB7;
    } else {
        DISPLAYDATAOUT &= ~DB7;
    }
    if (data & BIT2) {
        DISPLAYDATAOUT |= DB6;
    } else {
        DISPLAYDATAOUT &= ~DB6;
    }
    if (data & BIT1) {
        DISPLAYDATAOUT |= DB5;
    } else {
        DISPLAYDATAOUT &= ~DB5;
    }
    if (data & BIT0) {
        DISPLAYDATAOUT |= DB4;
    } else {
        DISPLAYDATAOUT &= ~DB4;
    }
}

/***
 * Clocks the line operation enable bit, ENx,
 * high then low. A falling edge executes a latch
 * from the data lines, RS, and R/W.
 *
 * A device controller clock speed of below 5.2MHz
 * is required to satisfy display timing requirements.
 *
 * See: Display Datasheet, pgs 4,7,8.
 */
void clockEN(enum DisplayLine line) {
    switch (line) {
    case TOP:
        DISPLAYCNTLOUT |= EN1;
        DISPLAYCNTLOUT &= ~EN1;
        break;
    case BOTTOM:
        DISPLAYCNTLOUT |= EN2;
        DISPLAYCNTLOUT &= ~EN2;
        break;
    default:
        break;
    }
}


