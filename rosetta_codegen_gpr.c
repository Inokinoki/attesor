/* ============================================================================
 * Rosetta Binary Translator - GPR Instruction Emission
 * ============================================================================
 *
 * This module contains x86_64 general purpose register instruction emission.
 * Each function emits machine code bytes for GPR operations.
 * ============================================================================ */

#include "rosetta_codegen_gpr.h"

/* ============================================================================
 * General Purpose Register Instructions
 * ============================================================================ */

void emit_mov_reg_imm64(code_buffer_t *buf, u8 dst, u64 imm) {
    /* MOV r64, imm64: 48 B8 iw */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x04;  /* REX.B */

    emit_byte(buf, rex);
    emit_byte(buf, 0xB8 + (dst & 7));
    emit_word64(buf, imm);
}

void emit_mov_reg_reg(code_buffer_t *buf, u8 dst, u8 src) {
    /* MOV r64, r64: 48 89 C0 + src*8 + dst */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x04;  /* REX.R */
    if (src >= 8) rex |= 0x01;  /* REX.B */

    emit_byte(buf, rex);
    emit_byte(buf, 0x89);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_mov_mem_reg(code_buffer_t *buf, u8 dst_reg, u8 src_reg, s32 disp) {
    /* MOV [reg + disp], src: 48 89 src, [dst_reg + disp] */
    u8 rex = 0x48;
    if (src_reg >= 8) rex |= 0x04;  /* REX.R */
    if (dst_reg >= 8) rex |= 0x01;  /* REX.B */

    if (rex != 0x48) emit_byte(buf, rex);
    emit_byte(buf, 0x89);

    /* ModR/M: mod=00 (no disp), reg=src, rm=dst */
    /* For simplicity, use disp32 encoding */
    emit_byte(buf, 0x80 + (src_reg & 7) + ((dst_reg & 7) << 3));
    emit_word32(buf, (u32)disp);
}

void emit_mov_reg_mem(code_buffer_t *buf, u8 dst_reg, u8 src_reg, s32 disp) {
    /* MOV dst, [reg + disp]: 48 8B dst, [src_reg + disp] */
    u8 rex = 0x48;
    if (dst_reg >= 8) rex |= 0x04;  /* REX.R */
    if (src_reg >= 8) rex |= 0x01;  /* REX.B */

    if (rex != 0x48) emit_byte(buf, rex);
    emit_byte(buf, 0x8B);
    emit_byte(buf, 0x80 + (dst_reg & 7) + ((src_reg & 7) << 3));
    emit_word32(buf, (u32)disp);
}

void emit_add_reg_reg(code_buffer_t *buf, u8 dst, u8 src) {
    /* ADD r64, r64: 48 01 C0 + src*8 + dst */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x01);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_add_reg_imm32(code_buffer_t *buf, u8 dst, u32 imm) {
    /* ADD r64, imm32: 48 81 C0 + dst iw */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x81);
    emit_byte(buf, 0xC0 + (dst & 7));
    emit_word32(buf, imm);
}

void emit_sub_reg_reg(code_buffer_t *buf, u8 dst, u8 src) {
    /* SUB r64, r64: 48 29 C0 + src*8 + dst */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x29);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_sub_reg_imm32(code_buffer_t *buf, u8 dst, u32 imm) {
    /* SUB r64, imm32: 48 81 E8 + dst iw */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x81);
    emit_byte(buf, 0xE8 + (dst & 7));
    emit_word32(buf, imm);
}

void emit_and_reg_reg(code_buffer_t *buf, u8 dst, u8 src) {
    /* AND r64, r64: 48 21 C0 + src*8 + dst */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x21);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_and_reg_imm32(code_buffer_t *buf, u8 dst, u32 imm) {
    /* AND r64, imm32: 48 81 E0 + dst iw */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x81);
    emit_byte(buf, 0xE0 + (dst & 7));
    emit_word32(buf, imm);
}

void emit_orr_reg_reg(code_buffer_t *buf, u8 dst, u8 src) {
    /* OR r64, r64: 48 09 C0 + src*8 + dst */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x09);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_orr_reg_imm32(code_buffer_t *buf, u8 dst, u32 imm) {
    /* OR r64, imm32: 48 81 C8 + dst iw */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x81);
    emit_byte(buf, 0xC8 + (dst & 7));
    emit_word32(buf, imm);
}

