/* ============================================================================
 * Rosetta Binary Translator - Control Flow Instruction Emission Header
 * ============================================================================
 *
 * This header defines the interface for x86_64 control flow instruction emission.
 * ============================================================================ */

#ifndef ROSETTA_CODEGEN_CTRL_H
#define ROSETTA_CODEGEN_CTRL_H

#include "rosetta_types.h"
#include "rosetta_codegen_buf.h"
#include "rosetta_codegen_gpr.h"

/* ============================================================================
 * ARM64 Condition Codes (for mapping to x86)
 * ============================================================================ */

/* arm64_cond_t is defined in rosetta_types.h */

/* ============================================================================
 * Control Flow Instructions
 * ============================================================================ */

/**
 * emit_jmp_rel32 - Emit unconditional jump (relative 32-bit)
 * @buf: Code buffer
 * Returns: Offset of placeholder (for later patching)
 */
u32 emit_jmp_rel32(code_buffer_t *buf);

/**
 * emit_je_rel32 - Emit jump if equal (relative 32-bit)
 * @buf: Code buffer
 * Returns: Offset of placeholder
 */
u32 emit_je_rel32(code_buffer_t *buf);

/**
 * emit_jne_rel32 - Emit jump if not equal (relative 32-bit)
 * @buf: Code buffer
 * Returns: Offset of placeholder
 */
u32 emit_jne_rel32(code_buffer_t *buf);

/**
 * emit_jl_rel32 - Emit jump if less than (relative 32-bit)
 * @buf: Code buffer
 * Returns: Offset of placeholder
 */
u32 emit_jl_rel32(code_buffer_t *buf);

/**
 * emit_jge_rel32 - Emit jump if greater or equal (relative 32-bit)
 * @buf: Code buffer
 * Returns: Offset of placeholder
 */
u32 emit_jge_rel32(code_buffer_t *buf);

/**
 * emit_jle_rel32 - Emit jump if less or equal (relative 32-bit)
 * @buf: Code buffer
 * Returns: Offset of placeholder
 */
u32 emit_jle_rel32(code_buffer_t *buf);

/**
 * emit_jg_rel32 - Emit jump if greater than (relative 32-bit)
 * @buf: Code buffer
 * Returns: Offset of placeholder
 */
u32 emit_jg_rel32(code_buffer_t *buf);

/**
 * emit_jae_rel32 - Emit jump if above or equal (relative 32-bit)
 * @buf: Code buffer
 * Returns: Offset of placeholder
 */
u32 emit_jae_rel32(code_buffer_t *buf);

/**
 * emit_jb_rel32 - Emit jump if below (relative 32-bit)
 * @buf: Code buffer
 * Returns: Offset of placeholder
 */
u32 emit_jb_rel32(code_buffer_t *buf);

/**
 * emit_js_rel32 - Emit jump if sign (relative 32-bit)
 * @buf: Code buffer
 * Returns: Offset of placeholder
 */
u32 emit_js_rel32(code_buffer_t *buf);

/**
 * emit_jns_rel32 - Emit jump if no sign (relative 32-bit)
 * @buf: Code buffer
 * Returns: Offset of placeholder
 */
u32 emit_jns_rel32(code_buffer_t *buf);

/**
 * emit_jo_rel32 - Emit jump if overflow (relative 32-bit)
 * @buf: Code buffer
 * Returns: Offset of placeholder
 */
u32 emit_jo_rel32(code_buffer_t *buf);

/**
 * emit_jno_rel32 - Emit jump if no overflow (relative 32-bit)
 * @buf: Code buffer
 * Returns: Offset of placeholder
 */
u32 emit_jno_rel32(code_buffer_t *buf);

/**
 * emit_ja_rel32 - Emit jump if above (relative 32-bit)
 * @buf: Code buffer
 * Returns: Offset of placeholder
 */
u32 emit_ja_rel32(code_buffer_t *buf);

/**
 * emit_jbe_rel32 - Emit jump if below or equal (relative 32-bit)
 * @buf: Code buffer
 * Returns: Offset of placeholder
 */
u32 emit_jbe_rel32(code_buffer_t *buf);

/**
 * emit_patch_rel32 - Patch a relative 32-bit offset
 * @buf: Code buffer
 * @offset: Offset of placeholder
 * @target: Target address
 */
void emit_patch_rel32(code_buffer_t *buf, u32 offset, u32 target);

/**
 * emit_cond_branch - Emit conditional branch based on ARM64 condition
 * @buf: Code buffer
 * @cond: ARM64 condition code
 * Returns: Offset of placeholder (0 for ALways)
 */
u32 emit_cond_branch(code_buffer_t *buf, arm64_cond_t cond);

/**
 * emit_call_reg - Call register
 * @buf: Code buffer
 * @reg: Register containing target address
 */
void emit_call_reg(code_buffer_t *buf, u8 reg);

/**
 * emit_call_rel32 - Emit call (relative 32-bit)
 * @buf: Code buffer
 * Returns: Offset of placeholder
 */
u32 emit_call_rel32(code_buffer_t *buf);

/**
 * emit_ret - Emit return instruction
 * @buf: Code buffer
 */
void emit_ret(code_buffer_t *buf);

/**
 * emit_nop - Emit no-operation
 * @buf: Code buffer
 */
void emit_nop(code_buffer_t *buf);

/* ============================================================================
 * Flag Handling
 * ============================================================================ */

/**
 * emit_read_flags_to_nzcv - Read x86 flags and convert to ARM64 NZCV
 * @buf: Code buffer
 * @nzcv_reg: Register to store NZCV flags
 */
void emit_read_flags_to_nzcv(code_buffer_t *buf, u8 nzcv_reg);

/**
 * emit_update_flags_add - Update flags after ADD operation
 * @buf: Code buffer
 * @dst: Destination register
 * @op1: First operand
 * @op2: Second operand
 */
void emit_update_flags_add(code_buffer_t *buf, u8 dst, u8 op1, u8 op2);

#endif /* ROSETTA_CODEGEN_CTRL_H */
