/* ============================================================================
 * Rosetta Translator - Bitfield Instruction Translation Implementation
 * ============================================================================
 *
 * This module implements translation of ARM64 bitfield instructions
 * to x86_64 machine code.
 * ============================================================================ */

#include "rosetta_translate_bitfield.h"
#include "rosetta_emit_x86.h"
#include <stdbool.h>

/* External register mapping function from ALU module */
extern uint8_t translate_get_x86_reg(uint8_t arm_reg);

/* ============================================================================
 * Bitfield Move (BFM) Instruction
 * ============================================================================ */

/**
 * translate_bitfield_bfm - Translate BFM instruction
 * BFM Rd, Rn, #immr, #imms  ->  Move bitfield from Rn to Rd
 *
 * Encoding: 1101001100iiiiii01iiiiirrrrrdddd
 *           |  opcode  | immr | 1 | imms |  Rn  | Rd |
 */
int translate_bitfield_bfm(uint32_t encoding, code_buf_t *code_buf,
                           uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t imms = (encoding >> 10) & 0x3F;
    uint8_t immr = (encoding >> 16) & 0x3F;
    uint8_t N = (encoding >> 22) & 1;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* BFM moves bits [imms:0] from Rn to Rd[immr:immr-imms]
     * This is a general bitfield move that can implement:
     * - Shift left: immr > imms
     * - Shift right: immr <= imms
     * - Bit insert: specific ranges
     */

    /* Calculate shift amount and width */
    uint64_t width = imms + 1;
    uint64_t shift = immr;

    /* Update guest state */
    uint64_t src_val = guest_state[rn];
    uint64_t dst_val = guest_state[rd];
    uint64_t mask = (1ULL << width) - 1;
    uint64_t extracted = (src_val >> (64 - width)) & mask;

    if (N) {
        /* 64-bit operation */
        if (immr >= imms) {
            /* Right shift */
            uint64_t result = extracted << (immr - imms);
            guest_state[rd] = result;
        } else {
            /* Left shift */
            uint64_t result = extracted << (64 - (imms - immr + 1));
            guest_state[rd] = result;
        }
    } else {
        /* 32-bit operation - zero extend */
        uint32_t result;
        if (immr >= imms) {
            result = (uint32_t)(extracted << (immr - imms));
        } else {
            result = (uint32_t)(extracted << (32 - (imms - immr + 1)));
        }
        guest_state[rd] = result;
    }

    /* Emit x86_64 code */
    /* MOV rd, rn */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    /* Shift based on immr/imms relationship */
    if (immr >= imms) {
        /* Right shift scenario - actually a left shift in x86 */
        uint8_t shift_amt = immr - imms;
        if (shift_amt == 0) {
            /* No shift needed, but we need to mask */
            emit_x86_and_reg_imm32(code_buf, x86_rd, (uint32_t)mask);
        } else {
            emit_x86_shl_reg_imm8(code_buf, x86_rd, shift_amt);
        }
    } else {
        /* Left shift scenario */
        uint8_t shift_amt = 64 - (imms - immr + 1);
        if (N) {
            emit_x86_shl_reg_imm8(code_buf, x86_rd, shift_amt);
        } else {
            /* 32-bit operation */
            shift_amt = 32 - (imms - immr + 1);
            emit_x86_shl_reg_imm8(code_buf, x86_rd, shift_amt);
        }
    }

    /* Zero extend for 32-bit operations */
    if (!N) {
        emit_x86_and_reg_imm32(code_buf, x86_rd, 0xFFFFFFFF);
    }

    return 0;
}

/**
 * translate_bitfield_bfmw - Translate BFMW (32-bit) instruction
 */
int translate_bitfield_bfmw(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state)
{
    /* Similar to BFM but always 32-bit */
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* MOV rd, rn (32-bit automatically zero-extends) */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    /* Rest is similar to BFM but with 32-bit semantics */
    /* For now, use same implementation as BFM */
    return translate_bitfield_bfm(encoding, code_buf, guest_state);
}

/* ============================================================================
 * Bitfield Insert (BFI) Instruction
 * ============================================================================ */