void emit_xor_reg_reg(code_buffer_t *buf, u8 dst, u8 src) {
    /* XOR r64, r64: 48 31 C0 + src*8 + dst */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x31);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_xor_reg_imm32(code_buffer_t *buf, u8 dst, u32 imm) {
    /* XOR r64, imm32: 48 81 F0 + dst iw */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x81);
    emit_byte(buf, 0xF0 + (dst & 7));
    emit_word32(buf, imm);
}

void emit_mvn_reg_reg(code_buffer_t *buf, u8 dst, u8 src) {
    /* NOT r64: 48 F7 D0 + dst (using NOT instruction) */
    /* First copy src to dst, then NOT dst */
    emit_mov_reg_reg(buf, dst, src);

    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0xF7);
    emit_byte(buf, 0xD0 + (dst & 7));
}

void emit_mul_reg(code_buffer_t *buf, u8 dst, u8 src1, u8 src2) {
    /* IMUL r64, r64, r64: 4C 0F AF C0 + src2*8 + src1 */
    u8 rex = 0x4C;  /* REX.W + REX.R */
    if (src1 >= 8) rex |= 0x01;  /* REX.B */
    if (src2 >= 8) rex |= 0x04;  /* REX.R (for src2 as reg field) */

    emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xAF);
    emit_byte(buf, 0xC0 + (src2 & 7) + ((src1 & 7) << 3));
    (void)dst;  /* dst = src1 for two-operand form */
}

void emit_div_reg(code_buffer_t *buf, u8 src) {
    /* IDIV src: 48 F7 F8 + src (divides RDX:RAX by src) */
    u8 rex = 0x48;
    if (src >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0xF7);
    emit_byte(buf, 0xF8 + (src & 7));
}

void emit_cmp_reg_reg(code_buffer_t *buf, u8 op1, u8 op2) {
    /* CMP r64, r64: 48 39 C0 + op2*8 + op1 */
    u8 rex = 0x48;
    if (op1 >= 8) rex |= 0x01;
    if (op2 >= 8) rex |= 0x04;

    emit_byte(buf, rex);
    emit_byte(buf, 0x39);
    emit_byte(buf, 0xC0 + (op2 & 7) + ((op1 & 7) << 3));
}

void emit_cmp_reg_imm32(code_buffer_t *buf, u8 reg, u32 imm) {
    /* CMP r64, imm32: 48 81 F8 + reg iw */
    u8 rex = 0x48;
    if (reg >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x81);
    emit_byte(buf, 0xF8 + (reg & 7));
    emit_word32(buf, imm);
}

void emit_test_reg_reg(code_buffer_t *buf, u8 op1, u8 op2) {
    /* TEST r64, r64: 48 85 C0 + op2*8 + op1 */
    u8 rex = 0x48;
    if (op1 >= 8) rex |= 0x01;
    if (op2 >= 8) rex |= 0x04;

    emit_byte(buf, rex);
    emit_byte(buf, 0x85);
    emit_byte(buf, 0xC0 + (op2 & 7) + ((op1 & 7) << 3));
}

void emit_test_reg_imm32(code_buffer_t *buf, u8 reg, u32 imm) {
    /* TEST r64, imm32: 48 F7 C0 + reg iw */
    u8 rex = 0x48;
    if (reg >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0xF7);
    emit_byte(buf, 0xC0 + (reg & 7));
    emit_word32(buf, imm);
}

void emit_lea_reg_disp(code_buffer_t *buf, u8 dst, u8 base, s32 disp) {
    /* LEA dst, [base + disp]: 48 8D 80 + dst*8 + base disp */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x04;
    if (base >= 8) rex |= 0x01;

    if (rex != 0x48) emit_byte(buf, rex);
    emit_byte(buf, 0x8D);
    emit_byte(buf, 0x80 + (dst & 7) + ((base & 7) << 3));
    emit_word32(buf, (u32)disp);
}

void emit_push_reg(code_buffer_t *buf, u8 reg) {
    /* PUSH r64: 50 + reg */
    u8 rex = 0x50;
    if (reg >= 8) rex |= 0x01;

    emit_byte(buf, rex + (reg & 7));
}

void emit_pop_reg(code_buffer_t *buf, u8 reg) {
    /* POP r64: 58 + reg */
    u8 rex = 0x58;
    if (reg >= 8) rex |= 0x01;

    emit_byte(buf, rex + (reg & 7));
}

/* ============================================================================
 * Bit Manipulation Instructions
 * ============================================================================ */

