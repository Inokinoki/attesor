/* ============================================================================
 * Rosetta Translator - String Operations Translation
 * ============================================================================
 *
 * This module handles translation/emulation of x86 string instructions
 * (MOVS, STOS, LODS, CMPS, SCAS)
 * ============================================================================ */

#ifndef ROSETTA_TRANS_STRING_H
#define ROSETTA_TRANS_STRING_H

#include "rosetta_refactored_types.h"

/* ============================================================================
 * String Translation Functions
 * ============================================================================ */

/**
 * translate_movs - Translate/emulate MOVS (move string) instruction
 * @param state Thread state
 * @param insn Instruction bytes (x86)
 * @param size Element size (1, 2, 4, or 8 bytes)
 * @param rep REP prefix flag
 * @param ecx Count register value
 * @return 0 on success, -1 on failure
 */
int translate_movs(ThreadState *state, const uint8_t *insn, int size, bool rep, uint32_t ecx);

/**
 * translate_stos - Translate/emulate STOS (store string) instruction
 * @param state Thread state
 * @param insn Instruction bytes (x86)
 * @param size Element size (1, 2, 4, or 8 bytes)
 * @param rep REP prefix flag
 * @param ecx Count register value
 * @return 0 on success, -1 on failure
 */
int translate_stos(ThreadState *state, const uint8_t *insn, int size, bool rep, uint32_t ecx);

/**
 * translate_lods - Translate/emulate LODS (load string) instruction
 * @param state Thread state
 * @param insn Instruction bytes (x86)
 * @param size Element size (1, 2, 4, or 8 bytes)
 * @param rep REP prefix flag
 * @param ecx Count register value
 * @return 0 on success, -1 on failure
 */
int translate_lods(ThreadState *state, const uint8_t *insn, int size, bool rep, uint32_t ecx);

/**
 * translate_cmps - Translate/emulate CMPS (compare string) instruction
 * @param state Thread state
 * @param insn Instruction bytes (x86)
 * @param size Element size (1, 2, 4, or 8 bytes)
 * @param rep REP prefix flag
 * @param ecx Count register value
 * @return 0 on success, -1 on failure
 */
int translate_cmps(ThreadState *state, const uint8_t *insn, int size, bool rep, uint32_t ecx);

/**
 * translate_scas - Translate/emulate SCAS (scan string) instruction
 * @param state Thread state
 * @param insn Instruction bytes (x86)
 * @param size Element size (1, 2, 4, or 8 bytes)
 * @param rep REP prefix flag
 * @param ecx Count register value
 * @return 0 on success, -1 on failure
 */
int translate_scas(ThreadState *state, const uint8_t *insn, int size, bool rep, uint32_t ecx);

#endif /* ROSETTA_TRANS_STRING_H */