/**
 * translate_bitfield_bfi - Translate BFI instruction
 * BFI Rd, Rn, #lsb, #width  ->  Insert bitfield from Rn into Rd
 *
 * Encoding: 1101001100iiiiii01iiiiirrrrrdddd
 *           |  opcode  | lsb  | 1 |width-1| Rn   | Rd |
 */
int translate_bitfield_bfi(uint32_t encoding, code_buf_t *code_buf,
                           uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t imms = (encoding >> 10) & 0x3F;  /* width - 1 */
    uint8_t immr = (encoding >> 16) & 0x3F;  /* lsb */
    uint8_t N = (encoding >> 22) & 1;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* BFI inserts bits [width-1:0] from Rn into Rd[lsb+width-1:lsb] */
    uint64_t width = imms + 1;
    uint64_t lsb = immr;

    /* Update guest state */
    uint64_t src_val = guest_state[rn];
    uint64_t dst_val = guest_state[rd];
    uint64_t mask = ((1ULL << width) - 1) << lsb;

    if (N) {
        /* 64-bit operation */
        guest_state[rd] = (dst_val & ~mask) | ((src_val << lsb) & mask);
    } else {
        /* 32-bit operation */
        uint32_t dst32 = (uint32_t)dst_val;
        uint32_t src32 = (uint32_t)src_val;
        uint32_t mask32 = ((1U << width) - 1) << lsb;
        guest_state[rd] = (uint64_t)((dst32 & ~mask32) | ((src32 << lsb) & mask32));
    }

    /* Emit x86_64 code */
    /* We need to:
     * 1. Save Rd
     * 2. Clear the bitfield region in Rd
     * 3. Shift Rn by lsb
     * 4. OR them together
     */

    /* Use RCX as temporary */
    /* MOV rcx, rd - save original Rd */
    code_buf_emit_byte(code_buf, 0x48);
    code_buf_emit_byte(code_buf, 0x89);
    code_buf_emit_byte(code_buf, 0xC8 + x86_rd);  /* MOV RCX, RAX */

    /* Clear bits [lsb+width-1:lsb] in Rd */
    /* Create mask: ~(((1 << width) - 1) << lsb) */
    uint64_t clear_mask = ~(((1ULL << width) - 1) << lsb);

    emit_x86_and_reg_imm32(code_buf, x86_rd, (uint32_t)clear_mask);

    /* Shift Rn by lsb */
    emit_x86_mov_reg_reg(code_buf, EMIT_RCX, x86_rn);
    if (lsb > 0) {
        emit_x86_shl_reg_imm8(code_buf, EMIT_RCX, lsb);
    }

    /* Clear upper bits for 32-bit operations */
    if (!N && (lsb + width < 32)) {
        uint32_t upper_mask = (1U << (lsb + width)) - 1;
        emit_x86_and_reg_imm32(code_buf, EMIT_RCX, upper_mask);
    }

    /* OR Rd, RcX */
    emit_x86_or_reg_reg(code_buf, x86_rd, EMIT_RCX);

    /* Zero extend for 32-bit operations - use AND with 0xFFFFFFFF */
    if (!N) {
        emit_x86_and_reg_imm32(code_buf, x86_rd, 0xFFFFFFFF);
    }

    return 0;
}

/* ============================================================================
 * Unsigned Bitfield Extract (UBFX) Instruction
 * ============================================================================ */

/**
 * translate_bitfield_ubfx - Translate UBFX instruction
 * UBFX Rd, Rn, #lsb, #width  ->  Extract unsigned bitfield
 *
 * Encoding: 0101001100iiiiii01iiiiirrrrrdddd
 */
