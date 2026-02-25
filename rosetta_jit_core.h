/* ============================================================================
 * Rosetta Translator - JIT Core Header
 * ============================================================================
 *
 * Core JIT infrastructure, code buffer management, and ARM64 emission primitives.
 * Note: This header provides additional constants and helpers.
 *       Core functions are defined in rosetta_codegen.h/c.
 * ============================================================================ */

#ifndef ROSETTA_JIT_CORE_H
#define ROSETTA_JIT_CORE_H

#include "rosetta_codegen.h"

/* ============================================================================
 * ARM64 Register Constants (additional aliases)
 * ============================================================================ */

/* ARM64 General Purpose Registers */
#ifndef X0
#define X0  0
#endif
#ifndef X1
#define X1  1
#endif
#ifndef X2
#define X2  2
#endif
#ifndef X3
#define X3  3
#endif
#ifndef X4
#define X4  4
#endif
#ifndef X5
#define X5  5
#endif
#ifndef X6
#define X6  6
#endif
#ifndef X7
#define X7  7
#endif
#ifndef X8
#define X8  8
#endif
#ifndef X9
#define X9  9
#endif
#ifndef X10
#define X10 10
#endif
#ifndef X11
#define X11 11
#endif
#ifndef X12
#define X12 12
#endif
#ifndef X13
#define X13 13
#endif
#ifndef X14
#define X14 14
#endif
#ifndef X15
#define X15 15
#endif
#ifndef X16
#define X16 16
#endif
#ifndef X17
#define X17 17
#endif
#ifndef X18
#define X18 18
#endif
#ifndef X19
#define X19 19
#endif
#ifndef X20
#define X20 20
#endif
#ifndef X21
#define X21 21
#endif
#ifndef X22
#define X22 22
#endif
#ifndef X23
#define X23 23
#endif
#ifndef X24
#define X24 24
#endif
#ifndef X25
#define X25 25
#endif
#ifndef X26
#define X26 26
#endif
#ifndef X27
#define X27 27
#endif
#ifndef X28
#define X28 28
#endif
#ifndef X29
#define X29 29  /* FP */
#endif
#ifndef X30
#define X30 30  /* LR */
#endif
#ifndef X31
#define X31 31  /* SP/ZR */
#endif
#ifndef WZR
#define WZR 31  /* Zero register (32-bit) */
#endif
#ifndef XZR
#define XZR 31  /* Zero register (64-bit) */
#endif

/* ARM64 Condition Codes */
#ifndef ARM64_EQ
#define ARM64_EQ 0  /* Z == 1 */
#endif
#ifndef ARM64_NE
#define ARM64_NE 1  /* Z == 0 */
#endif
#ifndef ARM64_CS
#define ARM64_CS 2  /* C == 1 */
#endif
#ifndef ARM64_CC
#define ARM64_CC 3  /* C == 0 */
#endif
#ifndef ARM64_MI
#define ARM64_MI 4  /* N == 1 */
#endif
#ifndef ARM64_PL
#define ARM64_PL 5  /* N == 0 */
#endif
#ifndef ARM64_VS
#define ARM64_VS 6  /* V == 1 */
#endif
#ifndef ARM64_VC
#define ARM64_VC 7  /* V == 0 */
#endif
#ifndef ARM64_LT
#define ARM64_LT 8  /* N != V */
#endif
#ifndef ARM64_GE
#define ARM64_GE 9  /* N == V */
#endif
#ifndef ARM64_LE
#define ARM64_LE 10 /* Z || N != V */
#endif
#ifndef ARM64_GT
#define ARM64_GT 11 /* !Z && N == V */
#endif
#ifndef ARM64_AL
#define ARM64_AL 14 /* Always */
#endif
#ifndef ARM64_NV
#define ARM64_NV 15 /* Never */
#endif

/* x86_64 Register constants (only if not already defined) */
#ifndef X86_RAX
#define X86_RAX 0
#endif
#ifndef X86_RCX
#define X86_RCX 1
#endif
#ifndef X86_RDX
#define X86_RDX 2
#endif
#ifndef X86_RBX
#define X86_RBX 3
#endif
#ifndef X86_RSP
#define X86_RSP 4
#endif
#ifndef X86_RBP
#define X86_RBP 5
#endif
#ifndef X86_RSI
#define X86_RSI 6
#endif
#ifndef X86_RDI
#define X86_RDI 7
#endif
#ifndef X86_R8
#define X86_R8  8
#endif
#ifndef X86_R9
#define X86_R9  9
#endif
#ifndef X86_R10
#define X86_R10 10
#endif
#ifndef X86_R11
#define X86_R11 11
#endif
#ifndef X86_R12
#define X86_R12 12
#endif
#ifndef X86_R13
#define X86_R13 13
#endif
#ifndef X86_R14
#define X86_R14 14
#endif
#ifndef X86_R15
#define X86_R15 15
#endif

/* ============================================================================
 * Register Mapping
 * ============================================================================ */

/* Register mapping: x86_64 -> ARM64 */
static inline uint8_t map_x86_to_arm(uint8_t x86_reg)
{
    return x86_reg & 0x0F;  /* RAX-R15 -> X0-X15 */
}

