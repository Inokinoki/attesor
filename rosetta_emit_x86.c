/* ============================================================================
 * Rosetta Translator - x86_64 Code Emission Implementation
 * ============================================================================
 *
 * This module implements x86_64 machine code emission functions
 * for binary translation.
 * ============================================================================ */

#include "rosetta_emit_x86.h"
#include <string.h>

/* ============================================================================
 * Code Buffer Operations
 * ============================================================================ */

void code_buf_init(code_buf_t *buf, uint8_t *buffer, size_t size)
{
    if (!buf || !buffer) return;
    buf->buffer = buffer;
    buf->offset = 0;
    buf->size = size;
}

void code_buf_emit_byte(code_buf_t *buf, uint8_t byte)
{
    if (!buf || !buf->buffer) return;
    if (buf->offset < buf->size) {
        buf->buffer[buf->offset++] = byte;
    }
}

void code_buf_emit_word32(code_buf_t *buf, uint32_t word)
{
    code_buf_emit_byte(buf, word & 0xFF);
    code_buf_emit_byte(buf, (word >> 8) & 0xFF);
    code_buf_emit_byte(buf, (word >> 16) & 0xFF);
    code_buf_emit_byte(buf, (word >> 24) & 0xFF);
}

void code_buf_emit_word64(code_buf_t *buf, uint64_t word)
{
    code_buf_emit_word32(buf, word & 0xFFFFFFFF);
    code_buf_emit_word32(buf, (word >> 32) & 0xFFFFFFFF);
}

size_t code_buf_get_size(code_buf_t *buf)
{
    if (!buf) return 0;
    return buf->offset;
}

uint8_t *code_buf_get_buffer(code_buf_t *buf)
{
    if (!buf) return NULL;
    return buf->buffer;
}

/* ============================================================================
 * x86_64 Code Emission - Data Movement
 * ============================================================================ */

void emit_x86_mov_reg_imm64(code_buf_t *buf, uint8_t dst, uint64_t imm)
{
    /* MOV r64, imm64: 48 B8+rd /r */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0xB8 + (dst & 7));
    if (dst >= 8) code_buf_emit_byte(buf, 0x41);
    code_buf_emit_word64(buf, imm);
}

void emit_x86_mov_reg_imm32(code_buf_t *buf, uint8_t dst, uint32_t imm)
{
    /* MOV r32, imm32: 48 C7 /0 for zero-extended to 64-bit */
    /* Or simpler: MOV r32, imm32 (zero-extends to 64-bit) */
    if (dst >= 8) code_buf_emit_byte(buf, 0x41);
    code_buf_emit_byte(buf, 0xB8 + (dst & 7));
    code_buf_emit_word32(buf, imm);
}

void emit_x86_mov_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src)
{
    /* MOV r64, r64: 48 89 /r */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x89);
    code_buf_emit_byte(buf, 0xC0 + ((src & 7) << 3) + (dst & 7));
}

void emit_x86_mov_reg_mem(code_buf_t *buf, uint8_t dst, uint8_t base, int32_t disp)
{
    /* MOV r64, [r64 + disp32]: 48 8B /r [base + disp32] */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x8B);

    /* ModRM: mod=10 (disp32), reg=dst, rm=100 (SIB follows) */
    code_buf_emit_byte(buf, 0x80 + ((dst & 7) << 3) + 0x04);

    /* SIB: scale=00, index=100 (none), base=base */
    code_buf_emit_byte(buf, 0x20 + (base & 7));

    /* 32-bit displacement */
    code_buf_emit_word32(buf, disp);
}

void emit_x86_mov_mem_reg(code_buf_t *buf, uint8_t base, uint8_t src, int32_t disp)
{
    /* MOV [r64 + disp32], r64: 48 89 /r [base + disp32] */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x89);

    /* ModRM: mod=10 (disp32), reg=src, rm=100 (SIB follows) */
    code_buf_emit_byte(buf, 0x80 + ((src & 7) << 3) + 0x04);

    /* SIB: scale=00, index=100 (none), base=base */
    code_buf_emit_byte(buf, 0x20 + (base & 7));

    /* 32-bit displacement */
    code_buf_emit_word32(buf, disp);
}