int translate_bitfield_ubfx(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t imms = (encoding >> 10) & 0x3F;  /* width - 1 */
    uint8_t immr = (encoding >> 16) & 0x3F;  /* lsb */

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* UBFX extracts bits [lsb+width-1:lsb] and zero-extends */
    uint64_t width = imms + 1;
    uint64_t lsb = immr;

    /* Update guest state */
    uint64_t src_val = guest_state[rn];
    uint64_t mask = (1ULL << width) - 1;
    guest_state[rd] = (src_val >> lsb) & mask;

    /* Emit x86_64 code */
    /* MOV rd, rn */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    /* Right shift by lsb */
    if (lsb > 0) {
        emit_x86_shr_reg_imm8(code_buf, x86_rd, lsb);
    }

    /* Mask to width bits */
    if (width < 64) {
        uint32_t mask32 = (width == 32) ? 0xFFFFFFFF : ((1U << width) - 1);
        if (width <= 32) {
            emit_x86_and_reg_imm32(code_buf, x86_rd, mask32);
        } else {
            /* For width > 32, need 64-bit immediate */
            emit_x86_and_reg_imm32(code_buf, x86_rd, mask32);
        }
    }

    return 0;
}

/* ============================================================================
 * Signed Bitfield Extract (SBFX) Instruction
 * ============================================================================ */

/**
 * translate_bitfield_sbfx - Translate SBFX instruction
 * SBFX Rd, Rn, #lsb, #width  ->  Extract signed bitfield
 *
 * Encoding: 0001001100iiiiii01iiiiirrrrrdddd
 */
int translate_bitfield_sbfx(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t imms = (encoding >> 10) & 0x3F;  /* width - 1 */
    uint8_t immr = (encoding >> 16) & 0x3F;  /* lsb */

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* SBFX extracts bits [lsb+width-1:lsb] and sign-extends */
    uint64_t width = imms + 1;
    uint64_t lsb = immr;

    /* Update guest state */
    int64_t src_val = (int64_t)guest_state[rn];
    int64_t extracted = (src_val >> lsb) & ((1LL << width) - 1);

    /* Sign extend */
    int64_t sign_bit = 1LL << (width - 1);
    if (extracted & sign_bit) {
        extracted |= ~((1LL << width) - 1);
    }
    guest_state[rd] = (uint64_t)extracted;

    /* Emit x86_64 code */
    /* MOV rd, rn */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    /* Left shift to move sign bit to top */
    uint8_t left_shift = 64 - (lsb + width);
    emit_x86_shl_reg_imm8(code_buf, x86_rd, left_shift);

    /* Arithmetic right shift to sign-extend */
    emit_x86_sar_reg_imm8(code_buf, x86_rd, left_shift);

    return 0;
}

/**
 * translate_bitfield_ubfxw - Translate UBFXW (32-bit) instruction
 */
int translate_bitfield_ubfxw(uint32_t encoding, code_buf_t *code_buf,
                             uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t imms = (encoding >> 10) & 0x3F;
    uint8_t immr = (encoding >> 16) & 0x3F;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);

    uint64_t width = imms + 1;
    uint64_t lsb = immr;

    /* Update guest state (32-bit) */
    uint32_t src_val = (uint32_t)guest_state[rn];
    uint32_t mask = (1U << width) - 1;
    guest_state[rd] = (uint64_t)((src_val >> lsb) & mask);

    /* Emit x86_64 code */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    if (lsb > 0) {
        emit_x86_shr_reg_imm8(code_buf, x86_rd, lsb);
    }

    if (width < 32) {
        emit_x86_and_reg_imm32(code_buf, x86_rd, (1U << width) - 1);
    }

    /* Zero extend to 64 bits - 32-bit ops auto zero-extend, but use AND for clarity */
    emit_x86_and_reg_imm32(code_buf, x86_rd, 0xFFFFFFFF);

    return 0;
}

/**
 * translate_bitfield_sbfxw - Translate SBFXW (32-bit) instruction
 */
int translate_bitfield_sbfxw(uint32_t encoding, code_buf_t *code_buf,
                             uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t imms = (encoding >> 10) & 0x3F;
    uint8_t immr = (encoding >> 16) & 0x3F;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);

    uint64_t width = imms + 1;
    uint64_t lsb = immr;

    /* Update guest state (32-bit with sign extend) */
    int32_t src_val = (int32_t)guest_state[rn];
    int32_t extracted = (src_val >> lsb) & ((1 << width) - 1);

    /* Sign extend within 32 bits */
    int32_t sign_bit = 1 << (width - 1);
    if (extracted & sign_bit) {
        extracted |= ~((1 << width) - 1);
    }
    guest_state[rd] = (uint64_t)(int64_t)extracted;

    /* Emit x86_64 code */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    /* Left shift to position sign bit */
    uint8_t left_shift = 32 - (lsb + width);
    emit_x86_shl_reg_imm8(code_buf, x86_rd, left_shift);

    /* Arithmetic right shift to sign-extend */
    emit_x86_sar_reg_imm8(code_buf, x86_rd, left_shift);

    /* Sign extend to 64 bits - use MOVSXD */
    emit_x86_movsxd_reg_reg32(code_buf, x86_rd, x86_rd);

    return 0;
}

