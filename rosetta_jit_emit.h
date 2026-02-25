/* ============================================================================
 * Rosetta Translator - JIT Code Emitter
 * ============================================================================
 *
 * This module provides functions for emitting ARM64 instructions into a
 * code buffer for JIT compilation.
 * ============================================================================ */

#ifndef ROSETTA_JIT_EMIT_H
#define ROSETTA_JIT_EMIT_H

#include "rosetta_refactored_types.h"

/* ============================================================================
 * ARM64 Register Constants
 * ============================================================================ */

/* ARM64 General Purpose Registers */
#define X0  0
#define X1  1
#define X2  2
#define X3  3
#define X4  4
#define X5  5
#define X6  6
#define X7  7
#define X8  8
#define X9  9
#define X10 10
#define X11 11
#define X12 12
#define X13 13
#define X14 14
#define X15 15
#define X16 16
#define X17 17
#define X18 18
#define X19 19
#define X20 20
#define X21 21
#define X22 22
#define X23 23
#define X24 24
#define X25 25
#define X26 26
#define X27 27
#define X28 28
#define X29 29  /* FP */
#define X30 30  /* LR */
#define X31 31  /* SP/ZR */
#define WZR 31  /* Zero register (32-bit) */
#define XZR 31  /* Zero register (64-bit) */

/* ARM64 Condition Codes */
#define ARM64_EQ 0  /* Z == 1 */
#define ARM64_NE 1  /* Z == 0 */
#define ARM64_CS 2  /* C == 1 */
#define ARM64_CC 3  /* C == 0 */
#define ARM64_MI 4  /* N == 1 */
#define ARM64_PL 5  /* N == 0 */
#define ARM64_VS 6  /* V == 1 */
#define ARM64_VC 7  /* V == 0 */
#define ARM64_LT 8  /* N != V */
#define ARM64_GE 9  /* N == V */
#define ARM64_LE 10 /* Z || N != V */
#define ARM64_GT 11 /* !Z && N == V */
#define ARM64_AL 14 /* Always */
#define ARM64_NV 15 /* Never */

/* ============================================================================
 * Code Buffer Management
 * ============================================================================ */

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

/**
 * Emit a single byte
 * @param buf Code buffer
 * @param byte Byte to emit
 */
void emit_byte(CodeBuffer *buf, uint8_t byte);

/* Alias for emit_byte - used by translation modules */
#define jit_emit_byte(b) emit_byte(NULL, (b))

