/*
 * jtag_control.h
 *
 *  Created on: Sep 21, 2023
 *      Author: bapti
 */

#ifndef INCLUDE_JTAG_CONTROL_H_
#define INCLUDE_JTAG_CONTROL_H_

#include <stdint.h>

void getDevice();
bool setInstrFetch();
void setPC(uint16_t address);
void haltCPU();
void releaseCPU();
void executePOR();
void releaseDevice();
uint16_t readMem(uint16_t address);
uint16_t writeMem(uint16_t address, uint16_t data);


#endif /* INCLUDE_JTAG_CONTROL_H_ */
