/* ============================================================================
 * Rosetta Translator - Bit Manipulation Translation
 * ============================================================================
 *
 * This module handles translation/emulation of ARM64 bit manipulation
 * instructions (BFM, BFI, BFX, RBIT, REV, CLZ, CLS)
 * ============================================================================ */

#ifndef ROSETTA_TRANS_BIT_H
#define ROSETTA_TRANS_BIT_H

#include "rosetta_refactored_types.h"

/* ============================================================================
 * Bit Manipulation Translation Functions
 * ============================================================================ */

/**
 * translate_bfm - Translate/emulate BFM (bitfield move) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_bfm(ThreadState *state, const uint8_t *insn);

/**
 * translate_bfi - Translate/emulate BFI (bitfield insert) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_bfi(ThreadState *state, const uint8_t *insn);

/**
 * translate_bfx - Translate/emulate BFX (bitfield extract) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_bfx(ThreadState *state, const uint8_t *insn);

/**
 * translate_rbit - Translate/emulate RBIT (reverse bits) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_rbit(ThreadState *state, const uint8_t *insn);

/**
 * translate_rev - Translate/emulate REV (reverse bytes) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_rev(ThreadState *state, const uint8_t *insn);

/**
 * translate_rev16 - Translate/emulate REV16 (reverse halfwords) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_rev16(ThreadState *state, const uint8_t *insn);

/**
 * translate_rev32 - Translate/emulate REV32 (reverse words) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_rev32(ThreadState *state, const uint8_t *insn);

/**
 * translate_clz - Translate/emulate CLZ (count leading zeros) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_clz(ThreadState *state, const uint8_t *insn);

/**
 * translate_cls - Translate/emulate CLS (count leading sign bits) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cls(ThreadState *state, const uint8_t *insn);

/**
 * translate_popcnt - Translate/emulate POPCNT (population count) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_popcnt(ThreadState *state, const uint8_t *insn);

#endif /* ROSETTA_TRANS_BIT_H */
