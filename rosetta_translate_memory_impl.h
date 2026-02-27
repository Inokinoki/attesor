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

/* ============================================================================
 * PC-Relative Addressing Translation
 * ============================================================================ */

/**
 * translate_adr - Translate ARM64 ADR (PC-relative address)
 * @code_buf: Code buffer for x86_64 emission
 * @insn_ptr: Pointer to ARM64 instruction
 * @arm_rd: Destination ARM64 register
 * @pc: Current program counter
 */
void translate_adr(code_buffer_t *code_buf, const void *insn_ptr,
                   uint8_t arm_rd, uint64_t pc);

/**
 * translate_adrp - Translate ARM64 ADRP (PC-relative page address)
 * @code_buf: Code buffer for x86_64 emission
 * @insn_ptr: Pointer to ARM64 instruction
 * @arm_rd: Destination ARM64 register
 * @pc: Current program counter (page-aligned)
 */
void translate_adrp(code_buffer_t *code_buf, const void *insn_ptr,
                    uint8_t arm_rd, uint64_t pc);

/**
 * translate_adr_imm - Translate ADR with immediate offset
 * @code_buf: Code buffer
 * @imm: Immediate offset
 * @arm_rd: Destination register
 * @pc: Current PC
 */
void translate_adr_imm(code_buffer_t *code_buf, int64_t imm,
                       uint8_t arm_rd, uint64_t pc);

/**
 * translate_pcrel_load - Translate PC-relative load (LDR literal)
 * @code_buf: Code buffer for x86_64 emission
 * @insn_ptr: Pointer to ARM64 instruction
 * @arm_rd: Destination ARM64 register
 * @pc: Current program counter
 */
void translate_pcrel_load(code_buffer_t *code_buf, const void *insn_ptr,
                          uint8_t arm_rd, uint64_t pc);

/**
 * translate_pcrel_store - Translate PC-relative store (STR literal)
 * @code_buf: Code buffer for x86_64 emission
 * @insn_ptr: Pointer to ARM64 instruction
 * @arm_rd: Source ARM64 register
 * @pc: Current program counter
 */
void translate_pcrel_store(code_buffer_t *code_buf, const void *insn_ptr,
                           uint8_t arm_rd, uint64_t pc);

#endif /* ROSETTA_TRANSLATE_MEMORY_IMPL_H */
