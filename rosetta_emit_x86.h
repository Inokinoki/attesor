/* ============================================================================
 * Rosetta Translator - x86_64 Code Emission Header
 * ============================================================================
 *
 * This module provides x86_64 machine code emission functions for
 * binary translation. All functions emit raw x86_64 opcodes into
 * a code buffer.
 * ============================================================================ */

#ifndef ROSETTA_EMIT_X86_H
#define ROSETTA_EMIT_X86_H

#include <stdint.h>
#include <stddef.h>

/* Code buffer structure for emission */
typedef struct {
    uint8_t *buffer;
    size_t offset;
    size_t size;
} code_buf_t;

/* x86_64 register constants for code emission
 * Note: These are the actual register encoding numbers used in x86_64 opcodes
 * Use X86_RAX_X86_R15 from rosetta_types.h for general references */
#define EMIT_RAX  0
#define EMIT_RCX  1
#define EMIT_RDX  2
#define EMIT_RBX  3
#define EMIT_RSP  4
#define EMIT_RBP  5
#define EMIT_RSI  6
#define EMIT_RDI  7
#define EMIT_R8   8
#define EMIT_R9   9
#define EMIT_R10  10
#define EMIT_R11  11
#define EMIT_R12  12
#define EMIT_R13  13
#define EMIT_R14  14
#define EMIT_R15  15

/* ============================================================================
 * Code Buffer Operations
 * ============================================================================ */

/**
 * code_buf_init - Initialize code buffer
 * @buf: Code buffer to initialize
 * @buffer: Pre-allocated buffer
 * @size: Buffer size
 */
void code_buf_init(code_buf_t *buf, uint8_t *buffer, size_t size);

/**
 * code_buf_emit_byte - Emit a single byte
 * @buf: Code buffer
 * @byte: Byte to emit
 */
void code_buf_emit_byte(code_buf_t *buf, uint8_t byte);

/**
 * code_buf_emit_word32 - Emit a 32-bit word (little-endian)
 * @buf: Code buffer
 * @word: Word to emit
 */
void code_buf_emit_word32(code_buf_t *buf, uint32_t word);

/**
 * code_buf_emit_word64 - Emit a 64-bit word (little-endian)
 * @buf: Code buffer
 * @word: Word to emit
 */
void code_buf_emit_word64(code_buf_t *buf, uint64_t word);

/**
 * code_buf_get_size - Get current code size
 * @buf: Code buffer
 * Returns: Current offset (bytes emitted)
 */
size_t code_buf_get_size(code_buf_t *buf);

/**
 * code_buf_get_buffer - Get buffer pointer
 * @buf: Code buffer
 * Returns: Pointer to buffer start
 */
uint8_t *code_buf_get_buffer(code_buf_t *buf);

/* ============================================================================
 * x86_64 Code Emission - Data Movement
 * ============================================================================ */

/**
 * emit_x86_mov_reg_imm64 - Emit MOV reg64, imm64
 * @buf: Code buffer
 * @dst: Destination register
 * @imm: 64-bit immediate
 */
void emit_x86_mov_reg_imm64(code_buf_t *buf, uint8_t dst, uint64_t imm);

/**
 * emit_x86_mov_reg_imm32 - Emit MOV reg32, imm32
 * @buf: Code buffer
 * @dst: Destination register
 * @imm: 32-bit immediate
 */
void emit_x86_mov_reg_imm32(code_buf_t *buf, uint8_t dst, uint32_t imm);

/**
 * emit_x86_mov_reg_reg - Emit MOV reg64, reg64
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_x86_mov_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src);

/**
 * emit_x86_mov_reg_mem - Emit MOV reg64, [reg64 + disp]
 * @buf: Code buffer
 * @dst: Destination register
 * @base: Base register for address
 * @disp: Displacement (signed 32-bit)
 */
void emit_x86_mov_reg_mem(code_buf_t *buf, uint8_t dst, uint8_t base, int32_t disp);

/**
 * emit_x86_mov_mem_reg - Emit MOV [reg64 + disp], reg64
 * @buf: Code buffer
 * @base: Base register for address
 * @src: Source register
 * @disp: Displacement (signed 32-bit)
 */
void emit_x86_mov_mem_reg(code_buf_t *buf, uint8_t base, uint8_t src, int32_t disp);

