/* ============================================================================
 * Rosetta Memory Translation Module
 * ============================================================================
 *
 * This module handles translation of x86_64 memory operations including
 * MOV, MOVZX, MOVSX, LEA, PUSH, and POP instructions.
 * ============================================================================ */

#include "rosetta_translate_memory.h"
#include "rosetta_arm64_emit.h"
#include <stdint.h>

/* ============================================================================
 * Memory Translation Functions
 * ============================================================================ */

void translate_memory_mov(code_buffer_t *code_buf, const x86_insn_t *insn,
                          uint8_t arm_rd, uint8_t arm_rm)
{
    if (x86_is_mov_imm64(insn)) {
        /* MOV r64, imm64 - use MOVZ + MOVK sequence */
        uint64_t imm = (uint64_t)insn->imm;
        emit_movz(code_buf, arm_rd, (uint16_t)(imm & 0xFFFF), 0);
        if (imm >> 16) emit_movk(code_buf, arm_rd, (uint16_t)((imm >> 16) & 0xFFFF), 1);
        if (imm >> 32) emit_movk(code_buf, arm_rd, (uint16_t)((imm >> 32) & 0xFFFF), 2);
        if (imm >> 48) emit_movk(code_buf, arm_rd, (uint16_t)((imm >> 48) & 0xFFFF), 3);
    } else if (insn->has_modrm && insn->mod == 3) {
        /* MOV: register to register */
        emit_mov_reg(code_buf, arm_rd, arm_rm);
    } else if (!insn->has_modrm || insn->mod != 3) {
        /* Memory operand: load from memory
         * The arm_rm should contain the address
         * Use LDR to load from [arm_rm]
         */
        emit_ldr_reg(code_buf, arm_rd, arm_rm, XZR);
    }
}

void translate_memory_movzx(code_buffer_t *code_buf, const x86_insn_t *insn,
                            uint8_t arm_rd, uint8_t arm_rm)
{
    /* MOVZX: zero-extend r8/m8 or r16/m16 to r64
     * Opcode 0F B6 = MOVZX r32, r8/m8
     * Opcode 0F B7 = MOVZX r32, r16/m16
     */
    int is_16bit = (insn->opcode2 == 0xB7);
    if (is_16bit) {
        /* Zero-extend from 16-bit: UXTH */
        emit_mov_extend(code_buf, arm_rd, arm_rm, 0, 1);
    } else {
        /* Zero-extend from 8-bit: UXTB */
        emit_mov_extend(code_buf, arm_rd, arm_rm, 0, 0);
    }
}

void translate_memory_movsx(code_buffer_t *code_buf, const x86_insn_t *insn,
                            uint8_t arm_rd, uint8_t arm_rm)
{
    /* MOVSX: sign-extend r8/m8 or r16/m16 to r64
     * Opcode 0F BE = MOVSX r32, r8/m8
     * Opcode 0F BF = MOVSX r32, r16/m16
     */
    int is_16bit = (insn->opcode2 == 0xBF);
    if (is_16bit) {
        /* Sign-extend from 16-bit: SXTH */
        emit_mov_extend(code_buf, arm_rd, arm_rm, 1, 1);
    } else {
        /* Sign-extend from 8-bit: SXTB */
        emit_mov_extend(code_buf, arm_rd, arm_rm, 1, 0);
    }
}

void translate_memory_movsxd(code_buffer_t *code_buf, const x86_insn_t *insn,
                             uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* MOVSXD: sign-extend r32 to r64
     * SXTW: sign-extend 32-bit to 64-bit
     * Encoding: 93407C00 | Rd | (Rn << 5)
     */
    emit_arm64_insn(code_buf, 0x93407C00 | ((arm_rd & 0x1F) << 0) | ((arm_rm & 0x1F) << 5));
}

