/* ============================================================================
 * Rosetta Translator - Compare Instruction Translation Header
 * ============================================================================
 *
 * This module provides translation functions for ARM64 compare instructions
 * to x86_64 machine code.
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_COMPARE_H
#define ROSETTA_TRANSLATE_COMPARE_H

#include <stdint.h>
#include <stddef.h>
#include "rosetta_emit_x86.h"

/* Compare instruction encoding masks */
#define CMP_MASK    0x7F000000
#define CMP_VAL     0xEB000000
#define CMN_MASK    0x7F000000
#define CMN_VAL     0xAB000000
#define TST_MASK    0xFF200000
#define TST_VAL     0xEA000000

/* NZCV flag bit positions */
#define FLAG_N_BIT  31  /* Negative */
#define FLAG_Z_BIT  30  /* Zero */
#define FLAG_C_BIT  29  /* Carry */
#define FLAG_V_BIT  28  /* Overflow */

/* Flag masks */
#define FLAG_N_MASK (1ULL << FLAG_N_BIT)
#define FLAG_Z_MASK (1ULL << FLAG_Z_BIT)
#define FLAG_C_MASK (1ULL << FLAG_C_BIT)
#define FLAG_V_MASK (1ULL << FLAG_V_BIT)
#define FLAG_NZCV_MASK (FLAG_N_MASK | FLAG_Z_MASK | FLAG_C_MASK | FLAG_V_MASK)

/* Compare translation functions */
int translate_compare_cmp(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t *pstate);
int translate_compare_cmn(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t *pstate);
int translate_compare_tst(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t *pstate);

/* Dispatch function - returns 0 if instruction handled, -1 otherwise */
int translate_compare_dispatch(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t *pstate);

/* Flag helper functions */
void translate_set_flag_n(uint64_t *pstate, int set);
void translate_set_flag_z(uint64_t *pstate, int set);
void translate_set_flag_c(uint64_t *pstate, int set);
void translate_set_flag_v(uint64_t *pstate, int set);

#endif /* ROSETTA_TRANSLATE_COMPARE_H */
