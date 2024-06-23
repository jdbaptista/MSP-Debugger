/*
 * display_config.h
 *
 *  Created on: Jun 20, 2024
 *      Author: bapti
 */

#ifndef INCLUDE_DISPLAY_CONFIG_H_
#define INCLUDE_DISPLAY_CONFIG_H_

#ifdef __MSP430G2553__
#define DISPLAYDATADIR      (P1DIR)     // data port direction
#define DISPLAYDATASEL      (P1SEL)     // display port select 1
#define DISPLAYDATASEL2     (P1SEL2)    // display port select 2
#define DISPLAYDATAIN       (P1IN)      // data port input
#define DISPLAYDATAOUT      (P1OUT)     // data port output
#define DISPLAYDATAREN      (P1REN)     // data port register enable
#define DB4                 (BIT4)      // data bit 4 pin
#define DB5                 (BIT5)      // data bit 5 pin
#define DB6                 (BIT6)      // data bit 6 pin
#define DB7                 (BIT7)      // data bit 7 pin
#define DISPLAYCNTLDIR      (P2DIR)     // control port direction
#define DISPLAYCNTLIN       (P2IN)      // control port input
#define DISPLAYCNTLOUT      (P2OUT)     // control port output
#define DISPLAYCNTLREN      (P2REN)     // control port register enable
#define DISPLAYCNTLSEL      (P2SEL)     // control port select 1
#define DISPLAYCNTLSEL2     (P2SEL2)    // control port select 2
#define RW                  (BIT0)      // select read or write. 0: write, 1: read
#define EN1                 (BIT1)      // top lines operation enable, falling edge triggered
#define EN2                 (BIT2)      // bottom lines operation enable, falling edge triggered
#define RS                  (BIT3)      // register select. 0: command, 1: data
#endif

#endif /* INCLUDE_DISPLAY_CONFIG_H_ */