void translate_memory_lea(code_buffer_t *code_buf, const x86_insn_t *insn,
                          uint8_t arm_rd)
{
    /* LEA: Load Effective Address
     * x86: lea reg, [base + index*scale + disp]
     * ARM64: compute the effective address and store in arm_rd
     *
     * Addressing modes:
     * - [disp] (absolute)
     * - [base + disp]
     * - [base + index*scale + disp]
     * - RIP-relative: [RIP + disp32]
     */

    uint8_t mod = insn->mod;
    uint8_t rm = insn->rm;
    int64_t disp = insn->disp;

    /* Map x86 registers to ARM64 */
    static const uint8_t x86_to_arm[8] = {0, 1, 2, 3, 4, 5, 6, 7};

    /* Handle RIP-relative addressing (mod == 0, rm == 5) */
    if (mod == 0 && rm == 5) {
        /* RIP-relative: PC-relative addressing */
        /* For now, load the displacement as immediate */
        /* In full impl, would compute PC + disp */
        uint64_t addr = (uint64_t)disp;
        emit_movz(code_buf, arm_rd, (uint16_t)(addr & 0xFFFF), 0);
        if (addr >> 16) emit_movk(code_buf, arm_rd, (uint16_t)((addr >> 16) & 0xFFFF), 1);
        if (addr >> 32) emit_movk(code_buf, arm_rd, (uint16_t)((addr >> 32) & 0xFFFF), 2);
        if (addr >> 48) emit_movk(code_buf, arm_rd, (uint16_t)((addr >> 48) & 0xFFFF), 3);
        return;
    }

    /* Handle register-indirect with displacement: [base + disp] */
    if (rm != 4) {  /* rm != 4 means no SIB byte */
        uint8_t base_reg = x86_to_arm[rm & 0x7];

        if (mod == 0 && (rm & 0x7) == 5) {
            /* disp32 only (no base register) */
            uint64_t udisp = (uint64_t)disp;
            emit_movz(code_buf, arm_rd, (uint16_t)(udisp & 0xFFFF), 0);
            if (udisp >> 16) emit_movk(code_buf, arm_rd, (uint16_t)((udisp >> 16) & 0xFFFF), 1);
            if (udisp >> 32) emit_movk(code_buf, arm_rd, (uint16_t)((udisp >> 32) & 0xFFFF), 2);
            if (udisp >> 48) emit_movk(code_buf, arm_rd, (uint16_t)((udisp >> 48) & 0xFFFF), 3);
        } else if (disp == 0) {
            /* Simple register move: addr = base */
            emit_mov_reg(code_buf, arm_rd, base_reg);
        } else if (disp >= -256 && disp <= 255) {
            /* Small displacement: use ADD/SUB immediate */
            emit_mov_reg(code_buf, arm_rd, base_reg);
            if (disp > 0) {
                emit_add_imm(code_buf, arm_rd, arm_rd, (uint16_t)disp);
            } else if (disp < 0) {
                emit_sub_imm(code_buf, arm_rd, arm_rd, (uint16_t)(-disp));
            }
        } else {
            /* Large displacement: load disp then add */
            uint64_t udisp = (uint64_t)disp;
            emit_movz(code_buf, arm_rd, (uint16_t)(udisp & 0xFFFF), 0);
            if (udisp >> 16) emit_movk(code_buf, arm_rd, (uint16_t)((udisp >> 16) & 0xFFFF), 1);
            if (udisp >> 32) emit_movk(code_buf, arm_rd, (uint16_t)((udisp >> 32) & 0xFFFF), 2);
            if (udisp >> 48) emit_movk(code_buf, arm_rd, (uint16_t)((udisp >> 48) & 0xFFFF), 3);
            emit_add_reg(code_buf, arm_rd, arm_rd, base_reg);
        }
    } else {
        /* SIB byte present - handle base + index*scale + disp */
        /* This is a more complex case; for now handle common subcase */
        /* In full impl, decode SIB byte for index, scale, base */
        if (disp == 0) {
            /* [base] with SIB - just copy base */
            uint8_t base_reg = x86_to_arm[rm & 0x7];
            emit_mov_reg(code_buf, arm_rd, base_reg);
        } else {
            /* Load displacement and add base */
            uint64_t udisp = (uint64_t)disp;
            emit_movz(code_buf, arm_rd, (uint16_t)(udisp & 0xFFFF), 0);
            if (udisp >> 16) emit_movk(code_buf, arm_rd, (uint16_t)((udisp >> 16) & 0xFFFF), 1);
            if (udisp >> 32) emit_movk(code_buf, arm_rd, (uint16_t)((udisp >> 32) & 0xFFFF), 2);
            if (udisp >> 48) emit_movk(code_buf, arm_rd, (uint16_t)((udisp >> 48) & 0xFFFF), 3);
            uint8_t base_reg = x86_to_arm[rm & 0x7];
            emit_add_reg(code_buf, arm_rd, arm_rd, base_reg);
        }
    }
}

