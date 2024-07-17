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
}

void handleJump(uint16_t *curr_addr) {
    uint16_t jump_addr, operator, src_bytes;
    opCode opcode;

    operator = readMem(*curr_addr);
    opcode = getOpCode(operator);
    jump_addr = 0;
    if (opcode.format == JUMP) {
        jump_addr = getJumpLocation(operator, *curr_addr);
    } else if (isCall(&opcode)) {
        src_bytes = readMem(*curr_addr + 2);
        jump_addr = getCallLocation(operator, src_bytes, *curr_addr);
    }
    if (jump_addr != 0 && jump_addr >= 0xC000 && jump_addr < 0xFFFF) {
        *curr_addr = jump_addr;
    }
}

void handleUp(uint16_t *curr_addr) {
    Instruction instr;
    uint16_t prev_addr;

    instr.address = 0xC000;
    while (instr.address < *curr_addr) {
        instr.operator = readMem(instr.address);
        instr.source = readMem(instr.address + 2);
        instr.destination = readMem(instr.address + 4);
        prev_addr = instr.address;
        nextAddress(&(instr.address), &instr);
    }
    *curr_addr = prev_addr;
}

void handleDown(uint16_t *curr_addr) {
    Instruction instr;

    instr.address = *curr_addr;
    instr.operator = readMem(instr.address);
    instr.source = readMem(instr.address + 2);
    instr.destination = readMem(instr.address + 4);
    nextAddress(curr_addr, &instr);
}

void displayAsm(uint16_t addr) {
    Instruction instr;
    char buffer[31];
    uint16_t i;

    instr.address = addr;
    for (i = 4; i > 0; i--) {
        instr.operator = readMem(instr.address);
        instr.source = readMem(instr.address + 2);
        instr.destination = readMem(instr.address + 4);
        wait_print_hex(instr.address);
        wait_print(": ");
        getInstruction(buffer, &instr);
        wait_print(buffer);
        if (i == 4) {
            wait_print(" <");
        }
        wait_print("\033[E"); // newline command
        nextAddress(&(instr.address), &instr);
    }
}

void displayBin(uint16_t addr) {
    int encodingLength;
    Instruction instr;
    uint16_t i;

    instr.address = addr;
    for (i = 4; i > 0; i--) {
        instr.operator = readMem(instr.address);
        instr.source = readMem(instr.address + 2);
        instr.destination = readMem(instr.address + 4);
        wait_print_hex(instr.address);
        wait_print(": ");
        encodingLength = nextAddress(NULL, &instr);
        if (encodingLength == -1) {
            encodingLength = 1; // next address internally moves 1 word upon error
        }
        if (encodingLength >= 1) {
            wait_print_hex(instr.operator);
            wait_print(" ");
        }
        if (encodingLength >= 2) {
            wait_print_hex(instr.source);
            wait_print(" ");
        }
        if (encodingLength >= 3) {
            wait_print_hex(instr.destination);
            wait_print(" ");
        }
        if (i == 4) {
            wait_print("<");
        }
        wait_print("\033[E"); // newline command
        nextAddress(&(instr.address), &instr);
    }
}

/**
 * main.c
 */
int main(void)
{
    uint16_t curr_addr;

    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

    initButtons();
    initBackchannel();

    __bis_SR_register(GIE);
    wait_print("\033[2J"); // clear screen command
    wait_print("\033[H"); // home cursor command

    initFSM();
    getDevice();
    haltCPU();

    curr_addr = 0xC000; // TODO: replace with address from reset vector
    update = true;
    while (true) {
        while (!update) {} // TODO: implement sleep mode
        if (jump_asm) {
            handleJump(&curr_addr);
            jump_asm = false;
        }
        if (up_asm) {
            handleUp(&curr_addr);
            up_asm = false;
        }
        if (down_asm) {
            handleDown(&curr_addr);
            down_asm = false;
        }
        // display current instruction state
        wait_print("\033[2J"); // clear screen command
        wait_print("\033[H"); // home cursor command
        if (show_asm) {
            displayAsm(curr_addr);
        } else {
            displayBin(curr_addr);
        }
        update = false;
    }
}
