/* ============================================================================
 * Rosetta Translator - Memory Translation Implementation Header
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_MEMORY_IMPL_H
#define ROSETTA_TRANSLATE_MEMORY_IMPL_H

#include "rosetta_codegen.h"

void translate_memory_mov(code_buffer_t *code_buf, const void *insn,
                          uint8_t arm_rd, uint8_t arm_rm);
void translate_memory_movzx(code_buffer_t *code_buf, const void *insn,
                            uint8_t arm_rd, uint8_t arm_rm);
void translate_memory_movsx(code_buffer_t *code_buf, const void *insn,
                            uint8_t arm_rd, uint8_t arm_rm);
void translate_memory_movsxd(code_buffer_t *code_buf, const void *insn,
                             uint8_t arm_rd, uint8_t arm_rm);
void translate_memory_lea(code_buffer_t *code_buf, const void *insn, uint8_t arm_rd);
void translate_memory_push(code_buffer_t *code_buf, const void *insn, uint8_t arm_rd);
void translate_memory_pop(code_buffer_t *code_buf, const void *insn, uint8_t arm_rd);
void translate_memory_cmp(code_buffer_t *code_buf, const void *insn,
                          uint8_t arm_rd, uint8_t arm_rm);
void translate_memory_test(code_buffer_t *code_buf, const void *insn,
                           uint8_t arm_rd, uint8_t arm_rm);
void emit_load_imm64(code_buffer_t *code_buf, uint8_t reg, uint64_t imm);

#endif /* ROSETTA_TRANSLATE_MEMORY_IMPL_H */
