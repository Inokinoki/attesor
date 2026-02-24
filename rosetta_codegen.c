/* ============================================================================
 * Rosetta Binary Translator - x86_64 Code Generation
 * ============================================================================
 *
 * This module contains all x86_64 code emission functions.
 * Each function emits machine code bytes into a code buffer.
 *
 * ============================================================================ */

#include "rosetta_codegen.h"
#include <string.h>
#include <stdlib.h>

/* ============================================================================
 * Code Buffer Management
 * ============================================================================ */

int code_buffer_init(code_buffer_t *buf, u8 *buffer, u32 size) {
    if (!buf) return ROSETTA_ERR_INVAL;

    if (buffer) {
        buf->buffer = buffer;
        buf->owns_buffer = false;
    } else {
        buf->buffer = (u8 *)malloc(size);
        if (!buf->buffer) return ROSETTA_ERR_NOMEM;
        buf->owns_buffer = true;
    }

    buf->size = size;
    buf->offset = 0;
    buf->error = false;
    return ROSETTA_OK;
}

void code_buffer_cleanup(code_buffer_t *buf) {
    if (!buf) return;

    if (buf->owns_buffer && buf->buffer) {
        free(buf->buffer);
    }
    buf->buffer = NULL;
    buf->size = 0;
    buf->offset = 0;
}

void emit_byte(code_buffer_t *buf, u8 byte) {
    if (!buf || !buf->buffer) {
        return;
    }

    if (buf->offset >= buf->size) {
        buf->error = true;
        return;
    }

    buf->buffer[buf->offset++] = byte;
}

void emit_word32(code_buffer_t *buf, u32 word) {
    emit_byte(buf, (word >> 0) & 0xFF);
    emit_byte(buf, (word >> 8) & 0xFF);
    emit_byte(buf, (word >> 16) & 0xFF);
    emit_byte(buf, (word >> 24) & 0xFF);
}

void emit_word64(code_buffer_t *buf, u64 word) {
    emit_word32(buf, (u32)(word & 0xFFFFFFFF));
    emit_word32(buf, (u32)(word >> 32));
}

u8 *code_buffer_get_ptr(code_buffer_t *buf) {
    if (!buf) return NULL;
    return &buf->buffer[buf->offset];
}

u32 code_buffer_get_size(code_buffer_t *buf) {
    if (!buf) return 0;
    return buf->offset;
}

/* ============================================================================
 * x86_64 Register Mapping
 * ============================================================================ */

u8 x86_map_gpr(u8 arm64_reg) {
    /* Simple mapping: ARM64 X0-X15 -> x86_64 RAX-R15 */
    static const u8 reg_map[32] = {
        0, 1, 2, 3, 4, 5, 6, 7,   /* X0-X7 -> RAX-RCX-RDX-RBX-RSI-RDI-RBP-RSP */
        8, 9, 10, 11, 12, 13, 14, 15, /* X8-X15 -> R8-R15 */
        0, 1, 2, 3, 4, 5, 6, 7,   /* X16-X23 -> repeat */
        8, 9, 10, 11, 12, 13, 14, 15  /* X24-X31 -> repeat */
    };
    return reg_map[arm64_reg & 0x1F];
}

u8 x86_map_xmm(u8 arm64_vreg) {
    /* Wrap around: V0-V15 -> XMM0-XMM15, V16-V31 -> XMM0-XMM15 */
    return arm64_vreg & 0x0F;
}

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

void emit_mul_reg(code_buffer_t *buf, u8 dst, u8 src) {
    /* IMUL dst, src: 48 0F AF C0 + src*8 + dst */
    u8 rex = 0x48;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xAF);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
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
 * Control Flow Instructions
 * ============================================================================ */

u32 emit_jmp_rel32(code_buffer_t *buf) {
    /* JMP rel32: E9 iw */
    emit_byte(buf, 0xE9);
    u32 offset = buf->offset;
    emit_word32(buf, 0);  /* Placeholder */
    return offset;
}