/* Map ARM64 register to x86_64 register - simple identity mapping for now */
static inline uint8_t map_arm64_to_x86_gpr(uint8_t arm64_reg)
{
    /* Simple mapping: ARM64 X0-X7 -> x86_64 RAX-RCX (simplified) */
    static const uint8_t reg_map[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    return (arm64_reg < 16) ? reg_map[arm64_reg] : 0;
}

/* JIT emit helpers - inline x86_64 code emission */
static inline void jit_emit_mov_reg_imm(uint8_t reg, uint64_t imm)
{
    /* MOV r64, imm64: 48 B8 iw rd - simplified */
    (void)reg;
    (void)imm;
    /* Placeholder - actual implementation would emit x86_64 MOV */
}

static inline void jit_emit_mov_reg_mem64(uint8_t reg, uint64_t addr)
{
    /* MOV r64, [imm64]: 48 8B /0 - simplified */
    (void)reg;
    (void)addr;
    /* Placeholder - actual implementation would emit x86_64 MOV */
}

static inline void jit_emit_mem64_mov_reg(uint64_t addr, uint8_t reg)
{
    /* MOV [imm64], r64: 48 89 /0 - simplified */
    (void)addr;
    (void)reg;
    /* Placeholder - actual implementation would emit x86_64 MOV */
}

static inline void jit_emit_stmxcsr(uint8_t reg)
{
    /* STMXCSR [reg]: F3 0F AE /3 */
    (void)reg;
    /* Placeholder */
}

static inline void jit_emit_ldmxcsr(uint8_t reg)
{
    /* LDMXCSR [reg]: F3 0F AE /2 */
    (void)reg;
    /* Placeholder */
}

/**
 * Emit a 32-bit word
 * @param buf Code buffer
 * @param word Word to emit
 */
void emit_word32(CodeBuffer *buf, uint32_t word);

/**
 * Emit a 64-bit word
 * @param buf Code buffer
 * @param word Word to emit
 */
void emit_word64(CodeBuffer *buf, uint64_t word);

/**
 * Emit ARM64 instruction (32-bit, little-endian) - inline helper
 * @param buf Code buffer
 * @param insn ARM64 instruction to emit
 */
static inline void emit_arm64_insn(CodeBuffer *buf, uint32_t insn)
{
    emit_byte(buf, (insn >> 0) & 0xFF);
    emit_byte(buf, (insn >> 8) & 0xFF);
    emit_byte(buf, (insn >> 16) & 0xFF);
    emit_byte(buf, (insn >> 24) & 0xFF);
}

/* ============================================================================
 * ARM64 Instruction Emitters - Data Processing
 * ============================================================================ */

/**
 * Emit MOVZ Xd, #imm16, LSL #shift
 * @param buf Code buffer
 * @param dst Destination register (0-30)
 * @param imm 16-bit immediate
 * @param shift Shift amount (0, 1, 2, or 3 for LSL #0/16/32/48)
 */
void emit_movz(CodeBuffer *buf, uint8_t dst, uint16_t imm, uint8_t shift);

/**
 * Emit MOVK Xd, #imm16, LSL #shift
 * @param buf Code buffer
 * @param dst Destination register (0-30)
 * @param imm 16-bit immediate
 * @param shift Shift amount (0, 1, 2, or 3)
 */
void emit_movk(CodeBuffer *buf, uint8_t dst, uint16_t imm, uint8_t shift);

/**
 * Emit MOVN Xd, #imm16, LSL #shift
 * @param buf Code buffer
 * @param dst Destination register (0-30)
 * @param imm 16-bit immediate
 * @param shift Shift amount (0, 1, 2, or 3)
 */
void emit_movn(CodeBuffer *buf, uint8_t dst, uint16_t imm, uint8_t shift);

/**
 * Emit ADD Xd, Xn, Xm
 * @param buf Code buffer
 * @param dst Destination register
 * @param src1 First source register
 * @param src2 Second source register
 */
void emit_add_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2);

/**
 * Emit ADD Xd, Xn, #imm12
 * @param buf Code buffer
 * @param dst Destination register
 * @param src Source register
 * @param imm 12-bit immediate (0-4095)
 */
void emit_add_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint16_t imm);

/**
 * Emit SUB Xd, Xn, Xm
 * @param buf Code buffer
 * @param dst Destination register
 * @param src1 First source register
 * @param src2 Second register (subtracted)
 */
void emit_sub_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2);

/**
 * Emit SUB Xd, Xn, #imm12
 * @param buf Code buffer
 * @param dst Destination register
 * @param src Source register
 * @param imm 12-bit immediate
 */
void emit_sub_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint16_t imm);

/**
 * Emit AND Xd, Xn, Xm
 * @param buf Code buffer
 * @param dst Destination register
 * @param src1 First source register
 * @param src2 Second source register
 */
void emit_and_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2);

/**
 * Emit AND Xd, Xn, #imm
 * @param buf Code buffer
 * @param dst Destination register
 * @param src Source register
 * @param imm Immediate value
 */
void emit_and_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint16_t imm);

/**
 * Emit ORR Xd, Xn, Xm
 * @param buf Code buffer
 * @param dst Destination register
 * @param src1 First source register
 * @param src2 Second source register
 */
void emit_orr_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2);

