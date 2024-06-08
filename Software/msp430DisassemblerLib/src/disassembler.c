/*
 * disassembler.c
 *
 *  Created on: Jul 5, 2023
 *      Author: Jaden Baptista
 */


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "disassembler.h"
#include "masks.h"
#include "types.h"


/**
 * Fills result with the string assembly instruction of the instruction beginning at start_addr,
 * which should correspond to the first word presented in byte_code. The function returns the
 * next assembly instruction address in next_addr because the byte length of instructions varies
 * depending on register addressing modes used.
 *
 * @param result: A string buffer of minimum length 31 that will store the assembly instruction.
 * @param start_addr: The location in memory of the instruction to decode.
 * @param byte_code: An array of 3 16-bit words corresponding to the memory beginning at start_addr.
 *                   Remains unchanged over this function's execution.
 * @param next_addr: A pointer to where this function should return the memory location of the
 *                   next instruction to decode, necessary because of variable length instructions.
 *
 * Returns: The number of 16-bit words the assembly instruction 'consumed'. Returns -1 if an error
 *          occurred, in which case next_addr equals start_addr + 2 and result stores "ERROR".
 */
int nextInstruction(char* result, uint16_t start_addr, uint16_t *byte_code, uint16_t *next_addr) {
    uint16_t op_bytes = byte_code[0];
    uint16_t srcOffset = byte_code[1];
    uint16_t destOffset = byte_code[2];
    char srcRegStr[4];
    char srcOffsetStr[7];
    char destRegStr[4];
    char destOffsetStr[7];

    opCode op = getOpCode(op_bytes);
    strcpy(result, op.repr);

    switch(op.format) {
    case DOUBLE: {
        addressingMode srcMode = getSourceRegisterMode(op_bytes, DOUBLE);

        addressingMode destMode = getDestRegisterMode(op_bytes);
        uint16_t srcReg = getSourceRegister(op_bytes, DOUBLE);
        uint16_t destReg = getDestRegister(op_bytes);

        if (isByteOperation(op_bytes)) {
            strcat(result, ".B ");
        } else {
            strcat(result, ".W ");
        }

        parseRegisterNum(srcRegStr, srcReg);
        parseRegisterNum(destRegStr, destReg);

        // handle immediate mode and constant generators
        if (!(srcMode == AUTOINCREMENT && srcReg == 0) &&
                (srcMode != INDEXED || srcReg == 3)) {
            // this case handles non immediate mode,
            // non indexed mode, non constant generator cases
            destOffset = byte_code[1];
        }

        uintToHex(srcOffsetStr, srcOffset);
        uintToHex(destOffsetStr, destOffset);

        if ((srcMode == AUTOINCREMENT && srcReg == 0) || srcReg == 2 || srcReg == 3) {
            appendConstant(result, srcReg, srcMode, srcOffsetStr);
        } else if (srcMode == INDEXED && (srcReg == 0 || srcReg == 2)) {
            appendPointer(result, srcRegStr, start_addr, srcReg, srcOffset);
        } else {
            appendOperand(result, srcRegStr, srcMode, srcOffsetStr);
        }
        strcat(result, " ");
        if (destMode == INDEXED && (destReg == 0 || destReg == 2)) {
            appendPointer(result, destRegStr, start_addr, destReg, destOffset);
        } else {
            appendOperand(result, destRegStr, destMode, destOffsetStr);
        }

        if ((srcMode == INDEXED && destMode == INDEXED && srcReg != 3) ||
            (srcMode == AUTOINCREMENT && srcReg == 0 && destMode == INDEXED))
        {
            *next_addr = start_addr + 6;
            return 3;
        } else if ((srcMode == INDEXED && srcReg != 3) || destMode == INDEXED) {
            *next_addr = start_addr + 4;
            return 2;
        } else {
            *next_addr = start_addr + 2;
            return 1;
        }
        break;
    }
    case SINGLE: {
        // the documentation calls this the destination,
        // however it aligns with src more accurately.
        addressingMode srcMode = getSourceRegisterMode(op_bytes, SINGLE);
        uint16_t srcReg = getSourceRegister(op_bytes, SINGLE);

        if (isByteOperation(op_bytes)) {
            strcat(result, ".B ");
        } else {
            strcat(result, ".W ");
        }

        parseRegisterNum(srcRegStr, srcReg);
        uintToHex(srcOffsetStr, srcOffset);

        if (srcReg < 4) { //TODO: double check this for immediate mode
            appendConstant(result, srcReg, srcMode, srcOffsetStr);
        } else if (srcMode == INDEXED && (srcReg == 0 || srcReg == 2)) {
            appendPointer(result, srcRegStr, start_addr, srcReg, srcOffset);
        } else {
            appendOperand(result, srcRegStr, srcMode, srcOffsetStr);
        }

        if (srcMode == INDEXED && srcReg != 3) {
            *next_addr = start_addr + 4;
            return 2;
        } else {
            *next_addr = start_addr + 2;
            return 1;
        }
        break;
    }
    case JUMP: {
        uint16_t pcNew = getJumpLocation(op_bytes, start_addr);
        uintToHex(srcOffsetStr, pcNew); // use srcOffsetStr to save memory
        strcat(result, " ");
        strcat(result, srcOffsetStr);

        *next_addr = start_addr + 2;
        return 1;
        break;
    }
    default: {
        *next_addr = start_addr + 2;
        return -1;
    }
    }

    return -1; // should not end up here
}

