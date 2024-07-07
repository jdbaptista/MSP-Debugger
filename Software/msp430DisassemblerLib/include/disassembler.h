/*
 * disassembler.h
 *
 *  Created on: Jul 5, 2023
 *      Author: Jaden Baptista
 */

#ifndef INCLUDE_DISASSEMBLER_H_
#define INCLUDE_DISASSEMBLER_H_

#include <stdint.h>
#include "types.h"
#include "masks.h"

struct Instruction {
    /* The address where this instruction is located in target flash */
    uint16_t address;
    /* The word that encode the type of operation and registers */
    uint16_t operator;
    /* The word directly after the operator in memory,
     * potentially encoding the source operand offset */
    uint16_t source;
    /* The word after the source operator in memory,
     * potentially encoding the destination offset */
    uint16_t destination;
};

typedef struct Instruction Instruction;

int nextAddress(uint16_t *next_addr, Instruction *instr);
bool getInstruction(char *buffer, Instruction *instr);
bool appendOperand(char *result, uint16_t pc, uint16_t reg, uint16_t word, addressingMode mode);
int searchEmulated(char *result, opCode *op, uint16_t start_addr, uint16_t *byte_code);
opCode getOpCode(uint16_t byteCode);
addressingMode getDestRegisterMode(uint16_t byteCode);
addressingMode getSourceRegisterMode(uint16_t byteCode, formatType type);
uint16_t getDestRegister(uint16_t byteCode);
uint16_t getSourceRegister(uint16_t byteCode, formatType type);
inline bool isByteOperation(uint16_t byteCode);
uint16_t getJumpLocation(uint16_t byteCode, uint16_t currAddress);
void byteCodeToOffset(char* result, uint16_t byteCode);
void parseRegisterNum(char* result, uint16_t regNum);
void uintToHex(char* result, uint16_t input);

// order matters!
static const opCode CODES[] = {
       {AND_MASK, AND_TYPE, "AND"},
       {XOR_MASK, XOR_TYPE, "XOR"},
       {BIS_MASK, BIS_TYPE, "BIS"},
       {BIC_MASK, BIC_TYPE, "BIC"},
       {BIT_MASK, BIT_TYPE, "BIT"},
       {DADD_MASK, DADD_TYPE, "DADD"},
       {CMP_MASK, CMP_TYPE, "CMP"},
       {SUB_MASK, SUB_TYPE, "SUB"},
       {SUBC_MASK, SUBC_TYPE, "SUBC"},
       {ADDC_MASK, ADDC_TYPE, "ADDC"},
       {ADD_MASK, ADD_TYPE, "ADD"},
       {MOV_MASK, MOV_TYPE, "MOV"},
       {JMP_MASK, JMP_TYPE, "JMP"},
       {JL_MASK, JL_TYPE, "JL"},
       {JGE_MASK, JGE_TYPE, "JGE"},
       {JN_MASK, JN_TYPE, "JN"},
       {JC_MASK, JC_TYPE, "JC"},
       {JNC_MASK, JNC_TYPE, "JNC"},
       {JEQ_MASK, JEQ_TYPE, "JEQ"},
       {JNE_MASK, JNE_TYPE, "JNE"},
       {RETI_MASK, RETI_TYPE, "RETI"},
       {CALL_MASK, CALL_TYPE, "CALL"},
       {PUSH_MASK, PUSH_TYPE, "PUSH"},
       {SXT_MASK, SXT_TYPE, "SXT"},
       {RRA_MASK, RRA_TYPE, "RRA"},
       {SWPB_MASK, SWPB_TYPE, "SWPB"},
       {RRC_MASK, RRC_TYPE, "RRC"},
       {0xFFFF, FORMATERROR, "ERROR"} // error acts as an end point of iteration in getOpCode()
};

#endif /* INCLUDE_DISASSEMBLER_H_ */