/**
 * Emit EOR Xd, Xn, Xm
 * @param buf Code buffer
 * @param dst Destination register
 * @param src1 First source register
 * @param src2 Second source register
 */
void emit_eor_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2);

/**
 * Emit MVN Xd, Xm
 * @param buf Code buffer
 * @param dst Destination register
 * @param src Source register
 */
void emit_mvn_reg(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit MOV Xd, Xm (alias of ORR Xd, XZR, Xm)
 * @param buf Code buffer
 * @param dst Destination register
 * @param src Source register
 */
void emit_mov_reg(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit MUL Xd, Xn, Xm (lower 64 bits of multiply)
 * @param buf Code buffer
 * @param dst Destination register
 * @param src1 First source register
 * @param src2 Second source register
 */
void emit_mul_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2);

/**
 * Emit SDIV Xd, Xn, Xm (signed divide)
 * @param buf Code buffer
 * @param dst Destination register
 * @param src1 Dividend register
 * @param src2 Divisor register
 */
void emit_sdiv_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2);

/**
 * Emit UDIV Xd, Xn, Xm (unsigned divide)
 * @param buf Code buffer
 * @param dst Destination register
 * @param src1 Dividend register
 * @param src2 Divisor register
 */
void emit_udiv_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2);

/* ============================================================================
 * ARM64 Instruction Emitters - Compare and Test
 * ============================================================================ */

/**
 * Emit CMP Xn, Xm (alias of SUBS XZR, Xn, Xm)
 * @param buf Code buffer
 * @param src1 First operand
 * @param src2 Second operand
 */
void emit_cmp_reg(CodeBuffer *buf, uint8_t src1, uint8_t src2);

/**
 * Emit CMN Xn, Xm (alias of ADDS XZR, Xn, Xm)
 * @param buf Code buffer
 * @param src1 First operand
 * @param src2 Second operand
 */
void emit_cmn_reg(CodeBuffer *buf, uint8_t src1, uint8_t src2);

/**
 * Emit TST Xn, Xm (alias of ANDS XZR, Xn, Xm)
 * @param buf Code buffer
 * @param src1 First operand
 * @param src2 Second operand
 */
void emit_tst_reg(CodeBuffer *buf, uint8_t src1, uint8_t src2);

/* ============================================================================
 * ARM64 Instruction Emitters - Branch and Control Flow
 * ============================================================================ */

/**
 * Emit B label (unconditional branch)
 * @param buf Code buffer
 * @param imm26 26-bit signed immediate (relative offset / 4)
 */
void emit_b(CodeBuffer *buf, int32_t imm26);

/**
 * Emit BL label (branch with link)
 * @param buf Code buffer
 * @param imm26 26-bit signed immediate
 */
void emit_bl(CodeBuffer *buf, int32_t imm26);

/**
 * Emit BR Xm (branch to register)
 * @param buf Code buffer
 * @param src Source register
 */
void emit_br(CodeBuffer *buf, uint8_t src);

/**
 * Emit B.cond label (conditional branch)
 * @param buf Code buffer
 * @param cond Condition code (0-15)
 * @param imm19 19-bit signed immediate (relative offset / 4)
 */
void emit_bcond(CodeBuffer *buf, uint8_t cond, int32_t imm19);

/**
 * Emit CBNZ Xt, label (compare and branch if not zero)
 * @param buf Code buffer
 * @param src Source register
 * @param imm19 19-bit immediate
 */
void emit_cbnz(CodeBuffer *buf, uint8_t src, int32_t imm19);

/**
 * Emit CBZ Xt, label (compare and branch if zero)
 * @param buf Code buffer
 * @param src Source register
 * @param imm19 19-bit immediate
 */
void emit_cbz(CodeBuffer *buf, uint8_t src, int32_t imm19);

/**
 * Emit TBZ Xt, #bit, label (test bit and branch if zero)
 * @param buf Code buffer
 * @param src Source register
 * @param bit Bit position to test
 * @param imm14 14-bit immediate
 */