void emit_bsf_reg(code_buffer_t *buf, u8 dst, u8 src) {
    /* BSF dst, src: 48 0F BC C0 + src*8 + dst */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xBC);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_bsr_reg(code_buffer_t *buf, u8 dst, u8 src) {
    /* BSR dst, src: 48 0F BD C0 + src*8 + dst */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xBD);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_popcnt_reg(code_buffer_t *buf, u8 dst, u8 src) {
    /* POPCNT dst, src: 48 0F B8 C0 + src*8 + dst */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xB8);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_bt_reg(code_buffer_t *buf, u8 dst, u8 src, u8 bit) {
    /* BT src, imm8: 48 0F BA E0 + src imm (bit test) */
    u8 rex = 0x48;
    if (src >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xBA);
    emit_byte(buf, 0xE0 + (src & 7));
    emit_byte(buf, bit & 0x3F);
    (void)dst;
}

void emit_bts_reg(code_buffer_t *buf, u8 dst, u8 src, u8 bit) {
    /* BTS src, imm8: 48 0F BA F8 + src imm (bit set) */
    u8 rex = 0x48;
    if (src >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xBA);
    emit_byte(buf, 0xF8 + (src & 7));
    emit_byte(buf, bit & 0x3F);
    (void)dst;
}

void emit_btr_reg(code_buffer_t *buf, u8 dst, u8 src, u8 bit) {
    /* BTR src, imm8: 48 0F BA E8 + src imm (bit reset) */
    u8 rex = 0x48;
    if (src >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xBA);
    emit_byte(buf, 0xE8 + (src & 7));
    emit_byte(buf, bit & 0x3F);
    (void)dst;
}

void emit_btc_reg(code_buffer_t *buf, u8 dst, u8 src, u8 bit) {
    /* BTC src, imm8: 48 0F BA F0 + src imm (bit complement) */
    u8 rex = 0x48;
    if (src >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xBA);
    emit_byte(buf, 0xF0 + (src & 7));
    emit_byte(buf, bit & 0x3F);
    (void)dst;
}

/* ============================================================================
 * Shift Instructions
 * ============================================================================ */

void emit_shl_reg_imm(code_buffer_t *buf, u8 dst, u8 imm) {
    /* SHL r64, imm8: 48 C1 E0 + dst imm */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0xC1);
    emit_byte(buf, 0xE0 + (dst & 7));
    emit_byte(buf, imm);
}

void emit_shr_reg_imm(code_buffer_t *buf, u8 dst, u8 imm) {
    /* SHR r64, imm8: 48 C1 E8 + dst imm */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0xC1);
    emit_byte(buf, 0xE8 + (dst & 7));
    emit_byte(buf, imm);
}

void emit_sar_reg_imm(code_buffer_t *buf, u8 dst, u8 imm) {
    /* SAR r64, imm8: 48 C1 F8 + dst imm */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0xC1);
    emit_byte(buf, 0xF8 + (dst & 7));
    emit_byte(buf, imm);
}

void emit_rol_reg_imm(code_buffer_t *buf, u8 dst, u8 imm) {
    /* ROL r64, imm8: 48 C1 C0 + dst imm */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0xC1);
    emit_byte(buf, 0xC0 + (dst & 7));
    emit_byte(buf, imm);
}

void emit_ror_reg_imm(code_buffer_t *buf, u8 dst, u8 imm) {
    /* ROR r64, imm8: 48 C1 C8 + dst imm */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0xC1);
    emit_byte(buf, 0xC8 + (dst & 7));
    emit_byte(buf, imm);
}

void emit_shld(code_buffer_t *buf, u8 dst, u8 src, u8 shift) {
    /* SHLD dst, src, imm8: 48 0F A4 C0 + dst*8 + src imm */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xA4);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
    emit_byte(buf, shift);
}

void emit_shrd(code_buffer_t *buf, u8 dst, u8 src, u8 shift) {
    /* SHRD dst, src, imm8: 48 0F AC C0 + dst*8 + src imm */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xAC);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
    emit_byte(buf, shift);
}

/* ============================================================================
 * Special Instructions
 * ============================================================================ */

void emit_cqo(code_buffer_t *buf) {
    /* CQO: 48 99 (sign-extend RAX to RDX:RAX) */
    emit_byte(buf, 0x48);
    emit_byte(buf, 0x99);
}

void emit_cli(code_buffer_t *buf) {
    /* CLI: FA (clear interrupt flag) */
    emit_byte(buf, 0xFA);
}

void emit_sti(code_buffer_t *buf) {
    /* STI: FB (set interrupt flag) */
    emit_byte(buf, 0xFB);
}

void emit_cpuid(code_buffer_t *buf) {
    /* CPUID: 0F A2 */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xA2);
}

void emit_rdtsc(code_buffer_t *buf) {
    /* RDTSC: 0F 31 */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x31);
}