void emit_x86_movsx_reg_mem8(code_buf_t *buf, uint8_t dst, uint8_t base)
{
    /* MOVSX r64, byte ptr [r64]: 48 0F BE /r [base] */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0xBE);

    /* ModRM: mod=00 (no disp), reg=dst, rm=base */
    code_buf_emit_byte(buf, 0x00 + ((dst & 7) << 3) + (base & 7));
}

void emit_x86_movsx_reg_mem16(code_buf_t *buf, uint8_t dst, uint8_t base)
{
    /* MOVSX r64, word ptr [r64]: 48 0F BF /r [base] */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0xBF);

    /* ModRM: mod=00 (no disp), reg=dst, rm=base */
    code_buf_emit_byte(buf, 0x00 + ((dst & 7) << 3) + (base & 7));
}

void emit_x86_movsxd_reg_reg32(code_buf_t *buf, uint8_t dst, uint8_t src)
{
    /* MOVSXD r64, r32: 48 63 /r */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x63);
    code_buf_emit_byte(buf, 0xC0 + ((dst & 7) << 3) + (src & 7));
}

void emit_x86_movsxd_reg_mem(code_buf_t *buf, uint8_t dst, uint8_t base, int32_t disp)
{
    /* MOVSXD r64, dword ptr [r64 + disp]: 48 63 /m */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x63);

    /* ModRM and SIB for [base + disp] */
    if (disp == 0 && (base & 7) != 5) {
        /* [base] - no displacement */
        code_buf_emit_byte(buf, 0x00 + ((dst & 7) << 3) + (base & 7));
    } else if (disp >= -128 && disp <= 127) {
        /* [base + disp8] - 8-bit displacement */
        code_buf_emit_byte(buf, 0x40 + ((dst & 7) << 3) + (base & 7));
        code_buf_emit_byte(buf, disp & 0xFF);
    } else {
        /* [base + disp32] - 32-bit displacement */
        code_buf_emit_byte(buf, 0x80 + ((dst & 7) << 3) + (base & 7));
        code_buf_emit_byte(buf, disp & 0xFF);
        code_buf_emit_byte(buf, (disp >> 8) & 0xFF);
        code_buf_emit_byte(buf, (disp >> 16) & 0xFF);
        code_buf_emit_byte(buf, (disp >> 24) & 0xFF);
    }
}

void emit_x86_movzx_reg_mem8(code_buf_t *buf, uint8_t dst, uint8_t base)
{
    /* MOVZX r64, byte ptr [r64]: 48 0F B6 /r [base] */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0xB6);

    /* ModRM: mod=00 (no disp), reg=dst, rm=base */
    code_buf_emit_byte(buf, 0x00 + ((dst & 7) << 3) + (base & 7));
}

void emit_x86_movzx_reg_mem16(code_buf_t *buf, uint8_t dst, uint8_t base)
{
    /* MOVZX r64, word ptr [r64]: 48 0F B7 /r [base] */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0xB7);

    /* ModRM: mod=00 (no disp), reg=dst, rm=base */
    code_buf_emit_byte(buf, 0x00 + ((dst & 7) << 3) + (base & 7));
}

/* ============================================================================
 * x86_64 Code Emission - Arithmetic
 * ============================================================================ */

void emit_x86_add_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src)
{
    /* ADD r64, r64: 48 01 /r */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x01);
    code_buf_emit_byte(buf, 0xC0 + ((dst & 7) << 3) + (src & 7));
}

void emit_x86_add_reg_imm32(code_buf_t *buf, uint8_t dst, uint32_t imm)
{
    /* ADD r64, imm32: 48 81 /0 imm32 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x81);
    code_buf_emit_byte(buf, 0xC0 + (dst & 7));
    code_buf_emit_word32(buf, imm);
}

void emit_x86_sub_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src)
{
    /* SUB r64, r64: 48 29 /r */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x29);
    code_buf_emit_byte(buf, 0xC0 + ((dst & 7) << 3) + (src & 7));
}

