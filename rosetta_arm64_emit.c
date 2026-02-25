/* ============================================================================
 * Rosetta ARM64 Code Emitter Implementation
 * ============================================================================
 *
 * This module implements ARM64 instruction emission for the JIT compiler.
 * Each function emits a specific ARM64 instruction encoding.
 * ============================================================================ */

#include "rosetta_arm64_emit.h"
#include <string.h>
#include <stdlib.h>

/* Global code cache for JIT emission */
static uint8_t g_code_cache[1024 * 1024];  /* 1MB code cache */
static size_t g_code_cache_offset = 0;

/* ============================================================================
 * Code Buffer Management
 * ============================================================================ */

void code_buffer_init_arm64(code_buffer_t *buf, u8 *buffer_ptr, u32 size)
{
    buf->buffer = buffer_ptr ? buffer_ptr : g_code_cache + g_code_cache_offset;
    buf->size = size;
    buf->offset = 0;
    buf->error = 0;
}

u32 code_buffer_get_size_arm64(code_buffer_t *buf)
{
    return (u32)buf->offset;
}

/* ============================================================================
 * Basic Emit Functions
 * ============================================================================ */

void emit_byte_arm64(code_buffer_t *buf, u8 byte)
{
    if (buf->offset >= buf->size) {
        buf->error = 1;
        return;
    }
    buf->buffer[buf->offset++] = byte;
}

void emit_word32_arm64(code_buffer_t *buf, u32 word)
{
    emit_byte_arm64(buf, (word >> 0) & 0xFF);
    emit_byte_arm64(buf, (word >> 8) & 0xFF);
    emit_byte_arm64(buf, (word >> 16) & 0xFF);
    emit_byte_arm64(buf, (word >> 24) & 0xFF);
}

void emit_arm64_insn(code_buffer_t *buf, u32 insn)
{
    emit_word32_arm64(buf, insn);
}

/* ============================================================================
 * Register Mapping
 * ============================================================================ */

uint8_t map_x86_to_arm(uint8_t x86_reg)
{
    /* Direct mapping: x86_64 R0-R15 -> ARM64 X0-X15 */
    static const uint8_t mapping[16] = {
        0,  /* RAX -> X0 */
        1,  /* RCX -> X1 */
        2,  /* RDX -> X2 */
        3,  /* RBX -> X3 */
        4,  /* RSP -> X4 */
        5,  /* RBP -> X5 */
        6,  /* RSI -> X6 */
        7,  /* RDI -> X7 */
        8,  /* R8  -> X8 */
        9,  /* R9  -> X9 */
        10, /* R10 -> X10 */
        11, /* R11 -> X11 */
        12, /* R12 -> X12 */
        13, /* R13 -> X13 */
        14, /* R14 -> X14 */
        15  /* R15 -> X15 */
    };
    return (x86_reg < 16) ? mapping[x86_reg] : x86_reg;
}

uint8_t x86_map_gpr(uint8_t arm64_reg)
{
    /* Inverse mapping: ARM64 X0-X15 -> x86_64 R0-R15 */
    return arm64_reg & 0xF;
}

/* ============================================================================
 * Data Processing (Register) - ALU Operations
 * ============================================================================ */

void emit_add_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    /* ADD Xd, Xn, Xm: 00001011000mmmmm000000nnnnnddddd */
    u32 insn = 0x0B000000;
    insn |= (dst & 0x1F) << 0;
    insn |= (src1 & 0x1F) << 5;
    insn |= (src2 & 0x1F) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_sub_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    /* SUB Xd, Xn, Xm: 11001011000mmmmm000000nnnnnddddd */
    u32 insn = 0xCB000000;
    insn |= (dst & 0x1F) << 0;
    insn |= (src1 & 0x1F) << 5;
    insn |= (src2 & 0x1F) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_and_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    /* AND Xd, Xn, Xm: 00001010000mmmmm000000nnnnnddddd */
    u32 insn = 0x0A000000;
    insn |= (dst & 0x1F) << 0;
    insn |= (src1 & 0x1F) << 5;
    insn |= (src2 & 0x1F) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_and_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint16_t imm)
{
    /* AND Xd, Xn, #imm: 0001001000000000000000nnnnnddddd */
    u32 insn = 0x12000000;
    insn |= (dst & 0x1F) << 0;
    insn |= (src & 0x1F) << 5;
    /* Encode immediate (simplified - full encoding requires more bits) */
    insn |= (imm & 0xFFF) << 10;
    emit_arm64_insn(buf, insn);
}