u32 emit_je_rel32(code_buffer_t *buf) {
    /* JE rel32: 0F 84 iw */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x84);
    u32 offset = buf->offset;
    emit_word32(buf, 0);  /* Placeholder */
    return offset;
}

u32 emit_jne_rel32(code_buffer_t *buf) {
    /* JNE rel32: 0F 85 iw */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x85);
    u32 offset = buf->offset;
    emit_word32(buf, 0);  /* Placeholder */
    return offset;
}

u32 emit_jl_rel32(code_buffer_t *buf) {
    /* JL rel32: 0F 8C iw */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x8C);
    u32 offset = buf->offset;
    emit_word32(buf, 0);  /* Placeholder */
    return offset;
}

u32 emit_jge_rel32(code_buffer_t *buf) {
    /* JGE rel32: 0F 8D iw */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x8D);
    u32 offset = buf->offset;
    emit_word32(buf, 0);  /* Placeholder */
    return offset;
}

u32 emit_jle_rel32(code_buffer_t *buf) {
    /* JLE rel32: 0F 8E iw */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x8E);
    u32 offset = buf->offset;
    emit_word32(buf, 0);  /* Placeholder */
    return offset;
}

u32 emit_jg_rel32(code_buffer_t *buf) {
    /* JG rel32: 0F 8F iw */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x8F);
    u32 offset = buf->offset;
    emit_word32(buf, 0);  /* Placeholder */
    return offset;
}

u32 emit_jae_rel32(code_buffer_t *buf) {
    /* JAE rel32: 0F 83 iw */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x83);
    u32 offset = buf->offset;
    emit_word32(buf, 0);  /* Placeholder */
    return offset;
}

u32 emit_jb_rel32(code_buffer_t *buf) {
    /* JB rel32: 0F 82 iw */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x82);
    u32 offset = buf->offset;
    emit_word32(buf, 0);  /* Placeholder */
    return offset;
}

u32 emit_js_rel32(code_buffer_t *buf) {
    /* JS rel32: 0F 88 iw */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x88);
    u32 offset = buf->offset;
    emit_word32(buf, 0);  /* Placeholder */
    return offset;
}

u32 emit_jns_rel32(code_buffer_t *buf) {
    /* JNS rel32: 0F 89 iw */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x89);
    u32 offset = buf->offset;
    emit_word32(buf, 0);  /* Placeholder */
    return offset;
}

u32 emit_jo_rel32(code_buffer_t *buf) {
    /* JO rel32: 0F 80 iw */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x80);
    u32 offset = buf->offset;
    emit_word32(buf, 0);  /* Placeholder */
    return offset;
}

u32 emit_jno_rel32(code_buffer_t *buf) {
    /* JNO rel32: 0F 81 iw */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x81);
    u32 offset = buf->offset;
    emit_word32(buf, 0);  /* Placeholder */
    return offset;
}

u32 emit_ja_rel32(code_buffer_t *buf) {
    /* JA rel32: 0F 87 iw */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x87);
    u32 offset = buf->offset;
    emit_word32(buf, 0);  /* Placeholder */
    return offset;
}

u32 emit_jbe_rel32(code_buffer_t *buf) {
    /* JBE rel32: 0F 86 iw */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x86);
    u32 offset = buf->offset;
    emit_word32(buf, 0);  /* Placeholder */
    return offset;
}

void emit_patch_rel32(code_buffer_t *buf, u32 offset, u32 target) {
    if (offset >= buf->size) return;

    u32 rel = target - (offset + 4);
    buf->buffer[offset + 0] = (rel >> 0) & 0xFF;
    buf->buffer[offset + 1] = (rel >> 8) & 0xFF;
    buf->buffer[offset + 2] = (rel >> 16) & 0xFF;
    buf->buffer[offset + 3] = (rel >> 24) & 0xFF;
}