/* ============================================================================
 * Bitfield Insert with Zero-extend Instructions
 * ============================================================================ */

/**
 * translate_bitfield_ubfiz - Translate UBFIZ instruction
 * UBFIZ Rd, Rn, #lsb, #width  ->  Insert bitfield with zero-extend
 *
 * Encoding: 0111001100iiiiii01iiiiirrrrrdddd
 */
int translate_bitfield_ubfiz(uint32_t encoding, code_buf_t *code_buf,
                             uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t imms = (encoding >> 10) & 0x3F;  /* width - 1 */
    uint8_t immr = (encoding >> 16) & 0x3F;  /* lsb */

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* UBFIZ: extract width bits from Rn, shift left by lsb, zero-extend */
    uint64_t width = imms + 1;
    uint64_t lsb = immr;

    /* Update guest state */
    uint64_t src_val = guest_state[rn];
    uint64_t mask = (1ULL << width) - 1;
    uint64_t extracted = src_val & mask;
    guest_state[rd] = extracted << lsb;

    /* Emit x86_64 code */
    /* MOV rd, rn */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    /* Mask to width bits */
    if (width < 64) {
        uint32_t mask32 = (width >= 32) ? 0xFFFFFFFF : ((1U << width) - 1);
        emit_x86_and_reg_imm32(code_buf, x86_rd, mask32);
    }

    /* Shift left by lsb */
    if (lsb > 0) {
        emit_x86_shl_reg_imm8(code_buf, x86_rd, lsb);
    }

    return 0;
}

/**
 * translate_bitfield_sbfiz - Translate SBFIZ instruction
 * SBFIZ Rd, Rn, #lsb, #width  ->  Insert bitfield with sign-extend
 *
 * Encoding: 0011001100iiiiii01iiiiirrrrrdddd
 */
int translate_bitfield_sbfiz(uint32_t encoding, code_buf_t *code_buf,
                             uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t imms = (encoding >> 10) & 0x3F;  /* width - 1 */
    uint8_t immr = (encoding >> 16) & 0x3F;  /* lsb */

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* SBFIZ: extract width bits from Rn, sign-extend, shift left by lsb */
    uint64_t width = imms + 1;
    uint64_t lsb = immr;

    /* Update guest state */
    int64_t src_val = (int64_t)guest_state[rn];
    int64_t extracted = src_val & ((1LL << width) - 1);

    /* Sign extend */
    int64_t sign_bit = 1LL << (width - 1);
    if (extracted & sign_bit) {
        extracted |= ~((1LL << width) - 1);
    }
    guest_state[rd] = (uint64_t)(extracted << lsb);

    /* Emit x86_64 code */
    /* MOV rd, rn */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    /* Mask to width bits */
    if (width < 64) {
        uint32_t mask32 = (width >= 32) ? 0xFFFFFFFF : ((1U << width) - 1);
        emit_x86_and_reg_imm32(code_buf, x86_rd, mask32);
    }

    /* Sign extend from width bits */
    if (width < 64) {
        uint8_t left_shift = 64 - width;
        emit_x86_shl_reg_imm8(code_buf, x86_rd, left_shift);
        emit_x86_sar_reg_imm8(code_buf, x86_rd, left_shift);
    }

    /* Shift left by lsb */
    if (lsb > 0) {
        emit_x86_shl_reg_imm8(code_buf, x86_rd, lsb);
    }

    return 0;
}

/* ============================================================================
 * Rotate Right (ROR) Instruction
 * ============================================================================ */