/**
 * Fills the place of appendOperand for constant generator registers 2 and 3,
 * either appending the correct constant or, in the case of 2 (SR), potentially
 * appending a simple register operand. Does not append a space!
 */
void appendConstant(char* result, uint16_t reg, addressingMode mode, char* offset) {
    // handle immediate mode
    if (reg == 0 && mode == AUTOINCREMENT) {
        strcat(result, "#");
        strcat(result, offset);
        return;
    }
    // handle constant generators
    switch (mode) {
    case REGISTER:
        if (reg == 2) {
            strcat(result, "SR");
        } else if (reg == 3) {
            strcat(result, "#0");
        }
        break;
    case INDEXED:
        if (reg == 2) {
            strcat(result, "#");
            strcat(result, offset);
        } else if (reg == 3) {
            strcat(result, "#1");
        }
        break;
    case INDIRECT:
        if (reg == 2) {
            strcat(result, "#4");
        } else if (reg == 3) {
            strcat(result, "#2");
        }
        break;
    case AUTOINCREMENT:
        if (reg == 2) {
            strcat(result, "#8");
        } else if (reg == 3) {
            strcat(result, "#0xFFFF");
        }
        break;
    default:
        break;
    }
}

void appendPointer(char* result, char* regStr, uint16_t pc, uint16_t reg, uint16_t offset) {
    if (reg == 0) { // R0/PC -> symbolic
        uintToHex(regStr, pc + offset);
        strcat(result, regStr);
    } else if (reg == 2) {
        uintToHex(regStr, offset);
        strcat(result, "&");
        strcat(result, regStr);
    }
}

/**
 * Does not append a space!
 */
void appendOperand(char* result, char* reg, addressingMode mode, char* offset) {
    switch (mode) {
        case REGISTER:
            strcat(result, reg); // result: "R15"
            break;
        case INDEXED:
            strcat(result, offset);
            strcat(result, "(");
            strcat(result, reg);
            strcat(result, ")"); // result: "0xFFFF(R15)"
            break;
        case INDIRECT:
            strcat(result, "@");
            strcat(result, reg); // result: "@R15"
            break;
        case AUTOINCREMENT:
            strcat(result, "@");
            strcat(result, reg);
            strcat(result, "+"); // result: "@R15+"
            break;
        default:
            strcat(result, "ERROR"); // ADDRESSINGERROR
            break;
    }
}

// NOTE: order matters here! some masks are submasks of others!
opCode getOpCode(uint16_t byteCode) {
    // Note: Order of opcodes is set in disassembler.h
    //       and is made so submasks are checked late.
    unsigned int i = 0;
    opCode currCode;
    while (true) {
        currCode = CODES[i];
        bool doneIterating = currCode.format == FORMATERROR;
        bool isCorrectCode = (currCode.mask & byteCode) == currCode.mask;
        if (doneIterating || isCorrectCode) {
            return currCode; // only exit
        }
        i++;
    }
}

