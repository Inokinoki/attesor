/* ============================================================================
 * Rosetta Binary Translator - GPR Instruction Emission Header
 * ============================================================================
 *
 * This header defines the interface for x86_64 GPR instruction emission.
 * ============================================================================ */

#ifndef ROSETTA_CODEGEN_GPR_H
#define ROSETTA_CODEGEN_GPR_H

#include "rosetta_types.h"
#include "rosetta_codegen_buf.h"

/* ============================================================================
 * General Purpose Register Instructions
 * ============================================================================ */

/**
 * emit_mov_reg_imm64 - Move 64-bit immediate to register
 * @buf: Code buffer
 * @dst: Destination register
 * @imm: Immediate value
 */
void emit_mov_reg_imm64(code_buffer_t *buf, u8 dst, u64 imm);

/**
 * emit_mov_reg_reg - Move register to register
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_mov_reg_reg(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_mov_mem_reg - Move register to memory
 * @buf: Code buffer
 * @dst_reg: Base register for memory address
 * @src_reg: Source register
 * @disp: Displacement
 */
void emit_mov_mem_reg(code_buffer_t *buf, u8 dst_reg, u8 src_reg, s32 disp);

/**
 * emit_mov_reg_mem - Move memory to register
 * @buf: Code buffer
 * @dst_reg: Destination register
 * @src_reg: Base register for memory address
 * @disp: Displacement
 */
void emit_mov_reg_mem(code_buffer_t *buf, u8 dst_reg, u8 src_reg, s32 disp);

/**
 * emit_add_reg_reg - Add register to register
 * @buf: Code buffer
 * @dst: Destination register (also accumulates)
 * @src: Source register
 */
