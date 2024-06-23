#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include "display_control.h"


/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	/* delay for 40ms for display power up */
	volatile uint16_t counter = 0xFFFF;
	for (counter; counter != 0; counter--) {}
	/* initialize display */
	use_display_pinout();
	initDisplay(TOP);
	initDisplay(BOTTOM);

	return 0;
}
