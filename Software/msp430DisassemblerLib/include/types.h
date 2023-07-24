/*
 * types.h
 *
 *  Created on: Jul 5, 2023
 *      Author: Jaden Baptista
 */

#ifndef INCLUDE_TYPES_H_
#define INCLUDE_TYPES_H_

typedef enum {
    ADDRESSINGERROR,
    REGISTER,
    INDEXED,
    INDIRECT,
    AUTOINCREMENT,
} addressingMode;

typedef enum {
    FORMATERROR,
    SINGLE,
    DOUBLE,
    JUMP
} formatType;

typedef struct {
    uint16_t mask;
    formatType format;
    const char* repr;
} opCode;

#endif /* INCLUDE_TYPES_H_ */
