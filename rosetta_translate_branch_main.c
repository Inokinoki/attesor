/* ============================================================================
 * Rosetta Translator - Branch Instruction Translation Implementation
 * ============================================================================
 *
 * This module implements translation of ARM64 branch instructions to x86_64.
 * Supported instructions: B, BL, BR, RET, B.cond, CBZ, CBNZ
 * ============================================================================ */

#include "rosetta_translate_branch_main.h"
#include "rosetta_refactored.h"

/* External register mapping function from ALU module */
extern uint8_t translate_get_x86_reg(uint8_t arm_reg);

/* Condition code mapping: ARM64 condition -> x86_64 jump opcode */
static const uint8_t g_condition_to_jmp[16] = {
    0x84,  /* EQ (Z=1) -> JE/JZ */
    0x85,  /* NE (Z=0) -> JNE/JNZ */
    0x82,  /* CS/HS (C=1) -> JAE/JNB */
    0x83,  /* CC/LO (C=0) -> JB/JNAE */
    0x88,  /* MI (N=1) -> JS */
    0x89,  /* PL (N=0) -> JNS */
    0x80,  /* VS (V=1) -> JO */
    0x81,  /* VC (V=0) -> JNO */
    0x87,  /* HI (C=1 && Z=0) -> JA/JNBE */
    0x86,  /* LS (C=0 || Z=1) -> JBE/JNA */
    0x8D,  /* GE (N=V) -> JGE/JNL */
    0x8C,  /* LT (N!=V) -> JL/JNGE */
    0x8F,  /* GT (Z=0 && N=V) -> JG/JNLE */
    0x8E,  /* LE (Z=1 || N!=V) -> JLE/JNG */
    0x90,  /* AL (always) -> JMP */
    0x90   /* NV (reserved) -> JMP */
};

/**
 * translate_branch_b - Translate B (unconditional branch) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * @pc: Current program counter
 * Returns: 0 on success
 */
int translate_branch_b(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t pc)
{
    int32_t imm26 = (int32_t)((encoding & 0x03FFFFFF) << 6) >> 4;
    uint64_t target = pc + imm26;

    /* Update guest state */
    guest_state[32] = target;  /* PC = target */

    /* Emit x86_64 code - for now, terminate block and let runtime handle branching */
    /* In a full implementation, we would chain to the next translated block */
    emit_x86_ret(code_buf);

    return 0;
}

/**
 * translate_branch_bl - Translate BL (branch with link) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * @pc: Current program counter
 * Returns: 0 on success
 */
int translate_branch_bl(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t pc)
{
    int32_t imm26 = (int32_t)((encoding & 0x03FFFFFF) << 6) >> 4;
    uint64_t target = pc + imm26;
    uint8_t x86_r30 = translate_get_x86_reg(30);  /* LR */

    /* Update guest state - save return address in LR (x30) */
    guest_state[30] = pc + 4;  /* LR = return address */
    guest_state[32] = target;  /* PC = target */

    /* Emit x86_64 code - save return address and jump */
    /* MOV r30, imm64 (return address) */
    emit_x86_mov_reg_imm64(code_buf, x86_r30, pc + 4);

    /* Return to let runtime handle the branch */
    emit_x86_ret(code_buf);

    return 0;
}

/**
 * translate_branch_br - Translate BR (branch register) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_branch_br(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* Update guest state */
    guest_state[32] = guest_state[rn];  /* PC = xn */

    /* Emit x86_64 code - indirect jump through register */
    emit_x86_jmp_reg(code_buf, x86_rn);

    return 0;
}

/**
 * translate_branch_blr - Translate BLR (branch with link to register) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_branch_blr(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_r30 = translate_get_x86_reg(30);  /* LR */

    /* Update guest state - save return address in LR (x30) */
    guest_state[30] = guest_state[32] + 4;  /* LR = PC + 4 */
    guest_state[32] = guest_state[rn];      /* PC = xn */

    /* Emit x86_64 code - save return address and indirect jump */
    /* MOV r30, imm64 (return address) */
    emit_x86_mov_reg_imm64(code_buf, x86_r30, guest_state[32] + 4);

    /* Indirect jump through register */
    emit_x86_jmp_reg(code_buf, x86_rn);

    return 0;
}

/**
 * translate_branch_ret - Translate RET (return) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_branch_ret(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    /* Update guest state */
    guest_state[32] = guest_state[30];  /* PC = LR */

    /* Emit x86_64 code - return */
    emit_x86_ret(code_buf);

    return 0;
}

/**
 * translate_branch_bcond - Translate B.cond (conditional branch) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * @pc: Current program counter
 * Returns: 0 on success
 */