void emit_orr_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    /* ORR Xd, Xn, Xm: 00101010000mmmmm000000nnnnnddddd */
    u32 insn = 0x2A000000;
    insn |= (dst & 0x1F) << 0;
    insn |= (src1 & 0x1F) << 5;
    insn |= (src2 & 0x1F) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_eor_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    /* EOR Xd, Xn, Xm: 01001010000mmmmm000000nnnnnddddd */
    u32 insn = 0x4A000000;
    insn |= (dst & 0x1F) << 0;
    insn |= (src1 & 0x1F) << 5;
    insn |= (src2 & 0x1F) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_mov_reg(code_buffer_t *buf, uint8_t dst, uint8_t src)
{
    /* MOV Xd, Xm (alias of ORR Xd, XZR, Xm) */
    emit_orr_reg(buf, dst, 31, src);
}

/* ============================================================================
 * Data Processing (Immediate)
 * ============================================================================ */

void emit_add_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint16_t imm)
{
    /* ADD Xd, Xn, #imm12: 1001000100000000000000nnnnnddddd */
    u32 insn = 0x91000000;
    insn |= (dst & 0x1F) << 0;
    insn |= (src & 0x1F) << 5;
    insn |= (imm & 0xFFF) << 10;
    emit_arm64_insn(buf, insn);
}

void emit_sub_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint16_t imm)
{
    /* SUB Xd, Xn, #imm12: 1101000100000000000000nnnnnddddd */
    u32 insn = 0xD1000000;
    insn |= (dst & 0x1F) << 0;
    insn |= (src & 0x1F) << 5;
    insn |= (imm & 0xFFF) << 10;
    emit_arm64_insn(buf, insn);
}

void emit_movz(code_buffer_t *buf, uint8_t dst, uint16_t imm, uint8_t shift)
{
    /* MOVZ Xd, #imm16, LSL #shift: 110100101000000000000000000ddddd */
    u32 insn = 0xD2800000;
    insn |= (imm & 0xFFFF);
    insn |= (shift & 0x3) << 21;
    insn |= (dst & 0x1F) << 0;
    emit_arm64_insn(buf, insn);
}

void emit_movk(code_buffer_t *buf, uint8_t dst, uint16_t imm, uint8_t shift)
{
    /* MOVK Xd, #imm16, LSL #shift: 111100101000000000000000000ddddd */
    u32 insn = 0xF2800000;
    insn |= (imm & 0xFFFF);
    insn |= (shift & 0x3) << 21;
    insn |= (dst & 0x1F) << 0;
    emit_arm64_insn(buf, insn);
}

void emit_movn(code_buffer_t *buf, uint8_t dst, uint16_t imm, uint8_t shift)
{
    /* MOVN Xd, #imm16, LSL #shift: 100100101000000000000000000ddddd */
    u32 insn = 0x92800000;
    insn |= (imm & 0xFFFF);
    insn |= (shift & 0x3) << 21;
    insn |= (dst & 0x1F) << 0;
    emit_arm64_insn(buf, insn);
}

/* ============================================================================
 * Multiply/Divide
 * ============================================================================ */

void emit_mul_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    /* MUL Xd, Xn, Xm: 00011011000mmmmm000000nnnnnddddd */
    u32 insn = 0x1B000000;
    insn |= (dst & 0x1F) << 0;
    insn |= (src1 & 0x1F) << 5;
    insn |= (src2 & 0x1F) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_sdiv_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    /* SDIV Xd, Xn, Xm: 1001101011000000000000nnnnnddddd */
    u32 insn = 0x9AC00000;
    insn |= (dst & 0x1F) << 0;
    insn |= (src1 & 0x1F) << 5;
    insn |= (src2 & 0x1F) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_udiv_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    /* UDIV Xd, Xn, Xm: 1001101010000000000000nnnnnddddd */
    u32 insn = 0x9A800000;
    insn |= (dst & 0x1F) << 0;
    insn |= (src1 & 0x1F) << 5;
    insn |= (src2 & 0x1F) << 16;
    emit_arm64_insn(buf, insn);
}

/* ============================================================================
 * Compare/Test
 * ============================================================================ */