void emit_x86_sub_reg_imm32(code_buf_t *buf, uint8_t dst, uint32_t imm)
{
    /* SUB r64, imm32: 48 81 /5 imm32 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x81);
    code_buf_emit_byte(buf, 0xE8 + (dst & 7));
    code_buf_emit_word32(buf, imm);
}

void emit_x86_neg_reg(code_buf_t *buf, uint8_t reg)
{
    /* NEG r64: 48 F7 /3 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0xF7);
    code_buf_emit_byte(buf, 0xD8 + (reg & 7));
}

void emit_x86_imul_reg_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    /* IMUL r64, r64, r64: 4C 0F AF /r */
    code_buf_emit_byte(buf, 0x4C);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0xAF);
    code_buf_emit_byte(buf, 0xC0 + ((dst & 7) << 3) + ((src1 & 7) | ((src2 & 8) >> 3)));
}

void emit_x86_imul_reg_reg_imm32(code_buf_t *buf, uint8_t dst, uint8_t src, uint32_t imm)
{
    /* IMUL r64, r64, imm32: 49 69 /r imm32 */
    code_buf_emit_byte(buf, 0x49);
    code_buf_emit_byte(buf, 0x69);
    code_buf_emit_byte(buf, 0xC0 + ((dst & 7) << 3) + (src & 7));
    code_buf_emit_word32(buf, imm);
}

void emit_x86_mul_reg(code_buf_t *buf, uint8_t reg)
{
    /* MUL r64: 48 F7 /4 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0xF7);
    code_buf_emit_byte(buf, 0xE0 + (reg & 7));
}

void emit_x86_div_reg(code_buf_t *buf, uint8_t reg)
{
    /* DIV r64: 48 F7 /6 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0xF7);
    code_buf_emit_byte(buf, 0xF0 + (reg & 7));
}

void emit_x86_idiv_reg(code_buf_t *buf, uint8_t reg)
{
    /* IDIV r64: 48 F7 /7 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0xF7);
    code_buf_emit_byte(buf, 0xF8 + (reg & 7));
}

/* ============================================================================
 * x86_64 Code Emission - Logical
 * ============================================================================ */

void emit_x86_and_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src)
{
    /* AND r64, r64: 48 21 /r */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x21);
    code_buf_emit_byte(buf, 0xC0 + ((dst & 7) << 3) + (src & 7));
}

void emit_x86_and_reg_imm32(code_buf_t *buf, uint8_t dst, uint32_t imm)
{
    /* AND r64, imm32: 48 81 /4 imm32 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x81);
    code_buf_emit_byte(buf, 0xE0 + (dst & 7));
    code_buf_emit_word32(buf, imm);
}

void emit_x86_or_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src)
{
    /* OR r64, r64: 48 09 /r */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x09);
    code_buf_emit_byte(buf, 0xC0 + ((dst & 7) << 3) + (src & 7));
}

void emit_x86_or_reg_imm32(code_buf_t *buf, uint8_t dst, uint32_t imm)
{
    /* OR r64, imm32: 48 81 /1 imm32 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x81);
    code_buf_emit_byte(buf, 0xC8 + (dst & 7));
    code_buf_emit_word32(buf, imm);
}

void emit_x86_xor_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src)
{
    /* XOR r64, r64: 48 31 /r */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x31);
    code_buf_emit_byte(buf, 0xC0 + ((dst & 7) << 3) + (src & 7));
}

