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

/**
 * Takes in a length 12 list of words (byte_code) and converts them
 * to a length 4 list of assembly instruction strings (result).
 *
 * @param result    An array where the result of this operation
 *                  will be stored. This array should be of
 *                  length 4 (the maximum number of instructions
 *                  in the byteCode), containing strings of length
 *                  26 (the maximum length of an instruction).
 * @param stop_addr A pointer to an integer that will be updated
 *                  with the flash location of the machine
 *                  instruction directly after the 4 that were
 *                  disassembled and stored in result.
 * @param byte_code An array of words consisting of machine code
 *                  for the MSP430G2553. This array should be of
 *                  length 12 (4 * maximum words to describe one
 *                  assembly instruction).
 * @param addr      The location in flash memory of the first word
 *                  given in the byte_code. This is used for
 *                  determining proper jump offsets from addresses.
 * @returns An error code describing the result of this operation.
 *          0: subroutine executed successfully.
 *          1: subroutine ran into a fatal error.
 */
bool bytesToInstructions(char *result[], uint16_t *stop_addr, uint16_t byte_code[], uint16_t addr);

/**
 * It is the responsibility of the programmer to first learn the register addressing modes of
 * the source and destination operand. Then, they must find the correct offset in the next two
 * words and format them with the formatOffset function. If there is no offset for either operand,
 * then they should be set to NULL to avoid errors.
 *
 * NOTE: For single operand instructions, DestOffset should be used instead of source offset!
 */
void getAsm(char* result, uint16_t byteCode, opCode* code, uint16_t currAddress, char* srcOffset, char* destOffset);

/**
 * It is the responsibility of the programmer to choose the correct function.
 * This choice should be based on getDestRegisterMode and getSourceRegisterMode.
 */
void getAsmSingleOffset(char* result, uint16_t byteCode, opCode* code, uint16_t srcOffset);

/**
 * It is the responsibility of the programmer to choose the correct function.
 * This choice should be based on getDestRegisterMode and getSourceRegisterMode.
 */
void getAsmDoubleOffset(char* result, uint16_t byteCode, opCode* code, uint16_t srcOffset, uint16_t destOffset);

opCode getOperation(uint16_t byteCode);

/**
 * Returns the addressing mode of the destination register, Ad, in Double-Operand
 * instructions. If called on Jump instructions, this will return an ERROR result.
 *
 * NOTE: Although the User Guide specifies Single-Operand instructions to use Ad,
 * this interface uses As instead as it lines up better with the size of
 * Double-Operand As addressing mode bits.
 */
addressingMode getDestRegisterMode(uint16_t byteCode, formatType type);

/**
 * Returns the addressing mode of the source register, As, in Double-Operand and
 * Single-Operand instructions. If called on Jump instructions, this will return an
 * ERROR result.
 */
addressingMode getSourceRegisterMode(uint16_t byteCode, formatType type);

/**
 * Returns the value of the destination register, D-Reg, in Double-Operand instructions.
 * If called on Jump or Single-Operand instructions, this will result in 0.
 */
uint16_t getDestRegister(uint16_t byteCode, formatType type);

/**
 * Returns the value of the source register, S-Reg, in Single-Operand and Double-Operand
 * instructions. If called on Jump instructions, this will result in 0.
 */
uint16_t getSourceRegister(uint16_t byteCode, formatType type);

/**
 * Returns true if B/W bit is high. This is meaningless in JUMP instructions.
 */
bool isByteOperation(uint16_t byteCode);

void getJumpOffset(char* result, uint16_t byteCode, uint16_t currAddress);

/**
 * Converts 16-bit byte code to an offset formatted string. Used when addressing
 * mode is indexed, so the next word is the offset.
 */
void byteCodeToOffset(char* result, uint16_t byteCode);

void parseRegisterNum(char* result, uint16_t regNum);

void uintToHex(char* result, uint16_t input);

static const opCode CODES[] = {
       {RETI_MASK, RETI_TYPE, "RETI"},
       {CALL_MASK, CALL_TYPE, "CALL"},
       {PUSH_MASK, PUSH_TYPE, "PUSH"},
       {SXT_MASK, SXT_TYPE, "SXT"},
       {RRA_MASK, RRA_TYPE, "RRA"},
       {SWPB_MASK, SWPB_TYPE, "SWPB"},
       {RRC_MASK, RRC_TYPE, "RRC"},
       {JNE_MASK, JNE_TYPE, "JNE"},
       {JEQ_MASK, JEQ_TYPE, "JEQ"},
       {JNC_MASK, JNC_TYPE, "JNC"},
       {JC_MASK, JC_TYPE, "JC"},
       {JN_MASK, JN_TYPE, "JN"},
       {JGE_MASK, JGE_TYPE, "JGE"},
       {JL_MASK, JL_TYPE, "JL"},
       {JMP_MASK, JMP_TYPE, "JMP"},
       {MOV_MASK, MOV_TYPE, "MOV"},
       {ADD_MASK, ADD_TYPE, "ADD"},
       {ADDC_MASK, ADDC_TYPE, "ADDC"},
       {SUBC_MASK, SUBC_TYPE, "SUBC"},
       {SUB_MASK, SUB_TYPE, "SUB"},
       {CMP_MASK, CMP_TYPE, "CMP"},
       {DADD_MASK, DADD_TYPE, "DADD"},
       {BIT_MASK, BIT_TYPE, "BIT"},
       {BIC_MASK, BIC_TYPE, "BIC"},
       {BIS_MASK, BIS_TYPE, "BIS"},
       {XOR_MASK, XOR_TYPE, "XOR"},
       {AND_MASK, AND_TYPE, "AND"},
       {0xFFFF, FORMATERROR, "ERROR"} // error acts as an end point of iteration in getOpCode()
};

#endif /* INCLUDE_DISASSEMBLER_H_ */
