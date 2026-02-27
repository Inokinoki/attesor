/* ============================================================================
 * Rosetta Translator - MOV Instruction Translation Header
 * ============================================================================
 *
 * This module provides translation functions for ARM64 MOV family instructions
 * to x86_64 machine code.
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_MOV_H
#define ROSETTA_TRANSLATE_MOV_H

#include <stdint.h>
#include <stddef.h>
#include "rosetta_emit_x86.h"

/* MOV instruction encoding masks */
#define MOV_MOVZ_MASK   0xFF800000
#define MOV_MOVZ_VAL    0xD2800000
#define MOV_MOVK_MASK   0xFF800000
#define MOV_MOVK_VAL    0xF2800000
#define MOV_MOVN_MASK   0xFF800000
#define MOV_MOVN_VAL    0x12800000
#define MOV_MOV_REG_MASK 0xFFC003FF
#define MOV_MOV_REG_VAL  0xAA0003E0

/* MOV translation functions */
int translate_mov_movz(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_mov_movk(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_mov_movn(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_mov_reg(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);

/* Dispatch function - returns 0 if instruction handled, -1 otherwise */
int translate_mov_dispatch(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);

#endif /* ROSETTA_TRANSLATE_MOV_H */