u32 emit_cond_branch(code_buffer_t *buf, arm64_cond_t cond) {
    /* Map ARM64 condition codes to x86 jumps */
    switch (cond) {
        case COND_EQ: return emit_je_rel32(buf);
        case COND_NE: return emit_jne_rel32(buf);
        case COND_CS: return emit_jae_rel32(buf);  /* CS == HS */
        case COND_CC: return emit_jb_rel32(buf);   /* CC == LO */
        case COND_MI: return emit_js_rel32(buf);
        case COND_PL: return emit_jns_rel32(buf);
        case COND_VS: return emit_jo_rel32(buf);
        case COND_VC: return emit_jno_rel32(buf);
        case COND_HI: return emit_ja_rel32(buf);
        case COND_LS: return emit_jbe_rel32(buf);
        case COND_LT: return emit_jl_rel32(buf);
        case COND_GE: return emit_jge_rel32(buf);
        case COND_LE: return emit_jle_rel32(buf);
        case COND_GT: return emit_jg_rel32(buf);
        case COND_AL:
        default:
            emit_nop(buf);  /* Always - no branch needed */
            return 0;
    }
}

void emit_call_reg(code_buffer_t *buf, u8 reg) {
    /* CALL r64: FF D0 + reg */
    u8 rex = 0x48;
    if (reg >= 8) rex |= 0x01;

    if (rex != 0x48) emit_byte(buf, rex);
    emit_byte(buf, 0xFF);
    emit_byte(buf, 0xD0 + (reg & 7));
}

u32 emit_call_rel32(code_buffer_t *buf) {
    /* CALL rel32: E8 iw */
    emit_byte(buf, 0xE8);
    u32 offset = buf->offset;
    emit_word32(buf, 0);  /* Placeholder */
    return offset;
}

void emit_ret(code_buffer_t *buf) {
    /* RET: C3 */
    emit_byte(buf, 0xC3);
}

void emit_nop(code_buffer_t *buf) {
    /* NOP: 90 */
    emit_byte(buf, 0x90);
}

void emit_ud2(code_buffer_t *buf) {
    /* UD2: 0F 0B */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x0B);
}

/* ============================================================================
 * Flag Handling
 * ============================================================================ */

void emit_read_flags_to_nzcv(code_buffer_t *buf, u8 nzcv_reg) {
    /* This is a complex operation that extracts x86 EFLAGS and
     * converts to ARM64 NZCV format. Simplified stub. */

    /* For now, emit a stub that zeros the register */
    emit_mov_reg_imm64(buf, nzcv_reg, 0);
}

void emit_update_flags_add(code_buffer_t *buf, u8 dst, u8 op1, u8 op2) {
    /* NZCV flags are implicitly set by x86 ADD instruction.
     * This stub documents where flag handling would occur. */
    (void)dst; (void)op1; (void)op2;
}

/* ============================================================================
 * SSE/SSSE3 Instructions (Scalar FP)
 * ============================================================================ */

