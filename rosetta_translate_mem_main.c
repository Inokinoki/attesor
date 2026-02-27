/* ============================================================================
 * Rosetta Translator - Memory Instruction Translation Implementation
 * ============================================================================
 *
 * This module implements translation of ARM64 memory instructions to x86_64.
 * Supported instructions: LDR (immediate), STR (immediate), LDP, STP
 * ============================================================================ */

#include "rosetta_translate_mem_main.h"
#include "rosetta_refactored.h"

/* External register mapping function from ALU module */
extern uint8_t translate_get_x86_reg(uint8_t arm_reg);

/**
 * translate_mem_ldr_imm - Translate LDR (immediate) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_mem_ldr_imm(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t size = (encoding >> 2) & 0x03;
    uint16_t imm12 = ((encoding >> 10) & 0xFFF) << size;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint64_t addr;

    /* Calculate address from guest state */
    addr = guest_state[rn] + imm12;

    /* Update guest state based on size - for interpreter fallback */
    switch (size) {
        case 0: /* Byte */
            guest_state[rd] = *(uint8_t *)(uintptr_t)addr;
            break;
        case 1: /* Halfword */
            guest_state[rd] = *(uint16_t *)(uintptr_t)addr;
            break;
        case 2: /* Word */
            guest_state[rd] = *(uint32_t *)(uintptr_t)addr;
            break;
        case 3: /* Doubleword */
            guest_state[rd] = *(uint64_t *)(uintptr_t)addr;
            break;
    }

    /* Emit x86_64 code for load with proper size handling */
    switch (size) {
        case 0: /* Byte - MOVZX r64, byte ptr [rn + disp] */
            emit_x86_movzx_reg_mem8_disp(code_buf, x86_rd, x86_rn, imm12);
            break;
        case 1: /* Halfword - MOVZX r64, word ptr [rn + disp] */
            emit_x86_movzx_reg_mem16_disp(code_buf, x86_rd, x86_rn, imm12);
            break;
        case 2: /* Word - MOV r32, dword ptr [rn + disp] (zero-extends) */
            emit_x86_mov_reg_mem(code_buf, x86_rd, x86_rn, imm12);
            break;
        case 3: /* Doubleword - MOV r64, qword ptr [rn + disp] */
            emit_x86_mov_reg_mem(code_buf, x86_rd, x86_rn, imm12);
            break;
    }

    return 0;
}

/**
 * translate_mem_str_imm - Translate STR (immediate) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_mem_str_imm(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t size = (encoding >> 2) & 0x03;
    uint16_t imm12 = ((encoding >> 10) & 0xFFF) << size;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint64_t addr;

    /* Calculate address from guest state */
    addr = guest_state[rn] + imm12;

    /* Update guest state based on size - for interpreter fallback */
    switch (size) {
        case 0: /* Byte */
            *(uint8_t *)(uintptr_t)addr = (uint8_t)guest_state[rd];
            break;
        case 1: /* Halfword */
            *(uint16_t *)(uintptr_t)addr = (uint16_t)guest_state[rd];
            break;
        case 2: /* Word */
            *(uint32_t *)(uintptr_t)addr = (uint32_t)guest_state[rd];
            break;
        case 3: /* Doubleword */
            *(uint64_t *)(uintptr_t)addr = guest_state[rd];
            break;
    }

    /* Emit x86_64 code for store with proper size handling */
    switch (size) {
        case 0: /* Byte - MOV byte ptr [rn + disp], rd (low 8 bits) */
            emit_x86_mov_mem8_reg(code_buf, x86_rn, x86_rd, imm12);
            break;
        case 1: /* Halfword - MOV word ptr [rn + disp], rd (low 16 bits) */
            emit_x86_mov_mem16_reg(code_buf, x86_rn, x86_rd, imm12);
            break;
        case 2: /* Word - MOV dword ptr [rn + disp], rd (low 32 bits) */
            emit_x86_mov_mem_reg32(code_buf, x86_rn, x86_rd, imm12);
            break;
        case 3: /* Doubleword - MOV qword ptr [rn + disp], rd */
            emit_x86_mov_mem_reg(code_buf, x86_rn, x86_rd, imm12);
            break;
    }

    return 0;
}

/**
 * translate_mem_ldp - Translate LDP (load pair) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_mem_ldp(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rt2 = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rt2 = translate_get_x86_reg(rt2);
    int64_t offset;
    uint64_t addr;

    /* Extract signed offset */
    offset = (int64_t)((int32_t)((encoding & 0x003FC000) << 8)) >> 8;
    offset *= 8;  /* Scale for doubleword pair */

    /* Load two registers from consecutive addresses */
    addr = guest_state[rn] + offset;
    guest_state[rd] = *(uint64_t *)(uintptr_t)addr;
    guest_state[rt2] = *(uint64_t *)(uintptr_t)(addr + 8);

    /* Emit x86_64 code for LDP */
    /* Load first register */
    emit_x86_mov_reg_mem(code_buf, x86_rd, x86_rn, (int32_t)offset);

    /* Load second register */
    emit_x86_mov_reg_mem(code_buf, x86_rt2, x86_rn, (int32_t)offset + 8);

    return 0;
}

