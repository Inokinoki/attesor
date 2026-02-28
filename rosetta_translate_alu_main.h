/* ============================================================================
 * Rosetta Translator - ALU Instruction Translation Header
 * ============================================================================
 *
 * This module provides translation functions for ARM64 ALU instructions
 * to x86_64 machine code.
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_ALU_MAIN_H
#define ROSETTA_TRANSLATE_ALU_MAIN_H

#include <stdint.h>
#include <stddef.h>
#include "rosetta_emit_x86.h"

/* ALU instruction encoding masks */
#define ALU_ADD_MASK    0x7F000000
#define ALU_ADD_VAL     0x0B000000
#define ALU_SUB_MASK    0x7F000000
#define ALU_SUB_VAL     0x4B000000
#define ALU_ADC_MASK    0x7F000000
#define ALU_ADC_VAL     0x3A000000
#define ALU_SBC_MASK    0x7F000000
#define ALU_SBC_VAL     0x5A000000
#define ALU_AND_MASK    0x7F000000
#define ALU_AND_VAL     0x0A000000
#define ALU_ORR_MASK    0x7F000000
#define ALU_ORR_VAL     0x2A000000
#define ALU_EOR_MASK    0x7F000000
#define ALU_EOR_VAL     0x4A000000
#define ALU_BIC_MASK    0x7F200000
#define ALU_BIC_VAL     0x0A200000
#define ALU_MVN_MASK    0x7FE00000
#define ALU_MVN_VAL     0x2A200000
#define ALU_MUL_MASK    0x7FE0FC00
#define ALU_MUL_VAL     0x1B007C00
#define ALU_UDIV_MASK   0x7FE0FC00
#define ALU_UDIV_VAL    0x1B000800
#define ALU_SDIV_MASK   0x7FE0FC00
#define ALU_SDIV_VAL    0x1B000C00
#define ALU_LSL_MASK    0x7F800000
#define ALU_LSL_VAL     0x1B000000
#define ALU_LSR_MASK    0x7F800000
#define ALU_LSR_VAL     0x5B000000
#define ALU_ASR_MASK    0x7F800000
#define ALU_ASR_VAL     0x1B004000
#define ALU_ROR_MASK    0x7F800000
#define ALU_ROR_VAL     0x1B008000

/* ALU instructions with flags update */
#define ALU_ADDS_MASK   0x7F000000
#define ALU_ADDS_VAL    0x2B000000
#define ALU_SUBS_MASK   0x7F000000
#define ALU_SUBS_VAL    0x6B000000
#define ALU_ANDS_MASK   0x7F200000
#define ALU_ANDS_VAL    0x6A200000

/* Multiply-add/subtract instructions */
#define ALU_MADD_MASK   0x7FE003C0
#define ALU_MADD_VAL    0x1B000000
#define ALU_MSUB_MASK   0x7FE003C0
#define ALU_MSUB_VAL    0x1B000200

/* Register mapping function */
uint8_t translate_get_x86_reg(uint8_t arm_reg);

/* Flag helper functions (for ALU with flags) */
void translate_set_flag_n(uint64_t *pstate, int set);
void translate_set_flag_z(uint64_t *pstate, int set);
void translate_set_flag_c(uint64_t *pstate, int set);
void translate_set_flag_v(uint64_t *pstate, int set);
void translate_update_flags_nzc(uint64_t *pstate, uint64_t result, int carry);

/* ALU translation functions */
int translate_alu_add(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_alu_sub(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_alu_adc(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_alu_sbc(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_alu_and(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_alu_orr(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_alu_eor(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_alu_bic(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_alu_mvn(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_alu_mul(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_alu_udiv(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_alu_sdiv(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_alu_lsl(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_alu_lsr(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_alu_asr(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_alu_ror(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);

/* ALU with flags update */
int translate_alu_adds(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t *pstate);
int translate_alu_subs(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t *pstate);
int translate_alu_ands(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t *pstate);

/* Multiply-add/subtract */
int translate_alu_madd(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_alu_msub(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);

/* Dispatch function - returns 0 if instruction handled, -1 otherwise */
int translate_alu_dispatch(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t *pstate);

#endif /* ROSETTA_TRANSLATE_ALU_MAIN_H */