void emit_cmp_reg(code_buffer_t *buf, uint8_t src1, uint8_t src2)
{
    /* CMP Xn, Xm (alias of SUBS XZR, Xn, Xm) */
    u32 insn = 0xEB000000;
    insn |= (31 & 0x1F) << 0;   /* XZR */
    insn |= (src1 & 0x1F) << 5;
    insn |= (src2 & 0x1F) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_cmn_reg(code_buffer_t *buf, uint8_t src1, uint8_t src2)
{
    /* CMN Xn, Xm (alias of ADDS XZR, Xn, Xm) */
    u32 insn = 0xAB000000;
    insn |= (31 & 0x1F) << 0;   /* XZR */
    insn |= (src1 & 0x1F) << 5;
    insn |= (src2 & 0x1F) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_tst_reg(code_buffer_t *buf, uint8_t src1, uint8_t src2)
{
    /* TST Xn, Xm (alias of ANDS XZR, Xn, Xm) */
    u32 insn = 0xEA000000;
    insn |= (31 & 0x1F) << 0;   /* XZR */
    insn |= (src1 & 0x1F) << 5;
    insn |= (src2 & 0x1F) << 16;
    emit_arm64_insn(buf, insn);
}

/* ============================================================================
 * Branch Instructions
 * ============================================================================ */

void emit_b(code_buffer_t *buf, int32_t imm26)
{
    /* B rel: 000101iiiiiiiiiiiiiiiiiiiiiiiiiii */
    u32 insn = 0x14000000;
    insn |= (imm26 >> 2) & 0x03FFFFFF;
    emit_arm64_insn(buf, insn);
}

void emit_bl(code_buffer_t *buf, int32_t imm26)
{
    /* BL rel: 100101iiiiiiiiiiiiiiiiiiiiiiiiiii */
    u32 insn = 0x94000000;
    insn |= (imm26 >> 2) & 0x03FFFFFF;
    emit_arm64_insn(buf, insn);
}

void emit_bcond(code_buffer_t *buf, uint8_t cond, int32_t imm19)
{
    /* B.cond rel: 01010100iiiiiiiiiiiiiiiiiiiinnnnn0 */
    u32 insn = 0x54000000;
    insn |= (imm19 >> 2) << 5;
    insn |= (cond & 0xF) << 0;
    emit_arm64_insn(buf, insn);
}

void emit_br(code_buffer_t *buf, uint8_t src)
{
    /* BR Xm: 1101011000100000000000mmmmm00000 */
    u32 insn = 0xD61F0000;
    insn |= (src & 0x1F) << 5;
    emit_arm64_insn(buf, insn);
}

void emit_ret(code_buffer_t *buf)
{
    /* RET: 110101100010000000000011110000 */
    emit_arm64_insn(buf, 0xD65F03C0);
}

void emit_cbnz(code_buffer_t *buf, uint8_t src, int32_t imm19)
{
    /* CBNZ Xt, rel: 00110101iiiiiiiiiiiiiiiiittttt0 */
    u32 insn = 0x35000000;
    insn |= ((imm19 >> 2) & 0x7FFFF) << 5;
    insn |= (src & 0x1F) << 0;
    emit_arm64_insn(buf, insn);
}

void emit_cbz(code_buffer_t *buf, uint8_t src, int32_t imm19)
{
    /* CBZ Xt, rel: 00110100iiiiiiiiiiiiiiiiittttt0 */
    u32 insn = 0x34000000;
    insn |= ((imm19 >> 2) & 0x7FFFF) << 5;
    insn |= (src & 0x1F) << 0;
    emit_arm64_insn(buf, insn);
}

void emit_tbz(code_buffer_t *buf, uint8_t src, uint8_t bit, int32_t imm14)
{
    /* TBZ Xt, #bit, rel: 00110110iiiiiiiitttttbbbbb0 */
    u32 insn = 0x36000000;
    insn |= ((imm14 >> 2) & 0x3FFF) << 5;
    insn |= (src & 0x1F) << 0;
    insn |= (bit & 0x1F) << 19;
    insn |= ((bit >> 5) & 0x1) << 31;
    emit_arm64_insn(buf, insn);
}

void emit_tbnz(code_buffer_t *buf, uint8_t src, uint8_t bit, int32_t imm14)
{
    /* TBNZ Xt, #bit, rel: 00110111iiiiiiiitttttbbbbb0 */
    u32 insn = 0x37000000;
    insn |= ((imm14 >> 2) & 0x3FFF) << 5;
    insn |= (src & 0x1F) << 0;
    insn |= (bit & 0x1F) << 19;
    insn |= ((bit >> 5) & 0x1) << 31;
    emit_arm64_insn(buf, insn);
}

/* ============================================================================
 * Load/Store Instructions
 * ============================================================================ */

void emit_ldr_imm(code_buffer_t *buf, uint8_t dst, uint8_t base, uint16_t imm)
{
    /* LDR Xt, [Xn, #imm]: 1111100001000000000000nnnnnddddd */
    u32 insn = 0xF8400000;
    insn |= (dst & 0x1F) << 0;
    insn |= (base & 0x1F) << 5;
    insn |= ((imm >> 3) & 0x1FF) << 10;
    emit_arm64_insn(buf, insn);
}

void emit_str_imm(code_buffer_t *buf, uint8_t src, uint8_t base, uint16_t imm)
{
    /* STR Xt, [Xn, #imm]: 1111100000000000000000nnnnnddddd */
    u32 insn = 0xF8000000;
    insn |= (src & 0x1F) << 0;
    insn |= (base & 0x1F) << 5;
    insn |= ((imm >> 3) & 0x1FF) << 10;
    emit_arm64_insn(buf, insn);
}

void emit_ldr_reg(code_buffer_t *buf, uint8_t dst, uint8_t base, uint8_t offset)
{
    /* LDR Xt, [Xn, Xm]: 11111000010mmmmm000000nnnnnddddd */
    u32 insn = 0xF8400000;
    insn |= (dst & 0x1F) << 0;
    insn |= (base & 0x1F) << 5;
    insn |= (offset & 0x1F) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_str_reg(code_buffer_t *buf, uint8_t src, uint8_t base, uint8_t offset)
{
    /* STR Xt, [Xn, Xm]: 11111000000mmmmm000000nnnnnddddd */
    u32 insn = 0xF8000000;
    insn |= (src & 0x1F) << 0;
    insn |= (base & 0x1F) << 5;
    insn |= (offset & 0x1F) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_ldp(code_buffer_t *buf, uint8_t dst1, uint8_t dst2, uint8_t base)
{
    /* LDP Xt1, Xt2, [Xn]: 0010100101000000000000nnnnnddddd */
    u32 insn = 0x29400000;
    insn |= (dst1 & 0x1F) << 0;
    insn |= (dst2 & 0x1F) << 10;
    insn |= (base & 0x1F) << 5;
    emit_arm64_insn(buf, insn);
}

void emit_stp(code_buffer_t *buf, uint8_t src1, uint8_t src2, uint8_t base)
{
    /* STP Xt1, Xt2, [Xn]: 0010100001000000000000nnnnnddddd */
    u32 insn = 0x29000000;
    insn |= (src1 & 0x1F) << 0;
    insn |= (src2 & 0x1F) << 10;
    insn |= (base & 0x1F) << 5;
    emit_arm64_insn(buf, insn);
}

/* ============================================================================
 * Address Calculation
 * ============================================================================ */

void emit_adr(code_buffer_t *buf, uint8_t dst, int32_t imm21)
{
    /* ADR Xd, rel: 00010000iiiiiiiiiiiiiiiiiiiiiiiii */
    u32 insn = 0x10000000;
    insn |= (dst & 0x1F) << 0;
    insn |= (imm21 & 0x1FFFFF) << 5;
    emit_arm64_insn(buf, insn);
}

/* ============================================================================
 * System Instructions
 * ============================================================================ */

void emit_nop(code_buffer_t *buf)
{
    /* NOP: 11010101001100100011110000000000 */
    emit_arm64_insn(buf, 0xD503201F);
}

void emit_hlt(code_buffer_t *buf)
{
    /* HLT #0: 11010100010000000000000000000000 */
    emit_arm64_insn(buf, 0xD4000000);
}

/* ============================================================================
 * Additional ARM64 Emit Helpers
 * ============================================================================ */

void emit_mov_extend(code_buffer_t *buf, uint8_t dst, uint8_t src, int is_signed, int is_16bit)
{
    if (is_16bit) {
        if (is_signed) {
            /* SXTH: sign-extend 16-bit to 64-bit */
            u32 insn = 0x93407C00;
            insn |= (dst & 0x1F) << 0;
            insn |= (src & 0x1F) << 5;
            emit_arm64_insn(buf, insn);
        } else {
            /* UXTH: zero-extend 16-bit to 64-bit */
            u32 insn = 0x53003C00;
            insn |= (dst & 0x1F) << 0;
            insn |= (src & 0x1F) << 5;
            emit_arm64_insn(buf, insn);
        }
    } else {
        if (is_signed) {
            /* SXTB: sign-extend 8-bit to 64-bit */
            u32 insn = 0x93401C00;
            insn |= (dst & 0x1F) << 0;
            insn |= (src & 0x1F) << 5;
            emit_arm64_insn(buf, insn);
        } else {
            /* UXTB: zero-extend 8-bit to 64-bit */
            u32 insn = 0x53001C00;
            insn |= (dst & 0x1F) << 0;
            insn |= (src & 0x1F) << 5;
            emit_arm64_insn(buf, insn);
        }
    }
}

void emit_movz_ext(code_buffer_t *buf, uint8_t dst, uint8_t src)
{
    emit_mov_extend(buf, dst, src, 0, 0);
}

void emit_movs_ext(code_buffer_t *buf, uint8_t dst, uint8_t src)
{
    emit_mov_extend(buf, dst, src, 1, 0);
}

void emit_inc_reg(code_buffer_t *buf, uint8_t dst)
{
    emit_add_imm(buf, dst, dst, 1);
}

void emit_dec_reg(code_buffer_t *buf, uint8_t dst)
{
    emit_sub_imm(buf, dst, dst, 1);
}

void emit_neg_reg(code_buffer_t *buf, uint8_t dst, uint8_t src)
{
    /* SUBS dst, XZR, src */
    u32 insn = 0xEB000000;
    insn |= (dst & 0x1F) << 0;
    insn |= (31 & 0x1F) << 5;
    insn |= (src & 0x1F) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_not_reg(code_buffer_t *buf, uint8_t dst, uint8_t src)
{
    /* MVN dst, src */
    u32 insn = 0x9A000000;
    insn |= (dst & 0x1F) << 0;
    insn |= (31 & 0x1F) << 5;
    insn |= (src & 0x1F) << 16;
    insn |= 0x3F << 10;
    emit_arm64_insn(buf, insn);
}

void emit_push_reg(code_buffer_t *buf, uint8_t reg)
{
    emit_sub_imm(buf, 31, 31, 8);
    emit_str_imm(buf, reg, 31, 0);
}

void emit_pop_reg(code_buffer_t *buf, uint8_t reg)
{
    emit_ldr_imm(buf, reg, 31, 0);
    emit_add_imm(buf, 31, 31, 8);
}

/* ============================================================================
 * Shift/Rotate Operations
 * ============================================================================ */

void emit_shl_reg_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift)
{
    /* LSL dst, src, #shift */
    u32 insn = 0xD37FF000;
    insn |= (dst & 0x1F) << 0;
    insn |= (src & 0x1F) << 5;
    insn |= (shift & 0x3F) << 10;
    emit_arm64_insn(buf, insn);
}

void emit_shr_reg_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift)
{
    /* LSR dst, src, #shift */
    u32 insn = 0xD35FF000;
    insn |= (dst & 0x1F) << 0;
    insn |= (src & 0x1F) << 5;
    insn |= (shift & 0x3F) << 10;
    emit_arm64_insn(buf, insn);
}

void emit_sar_reg_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift)
{
    /* ASR dst, src, #shift */
    u32 insn = 0xD34FF000;
    insn |= (dst & 0x1F) << 0;
    insn |= (src & 0x1F) << 5;
    insn |= (shift & 0x3F) << 10;
    emit_arm64_insn(buf, insn);
}

void emit_rol_reg_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift)
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

void emit_ror_reg_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift)
{
    /* ROR dst, src, #shift */
    u32 insn = 0x93C00000;
    insn |= (dst & 0x1F) << 0;
    insn |= (src & 0x1F) << 5;
    insn |= (shift & 0x3F) << 10;
    emit_arm64_insn(buf, insn);
}

