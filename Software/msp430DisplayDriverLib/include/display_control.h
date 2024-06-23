/*
 * display_control.h
 *
 *  Created on: Jun 20, 2024
 *      Author: bapti
 */

#ifndef INCLUDE_DISPLAY_CONTROL_H_
#define INCLUDE_DISPLAY_CONTROL_H_

#include <stdint.h>
#include <stdbool.h>

enum DisplayLine {
    TOP,
    BOTTOM,
};

void use_display_pinout();
void initDisplay(enum DisplayLine line);
bool isDisplayBusy(enum DisplayLine line);
inline void waitBusyFlagOff(enum DisplayLine line);
void sendInstruction(enum DisplayLine line, uint8_t data_lines);
void setDataLines(uint8_t data);
uint8_t displayAddressCounter(enum DisplayLine line);
void clockEN(enum DisplayLine line);

#endif /* INCLUDE_DISPLAY_CONTROL_H_ */