void emit_tbz(CodeBuffer *buf, uint8_t src, uint8_t bit, int32_t imm14);

/**
 * Emit TBNZ Xt, #bit, label (test bit and branch if not zero)
 * @param buf Code buffer
 * @param src Source register
 * @param bit Bit position to test
 * @param imm14 14-bit immediate
 */
void emit_tbnz(CodeBuffer *buf, uint8_t src, uint8_t bit, int32_t imm14);

/* ============================================================================
 * ARM64 Instruction Emitters - Load/Store
 * ============================================================================ */

/**
 * Emit LDR Xt, [Xn + #imm] (load register, immediate offset)
 * @param buf Code buffer
 * @param dst Destination register
 * @param base Base address register
 * @param imm 12-bit immediate offset
 */
void emit_ldr_imm(CodeBuffer *buf, uint8_t dst, uint8_t base, uint16_t imm);

/**
 * Emit STR Xt, [Xn + #imm] (store register, immediate offset)
 * @param buf Code buffer
 * @param src Source register
 * @param base Base address register
 * @param imm 12-bit immediate offset
 */
void emit_str_imm(CodeBuffer *buf, uint8_t src, uint8_t base, uint16_t imm);

/**
 * Emit LDR Xt, [Xn, Xm] (load register, register offset)
 * @param buf Code buffer
 * @param dst Destination register
 * @param base Base address register
 * @param offset Offset register
 */
void emit_ldr_reg(CodeBuffer *buf, uint8_t dst, uint8_t base, uint8_t offset);

/**
 * Emit STR Xt, [Xn, Xm] (store register, register offset)
 * @param buf Code buffer
 * @param src Source register
 * @param base Base address register
 * @param offset Offset register
 */
void emit_str_reg(CodeBuffer *buf, uint8_t src, uint8_t base, uint8_t offset);

/**
 * Emit LDP Xt, Xt2, [Xn] (load pair)
 * @param buf Code buffer
 * @param dst1 First destination register
 * @param dst2 Second destination register
 * @param base Base address register
 */
void emit_ldp(CodeBuffer *buf, uint8_t dst1, uint8_t dst2, uint8_t base);

/**
 * Emit STP Xt, Xt2, [Xn] (store pair)
 * @param buf Code buffer
 * @param src1 First source register
 * @param src2 Second source register
 * @param base Base address register
 */
void emit_stp(CodeBuffer *buf, uint8_t src1, uint8_t src2, uint8_t base);

/**
 * Emit ADR Xd, label (PC-relative address)
 * @param buf Code buffer
 * @param dst Destination register
 * @param imm21 21-bit immediate (PC-relative offset)
 */
void emit_adr(CodeBuffer *buf, uint8_t dst, int32_t imm21);

/* ============================================================================
 * ARM64 Instruction Emitters - Miscellaneous
 * ============================================================================ */

/**
 * Emit NOP (no operation)
 */
void emit_nop(CodeBuffer *buf);

/**
 * Emit RET (return to LR)
 */
void emit_ret(CodeBuffer *buf);

/**
 * Emit HLT #0 (halt - for debugging / unimplemented)
 */
void emit_hlt(CodeBuffer *buf);

/* ============================================================================
 * ARM64 Instruction Emitters - x86_64 Translation Helpers
 * ============================================================================ */

/**
 * Emit MOVZXT/MOVSXT - Move with zero/sign extend
 * @param buf Code buffer
 * @param dst Destination register
 * @param src Source register
 * @param is_signed 1 for sign-extend, 0 for zero-extend
 * @param is_16bit 1 for 16-bit source, 0 for 8-bit source
 */
void emit_mov_extend(CodeBuffer *buf, uint8_t dst, uint8_t src, int is_signed, int is_16bit);

/**
 * Emit zero-extend 8-bit to 64-bit
 */
