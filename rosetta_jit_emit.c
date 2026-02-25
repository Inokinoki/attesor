/* ============================================================================
 * Rosetta Translator - JIT Code Emitter Implementation
 * ============================================================================
 *
 * This module implements ARM64 instruction emission for JIT compilation.
 * ============================================================================ */

#include "rosetta_jit_emit.h"
#include <string.h>

/* Global code cache for JIT emission */
static uint8_t g_code_cache[1024 * 1024];  /* 1MB code cache */
static size_t g_code_cache_offset = 0;

/* ============================================================================
 * Code Buffer Management
 * ============================================================================ */

void code_buffer_init(CodeBuffer *buf, uint8_t *buffer_ptr, size_t size)
{
    buf->buffer = buffer_ptr ? buffer_ptr : g_code_cache + g_code_cache_offset;
    buf->size = size;
    buf->offset = 0;
    buf->error = 0;
}

size_t code_buffer_get_size(CodeBuffer *buf)
{
    return buf->offset;
}

void emit_byte(CodeBuffer *buf, uint8_t byte)
{
    if (buf->offset >= buf->size) {
        buf->error = 1;
        return;
    }
    buf->buffer[buf->offset++] = byte;
}

void emit_word32(CodeBuffer *buf, uint32_t word)
{
    emit_byte(buf, (word >> 0) & 0xFF);
    emit_byte(buf, (word >> 8) & 0xFF);
    emit_byte(buf, (word >> 16) & 0xFF);
    emit_byte(buf, (word >> 24) & 0xFF);
}

void emit_word64(CodeBuffer *buf, uint64_t word)
{
    emit_byte(buf, (word >> 0) & 0xFF);
    emit_byte(buf, (word >> 8) & 0xFF);
    emit_byte(buf, (word >> 16) & 0xFF);
    emit_byte(buf, (word >> 24) & 0xFF);
    emit_byte(buf, (word >> 32) & 0xFF);
    emit_byte(buf, (word >> 40) & 0xFF);
    emit_byte(buf, (word >> 48) & 0xFF);
    emit_byte(buf, (word >> 56) & 0xFF);
}

/* ============================================================================
 * ARM64 Instruction Emitters - Data Processing
 * ============================================================================ */

void emit_movz(CodeBuffer *buf, uint8_t dst, uint16_t imm, uint8_t shift)
{
    uint32_t insn = 0xD2800000;  /* MOVZ base */
    insn |= (uint32_t)imm;
    insn |= (uint32_t)(shift & 3) << 21;  /* hw */
    insn |= (uint32_t)(dst & 31) << 0;   /* Rd */
    emit_arm64_insn(buf, insn);
}

void emit_movk(CodeBuffer *buf, uint8_t dst, uint16_t imm, uint8_t shift)
{
    uint32_t insn = 0xF2800000;  /* MOVK base */
    insn |= (uint32_t)imm;
    insn |= (uint32_t)(shift & 3) << 21;
    insn |= (uint32_t)(dst & 31) << 0;
    emit_arm64_insn(buf, insn);
}

void emit_movn(CodeBuffer *buf, uint8_t dst, uint16_t imm, uint8_t shift)
{
    uint32_t insn = 0x92800000;  /* MOVN base */
    insn |= (uint32_t)imm;
    insn |= (uint32_t)(shift & 3) << 21;
    insn |= (uint32_t)(dst & 31) << 0;
    emit_arm64_insn(buf, insn);
}

void emit_add_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x0B000000;  /* ADD (shifted) */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_add_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint16_t imm)
{
    uint32_t insn = 0x91000000;  /* ADD immediate */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src & 31) << 5;
    insn |= (uint32_t)(imm & 0xFFF) << 10;
    emit_arm64_insn(buf, insn);
}

void emit_sub_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x4B000000;  /* SUB (shifted) */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_sub_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint16_t imm)
{
    uint32_t insn = 0xD1000000;  /* SUB immediate */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src & 31) << 5;
    insn |= (uint32_t)(imm & 0xFFF) << 10;
    emit_arm64_insn(buf, insn);
}