/**
 * Returns the addressing mode of the destination register, Ad, in Double-Operand
 * instructions. If called on Jump instructions, this will return an ERROR result.
 *
 * NOTE: Although the User Guide specifies Single-Operand instructions to use Ad,
 * this interface uses As instead as it lines up better with the size of
 * Double-Operand As addressing mode bits.
 */
addressingMode getDestRegisterMode(uint16_t byteCode) {
    switch (byteCode & 0x0080) {
        case 0x0080:
            return INDEXED;
        case 0x0000:
            return REGISTER;
        default:
            return ADDRESSINGERROR;
    }
}

/**
 * Returns the addressing mode of the source register, As, in Double-Operand
 * instructions and the mode of the destination register, Ad, in Single-Operand
 * instructions. If called on Jump instructions, this will return an
 * ERROR result.
 */
addressingMode getSourceRegisterMode(uint16_t byteCode, formatType type) {
    if (type != DOUBLE && type != SINGLE) {
        return ADDRESSINGERROR;
    }
    switch (byteCode & 0x0030) {
        case 0x0030:
            return AUTOINCREMENT;
        case 0x0020:
            return INDIRECT;
        case 0x0010:
            return INDEXED;
        case 0x0000:
            return REGISTER;
        default:
            return ADDRESSINGERROR;
    }
}

/**
 * Returns the value of the destination register, D-Reg, in Double-Operand instructions.
 * If called on Jump or Single-Operand instructions, this will result in 0.
 */
uint16_t getDestRegister(uint16_t byteCode) {
    return (byteCode & 0x000F);
}

/**
 * Returns the value of the source register, S-Reg, in Single-Operand and Double-Operand
 * instructions. If called on Jump instructions, this will result in 0.
 */
uint16_t getSourceRegister(uint16_t byteCode, formatType type) {
    switch (type) {
    case DOUBLE:
        return (byteCode & 0x0F00) >> 8;
    case SINGLE:
        return (byteCode & 0x000F);
    default:
        return 0;
    }
}

/**
 * Returns true if B/W bit is high. This is meaningless in JUMP instructions.
 */
inline bool isByteOperation(uint16_t byteCode) {
    return (byteCode & 0x0040);
}

uint16_t getJumpLocation(uint16_t byteCode, uint16_t currAddress) {
    // create proper 16-bit offset from 10-bit signed offset
    int16_t signedOffset = (((int16_t) (byteCode & 0x03FF)) << 6) >> 6; // >> is arithmetic shift which smears sign
    int16_t targetAddress = currAddress + 2 + (signedOffset * 2); // was (uint16_t) (currAddress + 2 + (signedOffset * 2))
    // convert to uint16_t from 2s complement
    return (uint16_t) (~targetAddress + 1);
}

/**
 * Converts 16-bit byte code to an offset formatted string. Used when addressing
 * mode is indexed, so the next word is the offset.
 */
void byteCodeToOffset(char* result, uint16_t byteCode) {
    uintToHex(result, byteCode);
}

/**
 * Fills result with the string representation of regNum,
 * formatted as R15. Result should be a buffer of minimum
 * size 4 to not overflow.
 */
void parseRegisterNum(char* result, uint16_t regNum) {
    static const char* map[] = { "PC", "SP", "SR", "CG2",
                                 "R4", "R5", "R6", "R7",
                                 "R8", "R9", "R10", "R11",
                                 "R12", "R13", "R14", "R15" };
    if (regNum > 15) {
        result = strcpy(result, "ERR");
    } else {
        result = strcpy(result, map[regNum]);
    }
}

/**
 * Fills result with the hexadecimal representation
 * of input, formatted as FFFFh. Result should be a
 * buffer of minimum size 7 to not overflow.
 */
void uintToHex(char* result, uint16_t input) {
    char* map = "0123456789ABCDEF";
    result[0] = '0';
    result[1] = 'x';
    result[2] = map[(input >> 12) & 0x000F];
    result[3] = map[(input >> 8) & 0x000F];
    result[4] = map[(input >> 4) & 0x000F];
    result[5] = map[input & 0x000F];
    result[6] = '\0';
}