/* ============================================================================
 * Basic ARM64 Instruction Emitters
 * ============================================================================ */

/* Data movement */
void emit_movz(code_buffer_t *buf, uint8_t dst, uint16_t imm, uint8_t shift);
void emit_movk(code_buffer_t *buf, uint8_t dst, uint16_t imm, uint8_t shift);
void emit_movn(code_buffer_t *buf, uint8_t dst, uint16_t imm, uint8_t shift);
void emit_mov_reg(code_buffer_t *buf, uint8_t dst, uint8_t src);

/* Arithmetic */
void emit_add_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_add_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint16_t imm);
void emit_sub_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_sub_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint16_t imm);
void emit_mul_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_sdiv_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_udiv_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_inc_reg(code_buffer_t *buf, uint8_t dst);
void emit_dec_reg(code_buffer_t *buf, uint8_t dst);

/* Logical */
void emit_and_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_and_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint16_t imm);
void emit_orr_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_eor_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_mvn_reg(code_buffer_t *buf, uint8_t dst, uint8_t src);
void emit_not_reg(code_buffer_t *buf, uint8_t dst, uint8_t src);

/* Compare */
void emit_cmp_reg(code_buffer_t *buf, uint8_t src1, uint8_t src2);
void emit_cmn_reg(code_buffer_t *buf, uint8_t src1, uint8_t src2);
void emit_tst_reg(code_buffer_t *buf, uint8_t src1, uint8_t src2);

/* Branch */
void emit_b(code_buffer_t *buf, int32_t imm26);
void emit_bl(code_buffer_t *buf, int32_t imm26);
void emit_br(code_buffer_t *buf, uint8_t src);
void emit_bcond(code_buffer_t *buf, uint8_t cond, int32_t imm19);
void emit_cbnz(code_buffer_t *buf, uint8_t src, int32_t imm19);
void emit_cbz(code_buffer_t *buf, uint8_t src, int32_t imm19);
void emit_tbz(code_buffer_t *buf, uint8_t src, uint8_t bit, int32_t imm14);
void emit_tbnz(code_buffer_t *buf, uint8_t src, uint8_t bit, int32_t imm14);

/* Load/Store */
void emit_ldr_imm(code_buffer_t *buf, uint8_t dst, uint8_t base, uint16_t imm);
void emit_str_imm(code_buffer_t *buf, uint8_t src, uint8_t base, uint16_t imm);
void emit_ldr_reg(code_buffer_t *buf, uint8_t dst, uint8_t base, uint8_t offset);
void emit_str_reg(code_buffer_t *buf, uint8_t src, uint8_t base, uint8_t offset);
void emit_ldp(code_buffer_t *buf, uint8_t dst1, uint8_t dst2, uint8_t base);
void emit_stp(code_buffer_t *buf, uint8_t src1, uint8_t src2, uint8_t base);

/* Misc */
void emit_adr(code_buffer_t *buf, uint8_t dst, int32_t imm21);
void emit_nop(code_buffer_t *buf);
void emit_ret(code_buffer_t *buf);
void emit_hlt(code_buffer_t *buf);

/* ============================================================================
 * Extended ARM64 Helpers
 * ============================================================================ */

/* Move with extend */
void emit_mov_extend(code_buffer_t *buf, uint8_t dst, uint8_t src, int is_signed, int is_16bit);
void emit_movz_ext(code_buffer_t *buf, uint8_t dst, uint8_t src);
void emit_movs_ext(code_buffer_t *buf, uint8_t dst, uint8_t src);

/* Shifts */
void emit_shl_reg_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift);
void emit_shr_reg_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift);
void emit_sar_reg_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift);
void emit_rol_reg_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift);
void emit_ror_reg_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift);

/* Conditional select */
void emit_csel_reg_reg_cond(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2, uint8_t cond);
void emit_setcc_reg_cond(code_buffer_t *buf, uint8_t dst, uint8_t cond);

/* Bit manipulation */
void emit_bsf_reg(code_buffer_t *buf, uint8_t dst, uint8_t src);
void emit_bsr_reg(code_buffer_t *buf, uint8_t dst, uint8_t src);
void emit_popcnt_reg(code_buffer_t *buf, uint8_t dst, uint8_t src);
void emit_bt_reg(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t bit_reg);
void emit_bts_reg(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t bit);
void emit_btr_reg(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t bit);
void emit_btc_reg(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t bit);

/* Stack */
void emit_push_reg(code_buffer_t *buf, uint8_t reg);
void emit_pop_reg(code_buffer_t *buf, uint8_t reg);

/* String operations */
void emit_movs(code_buffer_t *buf, int is_64bit);
void emit_stos(code_buffer_t *buf, int size);
void emit_lods(code_buffer_t *buf, int size);
void emit_cmps(code_buffer_t *buf, int size);
void emit_scas(code_buffer_t *buf, int size);

/* Special */
void emit_shld(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift);
void emit_shrd(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift);
void emit_cqo(code_buffer_t *buf);
void emit_cli(code_buffer_t *buf);
void emit_sti(code_buffer_t *buf);
void emit_cpuid(code_buffer_t *buf);
void emit_rdtsc(code_buffer_t *buf);

#endif /* ROSETTA_JIT_CORE_H */