void emit_x86_xor_reg_imm32(code_buf_t *buf, uint8_t dst, uint32_t imm)
{
    /* XOR r64, imm32: 48 81 /6 imm32 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x81);
    code_buf_emit_byte(buf, 0xF0 + (dst & 7));
    code_buf_emit_word32(buf, imm);
}

void emit_x86_not_reg(code_buf_t *buf, uint8_t reg)
{
    /* NOT r64: 48 F7 /2 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0xF7);
    code_buf_emit_byte(buf, 0xD0 + (reg & 7));
}

void emit_x86_shl_reg_imm8(code_buf_t *buf, uint8_t dst, uint8_t shift)
{
    /* SHL r64, imm8: 48 C1 /4 imm8 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0xC1);
    code_buf_emit_byte(buf, 0xE0 + (dst & 7));
    code_buf_emit_byte(buf, shift);
}

void emit_x86_shr_reg_imm8(code_buf_t *buf, uint8_t dst, uint8_t shift)
{
    /* SHR r64, imm8: 48 C1 /5 imm8 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0xC1);
    code_buf_emit_byte(buf, 0xE8 + (dst & 7));
    code_buf_emit_byte(buf, shift);
}

void emit_x86_sar_reg_imm8(code_buf_t *buf, uint8_t dst, uint8_t shift)
{
    /* SAR r64, imm8: 48 C1 /7 imm8 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0xC1);
    code_buf_emit_byte(buf, 0xF8 + (dst & 7));
    code_buf_emit_byte(buf, shift);
}

void emit_x86_rol_reg_imm8(code_buf_t *buf, uint8_t dst, uint8_t shift)
{
    /* ROL r64, imm8: 48 C1 /0 imm8 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0xC1);
    code_buf_emit_byte(buf, 0xC0 + (dst & 7));
    code_buf_emit_byte(buf, shift);
}

void emit_x86_ror_reg_imm8(code_buf_t *buf, uint8_t dst, uint8_t shift)
{
    /* ROR r64, imm8: 48 C1 /1 imm8 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0xC1);
    code_buf_emit_byte(buf, 0xC8 + (dst & 7));
    code_buf_emit_byte(buf, shift);
}

/* Shift by CL register (for variable shift amounts) */

void emit_x86_shl_reg_cl(code_buf_t *buf, uint8_t dst)
{
    /* SHL r64, CL: 48 D3 /4 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0xD3);
    code_buf_emit_byte(buf, 0xE0 + (dst & 7));
}

void emit_x86_shr_reg_cl(code_buf_t *buf, uint8_t dst)
{
    /* SHR r64, CL: 48 D3 /5 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0xD3);
    code_buf_emit_byte(buf, 0xE8 + (dst & 7));
}

void emit_x86_sar_reg_cl(code_buf_t *buf, uint8_t dst)
{
    /* SAR r64, CL: 48 D3 /7 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0xD3);
    code_buf_emit_byte(buf, 0xF8 + (dst & 7));
}

void emit_x86_ror_reg_cl(code_buf_t *buf, uint8_t dst)
{
    /* ROR r64, CL: 48 D3 /1 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0xD3);
    code_buf_emit_byte(buf, 0xC8 + (dst & 7));
}

/* ============================================================================
 * x86_64 Code Emission - Comparison and Test
 * ============================================================================ */

void emit_x86_cmp_reg_reg(code_buf_t *buf, uint8_t op1, uint8_t op2)
{
    /* CMP r64, r64: 48 39 /r */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x39);
    code_buf_emit_byte(buf, 0xC0 + ((op2 & 7) << 3) + (op1 & 7));
}

void emit_x86_cmp_reg_imm32(code_buf_t *buf, uint8_t op, uint32_t imm)
{
    /* CMP r64, imm32: 48 81 /7 imm32 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x81);
    code_buf_emit_byte(buf, 0xF8 + (op & 7));
    code_buf_emit_word32(buf, imm);
}

void emit_x86_test_reg_reg(code_buf_t *buf, uint8_t op1, uint8_t op2)
{
    /* TEST r64, r64: 48 85 /r */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x85);
    code_buf_emit_byte(buf, 0xC0 + ((op1 & 7) << 3) + (op2 & 7));
}