void emit_movz_ext(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit sign-extend 8-bit to 64-bit
 */
void emit_movs_ext(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit INC register (using ADD immediate)
 */
void emit_inc_reg(CodeBuffer *buf, uint8_t dst);

/**
 * Emit DEC register (using SUB immediate)
 */
void emit_dec_reg(CodeBuffer *buf, uint8_t dst);

/**
 * Emit NEG register (using SUBS from zero)
 */
void emit_neg_reg(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit NOT register (using MVN)
 */
void emit_not_reg(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PUSH register (decrement SP and store)
 */
void emit_push_reg(CodeBuffer *buf, uint8_t reg);

/**
 * Emit POP register (load and increment SP)
 */
void emit_pop_reg(CodeBuffer *buf, uint8_t reg);

/**
 * Emit SHL register by immediate
 */
void emit_shl_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift);

/**
 * Emit SHR register by immediate (logical shift right)
 */
void emit_shr_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift);

/**
 * Emit SAR register by immediate (arithmetic shift right)
 */
void emit_sar_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift);

/**
 * Emit ROL register by immediate
 */
void emit_rol_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift);

/**
 * Emit ROR register by immediate
 */
void emit_ror_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift);

/**
 * Emit CSEL Xd, Xn, Xm, cond (conditional select)
 */
void emit_csel_reg_reg_cond(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2, uint8_t cond);

/**
 * Emit SETCC - Set register based on condition
 */
void emit_setcc_reg_cond(CodeBuffer *buf, uint8_t dst, uint8_t cond);

/* ============================================================================
 * ARM64 Instruction Emitters - Bit Manipulation
 * ============================================================================ */

/**
 * Emit BSF register (bit scan forward)
 */
void emit_bsf_reg(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit BSR register (bit scan reverse)
 */
void emit_bsr_reg(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit POPCNT register (population count)
 */
void emit_popcnt_reg(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit BT register (bit test)
 */
void emit_bt_reg(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t bit_reg);

/**
 * Emit BTS register (bit test and set)
 */
void emit_bts_reg(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t bit);

/**
 * Emit BTR register (bit test and reset)
 */
void emit_btr_reg(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t bit);

/**
 * Emit BTC register (bit test and complement)
 */
void emit_btc_reg(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t bit);

/* ============================================================================
 * ARM64 Instruction Emitters - String Operations
 * ============================================================================ */

/**
 * Emit MOVS (string move)
 */
void emit_movs(CodeBuffer *buf, int is_64bit);

/**
 * Emit STOS (string store)
 */
void emit_stos(CodeBuffer *buf, int size);

/**
 * Emit LODS (string load)
 */
void emit_lods(CodeBuffer *buf, int size);

/**
 * Emit CMPS (string compare)
 */
void emit_cmps(CodeBuffer *buf, int size);

/**
 * Emit SCAS (string scan)
 */
void emit_scas(CodeBuffer *buf, int size);

/* ============================================================================
 * ARM64 Instruction Emitters - Special Instructions
 * ============================================================================ */

/**
 * Emit SHLD (double precision shift left)
 */
void emit_shld(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift);

/**
 * Emit SHRD (double precision shift right)
 */
void emit_shrd(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift);

/**
 * Emit CQO (sign extend RAX to RDX:RAX)
 */
void emit_cqo(CodeBuffer *buf);

/**
 * Emit CLI (clear interrupt flag)
 */
void emit_cli(CodeBuffer *buf);

/**
 * Emit STI (set interrupt flag)
 */
void emit_sti(CodeBuffer *buf);

/**
 * Emit CPUID (stub)
 */
void emit_cpuid(CodeBuffer *buf);

/**
 * Emit RDTSC (read timestamp counter)
 */
void emit_rdtsc(CodeBuffer *buf);

#endif /* ROSETTA_JIT_EMIT_H */
