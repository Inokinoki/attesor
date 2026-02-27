/* ============================================================================
 * Rosetta Refactored - ALU Instruction Header
 * ============================================================================
 *
 * Header file for ARM64 ALU instruction translation functions.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_ALU_H
#define ROSETTA_REFACTORED_ALU_H

#include "rosetta_refactored_types.h"
#include <stdint.h>

/* ============================================================================
 * Addition/Subtraction Instructions
 * ============================================================================ */

/**
 * translate_add - Translate ARM64 ADD instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 */
void translate_add(uint32_t encoding, ThreadState *state);

/**
 * translate_sub - Translate ARM64 SUB instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 */
void translate_sub(uint32_t encoding, ThreadState *state);

/**
 * translate_adc - Translate ARM64 ADC (add with carry) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 */
void translate_adc(uint32_t encoding, ThreadState *state);

/**
 * translate_sbc - Translate ARM64 SBC (subtract with carry) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 */
void translate_sbc(uint32_t encoding, ThreadState *state);

/* ============================================================================
 * Logical Operations
 * ============================================================================ */

/**
 * translate_and - Translate ARM64 AND instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 */
void translate_and(uint32_t encoding, ThreadState *state);

/**
 * translate_orr - Translate ARM64 ORR instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 */
void translate_orr(uint32_t encoding, ThreadState *state);

/**
 * translate_eor - Translate ARM64 EOR (XOR) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 */
void translate_eor(uint32_t encoding, ThreadState *state);

/**
 * translate_bic - Translate ARM64 BIC (bit clear) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 */
void translate_bic(uint32_t encoding, ThreadState *state);

/* ============================================================================
 * Multiplication/Division
 * ============================================================================ */

/**
 * translate_mul - Translate ARM64 MUL instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 */
void translate_mul(uint32_t encoding, ThreadState *state);

/**
 * translate_div - Translate ARM64 SDIV/UDIV instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 */
void translate_div(uint32_t encoding, ThreadState *state);

/* ============================================================================
 * Negation Operations
 * ============================================================================ */

/**
 * translate_mvn - Translate ARM64 MVN (move not) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 */
void translate_mvn(uint32_t encoding, ThreadState *state);

/**
 * translate_neg - Translate ARM64 NEG (negate) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 */
void translate_neg(uint32_t encoding, ThreadState *state);

/* ============================================================================
 * Flag Operations
 * ============================================================================ */

/**
 * update_nzcv_flags - Update NZCV flags based on result
 * @state: Thread state
 * @result: Operation result
 * @sf: 1 for 64-bit, 0 for 32-bit
 */
void update_nzcv_flags(ThreadState *state, uint64_t result, int sf);

#endif /* ROSETTA_REFACTORED_ALU_H */
