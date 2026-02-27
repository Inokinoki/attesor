/* ============================================================================
 * Rosetta Binary Translator - Control Flow Instruction Emission
 * ============================================================================
 *
 * This module contains x86_64 control flow instruction emission.
 * Each function emits machine code bytes for jumps, calls, and returns.
 * ============================================================================ */

#include "rosetta_codegen_ctrl.h"

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