/**
 * emit_x86_movsx_reg_mem8 - Emit MOVSX reg64, byte ptr [reg64]
 * @buf: Code buffer
 * @dst: Destination register
 * @base: Base register for address
 */
void emit_x86_movsx_reg_mem8(code_buf_t *buf, uint8_t dst, uint8_t base);

/**
 * emit_x86_movsx_reg_mem16 - Emit MOVSX reg64, word ptr [reg64]
 * @buf: Code buffer
 * @dst: Destination register
 * @base: Base register for address
 */
void emit_x86_movsx_reg_mem16(code_buf_t *buf, uint8_t dst, uint8_t base);

/**
 * emit_x86_movsxd_reg_reg32 - Emit MOVSXD reg64, reg32
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register (lower 32 bits)
 */
void emit_x86_movsxd_reg_reg32(code_buf_t *buf, uint8_t dst, uint8_t src);

/**
 * emit_x86_movsxd_reg_mem - Emit MOVSXD reg64, dword ptr [reg64 + disp]
 * @buf: Code buffer
 * @dst: Destination register
 * @base: Base register for address
 * @disp: Displacement offset
 */
void emit_x86_movsxd_reg_mem(code_buf_t *buf, uint8_t dst, uint8_t base, int32_t disp);

/**
 * emit_x86_movzx_reg_mem8 - Emit MOVZX reg64, byte ptr [reg64]
 * @buf: Code buffer
 * @dst: Destination register
 * @base: Base register for address
 */
void emit_x86_movzx_reg_mem8(code_buf_t *buf, uint8_t dst, uint8_t base);

/**
 * emit_x86_movzx_reg_mem16 - Emit MOVZX reg64, word ptr [reg64]
 * @buf: Code buffer
 * @dst: Destination register
 * @base: Base register for address
 */
void emit_x86_movzx_reg_mem16(code_buf_t *buf, uint8_t dst, uint8_t base);

/* ============================================================================
 * x86_64 Code Emission - Arithmetic
 * ============================================================================ */

/**
 * emit_x86_add_reg_reg - Emit ADD reg64, reg64
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_x86_add_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src);

/**
 * emit_x86_add_reg_imm32 - Emit ADD reg64, imm32
 * @buf: Code buffer
 * @dst: Destination register
 * @imm: 32-bit immediate
 */
void emit_x86_add_reg_imm32(code_buf_t *buf, uint8_t dst, uint32_t imm);

/**
 * emit_x86_sub_reg_reg - Emit SUB reg64, reg64
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_x86_sub_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src);

/**
 * emit_x86_sub_reg_imm32 - Emit SUB reg64, imm32
 * @buf: Code buffer
 * @dst: Destination register
 * @imm: 32-bit immediate
 */
void emit_x86_sub_reg_imm32(code_buf_t *buf, uint8_t dst, uint32_t imm);

/**
 * emit_x86_neg_reg - Emit NEG reg64
 * @buf: Code buffer
 * @reg: Register to negate
 */
void emit_x86_neg_reg(code_buf_t *buf, uint8_t reg);

/**
 * emit_x86_imul_reg_reg_reg - Emit IMUL reg64, reg64, reg64
 * @buf: Code buffer
 * @dst: Destination register
 * @src1: First source register
 * @src2: Second source register
 */
void emit_x86_imul_reg_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src1, uint8_t src2);

/**
 * emit_x86_imul_reg_reg_imm32 - Emit IMUL reg64, reg64, imm32
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 * @imm: 32-bit immediate
 */
void emit_x86_imul_reg_reg_imm32(code_buf_t *buf, uint8_t dst, uint8_t src, uint32_t imm);

/**
 * emit_x86_mul_reg - Emit MUL r64 (unsigned multiply, result in RDX:RAX)
 * @buf: Code buffer
 * @reg: Register to multiply by
 */
void emit_x86_mul_reg(code_buf_t *buf, uint8_t reg);

/**
 * emit_x86_div_reg - Emit DIV r64 (unsigned divide)
 * @buf: Code buffer
 * @reg: Register to divide by
 */
void emit_x86_div_reg(code_buf_t *buf, uint8_t reg);

/**
 * emit_x86_idiv_reg - Emit IDIV r64 (signed divide)
 * @buf: Code buffer
 * @reg: Register to divide by
 */
void emit_x86_idiv_reg(code_buf_t *buf, uint8_t reg);

/* ============================================================================
 * x86_64 Code Emission - Logical
 * ============================================================================ */

