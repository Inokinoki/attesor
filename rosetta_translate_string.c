/* ============================================================================
 * Rosetta String Translation Module
 * ============================================================================
 *
 * This module handles translation of x86_64 string operations including
 * MOVS, STOS, LODS, CMPS, and SCAS instructions.
 * ============================================================================ */

#include "rosetta_translate_string.h"
#include <stdint.h>

/* ============================================================================
 * String Translation Functions
 * ============================================================================ */

void translate_string_movs(code_buffer_t *code_buf, const x86_insn_t *insn)
{
    /* MOVS: move string */
    int is_64bit = (insn->opcode == 0xA5);  /* A5 = dword/qword, A4 = byte */
    emit_movs(code_buf, is_64bit);
}

void translate_string_stos(code_buffer_t *code_buf, const x86_insn_t *insn)
{
    /* STOS: store string */
    int size = (insn->opcode == 0xAB) ? 8 : (insn->opcode == 0xAB) ? 4 : 1;
    emit_stos(code_buf, size);
}

void translate_string_lods(code_buffer_t *code_buf, const x86_insn_t *insn)
{
    /* LODS: load string */
    int size = (insn->opcode == 0xAD) ? 8 : (insn->opcode == 0xAD) ? 4 : 1;
    emit_lods(code_buf, size);
}

void translate_string_cmps(code_buffer_t *code_buf, const x86_insn_t *insn)
{
    /* CMPS: compare string */
    int size = (insn->opcode == 0xA7) ? 8 : (insn->opcode == 0xA7) ? 4 : 1;
    emit_cmps(code_buf, size);
}

void translate_string_scas(code_buffer_t *code_buf, const x86_insn_t *insn)
{
    /* SCAS: scan string */
    int size = (insn->opcode == 0xAF) ? 8 : (insn->opcode == 0xAF) ? 4 : 1;
    emit_scas(code_buf, size);
}

/* End of rosetta_translate_string.c */