/**
 * translate_bitfield_ror - Translate ROR instruction
 * ROR Rd, Rn, Rm  ->  Rotate right by register amount
 *
 * Encoding: 00010011000mmmmm000011rrrrrdddd
 */
int translate_bitfield_ror(uint32_t encoding, code_buf_t *code_buf,
                           uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* Update guest state */
    uint64_t src_val = guest_state[rn];
    uint64_t shift_amt = guest_state[rm] & 0x3F;
    if (shift_amt == 0) {
        guest_state[rd] = src_val;
    } else {
        guest_state[rd] = (src_val >> shift_amt) | (src_val << (64 - shift_amt));
    }

    /* Emit x86_64 code */
    /* MOV rd, rn */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    /* ROR using RCX as shift count */
    /* MOV rcx, rm (shift count) */
    code_buf_emit_byte(code_buf, 0x48);
    code_buf_emit_byte(code_buf, 0x89);
    code_buf_emit_byte(code_buf, 0xC8 + x86_rm);  /* MOV RCX, RAX */

    /* AND rcx, 63 (ensure shift is in range 0-63) */
    emit_x86_and_reg_imm32(code_buf, EMIT_RCX, 0x3F);

    /* ROR rd, cl */
    code_buf_emit_byte(code_buf, 0x48);
    code_buf_emit_byte(code_buf, 0xD1);
    code_buf_emit_byte(code_buf, 0xC8 + x86_rd);  /* ROR RAX, CL */

    return 0;
}

/**
 * translate_bitfield_rorw - Translate RORW (32-bit) instruction
 */
int translate_bitfield_rorw(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* Update guest state (32-bit) */
    uint32_t src_val = (uint32_t)guest_state[rn];
    uint32_t shift_amt = (uint32_t)guest_state[rm] & 0x1F;
    if (shift_amt == 0) {
        guest_state[rd] = (uint64_t)src_val;
    } else {
        uint32_t result = (src_val >> shift_amt) | (src_val << (32 - shift_amt));
        guest_state[rd] = (uint64_t)result;
    }

    /* Emit x86_64 code (32-bit operations auto-zero-extend) */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    /* AND rm, 31 */
    emit_x86_and_reg_imm32(code_buf, x86_rm, 0x1F);

    /* Use RAX as temp */
    emit_x86_mov_reg_reg(code_buf, EMIT_RAX, x86_rm);

    /* ROR rd, eax (32-bit) */
    code_buf_emit_byte(code_buf, 0xD3);
    code_buf_emit_byte(code_buf, 0xC8 + x86_rd);  /* ROR EAX, CL */

    /* Zero extend result - 32-bit ops auto zero-extend */
    emit_x86_and_reg_imm32(code_buf, x86_rd, 0xFFFFFFFF);

    return 0;
}

/* ============================================================================
 * Extract Register (EXTR) Instruction
 * ============================================================================ */

/**
 * translate_bitfield_extr - Translate EXTR instruction
 * EXTR Rd, Rn, Rm, #shift  ->  Concatenate and extract
 *
 * Encoding: 00010011000mmmmmiiiiiiirrrrrdddd
 */
int translate_bitfield_extr(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t shift = (encoding >> 10) & 0x3F;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* EXTR: (Rn << shift) | (Rm >> (64-shift)) */
    /* This extracts bits from the concatenation of Rn:Rm */

    /* Update guest state */
    uint64_t rn_val = guest_state[rn];
    uint64_t rm_val = guest_state[rm];

    if (shift == 0) {
        guest_state[rd] = rm_val;
    } else {
        guest_state[rd] = (rn_val << shift) | (rm_val >> (64 - shift));
    }

    /* Emit x86_64 code */
    /* MOV rd, rm */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rm);

    if (shift == 0) {
        /* Just RM, already done */
        return 0;
    }

    /* Use RAX and RCX as temporaries */
    /* MOV rax, rn */
    emit_x86_mov_reg_reg(code_buf, EMIT_RAX, x86_rn);

    /* SHL rax, shift */
    emit_x86_shl_reg_imm8(code_buf, EMIT_RAX, shift);

    /* SHR rd, (64-shift) */
    emit_x86_shr_reg_imm8(code_buf, x86_rd, 64 - shift);

    /* OR rd, rax */
    emit_x86_or_reg_reg(code_buf, x86_rd, EMIT_RAX);

    return 0;
}

