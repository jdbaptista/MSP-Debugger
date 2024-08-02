/*
 * buttons.c
 *
 *  Created on: Aug 1, 2024
 *      Author: bapti
 */
#include <msp430.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/* The lowest 12 bits of the integer correspond
 * to various button debouncing flags. Specifically:
 * Bit 0: up_asm, Bit 1: down_asm, Bit 2: show_asm, Bit 3: jump_asm,
 * Bit 4: wait_up, Bit 5: wait_down, Bit 6: wait_show, Bit 7: wait_jump,
 * Bit 8: latch_up, Bit 9: latch_down, Bit 10: latch_show, Bit 11: latch_jump.
 */
static uint16_t button_bits;

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
 * Returns: True if out is set to true during this function,
 *          false otherwise. Should be used to wake up main.
 */
inline bool update_button(enum Button button) {
    if (button_latch_set(button)) {
        clr_button_latch(button);
        set_button_wait(button); // either already true, or initial latch
    } else if (button_wait_set(button)) {
        // no bounce has occurred
        set_button_cmd(button);
        clr_button_wait(button);
        return true;
    }
    return false;
}

inline void clr_buttons() {
    button_bits = 0;
}

bool button_cmd_set(enum Button button) {
    switch (button) {
    case UP_BTN:
        return button_bits & 0x0001;
    case DOWN_BTN:
        return button_bits & 0x0002;
    case SHOW_BTN:
        return button_bits & 0x0004;
    case JUMP_BTN:
        return button_bits & 0x0008;
    default:
        return false;
    }
}

void set_button_cmd(enum Button button) {
    switch (button) {
    case UP_BTN:
        button_bits |= 0x0001;
        break;
    case DOWN_BTN:
        button_bits |= 0x0002;
        break;
    case SHOW_BTN:
        button_bits |= 0x0004;
        break;
    case JUMP_BTN:
        button_bits |= 0x0008;
        break;
    default:
        break;
    }
}

void clr_button_cmd(enum Button button) {
    switch (button) {
    case UP_BTN:
        button_bits &= 0x0001;
        break;
    case DOWN_BTN:
        button_bits &= 0x0002;
        break;
    case SHOW_BTN:
        button_bits &= 0x0004;
        break;
    case JUMP_BTN:
        button_bits &= 0x0008;
        break;
    default:
        break;
    }
}

bool button_wait_set(enum Button button) {
    switch (button) {
    case UP_BTN:
        return button_bits & 0x0010;
    case DOWN_BTN:
        return button_bits & 0x0020;
    case SHOW_BTN:
        return button_bits & 0x0040;
    case JUMP_BTN:
        return button_bits & 0x0080;
    default:
        return false;
    }
}

void set_button_wait(enum Button button) {
    switch (button) {
    case UP_BTN:
        button_bits |= 0x0010;
        break;
    case DOWN_BTN:
        button_bits |= 0x0020;
        break;
    case SHOW_BTN:
        button_bits |= 0x0040;
        break;
    case JUMP_BTN:
        button_bits |= 0x0080;
        break;
    default:
        break;
    }
}

void clr_button_wait(enum Button button) {
    switch (button) {
    case UP_BTN:
        button_bits |= 0x0010;
        break;
    case DOWN_BTN:
        button_bits |= 0x0020;
        break;
    case SHOW_BTN:
        button_bits |= 0x0040;
        break;
    case JUMP_BTN:
        button_bits |= 0x0080;
        break;
    default:
        break;
    }
}

bool button_latch_set(enum Button button) {
    switch (button) {
    case UP_BTN:
        return button_bits & 0x0100;
    case DOWN_BTN:
        return button_bits & 0x0200;
    case SHOW_BTN:
        return button_bits & 0x0400;
    case JUMP_BTN:
        return button_bits & 0x0800;
    default:
        return false;
    }
}

void set_button_latch(enum Button button) {
    switch (button) {
    case UP_BTN:
        button_bits |= 0x0100;
        break;
    case DOWN_BTN:
        button_bits |= 0x0200;
        break;
    case SHOW_BTN:
        button_bits |= 0x0400;
        break;
    case JUMP_BTN:
        button_bits |= 0x0800;
        break;
    default:
        break;
    }
}

void clr_button_latch(enum Button button) {
    switch (button) {
    case UP_BTN:
        button_bits &= 0x0100;
        break;
    case DOWN_BTN:
        button_bits &= 0x0200;
        break;
    case SHOW_BTN:
        button_bits &= 0x0400;
        break;
    case JUMP_BTN:
        button_bits &= 0x0800;
        break;
    default:
        break;
    }
}


