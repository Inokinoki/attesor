/* ============================================================================
 * Rosetta Refactored - Control Flow Instruction Header
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_CONTROL_H
#define ROSETTA_REFACTORED_CONTROL_H

#include "rosetta_refactored_types.h"
#include <stdint.h>

/* ============================================================================
 * Immediate Branch Instructions
 * ============================================================================ */

/**
 * translate_b - Translate ARM64 B (unconditional branch) instruction
 */
void translate_b(uint32_t encoding, ThreadState *state);

/**
 * translate_bl - Translate ARM64 BL (branch with link) instruction
 */
void translate_bl(uint32_t encoding, ThreadState *state);

/* ============================================================================
 * Register Branch Instructions
 * ============================================================================ */

/**
 * translate_br - Translate ARM64 BR (branch to register) instruction
 */
void translate_br(uint32_t encoding, ThreadState *state);

/**
 * translate_blr - Translate ARM64 BLR (branch with link to register) instruction
 */
void translate_blr(uint32_t encoding, ThreadState *state);

/**
 * translate_ret - Translate ARM64 RET (return) instruction
 */
void translate_ret(uint32_t encoding, ThreadState *state);

/* ============================================================================
 * Conditional Branch Instructions
 * ============================================================================ */

/**
 * translate_bcond - Translate ARM64 B.cond (conditional branch) instruction
 */
void translate_bcond(uint32_t encoding, ThreadState *state);

/* ============================================================================
 * Compare and Branch Instructions
 * ============================================================================ */

/**
 * translate_cbz - Translate ARM64 CBZ (compare and branch if zero) instruction
 */
void translate_cbz(uint32_t encoding, ThreadState *state);

/**
 * translate_cbnz - Translate ARM64 CBNZ (compare and branch if not zero) instruction
 */
void translate_cbnz(uint32_t encoding, ThreadState *state);

/* ============================================================================
 * Test and Branch Instructions
 * ============================================================================ */

/**
 * translate_tbz - Translate ARM64 TBZ (test bit and branch if zero) instruction
 */
void translate_tbz(uint32_t encoding, ThreadState *state);

/**
 * translate_tbnz - Translate ARM64 TBNZ (test bit and branch if not zero) instruction
 */
void translate_tbnz(uint32_t encoding, ThreadState *state);

/* ============================================================================
 * Condition Helpers
 * ============================================================================ */

/**
 * check_condition - Check if a condition code is satisfied
 */
int check_condition(uint8_t cond, ThreadState *state);

#endif /* ROSETTA_REFACTORED_CONTROL_H */