int translate_branch_bcond(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t pc)
{
    uint8_t cond = (encoding >> 12) & 0x0F;
    int32_t imm19 = (int32_t)((encoding & 0x00FFFFE0) << 3) >> 11;
    uint64_t target = pc + imm19;
    uint8_t jmp_opcode = 0;  /* Used for condition mapping */

    /* Update guest state */
    guest_state[32] = target;

    /* Get the appropriate conditional jump opcode */
    if (cond < 16) {
        jmp_opcode = g_condition_to_jmp[cond];
    } else {
        jmp_opcode = 0x84;  /* Default to JE */
    }

    (void)jmp_opcode;  /* Used in future implementation */

    /* For conditional branches, we need to check ARM64 flags (NZCV) */
    /* and translate them to x86 flags. This is a simplified implementation. */

    /* In a full implementation, we would:
     * 1. Compare ARM64 NZCV flags with x86 EFLAGS
     * 2. Emit proper conditional jump based on condition
     * 3. Chain to the appropriate translated block
     */

    /* For now, emit a conditional jump placeholder */
    /* The actual condition check would happen at runtime */

    /* Placeholder: JE rel32 (will be replaced with proper condition) */
    emit_x86_nop(code_buf);

    return 0;
}

/**
 * translate_branch_cbz - Translate CBZ (compare and branch if zero) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * @pc: Current program counter
 * Returns: 0 on success
 */
int translate_branch_cbz(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t pc)
{
    uint8_t rn = (encoding >> 5) & 0x1F;
    int32_t imm19 = (int32_t)((encoding & 0x00FFFFE0) << 3) >> 11;
    uint64_t target = pc + imm19;
    uint8_t x86_rn = translate_get_x86_reg(rn);
    size_t jump_offset;

    /* Update guest state */
    if (guest_state[rn] == 0) {
        guest_state[32] = target;
    }

    /* Emit x86_64 code:
     * TEST rn, rn
     * JZ target
     */

    /* TEST rn, rn (set ZF if zero) */
    emit_x86_test_reg_reg(code_buf, x86_rn, x86_rn);

    /* Save offset for JZ instruction */
    jump_offset = code_buf_get_size(code_buf);
    (void)jump_offset;  /* Used for future block chaining */

    /* JZ rel32 - placeholder, will be patched */
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x84);  /* JZ rel32 */
    code_buf_emit_word32(code_buf, 0);   /* Placeholder for rel32 */

    /* Note: In a full implementation, we would chain blocks here */

    return 0;
}

/**
 * translate_branch_cbnz - Translate CBNZ (compare and branch if not zero) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * @pc: Current program counter
 * Returns: 0 on success
 */
int translate_branch_cbnz(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t pc)
{
    uint8_t rn = (encoding >> 5) & 0x1F;
    int32_t imm19 = (int32_t)((encoding & 0x00FFFFE0) << 3) >> 11;
    uint64_t target = pc + imm19;
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* Update guest state */
    if (guest_state[rn] != 0) {
        guest_state[32] = target;
    }

    /* Emit x86_64 code:
     * TEST rn, rn
     * JNZ target
     */

    /* TEST rn, rn (set ZF if zero) */
    emit_x86_test_reg_reg(code_buf, x86_rn, x86_rn);

    /* JNZ rel32 */
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x85);  /* JNZ rel32 */
    code_buf_emit_word32(code_buf, 0);   /* Placeholder for rel32 */

    return 0;
}

/**
 * translate_branch_tbz - Translate TBZ (test bit and branch if zero) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * @pc: Current program counter
 * Returns: 0 on success
 */
int translate_branch_tbz(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t pc)
{
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t b5 = (encoding >> 31) & 1;        /* Bit 5 of bit position */
    uint8_t b40 = (encoding >> 19) & 0x1F;    /* Bits 4:0 of bit position */
    uint8_t bit_pos = (b5 << 5) | b40;        /* Full bit position (0-63) */
    int32_t imm14 = (int32_t)((encoding & 0x00FFFFE0) << 3) >> 19;
    uint64_t target = pc + imm14;
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* Update guest state - test bit and branch if zero */
    uint64_t value = guest_state[rn];
    if (((value >> bit_pos) & 1) == 0) {
        guest_state[32] = target;
    }

    /* Emit x86_64 code:
     * MOV tmp, rn
     * SHR tmp, bit_pos
     * AND tmp, 1
     * JZ target
     */

    /* Use RCX as temporary register for shift count */
    /* MOV rcx, bit_pos */
    emit_x86_mov_reg_imm32(code_buf, EMIT_RCX, bit_pos);

    /* MOV rax, rn */
    emit_x86_mov_reg_reg(code_buf, EMIT_RAX, x86_rn);

    /* SHR rax, cl (shift by CL) */
    code_buf_emit_byte(code_buf, 0x48);
    code_buf_emit_byte(code_buf, 0xD3);
    code_buf_emit_byte(code_buf, 0xE8);

    /* AND rax, 1 */
    emit_x86_and_reg_imm32(code_buf, EMIT_RAX, 1);

    /* JZ rel32 */
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x84);
    code_buf_emit_word32(code_buf, 0);  /* Placeholder */

    return 0;
}