/**
 * emit_x86_and_reg_reg - Emit AND reg64, reg64
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_x86_and_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src);

/**
 * emit_x86_and_reg_imm32 - Emit AND reg64, imm32
 * @buf: Code buffer
 * @dst: Destination register
 * @imm: 32-bit immediate
 */
void emit_x86_and_reg_imm32(code_buf_t *buf, uint8_t dst, uint32_t imm);

/**
 * emit_x86_or_reg_reg - Emit OR reg64, reg64
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_x86_or_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src);

/**
 * emit_x86_or_reg_imm32 - Emit OR reg64, imm32
 * @buf: Code buffer
 * @dst: Destination register
 * @imm: 32-bit immediate
 */
void emit_x86_or_reg_imm32(code_buf_t *buf, uint8_t dst, uint32_t imm);

/**
 * emit_x86_xor_reg_reg - Emit XOR reg64, reg64
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_x86_xor_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src);

/**
 * emit_x86_xor_reg_imm32 - Emit XOR reg64, imm32
 * @buf: Code buffer
 * @dst: Destination register
 * @imm: 32-bit immediate
 */
void emit_x86_xor_reg_imm32(code_buf_t *buf, uint8_t dst, uint32_t imm);

/**
 * emit_x86_not_reg - Emit NOT reg64
 * @buf: Code buffer
 * @reg: Register to invert
 */
void emit_x86_not_reg(code_buf_t *buf, uint8_t reg);

/**
 * emit_x86_shl_reg_imm8 - Emit SHL reg64, imm8
 * @buf: Code buffer
 * @dst: Destination register
 * @shift: Shift amount
 */
void emit_x86_shl_reg_imm8(code_buf_t *buf, uint8_t dst, uint8_t shift);

/**
 * emit_x86_shr_reg_imm8 - Emit SHR reg64, imm8 (logical right shift)
 * @buf: Code buffer
 * @dst: Destination register
 * @shift: Shift amount
 */
void emit_x86_shr_reg_imm8(code_buf_t *buf, uint8_t dst, uint8_t shift);

/**
 * emit_x86_sar_reg_imm8 - Emit SAR reg64, imm8 (arithmetic right shift)
 * @buf: Code buffer
 * @dst: Destination register
 * @shift: Shift amount
 */
void emit_x86_sar_reg_imm8(code_buf_t *buf, uint8_t dst, uint8_t shift);

/**
 * emit_x86_rol_reg_imm8 - Emit ROL reg64, imm8
 * @buf: Code buffer
 * @dst: Destination register
 * @shift: Rotate amount
 */
void emit_x86_rol_reg_imm8(code_buf_t *buf, uint8_t dst, uint8_t shift);

/**
 * emit_x86_ror_reg_imm8 - Emit ROR reg64, imm8
 * @buf: Code buffer
 * @dst: Destination register
 * @shift: Rotate amount
 */
void emit_x86_ror_reg_imm8(code_buf_t *buf, uint8_t dst, uint8_t shift);

/**
 * emit_x86_shl_reg_cl - Emit SHL reg64, CL (shift by CL register)
 * @buf: Code buffer
 * @dst: Destination register
 */
void emit_x86_shl_reg_cl(code_buf_t *buf, uint8_t dst);

/**
 * emit_x86_shr_reg_cl - Emit SHR reg64, CL (logical right shift by CL)
 * @buf: Code buffer
 * @dst: Destination register
 */
void emit_x86_shr_reg_cl(code_buf_t *buf, uint8_t dst);

/**
 * emit_x86_sar_reg_cl - Emit SAR reg64, CL (arithmetic right shift by CL)
 * @buf: Code buffer
 * @dst: Destination register
 */
void emit_x86_sar_reg_cl(code_buf_t *buf, uint8_t dst);

/**
 * emit_x86_ror_reg_cl - Emit ROR reg64, CL (rotate right by CL)
 * @buf: Code buffer
 * @dst: Destination register
 */
void emit_x86_ror_reg_cl(code_buf_t *buf, uint8_t dst);

/* ============================================================================
 * x86_64 Code Emission - Comparison and Test
 * ============================================================================ */

/**
 * emit_x86_cmp_reg_reg - Emit CMP reg64, reg64
 * @buf: Code buffer
 * @op1: First operand
 * @op2: Second operand
 */