/**
 * translate_mem_stp - Translate STP (store pair) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_mem_stp(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rt2 = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rt2 = translate_get_x86_reg(rt2);
    int64_t offset;
    uint64_t addr;

    /* Extract signed offset */
    offset = (int64_t)((int32_t)((encoding & 0x003FC000) << 8)) >> 8;
    offset *= 8;  /* Scale for doubleword pair */

    /* Store two registers to consecutive addresses */
    addr = guest_state[rn] + offset;
    *(uint64_t *)(uintptr_t)addr = guest_state[rd];
    *(uint64_t *)(uintptr_t)(addr + 8) = guest_state[rt2];

    /* Emit x86_64 code for STP */
    /* Store first register */
    emit_x86_mov_mem_reg(code_buf, x86_rn, x86_rd, (int32_t)offset);

    /* Store second register */
    emit_x86_mov_mem_reg(code_buf, x86_rn, x86_rt2, (int32_t)offset + 8);

    return 0;
}

/**
 * translate_mem_ldrb - Translate LDRB (load register byte) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_mem_ldrb(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint16_t imm12 = (encoding >> 10) & 0xFFF;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint64_t addr;

    addr = guest_state[rn] + imm12;
    guest_state[rd] = *(uint8_t *)(uintptr_t)addr;  /* Zero-extend */

    /* Emit MOVZX r64, byte ptr [rn + disp] */
    emit_x86_movzx_reg_mem8_disp(code_buf, x86_rd, x86_rn, imm12);

    return 0;
}

/**
 * translate_mem_strb - Translate STRB (store register byte) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_mem_strb(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint16_t imm12 = (encoding >> 10) & 0xFFF;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint64_t addr;

    addr = guest_state[rn] + imm12;
    *(uint8_t *)(uintptr_t)addr = (uint8_t)guest_state[rd];

    /* Emit MOV byte ptr [rn + disp], rd */
    emit_x86_mov_mem8_reg(code_buf, x86_rn, x86_rd, imm12);

    return 0;
}

/**
 * translate_mem_ldrh - Translate LDRH (load register halfword) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_mem_ldrh(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint16_t imm12 = ((encoding >> 10) & 0x7FFF) << 1;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint64_t addr;

    addr = guest_state[rn] + imm12;
    guest_state[rd] = *(uint16_t *)(uintptr_t)addr;  /* Zero-extend */

    /* Emit MOVZX r64, word ptr [rn + disp] */
    emit_x86_movzx_reg_mem16_disp(code_buf, x86_rd, x86_rn, imm12);

    return 0;
}

/**
 * translate_mem_strh - Translate STRH (store register halfword) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_mem_strh(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint16_t imm12 = ((encoding >> 10) & 0x7FFF) << 1;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint64_t addr;

    addr = guest_state[rn] + imm12;
    *(uint16_t *)(uintptr_t)addr = (uint16_t)guest_state[rd];

    /* Emit MOV word ptr [rn + disp], rd */
    emit_x86_mov_mem16_reg(code_buf, x86_rn, x86_rd, imm12);

    return 0;
}

/**
 * translate_mem_ldrsb - Translate LDRSB (load register signed byte) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_mem_ldrsb(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint16_t imm12 = (encoding >> 10) & 0xFFF;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint64_t addr;

    addr = guest_state[rn] + imm12;
    guest_state[rd] = (uint64_t)(int64_t)*(int8_t *)(uintptr_t)addr;  /* Sign-extend */

    /* Emit MOVSX r64, byte ptr [rn + disp] */
    emit_x86_movsx_reg_mem8_disp(code_buf, x86_rd, x86_rn, imm12);

    return 0;
}

/**
 * translate_mem_ldrsh - Translate LDRSH (load register signed halfword) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_mem_ldrsh(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint16_t imm12 = ((encoding >> 10) & 0x7FFF) << 1;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint64_t addr;

    addr = guest_state[rn] + imm12;
    guest_state[rd] = (uint64_t)(int64_t)*(int16_t *)(uintptr_t)addr;  /* Sign-extend */

    /* Emit MOVSX r64, word ptr [rn + disp] */
    emit_x86_movsx_reg_mem16_disp(code_buf, x86_rd, x86_rn, imm12);

    return 0;
}

/**
 * translate_mem_ldrsw - Translate LDRSW (load register signed word) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_mem_ldrsw(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint16_t imm12 = ((encoding >> 10) & 0x7FFF) << 2;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint64_t addr;

    addr = guest_state[rn] + imm12;
    guest_state[rd] = (uint64_t)(int64_t)*(int32_t *)(uintptr_t)addr;  /* Sign-extend */

    /* Emit MOVSXD r64, dword ptr [rn + disp] */
    emit_x86_movsxd_reg_mem(code_buf, x86_rd, x86_rn, imm12);

    return 0;
}

