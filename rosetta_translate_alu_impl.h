/* ============================================================================
 * Rosetta Translator - ALU Translation Implementation Header
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_ALU_IMPL_H
#define ROSETTA_TRANSLATE_ALU_IMPL_H

#include "rosetta_codegen.h"

void translate_alu_add(code_buffer_t *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_alu_sub(code_buffer_t *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_alu_and(code_buffer_t *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_alu_or(code_buffer_t *code_buf, const void *insn,
                      uint8_t arm_rd, uint8_t arm_rm);
void translate_alu_xor(code_buffer_t *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_alu_mul(code_buffer_t *code_buf, const void *insn, uint8_t arm_rm);
void translate_alu_div(code_buffer_t *code_buf, const void *insn, uint8_t arm_rm);
void translate_alu_inc(code_buffer_t *code_buf, const void *insn, uint8_t arm_rd);
void translate_alu_dec(code_buffer_t *code_buf, const void *insn, uint8_t arm_rd);
void translate_alu_neg(code_buffer_t *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_alu_not(code_buffer_t *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_alu_shift(code_buffer_t *code_buf, const void *insn,
                         uint8_t arm_rd, uint8_t arm_rm);

#endif /* ROSETTA_TRANSLATE_ALU_IMPL_H */