void emit_and_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x0A000000;  /* AND (shifted) */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_and_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint16_t imm)
{
    if (imm <= 255) {
        uint32_t insn = 0x12000000;  /* ANDI */
        insn |= (uint32_t)(dst & 31) << 0;
        insn |= (uint32_t)(src & 31) << 5;
        insn |= (uint32_t)(imm & 0xFFF) << 10;
        emit_arm64_insn(buf, insn);
    } else {
        uint8_t tmp = 16;
        emit_movz(buf, tmp, imm, 0);
        emit_and_reg(buf, dst, src, tmp);
    }
}

void emit_orr_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x2A000000;  /* ORR (shifted) */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_eor_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x4A000000;  /* EOR (shifted) */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_mvn_reg(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    uint32_t insn = 0x2A2003E0;  /* MVN (shifted) */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src & 31) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_mov_reg(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_orr_reg(buf, dst, XZR, src);
}

void emit_mul_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x9B000000;  /* MUL */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_sdiv_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x9AC00000;  /* SDIV */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_udiv_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x9AC00400;  /* UDIV */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

/* ============================================================================
 * ARM64 Instruction Emitters - Compare and Test
 * ============================================================================ */

void emit_cmp_reg(CodeBuffer *buf, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0xEB00001F;  /* SUBS XZR, Xn, Xm */
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_cmn_reg(CodeBuffer *buf, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x2B00001F;  /* ADDS XZR, Xn, Xm */
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_tst_reg(CodeBuffer *buf, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x6A00001F;  /* ANDS XZR, Xn, Xm */
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

/* ============================================================================
 * ARM64 Instruction Emitters - Branch and Control Flow
 * ============================================================================ */

void emit_b(CodeBuffer *buf, int32_t imm26)
{
    uint32_t insn = 0x14000000;  /* B */
    insn |= (uint32_t)(imm26 & 0x03FFFFFF);
    emit_arm64_insn(buf, insn);
}

void emit_bl(CodeBuffer *buf, int32_t imm26)
{
    uint32_t insn = 0x94000000;  /* BL */
    insn |= (uint32_t)(imm26 & 0x03FFFFFF);
    emit_arm64_insn(buf, insn);
}

void emit_br(CodeBuffer *buf, uint8_t src)
{
    uint32_t insn = 0xD61F0000;  /* BR */
    insn |= (uint32_t)(src & 31) << 5;
    emit_arm64_insn(buf, insn);
}

void emit_bcond(CodeBuffer *buf, uint8_t cond, int32_t imm19)
{
    uint32_t insn = 0x54000000;  /* B.cond */
    insn |= (uint32_t)(cond & 0x0F) << 0;
    insn |= (uint32_t)(imm19 & 0x07FFFF) << 5;
    emit_arm64_insn(buf, insn);
}

void emit_cbnz(CodeBuffer *buf, uint8_t src, int32_t imm19)
{
    uint32_t insn = 0xB5000000;  /* CBNZ */
    insn |= (uint32_t)(src & 31) << 5;
    insn |= (uint32_t)(imm19 & 0x07FFFF) << 5;
    emit_arm64_insn(buf, insn);
}

void emit_cbz(CodeBuffer *buf, uint8_t src, int32_t imm19)
{
    uint32_t insn = 0xB4000000;  /* CBZ */
    insn |= (uint32_t)(src & 31) << 5;
    insn |= (uint32_t)(imm19 & 0x07FFFF) << 5;
    emit_arm64_insn(buf, insn);
}

void emit_tbz(CodeBuffer *buf, uint8_t src, uint8_t bit, int32_t imm14)
{
    uint32_t insn = 0x36000000;  /* TBZ */
    insn |= (uint32_t)(src & 31) << 5;
    insn |= (uint32_t)(bit & 0x3F) << 19;
    insn |= (uint32_t)(imm14 & 0x3FFF) << 5;
    emit_arm64_insn(buf, insn);
}

void emit_tbnz(CodeBuffer *buf, uint8_t src, uint8_t bit, int32_t imm14)
{
    uint32_t insn = 0x37000000;  /* TBNZ */
    insn |= (uint32_t)(src & 31) << 5;
    insn |= (uint32_t)(bit & 0x3F) << 19;
    insn |= (uint32_t)(imm14 & 0x3FFF) << 5;
    emit_arm64_insn(buf, insn);
}

/* ============================================================================
 * ARM64 Instruction Emitters - Load/Store
 * ============================================================================ */

void emit_ldr_imm(CodeBuffer *buf, uint8_t dst, uint8_t base, uint16_t imm)
{
    uint32_t insn = 0xF9400000;  /* LDR (immediate, scaled) */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(base & 31) << 5;
    insn |= (uint32_t)((imm >> 3) & 0xFFF) << 10;
    emit_arm64_insn(buf, insn);
}

void emit_str_imm(CodeBuffer *buf, uint8_t src, uint8_t base, uint16_t imm)
{
    uint32_t insn = 0xF9000000;  /* STR (immediate, scaled) */
    insn |= (uint32_t)(src & 31) << 0;
    insn |= (uint32_t)(base & 31) << 5;
    insn |= (uint32_t)((imm >> 3) & 0xFFF) << 10;
    emit_arm64_insn(buf, insn);
}

void emit_ldr_reg(CodeBuffer *buf, uint8_t dst, uint8_t base, uint8_t offset)
{
    uint32_t insn = 0xF8606800;  /* LDR (register) */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(base & 31) << 5;
    insn |= (uint32_t)(offset & 31) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_str_reg(CodeBuffer *buf, uint8_t src, uint8_t base, uint8_t offset)
{
    uint32_t insn = 0xF8206800;  /* STR (register) */
    insn |= (uint32_t)(src & 31) << 0;
    insn |= (uint32_t)(base & 31) << 5;
    insn |= (uint32_t)(offset & 31) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_ldp(CodeBuffer *buf, uint8_t dst1, uint8_t dst2, uint8_t base)
{
    uint32_t insn = 0x29400000;  /* LDP (post-index, offset 0) */
    insn |= (uint32_t)(dst1 & 31) << 0;
    insn |= (uint32_t)(dst2 & 31) << 10;
    insn |= (uint32_t)(base & 31) << 5;
    emit_arm64_insn(buf, insn);
}

void emit_stp(CodeBuffer *buf, uint8_t src1, uint8_t src2, uint8_t base)
{
    uint32_t insn = 0x29000000;  /* STP (post-index, offset 0) */
    insn |= (uint32_t)(src1 & 31) << 0;
    insn |= (uint32_t)(src2 & 31) << 10;
    insn |= (uint32_t)(base & 31) << 5;
    emit_arm64_insn(buf, insn);
}

void emit_adr(CodeBuffer *buf, uint8_t dst, int32_t imm21)
{
    uint32_t insn = 0x10000000;  /* ADR */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(imm21 & 0x1FFFFF) << 5;
    emit_arm64_insn(buf, insn);
}

/* ============================================================================
 * ARM64 Instruction Emitters - Miscellaneous
 * ============================================================================ */

void emit_nop(CodeBuffer *buf)
{
    emit_arm64_insn(buf, 0xD503201F);  /* NOP */
}

void emit_ret(CodeBuffer *buf)
{
    emit_arm64_insn(buf, 0xD65F03C0);  /* RET X30 */
}

void emit_hlt(CodeBuffer *buf)
{
    emit_arm64_insn(buf, 0xD4200000);  /* HLT #0 */
}

/* ============================================================================
 * ARM64 Instruction Emitters - x86_64 Translation Helpers
 * ============================================================================ */

void emit_mov_extend(CodeBuffer *buf, uint8_t dst, uint8_t src, int is_signed, int is_16bit)
{
    if (is_16bit) {
        if (is_signed) {
            /* SXTH: sign-extend 16-bit to 64-bit */
            emit_arm64_insn(buf, 0x93407C00 | ((dst & 0x1F) << 0) | ((src & 0x1F) << 5));
        } else {
            /* UXTH: zero-extend 16-bit to 64-bit */
            emit_arm64_insn(buf, 0x53003C00 | ((dst & 0x1F) << 0) | ((src & 0x1F) << 5));
        }
    } else {
        if (is_signed) {
            /* SXTB: sign-extend 8-bit to 64-bit */
            emit_arm64_insn(buf, 0x93401C00 | ((dst & 0x1F) << 0) | ((src & 0x1F) << 5));
        } else {
            /* UXTB: zero-extend 8-bit to 64-bit */
            emit_arm64_insn(buf, 0x53001C00 | ((dst & 0x1F) << 0) | ((src & 0x1F) << 5));
        }
    }
}

void emit_movz_ext(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_mov_extend(buf, dst, src, 0, 0);
}

void emit_movs_ext(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_mov_extend(buf, dst, src, 1, 0);
}

void emit_inc_reg(CodeBuffer *buf, uint8_t dst)
{
    emit_add_imm(buf, dst, dst, 1);
}

void emit_dec_reg(CodeBuffer *buf, uint8_t dst)
{
    emit_sub_imm(buf, dst, dst, 1);
}

void emit_neg_reg(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* SUBS dst, XZR, src */
    emit_arm64_insn(buf, 0xEB000000 | ((dst & 0x1F) << 0) | (0x1F << 5) | ((src & 0x1F) << 16));
}

void emit_not_reg(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* MVN dst, src */
    emit_arm64_insn(buf, 0x9A000000 | ((dst & 0x1F) << 0) | (0x1F << 5) | ((src & 0x1F) << 16) | (0x3F << 10));
}

void emit_push_reg(CodeBuffer *buf, uint8_t reg)
{
    emit_sub_imm(buf, 31, 31, 8);  /* SUB SP, SP, #8 */
    emit_str_imm(buf, reg, 31, 0);  /* STR reg, [SP] */
}

void emit_pop_reg(CodeBuffer *buf, uint8_t reg)
{
    emit_ldr_imm(buf, reg, 31, 0);  /* LDR reg, [SP] */
    emit_add_imm(buf, 31, 31, 8);  /* ADD SP, SP, #8 */
}

void emit_shl_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift)
{
    /* LSL dst, src, #shift */
    emit_arm64_insn(buf, 0xD37FF000 | ((dst & 0x1F) << 0) | ((src & 0x1F) << 5) | ((shift & 0x3F) << 10));
}

void emit_shr_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift)
{
    /* LSR dst, src, #shift */
    emit_arm64_insn(buf, 0xD35FF000 | ((dst & 0x1F) << 0) | ((src & 0x1F) << 5) | ((shift & 0x3F) << 10));
}

void emit_sar_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift)
{
    /* ASR dst, src, #shift */
    emit_arm64_insn(buf, 0xD34FF000 | ((dst & 0x1F) << 0) | ((src & 0x1F) << 5) | ((shift & 0x3F) << 10));
}

void emit_rol_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift)
{
    if (shift == 0) {
        emit_mov_reg(buf, dst, src);
    } else {
        uint8_t tmp = 16;
        emit_shr_reg_imm(buf, tmp, src, 64 - shift);
        emit_shl_reg_imm(buf, dst, src, shift);
        emit_orr_reg(buf, dst, dst, tmp);
    }
}

void emit_ror_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift)
{
    /* ARM64 ROR */
    emit_arm64_insn(buf, 0x93C00000 | ((dst & 0x1F) << 0) | ((src & 0x1F) << 5) | ((shift & 0x3F) << 10));
}

void emit_csel_reg_reg_cond(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2, uint8_t cond)
{
    /* CSEL dst, src1, src2, cond */
    emit_arm64_insn(buf, 0x9A800000 | ((dst & 0x1F) << 0) | ((src2 & 0x1F) << 5) | ((cond & 0xF) << 12) | ((src1 & 0x1F) << 16));
}

void emit_setcc_reg_cond(CodeBuffer *buf, uint8_t dst, uint8_t cond)
{
    /* CSET dst, cond -> CSINC dst, XZR, XZR, !cond */
    emit_arm64_insn(buf, 0x9A9F03E0 | ((dst & 0x1F) << 0) | ((cond ^ 1) << 12));
}