void emit_x86_cmp_reg_reg(code_buf_t *buf, uint8_t op1, uint8_t op2);

/**
 * emit_x86_cmp_reg_imm32 - Emit CMP reg64, imm32
 * @buf: Code buffer
 * @op: Register operand
 * @imm: Immediate operand
 */
void emit_x86_cmp_reg_imm32(code_buf_t *buf, uint8_t op, uint32_t imm);

/**
 * emit_x86_test_reg_reg - Emit TEST reg64, reg64
 * @buf: Code buffer
 * @op1: First operand
 * @op2: Second operand
 */
void emit_x86_test_reg_reg(code_buf_t *buf, uint8_t op1, uint8_t op2);

/**
 * emit_x86_test_reg_imm32 - Emit TEST reg64, imm32
 * @buf: Code buffer
 * @op: Register operand
 * @imm: Immediate operand
 */
void emit_x86_test_reg_imm32(code_buf_t *buf, uint8_t op, uint32_t imm);

/* ============================================================================
 * x86_64 Code Emission - Control Flow
 * ============================================================================ */

/**
 * emit_x86_jmp_reg - Emit JMP reg64
 * @buf: Code buffer
 * @reg: Register containing target address
 */
void emit_x86_jmp_reg(code_buf_t *buf, uint8_t reg);

/**
 * emit_x86_jmp_rel32 - Emit JMP rel32 (relative jump)
 * @buf: Code buffer
 * @target: Target address (for relative calculation)
 * @current: Current address (for relative calculation)
 */
void emit_x86_jmp_rel32(code_buf_t *buf, uint64_t target, uint64_t current);

/**
 * emit_x86_call_reg - Emit CALL reg64
 * @buf: Code buffer
 * @reg: Register containing target address
 */
void emit_x86_call_reg(code_buf_t *buf, uint8_t reg);

/**
 * emit_x86_call_rel32 - Emit CALL rel32 (relative call)
 * @buf: Code buffer
 * @target: Target address (for relative calculation)
 * @current: Current address (for relative calculation)
 */
void emit_x86_call_rel32(code_buf_t *buf, uint64_t target, uint64_t current);

/**
 * emit_x86_ret - Emit RET
 * @buf: Code buffer
 */
void emit_x86_ret(code_buf_t *buf);

/**
 * emit_x86_ret_imm16 - Emit RET imm16 (return with stack adjust)
 * @buf: Code buffer
 * @imm: Stack adjustment
 */
void emit_x86_ret_imm16(code_buf_t *buf, uint16_t imm);

/**
 * emit_x86_nop - Emit NOP
 * @buf: Code buffer
 */
void emit_x86_nop(code_buf_t *buf);

/**
 * emit_x86_int3 - Emit INT3 (breakpoint)
 * @buf: Code buffer
 */
void emit_x86_int3(code_buf_t *buf);

/* ============================================================================
 * x86_64 Code Emission - Conditional Jumps
 * ============================================================================ */

/**
 * emit_x86_je_rel32 - Emit JE/JZ rel32 (jump if equal/zero)
 * @buf: Code buffer
 * @target: Target address
 * @current: Current address
 */
void emit_x86_je_rel32(code_buf_t *buf, uint64_t target, uint64_t current);

/**
 * emit_x86_jne_rel32 - Emit JNE/JNZ rel32 (jump if not equal/not zero)
 * @buf: Code buffer
 * @target: Target address
 * @current: Current address
 */
void emit_x86_jne_rel32(code_buf_t *buf, uint64_t target, uint64_t current);

/**
 * emit_x86_jl_rel32 - Emit JL/JNGE rel32 (jump if less)
 * @buf: Code buffer
 * @target: Target address
 * @current: Current address
 */
void emit_x86_jl_rel32(code_buf_t *buf, uint64_t target, uint64_t current);

/**
 * emit_x86_jle_rel32 - Emit JLE/JNG rel32 (jump if less or equal)
 * @buf: Code buffer
 * @target: Target address
 * @current: Current address
 */
void emit_x86_jle_rel32(code_buf_t *buf, uint64_t target, uint64_t current);

/**
 * emit_x86_jg_rel32 - Emit JG/JNLE rel32 (jump if greater)
 * @buf: Code buffer
 * @target: Target address
 * @current: Current address
 */
void emit_x86_jg_rel32(code_buf_t *buf, uint64_t target, uint64_t current);

