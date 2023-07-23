/*
 * jtag_config.h
 *
 *  Created on: Jun 19, 2023
 *      Author: Jaden Baptista
 */

#ifndef JTAG_CONFIG_H_
#define JTAG_CONFIG_H_

#include <msp430.h>
#include <stdint.h>


/*
 * JTAG GPIO Pins
 *
 * Naming is as seen in the interface reference and viewed from the target POV.
 */
#define JTAGDIR     (P2DIR)     // port direction
#define JTAGIN      (P2IN)      // port input
#define JTAGOUT     (P2OUT)     // port output
#define RST         (0x01)      // Target MSP430 reset (0)
#define TMS         (0x04)      // JTAG FSM control    (2)
#define TCK         (0x08)      // JTAG clock input    (3)
#define TDI         (0x10)      // JTAG data input and TCLK input (4)
#define TDO         (0x20)      // JTAG data output (5)
#define TEST        (0x40)      // JTAG enable pins (6)

/*
 * JTAG Instruction Codes
 */
const uint8_t IR_ADDR_16BIT = 0x83;
const uint8_t IR_ADDR_CAPTURE = 0x84;
const uint8_t IR_DATA_TO_ADDR = 0x85;
const uint8_t IR_DATA_16BIT = 0x41;
const uint8_t IR_DATA_QUICK = 0x43;
const uint8_t IR_BYPASS = 0xFF;
const uint8_t IR_CNTRL_SIG_16BIT = 0x13;
const uint8_t IR_CNTRL_SIG_CAPTURE = 0x14;
const uint8_t IR_CNTRL_SIG_RELEASE = 0x15;
const uint8_t IR_DATA_PSA = 0x44;
const uint8_t IR_SHIFT_OUT_PSA = 0x46;
const uint8_t IR_Prepare_Blow = 0x22;
const uint8_t IR_Ex_Blow = 0x24;
const uint8_t IR_JMB_EXCHANGE = 0x61;

/*
 * Clocks port.pin.
 */
volatile inline void clock(int port, int pin) {
    port &= ~pin;
    port |= pin;
}

volatile inline void delay() {
    uint32_t timer = 0xFFFFFFFF;
    while (timer != 0) {
        timer--;
    }
}

volatile inline void slowClock(int port, int pin) {
    port &= ~pin;
    delay();
    port |= pin;
    delay();
}

/*
 * Sets port.pin high if value is true (1) and
 * low if value is false (0).
 */
volatile inline void setLevel(int port, int pin, int value) {
    if (value) {
        port |= pin;
    } else {
        port &= ~pin;
    }
}


#endif /* JTAG_CONFIG_H_ */
