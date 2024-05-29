/*
 * backchannel_config.h
 *
 *  Created on: Jan 1, 2024
 *      Author: Jaden Baptista
 */

#ifndef INCLUDE_BACKCHANNEL_CONFIG_H_
#define INCLUDE_BACKCHANNEL_CONFIG_H_

#include <msp430.h>


#define BC_INT_VEC USCI_A1_VECTOR
#define BCCTL0 UCA1CTL0
#define BCCTL1 UCA1CTL1
#define BCBR0 UCA1BR0
#define BCBR1 UCA1BR1
#define BCMCTL UCA1MCTL
#define BCSTAT UCA1STAT
#define BCRXBUF UCA1RXBUF
#define BCTXBUF UCA1TXBUF
#define BCABCTL UCA1ABCTL
#define BCIRTCTL UCA1IRTCTL
#define BCIRRCTL UCA1IRRCTL
#define BCIE UCA1IE
#define BCIFG UCA1IFG
#define BCIV UCA1IV


#endif /* INCLUDE_BACKCHANNEL_CONFIG_H_ */
