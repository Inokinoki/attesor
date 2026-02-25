/* ============================================================================
 * Rosetta Branch Translation Module
 * ============================================================================
 *
 * This module handles translation of x86_64 control flow instructions
 * including conditional branches, jumps, calls, returns, and conditional moves.
 * ============================================================================ */

#include "rosetta_translate_branch.h"
#include <stdint.h>

/* ============================================================================
 * Branch Translation Functions
 * ============================================================================ */

int translate_branch_jcc(CodeBuffer *code_buf, const x86_insn_t *insn,
                         uint64_t block_pc)
{
    (void)block_pc;
    /* Conditional branch (Jcc)
     * x86: 70-7F (short), 0F 80-8F (near)
     * ARM64: B.cond with condition code
     */
    uint8_t cond = map_x86_jcc_to_arm_cond(x86_get_jcc_cond(insn));
    int32_t target_offset = (int32_t)insn->imm;

    /* Calculate ARM64 branch offset (relative to PC, in instructions) */
    /* x86 offset is in bytes, ARM64 offset is in 4-byte instructions */
    int32_t br_offset = target_offset / 4;

    emit_bcond(code_buf, cond, br_offset);
    return 1;  /* Block ends */
}

int translate_branch_jmp(CodeBuffer *code_buf, const x86_insn_t *insn,
                         uint64_t block_pc)
{
    (void)block_pc;
    /* Unconditional branch (JMP)
     * x86: E9 (near), EB (short)
     * ARM64: B with offset
     */
    int32_t target_offset = (int32_t)insn->imm;
    int32_t br_offset = target_offset / 4;
    emit_b(code_buf, br_offset);
    return 1;  /* Block ends */
}

int translate_branch_call(CodeBuffer *code_buf, const x86_insn_t *insn,
                          uint64_t block_pc)
{
    /* CALL: save return address and branch
     * x86: E8 (relative), FF /2 (indirect)
     * ARM64: BL for direct, compute target for indirect
     */
    int32_t target_offset = (int32_t)insn->imm;

    /* Save return address in LR (X30)
     * The return address is block_pc + insn->length (next instruction)
     */
    uint64_t ret_addr = block_pc + insn->length;
    emit_movz(code_buf, X30, (uint16_t)(ret_addr & 0xFFFF), 0);
    if (ret_addr >> 16) emit_movk(code_buf, X30, (uint16_t)((ret_addr >> 16) & 0xFFFF), 1);
    if (ret_addr >> 32) emit_movk(code_buf, X30, (uint16_t)((ret_addr >> 32) & 0xFFFF), 2);
    if (ret_addr >> 48) emit_movk(code_buf, X30, (uint16_t)((ret_addr >> 48) & 0xFFFF), 3);

    /* Branch to target */
    int32_t br_offset = target_offset / 4;
    emit_bl(code_buf, br_offset);
    return 1;  /* Block ends */
}

int translate_branch_ret(CodeBuffer *code_buf)
{
    /* RET: return to caller
     * x86: C3 (near), C2 (with stack adjust)
     * ARM64: RET (return to address in LR)
     */
    emit_ret(code_buf);
    return 1;  /* Block ends */
}

void translate_branch_cmov(CodeBuffer *code_buf, const x86_insn_t *insn,
                           uint8_t arm_rd, uint8_t arm_rm)
{
    /* CMOVcc: conditional move
     * x86: 0F 40-4F
     * ARM64: CSEL (conditional select)
     *
     * CSEL Rd, Rn, Rm, cond
     * If cond is true: Rd = Rn
     * If cond is false: Rd = Rm
     *
     * For CMOV: if cond true, Rd = Rm; else Rd unchanged
     * So we use: CSEL Rd, Rm, Rd, cond
     */
    uint8_t cond = map_x86_jcc_to_arm_cond(x86_get_cmov_cond(insn));
    emit_csel_reg_reg_cond(code_buf, arm_rd, arm_rm, arm_rd, cond);
}

void translate_branch_setcc(CodeBuffer *code_buf, const x86_insn_t *insn,
                            uint8_t arm_rd)
{
    /* SETcc: set byte to 1 if condition true, else 0
     * x86: 0F 90-9F
     * ARM64: No direct equivalent, use:
     *   1. Set condition flags
     *   2. CSET to get 0/1 based on condition
     */
    uint8_t cond = map_x86_jcc_to_arm_cond(x86_get_setcc_cond(insn));
    emit_setcc_reg_cond(code_buf, arm_rd, cond);
}

void translate_branch_xchg(CodeBuffer *code_buf, const x86_insn_t *insn,
                           uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* XCHG: exchange registers
     * x86: 90-97 (with RAX), 86-87 (general)
     * ARM64: Use temporary register for swap
     *
     * Note: XCHG also implies LOCK prefix for atomic operation
     * when used with memory operands
     */
    uint8_t tmp = 16;  /* Use X16 as temporary */

    /* Swap: tmp = arm_rd, arm_rd = arm_rm, arm_rm = tmp */
    emit_mov_reg(code_buf, tmp, arm_rd);   /* X16 = arm_rd */
    emit_mov_reg(code_buf, arm_rd, arm_rm); /* arm_rd = arm_rm */
    emit_mov_reg(code_buf, arm_rm, tmp);   /* arm_rm = X16 */
}

/* End of rosetta_translate_branch.c */
