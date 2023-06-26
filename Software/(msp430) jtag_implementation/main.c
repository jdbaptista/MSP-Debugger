#include <msp430.h>
#include <jtag_fsm.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	initFSM();
	uint8_t output;
	while (true) {
	    output = IR_SHIFT(0x55);
	    printf("output: %d/n", output);
	}
}