void translate_memory_push(code_buffer_t *code_buf, const x86_insn_t *insn,
                           uint8_t arm_rd)
{
    (void)insn;
    /* PUSH: decrement SP by 8 and store register
     * ARM64: STR reg, [SP, #-8]!
     * Or use the emit helper
     */
    emit_push_reg(code_buf, arm_rd);
}

void translate_memory_pop(code_buffer_t *code_buf, const x86_insn_t *insn,
                          uint8_t arm_rd)
{
    (void)insn;
    /* POP: load from SP and increment by 8
     * ARM64: LDR reg, [SP], #8
     * Or use the emit helper
     */
    emit_pop_reg(code_buf, arm_rd);
}

void translate_memory_cmp(code_buffer_t *code_buf, const x86_insn_t *insn,
                          uint8_t arm_rd, uint8_t arm_rm)
{
    /* CMP: compare and set flags (does SUBS without storing result) */
    if (insn->has_modrm && insn->mod == 3) {
        /* Register comparison */
        emit_cmp_reg(code_buf, arm_rd, arm_rm);
    } else if (insn->imm_size > 0) {
        /* Compare with immediate - use CMN or load immediate first */
        uint64_t imm = (uint64_t)insn->imm;
        if (imm == 0) {
            /* CMP reg, 0 -> just check if reg is zero */
            emit_cmp_reg(code_buf, arm_rd, XZR);
        } else if (imm <= 0xFFFF) {
            /* Small immediate - load into temp and compare */
            uint8_t tmp = 16;
            emit_movz(code_buf, tmp, (uint16_t)imm, 0);
            emit_cmp_reg(code_buf, arm_rd, tmp);
        } else {
            /* Large immediate */
            uint8_t tmp = 16;
            emit_movz(code_buf, tmp, (uint16_t)(imm & 0xFFFF), 0);
            if (imm >> 16) emit_movk(code_buf, tmp, (uint16_t)((imm >> 16) & 0xFFFF), 1);
            if (imm >> 32) emit_movk(code_buf, tmp, (uint16_t)((imm >> 32) & 0xFFFF), 2);
            if (imm >> 48) emit_movk(code_buf, tmp, (uint16_t)((imm >> 48) & 0xFFFF), 3);
            emit_cmp_reg(code_buf, arm_rd, tmp);
        }
    } else {
        /* Memory operand - arm_rm should have the loaded value */
        emit_cmp_reg(code_buf, arm_rd, arm_rm);
    }
}

void translate_memory_test(code_buffer_t *code_buf, const x86_insn_t *insn,
                           uint8_t arm_rd, uint8_t arm_rm)
{
    /* TEST: AND without storing result, just sets flags */
    if (insn->has_modrm && insn->mod == 3) {
        /* Register test */
        emit_tst_reg(code_buf, arm_rd, arm_rm);
    } else if (insn->imm_size > 0) {
        /* Test with immediate */
        uint64_t imm = (uint64_t)insn->imm;
        uint8_t tmp = 16;
        emit_movz(code_buf, tmp, (uint16_t)(imm & 0xFFFF), 0);
        if (imm >> 16) emit_movk(code_buf, tmp, (uint16_t)((imm >> 16) & 0xFFFF), 1);
        if (imm >> 32) emit_movk(code_buf, tmp, (uint16_t)((imm >> 32) & 0xFFFF), 2);
        if (imm >> 48) emit_movk(code_buf, tmp, (uint16_t)((imm >> 48) & 0xFFFF), 3);
        emit_tst_reg(code_buf, arm_rd, tmp);
    } else {
        emit_tst_reg(code_buf, arm_rd, arm_rm);
    }
}

/* End of rosetta_translate_memory.c */