/**
 * emit_x86_jge_rel32 - Emit JGE/JNL rel32 (jump if greater or equal)
 * @buf: Code buffer
 * @target: Target address
 * @current: Current address
 */
void emit_x86_jge_rel32(code_buf_t *buf, uint64_t target, uint64_t current);

/**
 * emit_x86_ja_rel32 - Emit JA/JNBE rel32 (jump if above)
 * @buf: Code buffer
 * @target: Target address
 * @current: Current address
 */
void emit_x86_ja_rel32(code_buf_t *buf, uint64_t target, uint64_t current);

/**
 * emit_x86_jae_rel32 - Emit JAE/JNB rel32 (jump if above or equal)
 * @buf: Code buffer
 * @target: Target address
 * @current: Current address
 */
void emit_x86_jae_rel32(code_buf_t *buf, uint64_t target, uint64_t current);

/**
 * emit_x86_jb_rel32 - Emit JB/JNAE rel32 (jump if below)
 * @buf: Code buffer
 * @target: Target address
 * @current: Current address
 */
void emit_x86_jb_rel32(code_buf_t *buf, uint64_t target, uint64_t current);

/**
 * emit_x86_jbe_rel32 - Emit JBE/JNA rel32 (jump if below or equal)
 * @buf: Code buffer
 * @target: Target address
 * @current: Current address
 */
void emit_x86_jbe_rel32(code_buf_t *buf, uint64_t target, uint64_t current);

/**
 * emit_x86_js_rel32 - Emit JS rel32 (jump if sign)
 * @buf: Code buffer
 * @target: Target address
 * @current: Current address
 */
void emit_x86_js_rel32(code_buf_t *buf, uint64_t target, uint64_t current);

/**
 * emit_x86_jns_rel32 - Emit JNS rel32 (jump if not sign)
 * @buf: Code buffer
 * @target: Target address
 * @current: Current address
 */
void emit_x86_jns_rel32(code_buf_t *buf, uint64_t target, uint64_t current);

/**
 * emit_x86_jo_rel32 - Emit JO rel32 (jump if overflow)
 * @buf: Code buffer
 * @target: Target address
 * @current: Current address
 */
void emit_x86_jo_rel32(code_buf_t *buf, uint64_t target, uint64_t current);

/**
 * emit_x86_jno_rel32 - Emit JNO rel32 (jump if not overflow)
 * @buf: Code buffer
 * @target: Target address
 * @current: Current address
 */
void emit_x86_jno_rel32(code_buf_t *buf, uint64_t target, uint64_t current);

/* ============================================================================
 * x86_64 Code Emission - Conditional Move (CMOV)
 * ============================================================================ */

/**
 * emit_x86_cmove_reg_reg - Emit CMOVE/CMOVZ reg64, reg64
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_x86_cmove_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src);

/**
 * emit_x86_cmovne_reg_reg - Emit CMOVNE/CMOVNZ reg64, reg64
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_x86_cmovne_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src);

/**
 * emit_x86_cmovl_reg_reg - Emit CMOVL/CMOVNGE reg64, reg64
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_x86_cmovl_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src);

/**
 * emit_x86_cmovle_reg_reg - Emit CMOVLE/CMOVNG reg64, reg64
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_x86_cmovle_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src);

/**
 * emit_x86_cmovg_reg_reg - Emit CMOVG/CMOVNLE reg64, reg64
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_x86_cmovg_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src);

/**
 * emit_x86_cmovge_reg_reg - Emit CMOVGE/CMOVNL reg64, reg64
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_x86_cmovge_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src);

/**
 * emit_x86_cmova_reg_reg - Emit CMOVA/CMOVNBE reg64, reg64
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_x86_cmova_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src);

/**
 * emit_x86_cmovae_reg_reg - Emit CMOVAE/CMOVNB reg64, reg64
 * @buf: Code buffer
 * @dst: Destination register
 * @src: Source register
 */
void emit_x86_cmovae_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src);

/* ============================================================================
 * x86_64 Code Emission - Stack Operations
 * ============================================================================ */

/**
 * emit_x86_push_reg - Emit PUSH reg64
 * @buf: Code buffer
 * @reg: Register to push
 */
void emit_x86_push_reg(code_buf_t *buf, uint8_t reg);

/**
 * emit_x86_pop_reg - Emit POP reg64
 * @buf: Code buffer
 * @reg: Register to pop
 */
