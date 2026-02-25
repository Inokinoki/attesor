/* ============================================================================
 * Rosetta Translator - Branch Translation Implementation Header
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_BRANCH_IMPL_H
#define ROSETTA_TRANSLATE_BRANCH_IMPL_H

#include "rosetta_codegen.h"

int translate_branch_jcc(code_buffer_t *code_buf, const void *insn, uint64_t block_pc);
int translate_branch_jmp(code_buffer_t *code_buf, const void *insn, uint64_t block_pc);
int translate_branch_call(code_buffer_t *code_buf, const void *insn, uint64_t block_pc);
int translate_branch_ret(code_buffer_t *code_buf);
void translate_branch_cmov(code_buffer_t *code_buf, const void *insn,
                           uint8_t arm_rd, uint8_t arm_rm);
void translate_branch_setcc(code_buffer_t *code_buf, const void *insn, uint8_t arm_rd);
void translate_branch_xchg(code_buffer_t *code_buf, const void *insn,
                           uint8_t arm_rd, uint8_t arm_rm);

#endif /* ROSETTA_TRANSLATE_BRANCH_IMPL_H */
