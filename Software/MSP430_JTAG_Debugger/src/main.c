#include <msp430.h>
#include <jtag_fsm.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "bc_uart.h"
#include "jtag_fsm.h"
#include "jtag_control.h"
#include "disassembler.h"
#include "buttons.h"

#pragma vector=BTN_VECT
interrupt void buttonIRQ(void) {
    __bic_SR_register(GIE);
    __bis_SR_register_on_exit(GIE);
    if (BTN_IFG & HEX_BTN_IFG) {
        setButtonLatch(SHOW_BTN);
        BTN_IFG &= ~HEX_BTN_IFG;
    } else if (BTN_IFG & JMP_BTN_IFG) {
        setButtonLatch(JUMP_BTN);
        BTN_IFG &= ~JMP_BTN_IFG;
    } else if (BTN_IFG & DOWN_BTN_IFG) {
        setButtonLatch(DOWN_BTN);
        BTN_IFG &= ~DOWN_BTN_IFG;
    } else if (BTN_IFG & UP_BTN_IFG) {
        setButtonLatch(UP_BTN);
        BTN_IFG &= ~UP_BTN_IFG;
    } else {
        BTN_IFG &= ~BUTTONS;
    }
    return;
}

#pragma vector=TIMER0_A1_VECTOR
interrupt void timerDebounceIRQ(void) {
    __bic_SR_register(GIE);
    __bis_SR_register_on_exit(GIE);
    bool update = false;

    if (TA0IV & TA0IV_TAIFG != TA0IV_TAIFG) {
        return; // automatically removes highest ifg
    }
    update = updateButton(JUMP_BTN);
    update |= updateButton(UP_BTN);
    update |= updateButton(DOWN_BTN);
    // update latch button with a special cmd
    if (isButtonLatchSet(SHOW_BTN)) {
        clrButtonLatch(SHOW_BTN);
        setButtonWait(SHOW_BTN);
    } else if (isButtonWaitSet(SHOW_BTN)) {
        // XOR (toggle) the show button
        if (isButtonCmdSet(SHOW_BTN)) {
            clrButtonCmd(SHOW_BTN);
        } else {
            setButtonCmd(SHOW_BTN);
        }
        clrButtonWait(SHOW_BTN);
        update = true;
    }
    // return to Active Mode if an update should occur
    if (update) {
        __bic_SR_register_on_exit(SCG0 | SCG1 | CPUOFF);
    }
    TA0IV = 0;
}

inline void initButtons() {
    // clear button debouncing logic flags
    clrButtons();

    // set GPIO pin registers for buttons
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
    BCSCTL3 &= ~BIT4; // source ACLK from VLOCLK (12kHz)
    BCSCTL3 |= BIT5;
    TA0CTL &= ~TASSEL1; // select ACLK source
    TA0CTL |= TASSEL0;
    TA0CTL |= ID0; // select input divider 2
    TA0CTL &= ~ID1;
    TACCR0 = 0x02FF; // set the value to count up to
                     // manually calibrated to remove debouncing
    TA0CTL &= ~MC1; // select up mode
    TA0CTL |= MC0;
    TA0CCTL0 &= ~(CM0 + CM1); // disable CC interrupts
    TA0CCTL1 &= ~(CM0 + CM1);
    TA0CCTL2 &= ~(CM0 + CM1);
    TA0CTL &= ~TAIFG;
    TA0CTL |= TAIE; // enable timer overflow interrupt
}

inline void initBackchannel() {
    // setup backchannel at 9600 baud, 2 stop bits
    usciReset();
    BCCTL1 |= UCSSEL_3; // set uart to use SMCLK
    useBCUartPins();
    uartConfig();
    usciStart();
    enableUartTXInterrupt();
    clearUartTXInterruptFlag();
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

void displayAsm(uint16_t curr_addr) {
    Instruction instr;
    char buffer[31];
    uint16_t i;

    instr.address = curr_addr;
    for (i = 4; i > 0; i--) {
        instr.operator = readMem(instr.address);
        instr.source = readMem(instr.address + 2);
        instr.destination = readMem(instr.address + 4);
        waitPrintHex(instr.address);
        waitPrint(": ");
        getInstruction(buffer, &instr);
        waitPrint(buffer);
        if (i == 4) {
            waitPrint(" <");
        }
        waitPrint("\033[E"); // newline command
        nextAddress(&(instr.address), &instr);
    }
}

void displayBin(uint16_t curr_addr) {
    int encodingLength;
    Instruction instr;
    uint16_t i;

    instr.address = curr_addr;
    for (i = 4; i > 0; i--) {
        instr.operator = readMem(instr.address);
        instr.source = readMem(instr.address + 2);
        instr.destination = readMem(instr.address + 4);
        waitPrintHex(instr.address);
        waitPrint(": ");
        encodingLength = nextAddress(NULL, &instr);
        if (encodingLength == -1) {
            encodingLength = 1; // next address internally moves 1 word upon error
        }
        if (encodingLength >= 1) {
            waitPrintHex(instr.operator);
            waitPrint(" ");
            if (encodingLength >= 2) {
                waitPrintHex(instr.source);
                waitPrint(" ");
                if (encodingLength >= 3) {
                    waitPrintHex(instr.destination);
                    waitPrint(" ");
                }
            }
        }
        if (i == 4) {
            waitPrint("<");
        }
        waitPrint("\033[E"); // newline command
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
    setButtonCmd(SHOW_BTN); // indicates showing assembly
    initBackchannel();

    __bis_SR_register(GIE);
    waitPrint("\033[2J"); // clear screen command
    waitPrint("\033[H"); // home cursor command

    // take target under JTAG control
    initFSM();
    getDevice();
    haltCPU();

    curr_addr = 0xC000;
    while (true) {
        if (isButtonCmdSet(JUMP_BTN)) {
            handleJump(&curr_addr);
            clrButtonCmd(JUMP_BTN);
        }
        if (isButtonCmdSet(UP_BTN)) {
            handleUp(&curr_addr);
            clrButtonCmd(UP_BTN);
        }
        if (isButtonCmdSet(DOWN_BTN)) {
            handleDown(&curr_addr);
            clrButtonCmd(DOWN_BTN);
        }
        // display current instruction state
        waitPrint("\033[2J"); // clear screen command
        waitPrint("\033[H"); // home cursor command
        if (isButtonCmdSet(SHOW_BTN)) {
            displayAsm(curr_addr);
        } else {
            displayBin(curr_addr);
        }

        if (isButtonCmdSet(JUMP_BTN) || isButtonCmdSet(UP_BTN) || isButtonCmdSet(DOWN_BTN)) {
            continue; // missing an interrupt, update again
        }

        // go to sleep until woken from timer interrupt
        waitUart(); // finish sending uart data
        __bis_SR_register(GIE);
        __bis_SR_register(SCG0 | SCG1 | CPUOFF); // LPM3 until GPIO interrupt
    }
}