void emit_x86_pop_reg(code_buf_t *buf, uint8_t reg);

/**
 * emit_x86_pushf - Emit PUSHFQ (push flags)
 * @buf: Code buffer
 */
void emit_x86_pushf(code_buf_t *buf);

/**
 * emit_x86_popf - Emit POPFQ (pop flags)
 * @buf: Code buffer
 */
void emit_x86_popf(code_buf_t *buf);

/**
 * emit_x86_lea_reg_mem - Emit LEA reg64, [reg64 + disp]
 * @buf: Code buffer
 * @dst: Destination register
 * @base: Base register for address
 * @disp: Displacement (signed 32-bit)
 */
void emit_x86_lea_reg_mem(code_buf_t *buf, uint8_t dst, uint8_t base, int32_t disp);

/* ============================================================================
 * x86_64 Code Emission - Prefixes
 * ============================================================================ */

/**
 * emit_x86_rex - Emit REX prefix
 * @buf: Code buffer
 * @w: 1 for 64-bit operand size
 * @r: Extension of ModRM reg field
 * @x: Extension of SIB index field
 * @b: Extension of ModRM rm field or opcode reg field
 */
void emit_x86_rex(code_buf_t *buf, uint8_t w, uint8_t r, uint8_t x, uint8_t b);

/* ============================================================================
 * x86_64 Code Emission - Memory Store (byte/word)
 * ============================================================================ */

/**
 * emit_x86_mov_mem8_reg - Emit MOV byte ptr [reg64 + disp], reg8
 * @buf: Code buffer
 * @base: Base register for address
 * @src: Source register (low 8 bits used)
 * @disp: Displacement (signed 32-bit)
 */
void emit_x86_mov_mem8_reg(code_buf_t *buf, uint8_t base, uint8_t src, int32_t disp);

/**
 * emit_x86_mov_mem16_reg - Emit MOV word ptr [reg64 + disp], reg16
 * @buf: Code buffer
 * @base: Base register for address
 * @src: Source register (low 16 bits used)
 * @disp: Displacement (signed 32-bit)
 */
void emit_x86_mov_mem16_reg(code_buf_t *buf, uint8_t base, uint8_t src, int32_t disp);

/**
 * emit_x86_mov_mem_reg32 - Emit MOV dword ptr [reg64 + disp], reg32
 * @buf: Code buffer
 * @base: Base register for address
 * @src: Source register (low 32 bits used)
 * @disp: Displacement (signed 32-bit)
 */
void emit_x86_mov_mem_reg32(code_buf_t *buf, uint8_t base, uint8_t src, int32_t disp);

/* ============================================================================
 * x86_64 Code Emission - Load with Zero-Extend (byte/word with disp)
 * ============================================================================ */

/**
 * emit_x86_movzx_reg_mem8_disp - Emit MOVZX r64, byte ptr [r64 + disp]
 * @buf: Code buffer
 * @dst: Destination register
 * @base: Base register for address
 * @disp: Displacement (signed 32-bit)
 */
void emit_x86_movzx_reg_mem8_disp(code_buf_t *buf, uint8_t dst, uint8_t base, int32_t disp);

/**
 * emit_x86_movzx_reg_mem16_disp - Emit MOVZX r64, word ptr [r64 + disp]
 * @buf: Code buffer
 * @dst: Destination register
 * @base: Base register for address
 * @disp: Displacement (signed 32-bit)
 */
void emit_x86_movzx_reg_mem16_disp(code_buf_t *buf, uint8_t dst, uint8_t base, int32_t disp);

/**
 * emit_x86_movsx_reg_mem8_disp - Emit MOVSX r64, byte ptr [r64 + disp]
 * @buf: Code buffer
 * @dst: Destination register
 * @base: Base register for address
 * @disp: Displacement (signed 32-bit)
 */
void emit_x86_movsx_reg_mem8_disp(code_buf_t *buf, uint8_t dst, uint8_t base, int32_t disp);

/**
 * emit_x86_movsx_reg_mem16_disp - Emit MOVSX r64, word ptr [r64 + disp]
 * @buf: Code buffer
 * @dst: Destination register
 * @base: Base register for address
 * @disp: Displacement (signed 32-bit)
 */
void emit_x86_movsx_reg_mem16_disp(code_buf_t *buf, uint8_t dst, uint8_t base, int32_t disp);

#endif /* ROSETTA_EMIT_X86_H */