void emit_x86_test_reg_imm32(code_buf_t *buf, uint8_t op, uint32_t imm)
{
    /* TEST r64, imm32: 48 F7 /0 imm32 */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0xF7);
    code_buf_emit_byte(buf, 0xC0 + (op & 7));
    code_buf_emit_word32(buf, imm);
}

/* ============================================================================
 * x86_64 Code Emission - Control Flow
 * ============================================================================ */

void emit_x86_jmp_reg(code_buf_t *buf, uint8_t reg)
{
    /* JMP r64: FF /4 */
    code_buf_emit_byte(buf, 0xFF);
    code_buf_emit_byte(buf, 0xE0 + (reg & 7));
}

void emit_x86_jmp_rel32(code_buf_t *buf, uint64_t target, uint64_t current)
{
    /* JMP rel32: E9 rel32 */
    int32_t rel = (int32_t)(target - current - 5);
    code_buf_emit_byte(buf, 0xE9);
    code_buf_emit_word32(buf, rel);
}

void emit_x86_call_reg(code_buf_t *buf, uint8_t reg)
{
    /* CALL r64: FF /2 */
    code_buf_emit_byte(buf, 0xFF);
    code_buf_emit_byte(buf, 0xD0 + (reg & 7));
}

void emit_x86_call_rel32(code_buf_t *buf, uint64_t target, uint64_t current)
{
    /* CALL rel32: E8 rel32 */
    int32_t rel = (int32_t)(target - current - 5);
    code_buf_emit_byte(buf, 0xE8);
    code_buf_emit_word32(buf, rel);
}

void emit_x86_ret(code_buf_t *buf)
{
    /* RET: C3 */
    code_buf_emit_byte(buf, 0xC3);
}

void emit_x86_ret_imm16(code_buf_t *buf, uint16_t imm)
{
    /* RET imm16: C2 imm16 */
    code_buf_emit_byte(buf, 0xC2);
    code_buf_emit_byte(buf, imm & 0xFF);
    code_buf_emit_byte(buf, (imm >> 8) & 0xFF);
}

void emit_x86_nop(code_buf_t *buf)
{
    /* NOP: 90 */
    code_buf_emit_byte(buf, 0x90);
}

void emit_x86_int3(code_buf_t *buf)
{
    /* INT3: CC */
    code_buf_emit_byte(buf, 0xCC);
}

/* ============================================================================
 * x86_64 Code Emission - Conditional Jumps
 * ============================================================================ */

void emit_x86_je_rel32(code_buf_t *buf, uint64_t target, uint64_t current)
{
    /* JE/JZ rel32: 0F 84 rel32 */
    int32_t rel = (int32_t)(target - current - 6);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x84);
    code_buf_emit_word32(buf, rel);
}

void emit_x86_jne_rel32(code_buf_t *buf, uint64_t target, uint64_t current)
{
    /* JNE/JNZ rel32: 0F 85 rel32 */
    int32_t rel = (int32_t)(target - current - 6);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x85);
    code_buf_emit_word32(buf, rel);
}

void emit_x86_jl_rel32(code_buf_t *buf, uint64_t target, uint64_t current)
{
    /* JL/JNGE rel32: 0F 8C rel32 */
    int32_t rel = (int32_t)(target - current - 6);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x8C);
    code_buf_emit_word32(buf, rel);
}

void emit_x86_jle_rel32(code_buf_t *buf, uint64_t target, uint64_t current)
{
    /* JLE/JNG rel32: 0F 8E rel32 */
    int32_t rel = (int32_t)(target - current - 6);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x8E);
    code_buf_emit_word32(buf, rel);
}

void emit_x86_jg_rel32(code_buf_t *buf, uint64_t target, uint64_t current)
{
    /* JG/JNLE rel32: 0F 8F rel32 */
    int32_t rel = (int32_t)(target - current - 6);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x8F);
    code_buf_emit_word32(buf, rel);
}

void emit_x86_jge_rel32(code_buf_t *buf, uint64_t target, uint64_t current)
{
    /* JGE/JNL rel32: 0F 8D rel32 */
    int32_t rel = (int32_t)(target - current - 6);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x8D);
    code_buf_emit_word32(buf, rel);
}

