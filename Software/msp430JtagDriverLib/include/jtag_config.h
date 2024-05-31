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
#define JTAGREN     (P2REN)     // port resistor enable
#define RST         (0x01)      // Target MSP430 reset (0) (16)
#define TMS         (0x04)      // JTAG FSM control    (2) (7)
#define TCK         (0x08)      // JTAG clock input    (3) (6)
#define TDI         (0x10)      // JTAG data input and TCLK input (4) (14)
#define TDO         (0x20)      // JTAG data output (5) (15)
#define TEST        (0x40)      // JTAG enable pins (6) (17)


#endif /* JTAG_CONFIG_H_ */