/* ============================================================================
 * Extension Instructions
 * ============================================================================ */

/**
 * translate_bitfield_uxtb - Translate UXTB instruction
 * UXTB Rd, Rn  ->  Zero-extend byte to 64 bits
 *
 * Actually: UXTB Rd, Rn, LSL #imm (imm is 0, 8, 16, or 24)
 */
int translate_bitfield_uxtb(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t shift = ((encoding >> 10) & 0x03) * 8;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* Update guest state */
    uint8_t byte_val = (uint8_t)(guest_state[rn] >> shift);
    guest_state[rd] = (uint64_t)byte_val;

    /* Emit x86_64 code */
    /* MOV rd, rn */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    /* Shift if needed */
    if (shift > 0) {
        emit_x86_shr_reg_imm8(code_buf, x86_rd, shift);
    }

    /* Zero extend from AL - use AND with 0xFF */
    emit_x86_and_reg_imm32(code_buf, x86_rd, 0xFF);

    return 0;
}

/**
 * translate_bitfield_uxth - Translate UXTH instruction
 * UXTH Rd, Rn  ->  Zero-extend halfword to 64 bits
 */
int translate_bitfield_uxth(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t shift = ((encoding >> 10) & 0x03) * 8;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* Update guest state */
    uint16_t half_val = (uint16_t)(guest_state[rn] >> shift);
    guest_state[rd] = (uint64_t)half_val;

    /* Emit x86_64 code */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    if (shift > 0) {
        emit_x86_shr_reg_imm8(code_buf, x86_rd, shift);
    }

    /* Zero extend from AX */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x89);
    code_buf_emit_byte(code_buf, 0xC0 + (x86_rd & 7));  /* MOV AX, AX */

    return 0;
}

/**
 * translate_bitfield_sxtb - Translate SXTB instruction
 * SXTB Rd, Rn  ->  Sign-extend byte to 64 bits
 */
int translate_bitfield_sxtb(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t shift = ((encoding >> 10) & 0x03) * 8;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* Update guest state */
    int8_t byte_val = (int8_t)(guest_state[rn] >> shift);
    guest_state[rd] = (uint64_t)(int64_t)byte_val;

    /* Emit x86_64 code */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    if (shift > 0) {
        emit_x86_shr_reg_imm8(code_buf, x86_rd, shift);
    }

    /* MOVSXD rd, eax (sign extend from AL) */
    /* First move to eax with sign extension */
    code_buf_emit_byte(code_buf, 0x48);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0xBE);
    code_buf_emit_byte(code_buf, 0xC0 + (x86_rd & 7));  /* MOVSX RAX, AL */

    return 0;
}

/**
 * translate_bitfield_sxth - Translate SXTH instruction
 * SXTH Rd, Rn  ->  Sign-extend halfword to 64 bits
 */
int translate_bitfield_sxth(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t shift = ((encoding >> 10) & 0x03) * 8;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* Update guest state */
    int16_t half_val = (int16_t)(guest_state[rn] >> shift);
    guest_state[rd] = (uint64_t)(int64_t)half_val;

    /* Emit x86_64 code */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    if (shift > 0) {
        emit_x86_shr_reg_imm8(code_buf, x86_rd, shift);
    }

    /* MOVSXD rd, eax (sign extend from AX) */
    code_buf_emit_byte(code_buf, 0x48);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0xBF);
    code_buf_emit_byte(code_buf, 0xC0 + (x86_rd & 7));  /* MOVSX RAX, AX */

    return 0;
}

/**
 * translate_bitfield_sxtw - Translate SXTW instruction
 * SXTW Rd, Rn  ->  Sign-extend word to 64 bits
 */
int translate_bitfield_sxtw(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* Update guest state */
    int32_t word_val = (int32_t)guest_state[rn];
    guest_state[rd] = (uint64_t)(int64_t)word_val;

    /* Emit x86_64 code */
    /* MOVSXD automatically sign-extends EAX to RAX */
    code_buf_emit_byte(code_buf, 0x48);
    code_buf_emit_byte(code_buf, 0x63);
    code_buf_emit_byte(code_buf, 0xC0 + ((x86_rn & 8) >> 3) + (x86_rd & 7));

    return 0;
}

