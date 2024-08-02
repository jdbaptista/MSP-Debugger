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

bool updateButton(enum Button button);
void clrButtons();
bool isButtonCmdSet(enum Button button);
void setButtonCmd(enum Button button);
void clrButtonCmd(enum Button button);
bool isButtonWaitSet(enum Button button);
void setButtonWait(enum Button button);
void clrButtonWait(enum Button button);
bool isButtonLatchSet(enum Button button);
void setButtonLatch(enum Button button);
void clrButtonLatch(enum Button button);

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
#define BUTTONS (HEX_BTN_IFG + JMP_BTN_IFG + UP_BTN_IFG + DOWN_BTN_IFG)

#endif /* SRC_BUTTONS_H_ */
