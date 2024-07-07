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

static bool wait_up = false;
static bool wait_down = false;
static bool wait_show = false;
static bool wait_jump = false;

// These are necessary for the timer to
// know when to begin waiting. If latch
// is true while wait is true, then the
// switch was bouncing. Otherwise, if
// wait is true and latch is false, then
// the button was successfully pressed and
// the signal is sent to the main routine.
static bool latch_up = false;
static bool latch_down = false;
static bool latch_show = false;
static bool latch_jump = false;

#pragma vector=BTN_VECT
interrupt void button_irq(void) {
    if (BTN_IFG & HEX_BTN) {
        latch_show = true;
        BTN_IFG &= ~HEX_BTN;
        return;
    }
    if (BTN_IFG & JMP_BTN) {
        latch_jump = true;
        BTN_IFG &= ~JMP_BTN;
        return;
    }
    if (BTN_IFG & DOWN_BTN) {
        latch_down = true;
        BTN_IFG &= ~DOWN_BTN;
        return;
    }
    if (BTN_IFG & UP_BTN) {
        latch_up = true;
        BTN_IFG &= ~UP_BTN;
        return;
    }
    // default case
    BTN_IFG &= ~BUTTONS;
    return;
}

/***
 * Performs button debouncing logic. Meant to be used inside
 * the timer interrupt handler. The system works as follows:
 * 1. GPIO button interrupt occurs and latch is now true.
 * 2. Timer interrupt occurs and wait is now true, latch false.
 * 3. Bounce GPIO button interrupt occurs, and latch is true.
 * 4. Timer interrupt occurs, both wait and latch are true, so
 *    a bounce must have occurred and latch is set to false.
 * 5. Timer interrupt occurs, wait is true and latch is false,
 *    so a button steady state has been reached and out is set.
 *
 * Returns: True if out is set, false otherwise. Should be used
 *          to set update true so that main continues.
 */
inline bool update_button(bool *latch, bool *wait, bool *out) {
    if (*latch) {
        *latch = false;
        *wait = true; // either already true, or initial latch
    } else if (*wait) {
        // no bounce has occurred
        *out = true;
        *wait = false;
        return true;
    }
    return false;
}

#pragma vector=TIMER0_A1_VECTOR
interrupt void timer_debounce_irq(void) {
    if (!(TA0IV & TA0IV_TAIFG)) {
        return; // automatically removes highest ifg
    }
    update = update_button(&latch_up, &wait_up, &up_asm);
    update |= update_button(&latch_down, &wait_down, &down_asm);
    update |= update_button(&latch_jump, &wait_jump, &jump_asm);
    if (latch_show) {
        latch_show = false;
        wait_show = true; // either already true, or initial latch
    } else if (wait_show) {
        show_asm = (show_asm) ? false : true;
        wait_show = false;
        update = true;
    }
    TA0IV = 0;
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

    // setup timer A0 for debouncing
    TA0CTL &= ~TASSEL0; // select ACLK source
    TA0CTL |= TASSEL1;
    TA0CTL |= ID0; // select input divider 2
    TA0CTL &= ~ID1;
    TA0CTL &= ~MC0; // select continuous mode
    TA0CTL |= MC1;
    TA0CCTL0 &= ~(CM0 + CM1); // disable CC interrupts
    TA0CCTL1 &= ~(CM0 + CM1);
    TA0CCTL2 &= ~(CM0 + CM1);
    TA0CTL &= ~TAIFG;
    TA0CTL |= TAIE; // enable timer overflow interrupt
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
    char assembly2[31];
    char assembly3[31];
    char assembly4[31];
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
        while (!update) {} // TODO: implement sleep mode

        wait_print("\033[2J"); // clear screen command
        wait_print("\033[H"); // home cursor command

        if (jump_asm) {
            curr_addr = getJumpLocation(bytes[0], curr_addr);
            bytes[0] = readMem(curr_addr);
            bytes[1] = readMem(curr_addr + 2);
            bytes[2] = readMem(curr_addr + 4);
            numBytes = nextInstruction(assembly, curr_addr, bytes, &next_addr);
            if (numBytes < 0) {
                numBytes = 0;
            }
            jump_asm = false;
        }
        if (up_asm) {
            // iterate from beginning to find previous instruction
            uint16_t prev_addr = 0xC000;
            uint16_t temp_addr = 0;
            while (prev_addr < curr_addr) {
                temp_addr = nextInstrAddr(prev_addr, readMem(prev_addr));
                if (temp_addr == 0) {
                    // an error occurred, don't execute operation
                    prev_addr = 0;
                    break;
                }
                if (temp_addr == curr_addr) {
                    // prev_addr is the correct previous instruction
                    curr_addr = prev_addr;
                    break;
                }
                prev_addr = temp_addr;
            }
            if (prev_addr != 0) {
                bytes[0] = readMem(curr_addr);
                bytes[1] = readMem(curr_addr + 2);
                bytes[2] = readMem(curr_addr + 4);
                numBytes = nextInstruction(assembly, curr_addr, bytes, &next_addr);
            }

            up_asm = false;
        }
        if (down_asm) {
            uart_wait(); // for debugging
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
