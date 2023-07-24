/*
 * masks.h
 *
 *  Created on: Jul 5, 2023
 *      Author: Jaden Baptista
 */

#ifndef INCLUDE_MASKS_H_
#define INCLUDE_MASKS_H_

#include "types.h"

#define RRC_MASK  (0x1000)
#define SWPB_MASK (0x1080)
#define RRA_MASK  (0x1100)
#define SXT_MASK  (0x1180)
#define PUSH_MASK (0x1200)
#define CALL_MASK (0x1280)
#define RETI_MASK (0x1300)

#define RRC_TYPE  (SINGLE)
#define SWPB_TYPE (SINGLE)
#define RRA_TYPE  (SINGLE)
#define SXT_TYPE  (SINGLE)
#define PUSH_TYPE (SINGLE)
#define CALL_TYPE (SINGLE)
#define RETI_TYPE (SINGLE)

#define JNE_MASK  (0x2000)
#define JEQ_MASK  (0x2400)
#define JNC_MASK  (0x2800)
#define JC_MASK   (0x2C00)
#define JN_MASK   (0x3000)
#define JGE_MASK  (0x3400)
#define JL_MASK   (0x3800)
#define JMP_MASK  (0x3C00)

#define JNE_TYPE  (JUMP)
#define JEQ_TYPE  (JUMP)
#define JNC_TYPE  (JUMP)
#define JC_TYPE   (JUMP)
#define JN_TYPE   (JUMP)
#define JGE_TYPE  (JUMP)
#define JL_TYPE   (JUMP)
#define JMP_TYPE  (JUMP)

#define MOV_MASK  (0x4000)
#define ADD_MASK  (0x5000)
#define ADDC_MASK (0x6000)
#define SUBC_MASK (0x7000)
#define SUB_MASK  (0x8000)
#define CMP_MASK  (0x9000)
#define DADD_MASK (0xA000)
#define BIT_MASK  (0xB000)
#define BIC_MASK  (0xC000)
#define BIS_MASK  (0xD000)
#define XOR_MASK  (0xE000)
#define AND_MASK  (0xF000)

#define MOV_TYPE  (DOUBLE)
#define ADD_TYPE  (DOUBLE)
#define ADDC_TYPE (DOUBLE)
#define SUBC_TYPE (DOUBLE)
#define SUB_TYPE  (DOUBLE)
#define CMP_TYPE  (DOUBLE)
#define DADD_TYPE (DOUBLE)
#define BIT_TYPE  (DOUBLE)
#define BIC_TYPE  (DOUBLE)
#define BIS_TYPE  (DOUBLE)
#define XOR_TYPE  (DOUBLE)
#define AND_TYPE  (DOUBLE)


#endif /* INCLUDE_MASKS_H_ */
