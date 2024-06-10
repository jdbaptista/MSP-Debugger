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

inline void appendByteOp(char *result, bool isByteOp) {
    if (isByteOp) {
        strcat(result, ".B ");
    } else {
        strcat(result, ".W ");
    }
}


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
int nextInstruction(char *result, uint16_t start_addr, uint16_t *byte_code, uint16_t *next_addr) {
    int totWords = -1;
    uint16_t op_bytes = byte_code[0];
    uint16_t srcWord = byte_code[1];
    uint16_t destWord = byte_code[2];

    opCode op = getOpCode(op_bytes);
    bool isByteOp = isByteOperation(op_bytes);
    strcpy(result, op.repr);

    switch(op.format) {
    case DOUBLE:
    {
        // handle emulated instructions
        totWords = searchEmulated(result, op, start_addr, byte_code);
        if (totWords > 0) { // emulated instruction found
            break;
        } else { // no emulated instruction found
            totWords = 1;
        }

        addressingMode srcMode = getSourceRegisterMode(op_bytes, DOUBLE);
        addressingMode destMode = getDestRegisterMode(op_bytes);
        uint16_t srcReg = getSourceRegister(op_bytes, DOUBLE);
        uint16_t destReg = getDestRegister(op_bytes);

        appendByteOp(result, isByteOp);

        // append source operand
        switch (appendOperand(result, start_addr, srcReg, srcWord, srcMode)) {
        case 1: // srcWord was consumed
        {
            totWords++;
            break;
        }
        case 0: // srcWord was not consumed
        {
            destWord = srcWord;
            break;
        }
        default: // an error occurred
        {
            strcpy(result, "ERROR");
            *next_addr = start_addr + 2;
            return -1;
        }
        }
        strcat(result, " ");

        // append dest operand
        switch (appendOperand(result, start_addr, destReg, destWord, destMode)) {
        case 1: // destWord was consumed
        {
            totWords++;
            break;
        }
        case 0: // destWord was not consumed
        {
            break;
        }
        default: // an error occurred
        {
            strcpy(result, "ERROR");
            *next_addr = start_addr + 2;
            return -1;
        }
        }
        break;
    }
    case SINGLE:
    {
        totWords = 1;
        // the documentation calls this the destination,
        // however it aligns with src more accurately.
        addressingMode srcMode = getSourceRegisterMode(op_bytes, SINGLE);
        uint16_t srcReg = getSourceRegister(op_bytes, SINGLE);

        appendByteOp(result, isByteOp);

        // append source operand
        switch (appendOperand(result, start_addr, srcReg, srcWord, srcMode)) {
        case 1: // srcWord was consumed
        {
            totWords++;
            break;
        }
        case 0: // srcWord was not consumed
        {
            destWord = srcWord;
            break;
        }
        default: // an error occurred
        {
            strcpy(result, "ERROR");
            *next_addr = start_addr + 2;
            return -1;
        }
        }
        break;
    }
    case JUMP:
    {
        totWords = 1;
        uint16_t pcNew = getJumpLocation(op_bytes, start_addr);
        char pcNewStr[7]; // max #0xFFFF\0"
        uintToHex(pcNewStr, pcNew); // use srcOffsetStr to save memory
        strcat(result, " ");
        strcat(result, pcNewStr);
        break;
    }
    default:
    {
        break;
    }
    }

    if (totWords == -1) { // an error has occurred
        strcpy(result, "ERROR");
        *next_addr = start_addr + 2;
        return -1;
    } else {
        *next_addr = start_addr + (totWords << 1);
        return totWords;
    }
}

/**
 * Appends the correct operand format based on the addressing mode
 * to result, but does not append a space.
 *
 * @param result: The destination string that the operand will be
 *                appended to.
 * @param pc:     The program counter, ie start address, of the current
 *                instruction. This is used in symbolic mode.
 * @param reg:    The register number associated with the operand.
 * @param word:   The first or second word after the word associated with
 *                the current instruction, where first is used for the
 *                source operand and second is used for the dest operand.
 * @param mode:   The addressing mode of the operand, expressing the raw
 *                mode for symbolic, absolute, and immediate modes.
 *
 * Returns: The number of words consumed by the operand:
 *                1 if the word was used by the operand,
 *                0 if the word was not used by the operand,
 *               -1 if an error occurred and result is unchanged.
 */