void emit_movss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* MOVSS xmm1, xmm2: F3 0F 5E C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x11);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_movsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* MOVSD xmm1, xmm2: F2 0F 10 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF2);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x11);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_addss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* ADDSS xmm1, xmm2: F3 0F 58 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x58);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_addsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* ADDSD xmm1, xmm2: F2 0F 58 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF2);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x58);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_subss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* SUBSS xmm1, xmm2: F3 0F 5C C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x5C);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_subsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* SUBSD xmm1, xmm2: F2 0F 5C C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF2);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x5C);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_mulss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* MULSS xmm1, xmm2: F3 0F 59 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x59);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_mulsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* MULSD xmm1, xmm2: F2 0F 59 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF2);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x59);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_divss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* DIVSS xmm1, xmm2: F3 0F 5E C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x5E);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_divsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* DIVSD xmm1, xmm2: F2 0F 5E C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF2);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x5E);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_sqrtss_xmm(code_buffer_t *buf, u8 dst) {
    /* SQRTSS xmm1, xmm2: F3 0F 51 C0 + dst*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x51);
    emit_byte(buf, 0xC0 + (dst & 7) + ((dst & 7) << 3));
}

void emit_sqrtsd_xmm(code_buffer_t *buf, u8 dst) {
    /* SQRSD xmm1, xmm2: F2 0F 51 C0 + dst*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF2);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x51);
    emit_byte(buf, 0xC0 + (dst & 7) + ((dst & 7) << 3));
}

void emit_ucomiss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* UCOMISS xmm1, xmm2: 0F 2E C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x2E);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_ucomisd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* UCOMISD xmm1, xmm2: 66 0F 2E C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x2E);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_cvtss2sd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* CVTSS2SD xmm1, xmm2: F3 0F 5A C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x5A);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_cvtsd2ss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* CVTSD2SS xmm1, xmm2: F2 0F 5A C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF2);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x5A);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_rcpss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* RCPSS xmm1, xmm2: F3 0F 53 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x53);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_rsqrtps_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* RSQRTPS xmm1, xmm2: F3 0F 52 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x52);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_xorps_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* XORPS xmm1, xmm2: 0F 57 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x57);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_xorpd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* XORPD xmm1, xmm2: 66 0F 57 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x57);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

/* ============================================================================
 * SSE2/SSSE3 Instructions (Vector/Packed)
 * ============================================================================ */

void emit_paddd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PADDD xmm1, xmm2: 66 0F FE C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xFE);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_paddq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PADDQ xmm1, xmm2: 66 0F D4 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xD4);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_psubd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PSUBD xmm1, xmm2: 66 0F FA C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xFA);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_psubq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PSUBQ xmm1, xmm2: 66 0F FB C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xFB);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pand_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PAND xmm1, xmm2: 66 0F DB C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xDB);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_por_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* POR xmm1, xmm2: 66 0F EB C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xEB);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pxor_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PXOR xmm1, xmm2: 66 0F EF C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xEF);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pandn_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PANDN xmm1, xmm2: 66 0F DF C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xDF);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pmuludq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PMULUDQ xmm1, xmm2: 66 0F F4 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xF4);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pminud_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PMINUD xmm1, xmm2: 66 0F 3B C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3B);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pmaxud_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PMAXUD xmm1, xmm2: 66 0F 3F C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3F);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pminsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PMINSD xmm1, xmm2: 66 0F 3D C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3D);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pmaxsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PMAXSD xmm1, xmm2: 66 0F 3E C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3E);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pcmpgtd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PCMPGTD xmm1, xmm2: 66 0F 3A C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3A);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pcmpeqd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PCMPEQD xmm1, xmm2: 66 0F 76 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x76);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

/* ============================================================================
 * Shift Instructions (Vector)
 * ============================================================================ */

void emit_pslld_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm) {
    /* PSLLD xmm1, imm8: 66 0F 72 C0 + dst iw */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x72);
    emit_byte(buf, 0xC0 + (dst & 7));
    emit_byte(buf, imm);
}

void emit_psllq_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm) {
    /* PSLLQ xmm1, imm8: 66 0F 73 C0 + dst iw */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x73);
    emit_byte(buf, 0xC0 + (dst & 7));
    emit_byte(buf, imm);
}

void emit_psrld_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm) {
    /* PSRLD xmm1, imm8: 66 0F 72 C8 + dst iw */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x72);
    emit_byte(buf, 0xC8 + (dst & 7));
    emit_byte(buf, imm);
}

void emit_psrlq_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm) {
    /* PSRLQ xmm1, imm8: 66 0F 73 C0 + dst iw */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x73);
    emit_byte(buf, 0xC0 + (dst & 7));
    emit_byte(buf, imm);
}

void emit_psrad_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm) {
    /* PSRAD xmm1, imm8: 66 0F 72 E0 + dst iw */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x72);
    emit_byte(buf, 0xE0 + (dst & 7));
    emit_byte(buf, imm);
}

void emit_psraq_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm) {
    /* PSRAQ xmm1, imm8: 66 0F 73 E0 + dst iw */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x73);
    emit_byte(buf, 0xE0 + (dst & 7));
    emit_byte(buf, imm);
}

/* ============================================================================
 * Memory Operations (Register Indirect)
 * ============================================================================ */

