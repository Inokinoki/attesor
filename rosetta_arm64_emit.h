/* ============================================================================
 * Rosetta ARM64 Code Emitter
 * ============================================================================
 *
 * This module handles emission of ARM64 instructions for the JIT compiler.
 * Each function emits a specific ARM64 instruction encoding.
 * ============================================================================ */

#ifndef ROSETTA_ARM64_EMIT_H
#define ROSETTA_ARM64_EMIT_H

#include "rosetta_types.h"
#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * Code Buffer Management
 * ============================================================================ */

/**
 * Code buffer for JIT emission
 */
typedef struct {
    uint8_t *buffer;        /* Pointer to code buffer */
    size_t size;            /* Total buffer size */
    size_t offset;          /* Current write offset */
    int error;              /* Error flag */
} CodeBuffer;

/**
 * Initialize code buffer
 * @param buf Code buffer to initialize
 * @param buffer_ptr Pre-allocated buffer or NULL for internal cache
 * @param size Buffer size
 */
void code_buffer_init(CodeBuffer *buf, uint8_t *buffer_ptr, size_t size);

/**
 * Get code size
 * @param buf Code buffer
 * @return Current code size
 */
size_t code_buffer_get_size(CodeBuffer *buf);

/* ============================================================================
 * Basic Emit Functions
 * ============================================================================ */

void emit_byte(CodeBuffer *buf, uint8_t byte);
void emit_word32(CodeBuffer *buf, uint32_t word);
void emit_word64(CodeBuffer *buf, uint64_t word);
void emit_arm64_insn(CodeBuffer *buf, uint32_t insn);

/* ============================================================================
 * Register Constants
 * ============================================================================ */

#define XZR 31  /* ARM64 zero register */
#define WZR 31  /* ARM64 zero register (32-bit) */

/* x86_64 to ARM64 register mapping */
uint8_t map_x86_to_arm(uint8_t x86_reg);
uint8_t x86_map_gpr(uint8_t arm64_reg);

/* ============================================================================
 * Data Processing (Register) - ALU Operations
 * ============================================================================ */

void emit_add_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_sub_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_and_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_and_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint16_t imm);
void emit_orr_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_eor_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_mov_reg(CodeBuffer *buf, uint8_t dst, uint8_t src);

/* ============================================================================
 * Data Processing (Immediate)
 * ============================================================================ */

void emit_add_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint16_t imm);
void emit_sub_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint16_t imm);
void emit_movz(CodeBuffer *buf, uint8_t dst, uint16_t imm, uint8_t shift);
void emit_movk(CodeBuffer *buf, uint8_t dst, uint16_t imm, uint8_t shift);
void emit_movn(CodeBuffer *buf, uint8_t dst, uint16_t imm, uint8_t shift);

/* ============================================================================
 * Multiply/Divide
 * ============================================================================ */

void emit_mul_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_sdiv_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_udiv_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2);

/* ============================================================================
 * Compare/Test
 * ============================================================================ */

void emit_cmp_reg(CodeBuffer *buf, uint8_t src1, uint8_t src2);
void emit_cmn_reg(CodeBuffer *buf, uint8_t src1, uint8_t src2);
void emit_tst_reg(CodeBuffer *buf, uint8_t src1, uint8_t src2);

/* ============================================================================
 * Branch Instructions
 * ============================================================================ */

void emit_b(CodeBuffer *buf, int32_t imm26);
void emit_bl(CodeBuffer *buf, int32_t imm26);
void emit_bcond(CodeBuffer *buf, uint8_t cond, int32_t imm19);
void emit_br(CodeBuffer *buf, uint8_t src);
void emit_ret(CodeBuffer *buf);
void emit_cbnz(CodeBuffer *buf, uint8_t src, int32_t imm19);
void emit_cbz(CodeBuffer *buf, uint8_t src, int32_t imm19);
void emit_tbz(CodeBuffer *buf, uint8_t src, uint8_t bit, int32_t imm14);
void emit_tbnz(CodeBuffer *buf, uint8_t src, uint8_t bit, int32_t imm14);

/* ============================================================================
 * Load/Store Instructions
 * ============================================================================ */

void emit_ldr_imm(CodeBuffer *buf, uint8_t dst, uint8_t base, uint16_t imm);
void emit_str_imm(CodeBuffer *buf, uint8_t src, uint8_t base, uint16_t imm);
void emit_ldr_reg(CodeBuffer *buf, uint8_t dst, uint8_t base, uint8_t offset);
void emit_str_reg(CodeBuffer *buf, uint8_t src, uint8_t base, uint8_t offset);
void emit_ldp(CodeBuffer *buf, uint8_t dst1, uint8_t dst2, uint8_t base);
void emit_stp(CodeBuffer *buf, uint8_t src1, uint8_t src2, uint8_t base);

/* ============================================================================
 * Address Calculation
 * ============================================================================ */

void emit_adr(CodeBuffer *buf, uint8_t dst, int32_t imm21);

/* ============================================================================
 * System Instructions
 * ============================================================================ */

void emit_nop(CodeBuffer *buf);
void emit_hlt(CodeBuffer *buf);

/* ============================================================================
 * Additional ARM64 Emit Helpers (P0-P4)
 * ============================================================================ */

/* Move with extend */
void emit_mov_extend(CodeBuffer *buf, uint8_t dst, uint8_t src, int is_signed, int is_16bit);
void emit_movz_ext(CodeBuffer *buf, uint8_t dst, uint8_t src);
void emit_movs_ext(CodeBuffer *buf, uint8_t dst, uint8_t src);

/* Increment/Decrement */
void emit_inc_reg(CodeBuffer *buf, uint8_t dst);
void emit_dec_reg(CodeBuffer *buf, uint8_t dst);

/* Negate/Not */
void emit_neg_reg(CodeBuffer *buf, uint8_t dst, uint8_t src);
void emit_not_reg(CodeBuffer *buf, uint8_t dst, uint8_t src);

/* Stack operations */
void emit_push_reg(CodeBuffer *buf, uint8_t reg);
void emit_pop_reg(CodeBuffer *buf, uint8_t reg);

/* Shift/Rotate */
void emit_shl_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift);
void emit_shr_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift);
void emit_sar_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift);
void emit_rol_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift);
void emit_ror_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift);

/* Conditional operations */
void emit_csel_reg_reg_cond(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2, uint8_t cond);
void emit_setcc_reg_cond(CodeBuffer *buf, uint8_t dst, uint8_t cond);

/* Bit manipulation */
void emit_bsf_reg(CodeBuffer *buf, uint8_t dst, uint8_t src);
void emit_bsr_reg(CodeBuffer *buf, uint8_t dst, uint8_t src);
void emit_popcnt_reg(CodeBuffer *buf, uint8_t dst, uint8_t src);
void emit_bt_reg(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t bit);
void emit_bts_reg(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t bit);
void emit_btr_reg(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t bit);
void emit_btc_reg(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t bit);

/* String operations */
void emit_movs(CodeBuffer *buf, int is_64bit);
void emit_stos(CodeBuffer *buf, int size);
void emit_lods(CodeBuffer *buf, int size);
void emit_cmps(CodeBuffer *buf, int size);
void emit_scas(CodeBuffer *buf, int size);

/* Special instructions */
void emit_shld(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift);
void emit_shrd(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift);
void emit_cqo(CodeBuffer *buf);
void emit_cli(CodeBuffer *buf);
void emit_sti(CodeBuffer *buf);
void emit_cpuid(CodeBuffer *buf);
void emit_rdtsc(CodeBuffer *buf);

#endif /* ROSETTA_ARM64_EMIT_H */
