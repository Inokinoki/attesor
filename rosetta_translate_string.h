/* ============================================================================
 * Rosetta String Translation Module
 * ============================================================================
 *
 * This module handles translation of x86_64 string operations including
 * MOVS, STOS, LODS, CMPS, and SCAS instructions.
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_STRING_H
#define ROSETTA_TRANSLATE_STRING_H

#include "rosetta_types.h"
#include "rosetta_x86_decode.h"
#include "rosetta_codegen.h"
#include <stdint.h>

/* ============================================================================
 * String Translation Functions
 * ============================================================================ */

/**
 * Translate MOVS (move string)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 */
void translate_string_movs(code_buffer_t *code_buf, const x86_insn_t *insn);

/**
 * Translate STOS (store string)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 */
void translate_string_stos(code_buffer_t *code_buf, const x86_insn_t *insn);

/**
 * Translate LODS (load string)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 */
void translate_string_lods(code_buffer_t *code_buf, const x86_insn_t *insn);

/**
 * Translate CMPS (compare string)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 */
void translate_string_cmps(code_buffer_t *code_buf, const x86_insn_t *insn);

/**
 * Translate SCAS (scan string)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 */
void translate_string_scas(code_buffer_t *code_buf, const x86_insn_t *insn);

#endif /* ROSETTA_TRANSLATE_STRING_H */