int appendOperand(char* result, uint16_t pc, uint16_t reg, uint16_t word, addressingMode mode) {
    // todo: convert to const hashmap instead of function
    char regStr[4]; // max string "R15\0"
    char wordStr[7]; // max string "0xFFFF\0"

    parseRegisterNum(regStr, reg);
    uintToHex(wordStr, word);

    switch (mode) {
    case REGISTER:
    {
        switch (reg) {
        case 3:
            // R3/CG2 constant
            strcat(result, "#0");
            return 0;
        default:
            // register mode
            strcat(result, regStr);
            return 0;
        }
    }
    case INDEXED:
    {
        switch (reg) {
        case 0:
            // symbolic mode
            uintToHex(wordStr, pc + word);
            strcat(result, wordStr);
            return 1;
        case 2:
            // absolute mode
            strcat(result, "&");
            strcat(result, wordStr);
            return 1;
        case 3:
            // R3/CG2 constant
            strcat(result, "#1");
            return 0;
        default:
            // indexed mode
            strcat(result, wordStr);
            strcat(result, "(");
            strcat(result, regStr);
            strcat(result, ")");
            return 1;
        }
    }
    case INDIRECT:
    {
        switch (reg) {
        case 2:
            // R2/CG1 constant
            strcat(result, "#4");
            return 0;
        case 3:
            // R3/CG2 constant
            strcat(result, "#2");
            return 0;
        default:
            // indirect mode
            strcat(result, "@");
            strcat(result, regStr);
            return 0;
        }
    }
    case AUTOINCREMENT:
    {
        switch (reg) {
        case 0:
            // immediate mode
            strcat(result, "#");
            strcat(result, wordStr);
            return 1;
        case 2:
            // R2/CG1 constant
            strcat(result, "#8");
            return 0;
        case 3:
            // R3/CG2 constant
            strcat(result, "#-1");
            return 0;
        default:
            // autoincrement mode
            strcat(result, "@");
            strcat(result, regStr);
            strcat(result, "+");
            return 0;
        }
    }
    default:
    {
        return -1;
    }
    }
}

struct emulationData {
    uint16_t op_bytes;
    uint16_t srcWord;
    uint16_t destWord;
    addressingMode srcMode;
    addressingMode destMode;
    uint16_t srcReg;
    uint16_t destReg;
    uint16_t startAddr;
    bool isByteOp;
};

bool isADC(struct emulationData *data) {
    return data->srcReg == 3 && data->srcMode == REGISTER;
}

bool isRLC(struct emulationData *data) {
    return data->srcReg == data->destReg &&
           data->srcMode == REGISTER &&
           data->destMode == REGISTER;
}

bool isTest(struct emulationData *data) {
    return data->srcReg == 3 && data->srcMode == REGISTER;
}


int searchEmulated(char *result, opCode op, uint16_t start_addr, uint16_t *byte_code) {
    int totWords = -1;

    struct emulationData data = {
         byte_code[0], // op_bytes
         byte_code[1], // srcWord
         byte_code[2], // destWord
         getSourceRegisterMode(byte_code[0], DOUBLE), // srcMode
         getDestRegisterMode(byte_code[0]), // destMode
         getSourceRegister(byte_code[0], DOUBLE), // srcReg
         getDestRegister(byte_code[0]), // destReg
         start_addr, // startAddr
         isByteOperation(byte_code[0]), // isByteOp
    };

    switch (op.mask) {
    case ADD_MASK:
    {

        break;
    }
    case ADDC_MASK:
    {

        if (isADC(&data)) {
            // ADC(.B) dst | ADDC(.B) #0 dst | ADDC(.B) R3 dst
            strcpy(result, "ADC");
            appendByteOp(result, data.isByteOp);
            data.destWord = data.srcWord;
            totWords = 2;
            totWords += appendOperand(result, data.startAddr, data.destReg, data.destWord, data.destMode);
        } else if (isRLC(&data)) {
            strcpy(result, "RLC");
            appendByteOp(result, data.isByteOp);
            appendOperand(result, data.startAddr, data.srcReg, data.srcWord, data.srcMode);
            totWords = 1;
        }
        break;
    }
    case MOV_MASK:
    {
        break;
    }
    case BIC_MASK:
    {
        break;
    }
    case BIS_MASK:
    {
        break;
    }
    case DADD_MASK:
    {
        break;
    }
    case SUB_MASK:
    {
        break;
    }
    case SUBC_MASK:
    {
        break;
    }
    case XOR_MASK:
    {
        break;
    }
    case CMP_MASK:
    {
        if (isTest(&data)) {
            strcpy(result, "ADC");
            appendByteOp(result, data.isByteOp);
            data.destWord = data.srcWord;
            totWords = 2;
            totWords += appendOperand(result, data.startAddr, data.destReg, data.destWord, data.destMode);
        }
        break;
    }
    default: // no valid emulated instruction
    {
        break;
    }
    }
    return totWords;
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
    uint16_t unsignedOffset = byteCode & 0x01FF; // leave sign bit off
    if (byteCode & 0x0200) { // sign bit of offset is negative
        unsignedOffset = (unsignedOffset ^ 0x01FF) + 1; // convert to positive offset
        return currAddress + 2 - unsignedOffset - unsignedOffset;
    } else {
        return currAddress + 2 + unsignedOffset + unsignedOffset;
    }
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

