/* ============================================================================
 * Rosetta Translator - Translation Dispatch Header
 * ============================================================================
 *
 * Main block translation dispatch and instruction routing.
 * ============================================================================ */

#ifndef ROSETTA_TRANS_DISPATCH_H
#define ROSETTA_TRANS_DISPATCH_H

#include <stdint.h>
#include "rosetta_codegen.h"
#include "rosetta_x86_insns.h"

/* Maximum instructions per basic block */
#define MAX_BLOCK_INSTRUCTIONS 64

/* ============================================================================
 * Block Translation Functions
 * ============================================================================ */

/**
 * Translate x86_64 basic block to ARM64
 * @param guest_pc Guest x86_64 PC to translate
 * @return Translated block pointer, or NULL on failure
 */
void *translate_block(uint64_t guest_pc);

/**
 * Fast path translation for hot blocks
 * @param guest_pc Guest x86_64 PC to translate
 * @return Translated block pointer, or NULL on failure
 */
void *translate_block_fast(uint64_t guest_pc);

/**
 * Execute translated block
 * @param state Thread state
 * @param block Translated code block
 */
void execute_translated(ThreadState *state, void *block);

/* ============================================================================
 * Instruction Dispatch Functions
 * ============================================================================ */

/**
 * Dispatch ALU instruction to appropriate handler
 * @param code_buf Code buffer
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM64 register
 * @param arm_rm Source ARM64 register
 */
void dispatch_alu_insn(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm);

/**
 * Dispatch memory instruction to appropriate handler
 * @param code_buf Code buffer
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM64 register
 * @param arm_rm Source ARM64 register
 */
void dispatch_memory_insn(code_buffer_t *code_buf, const x86_insn_t *insn,
                          uint8_t arm_rd, uint8_t arm_rm);

/**
 * Dispatch branch instruction to appropriate handler
 * @param code_buf Code buffer
 * @param insn Decoded x86 instruction
 * @param block_pc Current block PC
 * @param arm_rd Destination ARM64 register
 * @param arm_rm Source ARM64 register
 * @return 1 if block ends, 0 otherwise
 */
int dispatch_branch_insn(code_buffer_t *code_buf, const x86_insn_t *insn,
                         uint64_t block_pc, uint8_t arm_rd, uint8_t arm_rm);

/**
 * Dispatch bit manipulation instruction to appropriate handler
 * @param code_buf Code buffer
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM64 register
 * @param arm_rm Source ARM64 register
 */
void dispatch_bit_insn(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm);

/**
 * Dispatch string instruction to appropriate handler
 * @param code_buf Code buffer
 * @param insn Decoded x86 instruction
 */
void dispatch_string_insn(code_buffer_t *code_buf, const x86_insn_t *insn);

/**
 * Dispatch special instruction to appropriate handler
 * @param code_buf Code buffer
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM64 register
 * @param arm_rm Source ARM64 register
 */
void dispatch_special_insn(code_buffer_t *code_buf, const x86_insn_t *insn,
                           uint8_t arm_rd, uint8_t arm_rm);

#endif /* ROSETTA_TRANS_DISPATCH_H */