/* ============================================================================
 * Conditional Operations
 * ============================================================================ */

void emit_csel_reg_reg_cond(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2, uint8_t cond)
{
    /* CSEL dst, src1, src2, cond */
    u32 insn = 0x9A800000;
    insn |= (dst & 0x1F) << 0;
    insn |= (src2 & 0x1F) << 5;
    insn |= (cond & 0xF) << 12;
    insn |= (src1 & 0x1F) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_setcc_reg_cond(code_buffer_t *buf, uint8_t dst, uint8_t cond)
{
    /* CSET dst, cond -> CSINC dst, XZR, XZR, !cond */
    u32 insn = 0x9A9F03E0;
    insn |= (dst & 0x1F) << 0;
    insn |= ((cond ^ 1) & 0xF) << 12;
    emit_arm64_insn(buf, insn);
}

/* ============================================================================
 * Bit Manipulation
 * ============================================================================ */

void emit_bsf_reg(code_buffer_t *buf, uint8_t dst, uint8_t src)
{
    /* ARM64: RBIT + CLZ + SUB from 63 */
    /* RBIT dst, src */
    u32 insn = 0xDAC00000;
    insn |= (dst & 0x1F) << 0;
    insn |= (src & 0x1F) << 5;
    emit_arm64_insn(buf, insn);
    /* CLZ dst, dst */
    insn = 0xDAC01000;
    insn |= (dst & 0x1F) << 0;
    insn |= (dst & 0x1F) << 5;
    emit_arm64_insn(buf, insn);
    /* SUB dst, #63, dst */
    insn = 0xD1000000;
    insn |= (dst & 0x1F) << 0;
    insn |= (0x3F & 0x1F) << 5;
    insn |= (dst & 0x1F) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_bsr_reg(code_buffer_t *buf, uint8_t dst, uint8_t src)
{
    /* ARM64: CLZ + SUB from 63 */
    /* CLZ dst, src */
    u32 insn = 0xDAC01000;
    insn |= (dst & 0x1F) << 0;
    insn |= (src & 0x1F) << 5;
    emit_arm64_insn(buf, insn);
    /* SUB dst, #63, dst */
    insn = 0xD1000000;
    insn |= (dst & 0x1F) << 0;
    insn |= (0x3F & 0x1F) << 5;
    insn |= (dst & 0x1F) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_popcnt_reg(code_buffer_t *buf, uint8_t dst, uint8_t src)
{
    /* Using NEON CNT + UADDV for population count */
    uint8_t tmp = 16;
    uint8_t tmp2 = 17;

    /* Initialize count to 0 */
    emit_movz(buf, dst, 0, 0);

    /* Copy src to tmp */
    emit_mov_reg(buf, tmp, src);

    /* Simple bit counting (optimized version) */
    /* tmp2 = tmp >> 1 */
    emit_shr_reg_imm(buf, tmp2, tmp, 1);
    /* count = tmp - tmp2 */
    emit_sub_reg(buf, dst, tmp, tmp2);
    /* Note: This is a simplified version. Full POPCNT needs more steps. */
}

void emit_bt_reg(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t bit)
{
    uint8_t tmp = 16;
    emit_shr_reg_imm(buf, tmp, src, bit);
    emit_and_imm(buf, dst, tmp, 1);
}

void emit_bts_reg(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t bit)
{
    uint8_t tmp = 16;
    emit_shr_reg_imm(buf, tmp, src, bit);
    emit_and_imm(buf, dst, tmp, 1);
    emit_movz(buf, tmp, 1, 0);
    emit_shl_reg_imm(buf, tmp, tmp, bit);
    emit_orr_reg(buf, src, src, tmp);
}

void emit_btr_reg(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t bit)
{
    uint8_t tmp = 16;
    emit_shr_reg_imm(buf, tmp, src, bit);
    emit_and_imm(buf, dst, tmp, 1);
    emit_movz(buf, tmp, 1, 0);
    emit_shl_reg_imm(buf, tmp, tmp, bit);
    emit_not_reg(buf, tmp, tmp);
    emit_and_reg(buf, src, src, tmp);
}

void emit_btc_reg(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t bit)
{
    uint8_t tmp = 16;
    emit_shr_reg_imm(buf, tmp, src, bit);
    emit_and_imm(buf, dst, tmp, 1);
    emit_movz(buf, tmp, 1, 0);
    emit_shl_reg_imm(buf, tmp, tmp, bit);
    emit_eor_reg(buf, src, src, tmp);
}

/* ============================================================================
 * String Operations
 * ============================================================================ */

void emit_movs(code_buffer_t *buf, int is_64bit)
{
    uint8_t tmp = 16;
    uint8_t rsi = 6;
    uint8_t rdi = 7;

    emit_ldr_imm(buf, tmp, rsi, 0);
    emit_str_imm(buf, tmp, rdi, 0);
    if (is_64bit) {
        emit_add_imm(buf, rsi, rsi, 8);
        emit_add_imm(buf, rdi, rdi, 8);
    } else {
        emit_add_imm(buf, rsi, rsi, 4);
        emit_add_imm(buf, rdi, rdi, 4);
    }
}

void emit_stos(code_buffer_t *buf, int size)
{
    uint8_t rdi = 7;
    uint8_t rax = 0;

    emit_str_imm(buf, rax, rdi, 0);

    if (size == 8) {
        emit_add_imm(buf, rdi, rdi, 8);
    } else if (size == 4) {
        emit_add_imm(buf, rdi, rdi, 4);
    } else if (size == 2) {
        emit_add_imm(buf, rdi, rdi, 2);
    } else {
        emit_add_imm(buf, rdi, rdi, 1);
    }
}

void emit_lods(code_buffer_t *buf, int size)
{
    uint8_t rsi = 6;
    uint8_t rax = 0;

    emit_ldr_imm(buf, rax, rsi, 0);

    if (size == 8) {
        emit_add_imm(buf, rsi, rsi, 8);
    } else if (size == 4) {
        emit_add_imm(buf, rsi, rsi, 4);
    } else if (size == 2) {
        emit_add_imm(buf, rsi, rsi, 2);
    } else {
        emit_add_imm(buf, rsi, rsi, 1);
    }
}

void emit_cmps(code_buffer_t *buf, int size)
{
    uint8_t tmp1 = 16;
    uint8_t tmp2 = 17;
    uint8_t rsi = 6;
    uint8_t rdi = 7;

    emit_ldr_imm(buf, tmp1, rsi, 0);
    emit_ldr_imm(buf, tmp2, rdi, 0);
    emit_cmp_reg(buf, tmp1, tmp2);

    int inc = (size == 8) ? 8 : (size == 4) ? 4 : (size == 2) ? 2 : 1;
    emit_add_imm(buf, rsi, rsi, inc);
    emit_add_imm(buf, rdi, rdi, inc);
}

void emit_scas(code_buffer_t *buf, int size)
{
    uint8_t tmp = 16;
    uint8_t rdi = 7;
    uint8_t rax = 0;

    emit_ldr_imm(buf, tmp, rdi, 0);
    emit_cmp_reg(buf, rax, tmp);

    int inc = (size == 8) ? 8 : (size == 4) ? 4 : (size == 2) ? 2 : 1;
    emit_add_imm(buf, rdi, rdi, inc);
}

/* ============================================================================
 * Special Instructions
 * ============================================================================ */

void emit_shld(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift)
{
    uint8_t tmp = 16;
    emit_shr_reg_imm(buf, tmp, src, 64 - shift);
    emit_shl_reg_imm(buf, dst, dst, shift);
    emit_orr_reg(buf, dst, dst, tmp);
}

void emit_shrd(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift)
{
    uint8_t tmp = 16;
    emit_shl_reg_imm(buf, tmp, src, 64 - shift);
    emit_shr_reg_imm(buf, dst, dst, shift);
    emit_orr_reg(buf, dst, dst, tmp);
}

void emit_cqo(code_buffer_t *buf)
{
    uint8_t rax = 0;
    uint8_t rdx = 2;
    /* ASR RDX, RAX, #63 */
    u32 insn = 0xD34FF000;
    insn |= (rdx & 0x1F) << 0;
    insn |= (rax & 0x1F) << 5;
    insn |= 63 << 10;
    emit_arm64_insn(buf, insn);
}

void emit_cli(code_buffer_t *buf)
{
    /* CLI - no-op in user mode */
    emit_nop(buf);
}

void emit_sti(code_buffer_t *buf)
{
    /* STI - no-op in user mode */
    emit_nop(buf);
}

void emit_cpuid(code_buffer_t *buf)
{
    /* CPUID - stub (requires hypervisor support) */
    uint8_t rax = 0;
    uint8_t rdx = 2;
    emit_movz(buf, rax, 0, 0);
    emit_movz(buf, rdx, 0, 0);
}

void emit_rdtsc(code_buffer_t *buf)
{
    /* RDTSC - stub (returns 0) */
    uint8_t rax = 0;
    uint8_t rdx = 2;
    emit_movz(buf, rax, 0, 0);
    emit_movz(buf, rdx, 0, 0);
}

/* End of rosetta_arm64_emit.c */