void emit_add_reg_reg(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_add_reg_imm32 - Add 32-bit immediate to register
 * @buf: Code buffer
 * @dst: Destination register
 * @imm: Immediate value
 */
void emit_add_reg_imm32(code_buffer_t *buf, u8 dst, u32 imm);

/**
 * emit_sub_reg_reg - Subtract register from register
 * @buf: Code buffer
 * @dst: Destination register (also accumulates)
 * @src: Source register
 */
void emit_sub_reg_reg(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_sub_reg_imm32 - Subtract 32-bit immediate from register
 * @buf: Code buffer
 * @dst: Destination register
 * @imm: Immediate value
 */
void emit_sub_reg_imm32(code_buffer_t *buf, u8 dst, u32 imm);

/**
 * emit_and_reg_reg - AND register with register
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_and_reg_reg(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_and_reg_imm32 - AND register with 32-bit immediate
 * @buf: Code buffer
 * @dst: Destination register
 * @imm: Immediate value
 */
void emit_and_reg_imm32(code_buffer_t *buf, u8 dst, u32 imm);

/**
 * emit_orr_reg_reg - OR register with register
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_orr_reg_reg(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_orr_reg_imm32 - OR register with 32-bit immediate
 * @buf: Code buffer
 * @dst: Destination register
 * @imm: Immediate value
 */
void emit_orr_reg_imm32(code_buffer_t *buf, u8 dst, u32 imm);

/**
 * emit_xor_reg_reg - XOR register with register
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_xor_reg_reg(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_xor_reg_imm32 - XOR register with 32-bit immediate
 * @buf: Code buffer
 * @dst: Destination register
 * @imm: Immediate value
 */
void emit_xor_reg_imm32(code_buffer_t *buf, u8 dst, u32 imm);

/**
 * emit_mvn_reg_reg - Bitwise NOT (move negated) register
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_mvn_reg_reg(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_mul_reg - Multiply registers (dst = src1 * src2)
 * @buf: Code buffer
 * @dst: Destination register
 * @src1: First source register
 * @src2: Second source register
 */
void emit_mul_reg(code_buffer_t *buf, u8 dst, u8 src1, u8 src2);

/**
 * emit_div_reg - Divide by register (RDX:RAX / src)
 * @buf: Code buffer
 * @src: Divisor register
 */
void emit_div_reg(code_buffer_t *buf, u8 src);

/**
 * emit_cmp_reg_reg - Compare registers
 * @buf: Code buffer
 * @op1: First operand
 * @op2: Second operand
 */
void emit_cmp_reg_reg(code_buffer_t *buf, u8 op1, u8 op2);

/**
 * emit_cmp_reg_imm32 - Compare register with immediate
 * @buf: Code buffer
 * @reg: Register to compare
 * @imm: Immediate value
 */
void emit_cmp_reg_imm32(code_buffer_t *buf, u8 reg, u32 imm);

/**
 * emit_test_reg_reg - Test registers (AND without storing)
 * @buf: Code buffer
 * @op1: First operand
 * @op2: Second operand
 */
void emit_test_reg_reg(code_buffer_t *buf, u8 op1, u8 op2);

/**
 * emit_test_reg_imm32 - Test register with immediate
 * @buf: Code buffer
 * @reg: Register to test
 * @imm: Immediate value
 */
void emit_test_reg_imm32(code_buffer_t *buf, u8 reg, u32 imm);

/**
 * emit_lea_reg_disp - Load effective address
 * @buf: Code buffer
 * @dst: Destination register
 * @base: Base register
 * @disp: Displacement
 */
void emit_lea_reg_disp(code_buffer_t *buf, u8 dst, u8 base, s32 disp);

/**
 * emit_push_reg - Push register onto stack
 * @buf: Code buffer
 * @reg: Register to push
 */
void emit_push_reg(code_buffer_t *buf, u8 reg);

/**
 * emit_pop_reg - Pop register from stack
 * @buf: Code buffer
 * @reg: Register to pop into
 */
void emit_pop_reg(code_buffer_t *buf, u8 reg);

/* ============================================================================
 * Bit Manipulation Instructions
 * ============================================================================ */

/**
 * emit_bsf_reg - Bit scan forward
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_bsf_reg(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_bsr_reg - Bit scan reverse
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_bsr_reg(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_popcnt_reg - Population count
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_popcnt_reg(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_bt_reg - Bit test
 * @buf: Code buffer
 * @dst: Destination (unused)
 * @src: Source register
 * @bit: Bit position
 */
void emit_bt_reg(code_buffer_t *buf, u8 dst, u8 src, u8 bit);

/**
 * emit_bts_reg - Bit test and set
 * @buf: Code buffer
 * @dst: Destination (unused)
 * @src: Source register
 * @bit: Bit position
 */
void emit_bts_reg(code_buffer_t *buf, u8 dst, u8 src, u8 bit);

/**
 * emit_btr_reg - Bit test and reset
 * @buf: Code buffer
 * @dst: Destination (unused)
 * @src: Source register
 * @bit: Bit position
 */
void emit_btr_reg(code_buffer_t *buf, u8 dst, u8 src, u8 bit);

/**
 * emit_btc_reg - Bit test and complement
 * @buf: Code buffer
 * @dst: Destination (unused)
 * @src: Source register
 * @bit: Bit position
 */
void emit_btc_reg(code_buffer_t *buf, u8 dst, u8 src, u8 bit);

/* ============================================================================
 * Shift Instructions
 * ============================================================================ */

/**
 * emit_shl_reg_imm - Shift left logical
 * @buf: Code buffer
 * @dst: Destination register
 * @imm: Shift amount
 */
void emit_shl_reg_imm(code_buffer_t *buf, u8 dst, u8 imm);

/**
 * emit_shr_reg_imm - Shift right logical
 * @buf: Code buffer
 * @dst: Destination register
 * @imm: Shift amount
 */
void emit_shr_reg_imm(code_buffer_t *buf, u8 dst, u8 imm);

/**
 * emit_sar_reg_imm - Shift right arithmetic
 * @buf: Code buffer
 * @dst: Destination register
 * @imm: Shift amount
 */
void emit_sar_reg_imm(code_buffer_t *buf, u8 dst, u8 imm);

/**
 * emit_rol_reg_imm - Rotate left
 * @buf: Code buffer
 * @dst: Destination register
 * @imm: Rotate amount
 */
void emit_rol_reg_imm(code_buffer_t *buf, u8 dst, u8 imm);

/**
 * emit_ror_reg_imm - Rotate right
 * @buf: Code buffer
 * @dst: Destination register
 * @imm: Rotate amount
 */
void emit_ror_reg_imm(code_buffer_t *buf, u8 dst, u8 imm);

/**
 * emit_shld - Shift left double
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 * @shift: Shift amount
 */
void emit_shld(code_buffer_t *buf, u8 dst, u8 src, u8 shift);

/**
 * emit_shrd - Shift right double
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 * @shift: Shift amount
 */
void emit_shrd(code_buffer_t *buf, u8 dst, u8 src, u8 shift);

/* ============================================================================
 * Special Instructions
 * ============================================================================ */

/**
 * emit_cqo - Convert quadword to octword (sign-extend RAX to RDX:RAX)
 * @buf: Code buffer
 */
void emit_cqo(code_buffer_t *buf);

/**
 * emit_cli - Clear interrupt flag
 * @buf: Code buffer
 */
void emit_cli(code_buffer_t *buf);

/**
 * emit_sti - Set interrupt flag
 * @buf: Code buffer
 */
void emit_sti(code_buffer_t *buf);

/**
 * emit_cpuid - CPU identification
 * @buf: Code buffer
 */
void emit_cpuid(code_buffer_t *buf);

/**
 * emit_rdtsc - Read time-stamp counter
 * @buf: Code buffer
 */
void emit_rdtsc(code_buffer_t *buf);

#endif /* ROSETTA_CODEGEN_GPR_H */
