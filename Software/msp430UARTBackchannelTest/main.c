#include <bc_uart.h>
#include <msp430.h>
#include <stdbool.h>



/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    // setup necessary for uart at 9600 baud
    usci_reset();

    // source UCSI from SMCLK
    BCCTL1 |= UCSSEL__SMCLK;

    use_bc_uart_pins();

    uart_config();

    usci_start();

    enable_uart_tx_interrupt();
    clear_uart_tx_interrupt_flag();
    __bis_SR_register(GIE);

    uart_send_string("Hello, world!");
    return 0;
}