void emit_x86_ja_rel32(code_buf_t *buf, uint64_t target, uint64_t current)
{
    /* JA/JNBE rel32: 0F 87 rel32 */
    int32_t rel = (int32_t)(target - current - 6);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x87);
    code_buf_emit_word32(buf, rel);
}

void emit_x86_jae_rel32(code_buf_t *buf, uint64_t target, uint64_t current)
{
    /* JAE/JNB rel32: 0F 83 rel32 */
    int32_t rel = (int32_t)(target - current - 6);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x83);
    code_buf_emit_word32(buf, rel);
}

void emit_x86_jb_rel32(code_buf_t *buf, uint64_t target, uint64_t current)
{
    /* JB/JNAE rel32: 0F 82 rel32 */
    int32_t rel = (int32_t)(target - current - 6);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x82);
    code_buf_emit_word32(buf, rel);
}

void emit_x86_jbe_rel32(code_buf_t *buf, uint64_t target, uint64_t current)
{
    /* JBE/JNA rel32: 0F 86 rel32 */
    int32_t rel = (int32_t)(target - current - 6);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x86);
    code_buf_emit_word32(buf, rel);
}

void emit_x86_js_rel32(code_buf_t *buf, uint64_t target, uint64_t current)
{
    /* JS rel32: 0F 88 rel32 */
    int32_t rel = (int32_t)(target - current - 6);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x88);
    code_buf_emit_word32(buf, rel);
}

void emit_x86_jns_rel32(code_buf_t *buf, uint64_t target, uint64_t current)
{
    /* JNS rel32: 0F 89 rel32 */
    int32_t rel = (int32_t)(target - current - 6);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x89);
    code_buf_emit_word32(buf, rel);
}

void emit_x86_jo_rel32(code_buf_t *buf, uint64_t target, uint64_t current)
{
    /* JO rel32: 0F 80 rel32 */
    int32_t rel = (int32_t)(target - current - 6);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x80);
    code_buf_emit_word32(buf, rel);
}

void emit_x86_jno_rel32(code_buf_t *buf, uint64_t target, uint64_t current)
{
    /* JNO rel32: 0F 81 rel32 */
    int32_t rel = (int32_t)(target - current - 6);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x81);
    code_buf_emit_word32(buf, rel);
}

/* ============================================================================
 * x86_64 Code Emission - Conditional Move (CMOV)
 * ============================================================================ */

void emit_x86_cmove_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src)
{
    /* CMOVE/CMOVZ r64, r64: 48 0F 44 /r */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x44);
    code_buf_emit_byte(buf, 0xC0 + ((dst & 7) << 3) + (src & 7));
}

void emit_x86_cmovne_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src)
{
    /* CMOVNE/CMOVNZ r64, r64: 48 0F 45 /r */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x45);
    code_buf_emit_byte(buf, 0xC0 + ((dst & 7) << 3) + (src & 7));
}

void emit_x86_cmovl_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src)
{
    /* CMOVL/CMOVNGE r64, r64: 48 0F 4C /r */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x4C);
    code_buf_emit_byte(buf, 0xC0 + ((dst & 7) << 3) + (src & 7));
}

void emit_x86_cmovle_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src)
{
    /* CMOVLE/CMOVNG r64, r64: 48 0F 4E /r */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x4E);
    code_buf_emit_byte(buf, 0xC0 + ((dst & 7) << 3) + (src & 7));
}

void emit_x86_cmovg_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src)
{
    /* CMOVG/CMOVNLE r64, r64: 48 0F 4F /r */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x4F);
    code_buf_emit_byte(buf, 0xC0 + ((dst & 7) << 3) + (src & 7));
}

void emit_x86_cmovge_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src)
{
    /* CMOVGE/CMOVNL r64, r64: 48 0F 4D /r */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x4D);
    code_buf_emit_byte(buf, 0xC0 + ((dst & 7) << 3) + (src & 7));
}

