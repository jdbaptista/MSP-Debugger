/*
 * buttons.h
 *
 *  Created on: Aug 1, 2024
 *      Author: bapti
 */

#ifndef SRC_BUTTONS_H_
#define SRC_BUTTONS_H_

enum Button {
    UP_BTN,
    DOWN_BTN,
    SHOW_BTN,
    JUMP_BTN,
};

inline void clr_buttons();
bool button_cmd_set(enum Button button);
void set_button_cmd(enum Button button);
void clr_button_cmd(enum Button button);
bool button_wait_set(enum Button button);
void set_button_wait(enum Button button);
void clr_button_wait(enum Button button);
bool buttion_latch_set(enum Button button);
void set_button_latch(enum Button button);
void clr_button_latch(enum Button button);

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
#define HEX_BTN_IFG  (BIT4)
#define JMP_BTN_IFG  (BIT5)
#define UP_BTN_IFG   (BIT6)
#define DOWN_BTN_IFG (BIT7)
#define BUTTONS (HEX_BTN + JMP_BTN + UP_BTN + DOWN_BTN)

#endif /* SRC_BUTTONS_H_ */
