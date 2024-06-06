#include <msp430.h> 
#include "bc_uart.h"

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	BCCTL1 |= UCSSEL_3; // source UCSI from SMCLK
	use_bc_uart_pins();
	uart_config();
	usci_start();
	enable_uart_tx_interrupt();
	clear_uart_tx_interrupt_flag();
	__bis_SR_register(GIE);
	wait_print("Hello, World!");
	
	return 0;
}