/**
 * translate_mem_ldur - Translate LDUR (load register unscaled) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_mem_ldur(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t size = (encoding >> 30) & 3;
    int16_t imm9 = (int16_t)((encoding >> 12) & 0x1FF);
    int16_t offset = (imm9 << 7) >> 7;  /* Sign-extend to 16 bits */
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint64_t addr;

    offset <<= size;  /* Scale by access size */
    addr = guest_state[rn] + offset;

    switch (size) {
        case 0: /* Byte */
            guest_state[rd] = *(uint8_t *)(uintptr_t)addr;
            emit_x86_movzx_reg_mem8_disp(code_buf, x86_rd, x86_rn, offset);
            break;
        case 1: /* Halfword */
            guest_state[rd] = *(uint16_t *)(uintptr_t)addr;
            emit_x86_movzx_reg_mem16_disp(code_buf, x86_rd, x86_rn, offset);
            break;
        case 2: /* Word */
            guest_state[rd] = *(uint32_t *)(uintptr_t)addr;
            emit_x86_mov_reg_mem(code_buf, x86_rd, x86_rn, offset);
            break;
        case 3: /* Doubleword */
            guest_state[rd] = *(uint64_t *)(uintptr_t)addr;
            emit_x86_mov_reg_mem(code_buf, x86_rd, x86_rn, offset);
            break;
    }

    return 0;
}

/**
 * translate_mem_stur - Translate STUR (store register unscaled) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_mem_stur(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t size = (encoding >> 30) & 3;
    int16_t imm9 = (int16_t)((encoding >> 12) & 0x1FF);
    int16_t offset = (imm9 << 7) >> 7;  /* Sign-extend to 16 bits */
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint64_t addr;

    offset <<= size;  /* Scale by access size */
    addr = guest_state[rn] + offset;

    switch (size) {
        case 0: /* Byte */
            *(uint8_t *)(uintptr_t)addr = (uint8_t)guest_state[rd];
            emit_x86_mov_mem8_reg(code_buf, x86_rn, x86_rd, offset);
            break;
        case 1: /* Halfword */
            *(uint16_t *)(uintptr_t)addr = (uint16_t)guest_state[rd];
            emit_x86_mov_mem16_reg(code_buf, x86_rn, x86_rd, offset);
            break;
        case 2: /* Word */
            *(uint32_t *)(uintptr_t)addr = (uint32_t)guest_state[rd];
            emit_x86_mov_mem_reg32(code_buf, x86_rn, x86_rd, offset);
            break;
        case 3: /* Doubleword */
            *(uint64_t *)(uintptr_t)addr = guest_state[rd];
            emit_x86_mov_mem_reg(code_buf, x86_rn, x86_rd, offset);
            break;
    }

    return 0;
}

/**
 * translate_mem_dispatch - Dispatch memory instruction based on encoding
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 if instruction handled, -1 otherwise
 */
int translate_mem_dispatch(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    if ((encoding & MEM_LDR_IMM_MASK) == MEM_LDR_IMM_VAL) {
        return translate_mem_ldr_imm(encoding, code_buf, guest_state);
    } else if ((encoding & MEM_STR_IMM_MASK) == MEM_STR_IMM_VAL) {
        return translate_mem_str_imm(encoding, code_buf, guest_state);
    } else if ((encoding & MEM_LDRB_MASK) == MEM_LDRB_VAL) {
        return translate_mem_ldrb(encoding, code_buf, guest_state);
    } else if ((encoding & MEM_STRB_MASK) == MEM_STRB_VAL) {
        return translate_mem_strb(encoding, code_buf, guest_state);
    } else if ((encoding & MEM_LDRH_MASK) == MEM_LDRH_VAL) {
        return translate_mem_ldrh(encoding, code_buf, guest_state);
    } else if ((encoding & MEM_STRH_MASK) == MEM_STRH_VAL) {
        return translate_mem_strh(encoding, code_buf, guest_state);
    } else if ((encoding & MEM_LDRSB_MASK) == MEM_LDRSB_VAL) {
        return translate_mem_ldrsb(encoding, code_buf, guest_state);
    } else if ((encoding & MEM_LDRSH_MASK) == MEM_LDRSH_VAL) {
        return translate_mem_ldrsh(encoding, code_buf, guest_state);
    } else if ((encoding & MEM_LDRSW_MASK) == MEM_LDRSW_VAL) {
        return translate_mem_ldrsw(encoding, code_buf, guest_state);
    } else if ((encoding & MEM_LDUR_MASK) == MEM_LDUR_VAL) {
        return translate_mem_ldur(encoding, code_buf, guest_state);
    } else if ((encoding & MEM_STUR_MASK) == MEM_STUR_VAL) {
        return translate_mem_stur(encoding, code_buf, guest_state);
    } else if ((encoding & MEM_LDP_MASK) == MEM_LDP_VAL) {
        return translate_mem_ldp(encoding, code_buf, guest_state);
    } else if ((encoding & MEM_STP_MASK) == MEM_STP_VAL) {
        return translate_mem_stp(encoding, code_buf, guest_state);
    }

    return -1;  /* Not a memory instruction */
}
