/* ============================================================================
 * Rosetta Translator - Conditional Instruction Translation Header
 * ============================================================================
 *
 * This module provides translation functions for ARM64 conditional
 * instructions to x86_64 machine code.
 *
 * Supported instructions:
 * - CSEL, CSINC, CSINV, CSNEG (Conditional Select variants)
 * - CSET, CSETM (Condition Set)
 * - CINC, CINVERT, CNEG (Conditional operations)
 * - FCSEL (Floating Point Conditional Select)
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_CONDITIONAL_H
#define ROSETTA_TRANSLATE_CONDITIONAL_H

#include <stdint.h>
#include <stddef.h>
#include "rosetta_emit_x86.h"

/* Conditional instruction encoding masks */
#define COND_CSEL_MASK      0xFFE0FC00
#define COND_CSEL_VAL       0x1A800000
#define COND_CSET_MASK      0xFFE0FFFF
#define COND_CSET_VAL       0x1A9F07E0
#define COND_CSETM_MASK     0xFFE0FFFF
#define COND_CSETM_VAL      0x1A9F03E0
#define COND_FCSEL_MASK     0xFFE0FC00
#define COND_FCSEL_VAL      0x1E200C00

/* Condition codes (ARM64 PSTATE encoding) */
#define COND_EQ  0x0  /* Z == 1 */
#define COND_NE  0x1  /* Z == 0 */
#define COND_CS  0x2  /* C == 1 */
#define COND_CC  0x3  /* C == 0 */
#define COND_MI  0x4  /* N == 1 */
#define COND_PL  0x5  /* N == 0 */
#define COND_VS  0x6  /* V == 1 */
#define COND_VC  0x7  /* V == 0 */
#define COND_HI  0x8  /* C == 1 && Z == 0 */
#define COND_LS  0x9  /* C == 0 || Z == 1 */
#define COND_GE  0xA  /* N == V */
#define COND_LT  0xB  /* N != V */
#define COND_GT  0xC  /* Z == 0 && N == V */
#define COND_LE  0xD  /* Z == 1 || N != V */
#define COND_AL  0xE  /* Always */
#define COND_NV  0xF  /* Never */

/* ============================================================================
 * Conditional Select Instructions
 * ============================================================================ */

/**
 * translate_cond_csel - Translate CSEL (conditional select) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * @pstate: Processor state (NZCV flags)
 * Returns: 0 on success
 */
int translate_cond_csel(uint32_t encoding, code_buf_t *code_buf,
                        uint64_t *guest_state, uint32_t *pstate);

/**
 * translate_cond_csinc - Translate CSINC (conditional select increment)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * @pstate: Processor state (NZCV flags)
 * Returns: 0 on success
 */
int translate_cond_csinc(uint32_t encoding, code_buf_t *code_buf,
                         uint64_t *guest_state, uint32_t *pstate);

/**
 * translate_cond_csinv - Translate CSINV (conditional select invert)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * @pstate: Processor state (NZCV flags)
 * Returns: 0 on success
 */
int translate_cond_csinv(uint32_t encoding, code_buf_t *code_buf,
                         uint64_t *guest_state, uint32_t *pstate);

/**
 * translate_cond_csneg - Translate CSNEG (conditional select negate)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * @pstate: Processor state (NZCV flags)
 * Returns: 0 on success
 */
int translate_cond_csneg(uint32_t encoding, code_buf_t *code_buf,
                         uint64_t *guest_state, uint32_t *pstate);

/* ============================================================================
 * Condition Set Instructions
 * ============================================================================ */

/**
 * translate_cond_cset - Translate CSET (condition set) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * @pstate: Processor state (NZCV flags)
 * Returns: 0 on success
 */
int translate_cond_cset(uint32_t encoding, code_buf_t *code_buf,
                        uint64_t *guest_state, uint32_t *pstate);

/**
 * translate_cond_csetm - Translate CSETM (condition set mask)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * @pstate: Processor state (NZCV flags)
 * Returns: 0 on success
 */
int translate_cond_csetm(uint32_t encoding, code_buf_t *code_buf,
                         uint64_t *guest_state, uint32_t *pstate);

/* ============================================================================
 * Conditional Operation Instructions
 * ============================================================================ */

/**
 * translate_cond_cinc - Translate CINC (conditional increment)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * @pstate: Processor state (NZCV flags)
 * Returns: 0 on success
 */
int translate_cond_cinc(uint32_t encoding, code_buf_t *code_buf,
                        uint64_t *guest_state, uint32_t *pstate);

/**
 * translate_cond_cinvert - Translate CINVERT (conditional invert)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * @pstate: Processor state (NZCV flags)
 * Returns: 0 on success
 */
int translate_cond_cinvert(uint32_t encoding, code_buf_t *code_buf,
                           uint64_t *guest_state, uint32_t *pstate);

/**
 * translate_cond_cneg - Translate CNEG (conditional negate)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * @pstate: Processor state (NZCV flags)
 * Returns: 0 on success
 */
int translate_cond_cneg(uint32_t encoding, code_buf_t *code_buf,
                        uint64_t *guest_state, uint32_t *pstate);

/* ============================================================================
 * Floating Point Conditional Select
 * ============================================================================ */

/**
 * translate_cond_fcsel - Translate FCSEL (FP conditional select)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * @pstate: Processor state (NZCV flags)
 * Returns: 0 on success
 */
int translate_cond_fcsel(uint32_t encoding, code_buf_t *code_buf,
                         uint64_t *guest_state, uint32_t *pstate);

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/**
 * translate_cond_get_x86_reg - Map ARM64 register to x86_64
 * @arm_reg: ARM64 register number
 * Returns: x86_64 register number
 */
uint8_t translate_cond_get_x86_reg(uint8_t arm_reg);

/**
 * translate_cond_dispatch - Dispatch conditional instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * @pstate: Processor state (NZCV flags)
 * Returns: 0 if handled, -1 otherwise
 */
int translate_cond_dispatch(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state, uint32_t *pstate);

#endif /* ROSETTA_TRANSLATE_CONDITIONAL_H */