/**
 * translate_branch_tbnz - Translate TBNZ (test bit and branch if not zero) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * @pc: Current program counter
 * Returns: 0 on success
 */
int translate_branch_tbnz(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t pc)
{
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t b5 = (encoding >> 31) & 1;        /* Bit 5 of bit position */
    uint8_t b40 = (encoding >> 19) & 0x1F;    /* Bits 4:0 of bit position */
    uint8_t bit_pos = (b5 << 5) | b40;        /* Full bit position (0-63) */
    int32_t imm14 = (int32_t)((encoding & 0x00FFFFE0) << 3) >> 19;
    uint64_t target = pc + imm14;
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* Update guest state - test bit and branch if not zero */
    uint64_t value = guest_state[rn];
    if (((value >> bit_pos) & 1) != 0) {
        guest_state[32] = target;
    }

    /* Emit x86_64 code:
     * MOV tmp, rn
     * SHR tmp, bit_pos
     * AND tmp, 1
     * JNZ target
     */

    /* Use RCX as temporary register for shift count */
    /* MOV rcx, bit_pos */
    emit_x86_mov_reg_imm32(code_buf, EMIT_RCX, bit_pos);

    /* MOV rax, rn */
    emit_x86_mov_reg_reg(code_buf, EMIT_RAX, x86_rn);

    /* SHR rax, cl (shift by CL) */
    code_buf_emit_byte(code_buf, 0x48);
    code_buf_emit_byte(code_buf, 0xD3);
    code_buf_emit_byte(code_buf, 0xE8);

    /* AND rax, 1 */
    emit_x86_and_reg_imm32(code_buf, EMIT_RAX, 1);

    /* JNZ rel32 */
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x85);
    code_buf_emit_word32(code_buf, 0);  /* Placeholder */

    return 0;
}

/**
 * translate_branch_dispatch - Dispatch branch instruction based on encoding
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * @pc: Current program counter
 * @terminated: Output flag indicating if block translation should end
 * Returns: 0 if instruction handled, -1 otherwise
 */
int translate_branch_dispatch(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t pc, int *terminated)
{
    if ((encoding & BRANCH_B_MASK) == BRANCH_B_VAL) {
        *terminated = 1;
        return translate_branch_b(encoding, code_buf, guest_state, pc);
    } else if ((encoding & BRANCH_BL_MASK) == BRANCH_BL_VAL) {
        *terminated = 1;
        return translate_branch_bl(encoding, code_buf, guest_state, pc);
    } else if ((encoding & BRANCH_RET_MASK) == BRANCH_RET_VAL) {
        *terminated = 1;
        return translate_branch_ret(encoding, code_buf, guest_state);
    } else if ((encoding & BRANCH_BLR_MASK) == BRANCH_BLR_VAL) {
        *terminated = 1;
        return translate_branch_blr(encoding, code_buf, guest_state);
    } else if ((encoding & BRANCH_BR_MASK) == BRANCH_BR_VAL) {
        *terminated = 1;
        return translate_branch_br(encoding, code_buf, guest_state);
    } else if ((encoding & BRANCH_BCOND_MASK) == BRANCH_BCOND_VAL) {
        *terminated = 1;
        return translate_branch_bcond(encoding, code_buf, guest_state, pc);
    } else if ((encoding & BRANCH_CBZ_MASK) == BRANCH_CBZ_VAL) {
        *terminated = 1;
        return translate_branch_cbz(encoding, code_buf, guest_state, pc);
    } else if ((encoding & BRANCH_CBNZ_MASK) == BRANCH_CBNZ_VAL) {
        *terminated = 1;
        return translate_branch_cbnz(encoding, code_buf, guest_state, pc);
    } else if ((encoding & BRANCH_TBZ_MASK) == BRANCH_TBZ_VAL) {
        *terminated = 1;
        return translate_branch_tbz(encoding, code_buf, guest_state, pc);
    } else if ((encoding & BRANCH_TBNZ_MASK) == BRANCH_TBNZ_VAL) {
        *terminated = 1;
        return translate_branch_tbnz(encoding, code_buf, guest_state, pc);
    }

    return -1;  /* Not a branch instruction */
}
