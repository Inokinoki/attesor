/* ============================================================================
 * Rosetta Special Instructions Translation Module
 * ============================================================================
 *
 * This module handles translation of x86_64 special instructions including
 * CPUID, RDTSC, SHLD, SHRD, CQO, and interrupt control instructions.
 * ============================================================================ */

#include "rosetta_translate_special.h"
#include <stdint.h>

/* ============================================================================
 * Special Instruction Translation Functions
 * ============================================================================ */

void translate_special_cpuid(code_buffer_t *code_buf, const x86_insn_t *insn)
{
    (void)insn;
    /* CPUID: CPU identification */
    emit_cpuid(code_buf);
}

void translate_special_rdtsc(code_buffer_t *code_buf, const x86_insn_t *insn)
{
    (void)insn;
    /* RDTSC: read timestamp counter */
    emit_rdtsc(code_buf);
}

void translate_special_shld(code_buffer_t *code_buf, const x86_insn_t *insn,
                            uint8_t arm_rd, uint8_t arm_rm)
{
    /* SHLD: double precision shift left */
    uint8_t shift = (insn->imm_size > 0) ? (uint8_t)insn->imm : 1;
    emit_shld(code_buf, arm_rd, arm_rm, shift);
}

void translate_special_shrd(code_buffer_t *code_buf, const x86_insn_t *insn,
                            uint8_t arm_rd, uint8_t arm_rm)
{
    /* SHRD: double precision shift right */
    uint8_t shift = (insn->imm_size > 0) ? (uint8_t)insn->imm : 1;
    emit_shrd(code_buf, arm_rd, arm_rm, shift);
}

void translate_special_cqo(code_buffer_t *code_buf, const x86_insn_t *insn)
{
    (void)insn;
    /* CWD/CDQ/CQO: sign extend RAX to RDX:RAX */
    emit_cqo(code_buf);
}

void translate_special_cli(code_buffer_t *code_buf, const x86_insn_t *insn)
{
    (void)insn;
    /* CLI: clear interrupt flag */
    emit_cli(code_buf);
}

void translate_special_sti(code_buffer_t *code_buf, const x86_insn_t *insn)
{
    (void)insn;
    /* STI: set interrupt flag */
    emit_sti(code_buf);
}

void translate_special_nop(code_buffer_t *code_buf, const x86_insn_t *insn)
{
    (void)insn;
    /* NOP: no operation */
    emit_nop(code_buf);
}

/* End of rosetta_translate_special.c */
