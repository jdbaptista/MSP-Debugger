#include <msp430.h>
#include <jtag_fsm.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    volatile uint8_t output8;
    volatile uint16_t output16;
    while (true) {
        initFSM();
        output8 = IR_SHIFT((uint8_t) 0x55);
        output16 = DR_SHIFT((uint16_t) 0xdead);
        output16 = DR_SHIFT((uint16_t) 0xbeef);
        output16 = DR_SHIFT((uint16_t) 0x0000);
    }
}
