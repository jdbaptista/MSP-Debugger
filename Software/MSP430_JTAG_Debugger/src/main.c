#include <msp430.h>
#include <jtag_fsm.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "bc_uart.h"
#include "jtag_fsm.h"
#include "jtag_control.h"
#include "disassembler.h"

#define BTN_VECT (PORT2_VECTOR)
#define BTN_IN   (P2IN)
#define BTN_OUT  (P2OUT)
#define BTN_DIR  (P2DIR)
#define BTN_IFG  (P2IFG)
#define BTN_IES  (P2IES)
#define BTN_IE   (P2IE)
#define BTN_SEL  (P2SEL)
#define BTN_SEL2 (P2SEL2)
#define BTN_REN  (P2REN)
#define HEX_BTN  (BIT4)
#define JMP_BTN  (BIT5)
#define UP_BTN   (BIT6)
#define DOWN_BTN (BIT7)
#define BUTTONS (HEX_BTN + JMP_BTN + UP_BTN + DOWN_BTN)

static bool update = false;
static bool up_asm = false;
static bool down_asm = false;
static bool show_asm = true;
static bool jump_asm = false;

#pragma vector=BTN_VECT
interrupt void button_irq(void) {
    if (BTN_IFG & HEX_BTN) {
        show_asm = (show_asm) ? false : true;
        update = true;
        BTN_IFG &= ~HEX_BTN;
        return;
    }
    if (BTN_IFG & JMP_BTN) {
        jump_asm = true;
        update = true;
        BTN_IFG &= ~JMP_BTN; // dont care about up and down
        return;
    }
    if (BTN_IFG & DOWN_BTN) { // actually goes up in memory, down in code
        down_asm = true;
        update = true;
        BTN_IFG &= ~DOWN_BTN;
        return;
    }
    if (BTN_IFG & UP_BTN) { // actually goes down in memory, up in code
        up_asm = true;
        update = true;
        BTN_IFG &= ~UP_BTN;
        return;
    }
    // default case
    BTN_IFG &= ~BUTTONS;
    return;
}

inline void initButtons() {
    BTN_SEL &= ~BUTTONS;
    BTN_SEL2 &= ~BUTTONS;
    BTN_OUT &= ~BUTTONS;
    BTN_IN &= ~BUTTONS;
    BTN_REN |= BUTTONS;
    BTN_DIR &= ~BUTTONS;
    BTN_IES &= ~BUTTONS;
    BTN_IFG &= ~BUTTONS;
    BTN_IE |= BUTTONS;
}

inline void initBackchannel() {
    // setup backchannel at 9600 baud, 2 stop bits
    usci_reset();
    BCCTL1 |= UCSSEL_3; // set uart to use SMCLK
    use_bc_uart_pins();
    uart_config();
    usci_start();
    enable_uart_tx_interrupt();
    clear_uart_tx_interrupt_flag();
    __bis_SR_register(GIE);
    wait_print("\033[2J"); // clear screen command
    wait_print("\033[H"); // home cursor command
}

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
    uint16_t curr_addr = 0xC000;
    uint16_t next_addr = curr_addr;

    uint16_t bytes[3];
    int numBytes = 0;
    char assembly[31];
    initButtons();
    initBackchannel();
    initFSM();
    getDevice();
    haltCPU();

    bytes[0] = readMem(curr_addr);
    bytes[1] = readMem(curr_addr + 2);
    bytes[2] = readMem(curr_addr + 4);
    numBytes = nextInstruction(assembly, curr_addr, bytes, &next_addr);

    update = true;
    while (true) {
        uart_wait();
        while (!update) {}

        wait_print("\033[2J"); // clear screen command
        wait_print("\033[H"); // home cursor command

        if (jump_asm) {
            // todo: implement logic
            jump_asm = false;
        }
        if (up_asm) {
            // todo: implement logic
            up_asm = false;
        }
        if (down_asm) {
            curr_addr = next_addr;
            bytes[0] = readMem(curr_addr);
            bytes[1] = readMem(curr_addr + 2);
            bytes[2] = readMem(curr_addr + 4);
            numBytes = nextInstruction(assembly, curr_addr, bytes, &next_addr);
            if (numBytes < 0) {
                numBytes = 0;
            }
            down_asm = false;
        }

        wait_print_hex(curr_addr);
        wait_print(": ");
        if (show_asm) {
            wait_print(assembly);
        } else {
            unsigned int i;
            for (i = 0; i < numBytes; i++) {
                wait_print_hex(bytes[i]);
                wait_print(" ");
            }
        }
        update = false;
    }

    releaseCPU();
    releaseDevice();
    releaseFSM();
}