void emit_x86_cmova_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src)
{
    /* CMOVA/CMOVNBE r64, r64: 48 0F 47 /r */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x47);
    code_buf_emit_byte(buf, 0xC0 + ((dst & 7) << 3) + (src & 7));
}

void emit_x86_cmovae_reg_reg(code_buf_t *buf, uint8_t dst, uint8_t src)
{
    /* CMOVAE/CMOVNB r64, r64: 48 0F 43 /r */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0x43);
    code_buf_emit_byte(buf, 0xC0 + ((dst & 7) << 3) + (src & 7));
}

/* ============================================================================
 * x86_64 Code Emission - Stack Operations
 * ============================================================================ */

void emit_x86_push_reg(code_buf_t *buf, uint8_t reg)
{
    /* PUSH r64: 50+rd */
    code_buf_emit_byte(buf, 0x50 + (reg & 7));
}

void emit_x86_pop_reg(code_buf_t *buf, uint8_t reg)
{
    /* POP r64: 58+rd */
    code_buf_emit_byte(buf, 0x58 + (reg & 7));
}

void emit_x86_pushf(code_buf_t *buf)
{
    /* PUSHFQ: 9C */
    code_buf_emit_byte(buf, 0x9C);
}

void emit_x86_popf(code_buf_t *buf)
{
    /* POPFQ: 9D */
    code_buf_emit_byte(buf, 0x9D);
}

void emit_x86_lea_reg_mem(code_buf_t *buf, uint8_t dst, uint8_t base, int32_t disp)
{
    /* LEA r64, [r64 + disp32]: 48 8D /r [base + disp32] */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x8D);

    /* ModRM: mod=10 (disp32), reg=dst, rm=100 (SIB follows) */
    code_buf_emit_byte(buf, 0x80 + ((dst & 7) << 3) + 0x04);

    /* SIB: scale=00, index=100 (none), base=base */
    code_buf_emit_byte(buf, 0x20 + (base & 7));

    /* 32-bit displacement */
    code_buf_emit_word32(buf, disp);
}

/* ============================================================================
 * x86_64 Code Emission - Prefixes
 * ============================================================================ */

void emit_x86_rex(code_buf_t *buf, uint8_t w, uint8_t r, uint8_t x, uint8_t b)
{
    /* REX prefix: 0100WRXB */
    uint8_t rex = 0x40 | ((w & 1) << 3) | ((r & 1) << 2) | ((x & 1) << 1) | (b & 1);
    code_buf_emit_byte(buf, rex);
}

/* ============================================================================
 * x86_64 Code Emission - Memory Store (byte/word)
 * ============================================================================ */

void emit_x86_mov_mem8_reg(code_buf_t *buf, uint8_t base, uint8_t src, int32_t disp)
{
    /* MOV byte ptr [r64 + disp32], r8 (low 8 bits of src) */
    /* 48 88 /r [base + disp32] */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x88);

    /* ModRM: mod=10 (disp32), reg=src (low 3 bits), rm=100 (SIB) */
    code_buf_emit_byte(buf, 0x80 + ((src & 7) << 3) + 0x04);

    /* SIB: scale=00, index=100 (none), base=base */
    code_buf_emit_byte(buf, 0x20 + (base & 7));

    /* 32-bit displacement */
    code_buf_emit_word32(buf, disp);
}

void emit_x86_mov_mem16_reg(code_buf_t *buf, uint8_t base, uint8_t src, int32_t disp)
{
    /* MOV word ptr [r64 + disp32], r16 (low 16 bits of src) */
    /* 66 48 89 /r [base + disp32] */
    code_buf_emit_byte(buf, 0x66);  /* Operand size override */
    code_buf_emit_byte(buf, 0x48);  /* REX.W */
    code_buf_emit_byte(buf, 0x89);

    /* ModRM: mod=10 (disp32), reg=src (low 3 bits), rm=100 (SIB) */
    code_buf_emit_byte(buf, 0x80 + ((src & 7) << 3) + 0x04);

    /* SIB: scale=00, index=100 (none), base=base */
    code_buf_emit_byte(buf, 0x20 + (base & 7));

    /* 32-bit displacement */
    code_buf_emit_word32(buf, disp);
}