void emit_movdqu_xmm_mem_reg(code_buffer_t *buf, u8 dst, u8 base) {
    /* MOVDQU xmm, [base]: F3 0F 6F 00 + dst*8 + base */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (base >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x6F);
    emit_byte(buf, 0x00 + (dst & 7) + ((base & 7) << 3));
}

void emit_movdqu_mem_reg_xmm(code_buffer_t *buf, u8 base, u8 src) {
    /* MOVDQU [base], xmm: F3 0F 7F 00 + src*8 + base */
    u8 rex = 0x40;
    if (src >= 8) rex |= 0x04;
    if (base >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x7F);
    emit_byte(buf, 0x00 + (src & 7) + ((base & 7) << 3));
}

void emit_movups_xmm_mem_reg(code_buffer_t *buf, u8 dst, u8 base) {
    /* MOVUPS xmm, [base]: 0F 10 00 + dst*8 + base */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (base >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x10);
    emit_byte(buf, 0x00 + (dst & 7) + ((base & 7) << 3));
}

void emit_movups_mem_reg_xmm(code_buffer_t *buf, u8 base, u8 src) {
    /* MOVUPS [base], xmm: 0F 11 00 + src*8 + base */
    u8 rex = 0x40;
    if (src >= 8) rex |= 0x04;
    if (base >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x11);
    emit_byte(buf, 0x00 + (src & 7) + ((base & 7) << 3));
}

void emit_movaps_xmm_mem_reg(code_buffer_t *buf, u8 dst, u8 base) {
    /* MOVAPS xmm, [base]: 0F 28 00 + dst*8 + base */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (base >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x28);
    emit_byte(buf, 0x00 + (dst & 7) + ((base & 7) << 3));
}

void emit_movaps_mem_reg_xmm(code_buffer_t *buf, u8 base, u8 src) {
    /* MOVAPS [base], xmm: 0F 29 00 + src*8 + base */
    u8 rex = 0x40;
    if (src >= 8) rex |= 0x04;
    if (base >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x29);
    emit_byte(buf, 0x00 + (src & 7) + ((base & 7) << 3));
}

/* ============================================================================
 * SSSE3 Shuffle and Permutation
 * ============================================================================ */

void emit_punpcklbw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PUNPCKLBW xmm1, xmm2: 66 0F 60 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x60);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_punpckhbw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PUNPCKHBW xmm1, xmm2: 66 0F 68 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x68);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_punpcklwd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PUNPCKLWD xmm1, xmm2: 66 0F 61 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x61);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_punpckhwd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PUNPCKHWD xmm1, xmm2: 66 0F 69 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x69);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_punpckldq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PUNPCKLDQ xmm1, xmm2: 66 0F 62 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x62);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_punpckhdq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PUNPCKHDQ xmm1, xmm2: 66 0F 6A C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x6A);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_palignr_xmm_xmm_imm(code_buffer_t *buf, u8 dst, u8 src, u8 imm) {
    /* PALIGNR xmm1, xmm2, imm8: 66 0F 3A 0F C0 + src*8 + dst ib */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3A);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
    emit_byte(buf, imm);
}

void emit_pshufb_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PSHUFB xmm1, xmm2: 66 0F 38 00 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x00);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_psignb_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PSIGNB xmm1, xmm2: 66 0F 38 08 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x08);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_psignw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PSIGNW xmm1, xmm2: 66 0F 38 09 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x09);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_psignd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PSIGND xmm1, xmm2: 66 0F 38 0A C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x0A);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pextrb_reg_xmm_imm(code_buffer_t *buf, u8 dst, u8 src, u8 imm) {
    /* PEXTRB r32, xmm, imm8: 66 0F 3A 14 C0 + src*8 + dst ib */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3A);
    emit_byte(buf, 0x14);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
    emit_byte(buf, imm);
}

void emit_pinsrb_xmm_reg_imm(code_buffer_t *buf, u8 dst, u8 src, u8 imm) {
    /* PINSRB xmm, r32, imm8: 66 0F 3A 20 C0 + src*8 + dst ib */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3A);
    emit_byte(buf, 0x20);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
    emit_byte(buf, imm);
}