/* ============================================================================
 * Dispatch Function
 * ============================================================================ */

/**
 * translate_bitfield_dispatch - Dispatch bitfield instruction
 */
int translate_bitfield_dispatch(uint32_t encoding, code_buf_t *code_buf,
                                uint64_t *guest_state)
{
    /* Check for BFM/BFI/UBFX/SBFX family */
    if ((encoding & BITFIELD_BFM_MASK) == BITFIELD_BFM_VAL) {
        uint8_t imms = (encoding >> 10) & 0x3F;
        uint8_t immr = (encoding >> 16) & 0x3F;
        uint8_t opc = (encoding >> 29) & 0x03;

        if (opc == 0) {
            /* SBFIZ/SBFX */
            if ((encoding >> 31) == 0) {
                if (immr == 0) {
                    return translate_bitfield_sbfiz(encoding, code_buf, guest_state);
                } else {
                    return translate_bitfield_sbfx(encoding, code_buf, guest_state);
                }
            } else {
                if (immr == 0) {
                    return translate_bitfield_sbfiz(encoding, code_buf, guest_state);
                } else {
                    return translate_bitfield_sbfxw(encoding, code_buf, guest_state);
                }
            }
        } else if (opc == 1) {
            /* UBFIZ/UBFX */
            if ((encoding >> 31) == 0) {
                if (immr == 0) {
                    return translate_bitfield_ubfiz(encoding, code_buf, guest_state);
                } else {
                    return translate_bitfield_ubfx(encoding, code_buf, guest_state);
                }
            } else {
                if (immr == 0) {
                    return translate_bitfield_ubfiz(encoding, code_buf, guest_state);
                } else {
                    return translate_bitfield_ubfxw(encoding, code_buf, guest_state);
                }
            }
        } else if (opc == 2 || opc == 3) {
            /* BFM/BFI */
            if ((encoding & (1 << 31)) == 0) {
                return translate_bitfield_bfm(encoding, code_buf, guest_state);
            } else {
                return translate_bitfield_bfmw(encoding, code_buf, guest_state);
            }
        }
    }

    /* Check for ROR/EXTR */
    if ((encoding & ROTATE_ROR_MASK) == ROTATE_ROR_VAL) {
        uint8_t opc = (encoding >> 29) & 0x03;
        if (opc == 0) {
            /* ROR */
            return translate_bitfield_ror(encoding, code_buf, guest_state);
        }
    }

    /* Check for EXTR */
    if ((encoding & EXTRACT_REG_EXTR_MASK) == EXTRACT_REG_EXTR_VAL) {
        if (((encoding >> 29) & 0x03) == 0) {
            return translate_bitfield_extr(encoding, code_buf, guest_state);
        }
    }

    /* Check for extension instructions (UXTB, UXTH, SXTB, SXTH, SXTW) */
    if ((encoding & 0x7FE00700) == 0x13000000) {
        /* Could be extension instruction */
        uint8_t opc = (encoding >> 29) & 0x03;
        if (opc == 0) {
            /* UXTB */
            return translate_bitfield_uxtb(encoding, code_buf, guest_state);
        } else if (opc == 1) {
            /* UXTH */
            return translate_bitfield_uxth(encoding, code_buf, guest_state);
        }
    }

    if ((encoding & 0x7FE00700) == 0x13800000) {
        /* Could be sign-extend instruction */
        uint8_t opc = (encoding >> 29) & 0x03;
        if (opc == 0) {
            /* SXTB */
            return translate_bitfield_sxtb(encoding, code_buf, guest_state);
        } else if (opc == 1) {
            /* SXTH */
            return translate_bitfield_sxth(encoding, code_buf, guest_state);
        } else if (opc == 2) {
            /* SXTW */
            return translate_bitfield_sxtw(encoding, code_buf, guest_state);
        }
    }

    return -1;  /* Not a bitfield instruction */
}