void emit_x86_mov_mem_reg32(code_buf_t *buf, uint8_t base, uint8_t src, int32_t disp)
{
    /* MOV dword ptr [r64 + disp32], r32 (zero-extends to 64 bits in memory) */
    /* 48 89 /r [base + disp32] - but only writes 32 bits */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x89);

    /* ModRM: mod=10 (disp32), reg=src (low 3 bits), rm=100 (SIB) */
    code_buf_emit_byte(buf, 0x80 + ((src & 7) << 3) + 0x04);

    /* SIB: scale=00, index=100 (none), base=base */
    code_buf_emit_byte(buf, 0x20 + (base & 7));

    /* 32-bit displacement */
    code_buf_emit_word32(buf, disp);
}

/* ============================================================================
 * x86_64 Code Emission - Load with Zero-Extend (byte/word with disp)
 * ============================================================================ */

void emit_x86_movzx_reg_mem8_disp(code_buf_t *buf, uint8_t dst, uint8_t base, int32_t disp)
{
    /* MOVZX r64, byte ptr [r64 + disp32]: 48 0F B6 /r [base + disp32] */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0xB6);

    /* ModRM: mod=10 (disp32), reg=dst, rm=100 (SIB) */
    code_buf_emit_byte(buf, 0x80 + ((dst & 7) << 3) + 0x04);

    /* SIB: scale=00, index=100 (none), base=base */
    code_buf_emit_byte(buf, 0x20 + (base & 7));

    /* 32-bit displacement */
    code_buf_emit_word32(buf, disp);
}

void emit_x86_movzx_reg_mem16_disp(code_buf_t *buf, uint8_t dst, uint8_t base, int32_t disp)
{
    /* MOVZX r64, word ptr [r64 + disp32]: 48 0F B7 /r [base + disp32] */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0xB7);

    /* ModRM: mod=10 (disp32), reg=dst, rm=100 (SIB) */
    code_buf_emit_byte(buf, 0x80 + ((dst & 7) << 3) + 0x04);

    /* SIB: scale=00, index=100 (none), base=base */
    code_buf_emit_byte(buf, 0x20 + (base & 7));

    /* 32-bit displacement */
    code_buf_emit_word32(buf, disp);
}

void emit_x86_movsx_reg_mem8_disp(code_buf_t *buf, uint8_t dst, uint8_t base, int32_t disp)
{
    /* MOVSX r64, byte ptr [r64 + disp32]: 48 0F BE /r [base + disp32] */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0xBE);

    /* ModRM: mod=10 (disp32), reg=dst, rm=100 (SIB) */
    code_buf_emit_byte(buf, 0x80 + ((dst & 7) << 3) + 0x04);

    /* SIB: scale=00, index=100 (none), base=base */
    code_buf_emit_byte(buf, 0x20 + (base & 7));

    /* 32-bit displacement */
    code_buf_emit_word32(buf, disp);
}

void emit_x86_movsx_reg_mem16_disp(code_buf_t *buf, uint8_t dst, uint8_t base, int32_t disp)
{
    /* MOVSX r64, word ptr [r64 + disp32]: 48 0F BF /r [base + disp32] */
    code_buf_emit_byte(buf, 0x48);
    code_buf_emit_byte(buf, 0x0F);
    code_buf_emit_byte(buf, 0xBF);

    /* ModRM: mod=10 (disp32), reg=dst, rm=100 (SIB) */
    code_buf_emit_byte(buf, 0x80 + ((dst & 7) << 3) + 0x04);

    /* SIB: scale=00, index=100 (none), base=base */
    code_buf_emit_byte(buf, 0x20 + (base & 7));

    /* 32-bit displacement */
    code_buf_emit_word32(buf, disp);
}
