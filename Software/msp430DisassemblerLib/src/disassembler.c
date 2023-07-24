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
#include "include/disassembler.h"

bool bytesToInstructions(char *result[], uint16_t *stop_addr, uint16_t byte_code[], uint16_t address) {
    // TODO: Implement function
    return false;
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
            if (offset == NULL) {
                strcat(result, "ERROR");
            } else {
                strcat(result, offset);
                strcat(result, "(");
                strcat(result, reg);
                strcat(result, ")"); // result: "FFFFh(R15)"
            }
            break;
        case INDIRECT:
            strcpy(result, "@");
            strcpy(result, reg); // result: "@R15"
            break;
        case AUTOINCREMENT:
            strcpy(result, "@");
            strcpy(result, reg);
            strcpy(result, "+"); // result: "@R15+"
            break;
        default:
            strcpy(result, "ERROR"); // ADDRESSINGERROR
            break;
    }
}

void getAsm(char* result, uint16_t byteCode, opCode* opCode, uint16_t currAddress, char* srcOffset, char* destOffset) {
    if (opCode->format == FORMATERROR) {
        strcpy(result, opCode->repr);
        return;
    }
    strcpy(result, opCode->repr); // result: "MOV"
    if (isByteOperation(byteCode)) {
        strcat(result, ".B "); // result: "MOV.B "
    } else {
        strcat(result, " "); // result: "MOV "
    }
    switch(opCode->format) {
        case DOUBLE: {
            addressingMode destMode = getDestRegisterMode(byteCode, DOUBLE);
            addressingMode srcMode = getSourceRegisterMode(byteCode, DOUBLE);
            char* dest = (char*) calloc(5, sizeof(char));
            char* src = (char*) calloc(5, sizeof(char));
            parseRegisterNum(dest, getDestRegister(byteCode, DOUBLE));
            parseRegisterNum(src, getSourceRegister(byteCode, DOUBLE));
            appendOperand(result, src, srcMode, srcOffset);
            strcat(result, " ");
            appendOperand(result, dest, destMode, destOffset);
            free(dest);
            free(src);
            break;
        }
        case SINGLE: {
            addressingMode destMode = getDestRegisterMode(byteCode, SINGLE);
            char* dest = (char*) calloc(5, sizeof(char));
            parseRegisterNum(dest, getDestRegister(byteCode, SINGLE));
            appendOperand(result, dest, destMode, destOffset);
            free(dest);
            break;
        }
        case JUMP: {
            char* pcOffset = (char*) calloc(8, sizeof(char));
            getJumpOffset(pcOffset, byteCode, currAddress);
            strcat(result, pcOffset);
            break;
        }
        default:
            strcpy(result, "ERROR");
            break;
    }
}




// NOTE: order matters here! some masks are submasks of others!
opCode getOpCode(uint16_t byteCode) {
    // Note: Order of opcodes is set in disassembler.h
    //       and is made so submasks are checked late.
    unsigned int i = 0;
    opCode currCode;
    while (true) {  // danger is my middle name
        currCode = CODES[i];
        bool doneIterating = currCode.format == FORMATERROR;
        bool isCorrectCode = (currCode.mask & byteCode) == currCode.mask;
        if (doneIterating || isCorrectCode) {
            return currCode; // only exit
        }
        i++;
    }
}

addressingMode getDestRegisterMode(uint16_t byteCode, formatType type) {
    if (type != DOUBLE) {
        return ADDRESSINGERROR;
    }
    switch (byteCode & 0x0080) {
        case 0x0080:
            return INDEXED;
        case 0x0000:
            return REGISTER;
        default:
            return ADDRESSINGERROR;
    }
}

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

uint16_t getDestRegister(uint16_t byteCode, formatType type) {
    if (type != DOUBLE) {
        return 0;
    }
    return (byteCode & 0x000F);
}

uint16_t getSourceRegister(uint16_t byteCode, formatType type) {
    switch (type) {
    case DOUBLE:
        return (byteCode & 0x0F00);
    case SINGLE:
        return (byteCode & 0x000F);
    default:
        return 0;
    }
}

bool isByteOperation(uint16_t byteCode) {
    return (byteCode & 0x0040);
}

void getJumpOffset(char* result, uint16_t byteCode, uint16_t currAddress) {
    // create proper 16-bit offset from 10-bit signed offset
    int16_t signedOffset = (((int16_t) (byteCode & 0x03FF)) << 6) >> 6; // >> is arithmetic shift which smears sign
    int16_t targetAddress = (uint16_t) (currAddress + 2 + (signedOffset * 2)); // addresses should not be negative
    uintToHex(result, targetAddress);
}

void byteCodeToOffset(char* result, uint16_t byteCode) {
    uintToHex(result, byteCode);
}

void parseRegisterNum(char* result, uint16_t regNum) {
    if (regNum > 15) {
        strcat(result, "ERR");
        return;
    }
    strcpy(result, ""); // clear result just in case
    char* map[] = { "PC", "SP", "SR", "CG2", "R4", "R5", "R6", "R7", "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15" };
    strcat(result, map[regNum]);
    return;
}

void uintToHex(char* result, uint16_t input) {
    char* map = "0123456789ABCDEF";
    strcpy(result, ""); // clear result just in case
    int i;
    for (i = 12; i >= 0; i -= 4) {
        char curr[2];
        uint16_t ndx = (input >> i);
        ndx = (ndx << 12); // clear top bits
        ndx = (ndx >> 12); // put target byte back to LSB
        curr[0] = map[ndx];
        curr[1] = '\0';
        strcat(result, curr);
    }
    strcat(result, "h"); // result: FFFFh
}

