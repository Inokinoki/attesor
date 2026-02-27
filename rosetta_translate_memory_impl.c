/* ============================================================================
 * Rosetta Translator - Memory Translation Implementation
 * ============================================================================ */

#include "rosetta_codegen.h"
#include "rosetta_translate_memory_impl.h"
#include <stdint.h>

typedef struct {
    uint16_t opcode;
    uint8_t modrm;
    uint8_t reg;
    uint8_t rm;
    uint8_t length;
    int32_t imm32;
    int64_t imm64;
    int32_t disp;
} x86_insn_t;

static inline int x86_is_mov(const x86_insn_t *insn)
{
    return (insn->opcode >= 0x88 && insn->opcode <= 0x8C) ||
           (insn->opcode >= 0xA0 && insn->opcode <= 0xA3);
}

static inline int x86_is_mov_imm64(const x86_insn_t *insn)
{
    return insn->opcode == 0xC7 || insn->opcode == 0xB8;
}

static inline int x86_is_movzx(const x86_insn_t *insn) __attribute__((unused));
static inline int x86_is_movzx(const x86_insn_t *insn)
{
    return insn->opcode == 0x0FB6 || insn->opcode == 0x0FB7;
}

static inline int x86_is_movsx(const x86_insn_t *insn) __attribute__((unused));
static inline int x86_is_movsx(const x86_insn_t *insn)
{
    return insn->opcode == 0x0FBE || insn->opcode == 0x0FBF;
}

static inline int x86_is_movsxd(const x86_insn_t *insn) __attribute__((unused));
static inline int x86_is_movsxd(const x86_insn_t *insn)
{
    return insn->opcode == 0x63;
}

static inline int x86_is_lea(const x86_insn_t *insn) __attribute__((unused));
static inline int x86_is_lea(const x86_insn_t *insn)
{
    return insn->opcode == 0x8D;
}

static inline int x86_is_push(const x86_insn_t *insn) __attribute__((unused));
static inline int x86_is_push(const x86_insn_t *insn)
{
    return (insn->opcode >= 0x50 && insn->opcode <= 0x57) ||
           (insn->opcode == 0x6A);
}

static inline int x86_is_pop(const x86_insn_t *insn) __attribute__((unused));
static inline int x86_is_pop(const x86_insn_t *insn)
{
    return insn->opcode >= 0x58 && insn->opcode <= 0x5F;
}

static inline int x86_is_cmp(const x86_insn_t *insn)
{
    return (insn->opcode >= 0x38 && insn->opcode <= 0x3D) ||
           (insn->opcode == 0x80 && (insn->modrm & 0x38) == 0x08);
}

static inline int x86_is_test(const x86_insn_t *insn) __attribute__((unused));
static inline int x86_is_test(const x86_insn_t *insn)
{
    return (insn->opcode >= 0x84 && insn->opcode <= 0x85) ||
           (insn->opcode == 0xF6 && (insn->modrm & 0x38) == 0x00);
}

void translate_memory_mov(code_buffer_t *code_buf, const void *insn_ptr,
                          uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;

    if (x86_is_mov(insn) || x86_is_mov_imm64(insn)) {
        if (insn->opcode == 0xB8) {
            emit_mov_reg_imm64(code_buf, arm_rd, (uint64_t)insn->imm64);
        } else if (insn->modrm & 0xC0) {
            emit_mov_reg_reg(code_buf, arm_rd, arm_rm);
        } else {
            emit_mov_reg_reg(code_buf, arm_rd, arm_rm);
        }
    }
}

void translate_memory_movzx(code_buffer_t *code_buf, const void *insn_ptr,
                            uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;

    if (!x86_is_movzx(insn)) {
        return;
    }

    /* MOVZX - Move with zero-extend
     * 0x0FB6: MOVZX r8/m8 to r16/r32/r64
     * 0x0FB7: MOVZX r16/m16 to r32/r64
     */
    if (insn->opcode == 0x0FB6) {
        /* Zero-extend from 8-bit: UXTB */
        /* UXTB Wd, Wn */
        uint32_t insn_arm = 0x53001C00 | ((arm_rd & 31) << 0) | ((arm_rm & 31) << 5);
        emit_arm64_insn(code_buf, insn_arm);
    } else if (insn->opcode == 0x0FB7) {
        /* Zero-extend from 16-bit: UXTH */
        /* UXTH Wd, Wn */
        uint32_t insn_arm = 0x53003C00 | ((arm_rd & 31) << 0) | ((arm_rm & 31) << 5);
        emit_arm64_insn(code_buf, insn_arm);
    }
}

void translate_memory_movsx(code_buffer_t *code_buf, const void *insn_ptr,
                            uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;

    if (!x86_is_movsx(insn)) {
        return;
    }

    /* MOVSX - Move with sign-extend
     * 0x0FBE: MOVSX r8/m8 to r16/r32/r64
     * 0x0FBF: MOVSX r16/m16 to r32/r64
     */
    if (insn->opcode == 0x0FBE) {
        /* Sign-extend from 8-bit: SXTB */
        /* SXTB Wd, Wn */
        uint32_t insn_arm = 0x53201C00 | ((arm_rd & 31) << 0) | ((arm_rm & 31) << 5);
        emit_arm64_insn(code_buf, insn_arm);
    } else if (insn->opcode == 0x0FBF) {
        /* Sign-extend from 16-bit: SXTH */
        /* SXTH Wd, Wn */
        uint32_t insn_arm = 0x53203C00 | ((arm_rd & 31) << 0) | ((arm_rm & 31) << 5);
        emit_arm64_insn(code_buf, insn_arm);
    }
}

void translate_memory_movsxd(code_buffer_t *code_buf, const void *insn_ptr,
                             uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;

    if (!x86_is_movsxd(insn)) {
        return;
    }

    /* MOVSXD - Move with sign-extend from 32-bit to 64-bit
     * Opcode 0x63: MOVSXD r32/m32 to r64
     * ARM64: SXTW (Sign extend word)
     */
    /* SXTW Xd, Wn */
    uint32_t insn_arm = 0x93407C00 | ((arm_rd & 31) << 0) | ((arm_rm & 31) << 5);
    emit_arm64_insn(code_buf, insn_arm);
}

void translate_memory_lea(code_buffer_t *code_buf, const void *insn_ptr, uint8_t arm_rd)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;

    if (!x86_is_lea(insn)) {
        return;
    }

    /* LEA - Load Effective Address
     * LEA calculates the effective address and stores it in the destination
     * For x86: lea reg, [base + index*scale + disp]
     * ARM64: ADD/ADDS with shifted register
     */
    int32_t disp = insn->disp;
    uint8_t base_reg = insn->rm;
    uint8_t index_reg = insn->reg;
    uint8_t mod = (insn->modrm >> 6) & 0x3;
    uint8_t rm = insn->modrm & 0x07;

    /* Scale factor: 00=1, 01=2, 10=4, 11=8 */
    uint8_t scale_bits = (insn->modrm >> 6) & 0x3;

    /* Check if we have SIB byte (rm == 4 and mod != 3) */
    bool has_sib = (rm == 4) && (mod != 3);
    (void)has_sib;  /* For now, simplified handling */

    /* ARM64 register mappings */
    uint8_t arm_base = base_reg & 0x1F;
    uint8_t arm_index = index_reg & 0x1F;
    uint8_t arm_dst = arm_rd & 0x1F;

    /* Step 1: Handle displacement */
    if (disp != 0) {
        if (disp >= -4096 && disp <= 4096) {
            /* ADD Xd, Xn, #disp (immediate) */
            uint32_t insn_arm = 0x91000000 | ((arm_dst & 31) << 0) | ((arm_base & 31) << 5) | ((disp & 0xFFF) << 10);
            emit_arm64_insn(code_buf, insn_arm);
        } else {
            /* Large displacement: load into temp register first */
            /* MOVZ/MOVK sequence or use temp register */
            uint8_t arm_tmp = 16;  /* Use X16 as temp */

            /* MOVZ X16, #imm16 */
            uint32_t insn_arm = 0xD2800000 | ((arm_tmp & 31) << 0) | ((disp & 0xFFFF) << 5);
            emit_arm64_insn(code_buf, insn_arm);

            /* MOVK X16, #imm16, LSL #16 (if needed) */
            if (disp & 0xFFFF0000) {
                insn_arm = 0xF2A00000 | ((arm_tmp & 31) << 0) | (((disp >> 16) & 0xFFFF) << 5);
                emit_arm64_insn(code_buf, insn_arm);
            }

            /* ADD Xd, Xn, Xm */
            insn_arm = 0x8B000000 | ((arm_dst & 31) << 0) | ((arm_base & 31) << 5) | ((arm_tmp & 31) << 16);
            emit_arm64_insn(code_buf, insn_arm);
        }
    } else {
        /* MOV Xd, Xn (copy base register) */
        uint32_t insn_arm = 0xAA000000 | ((arm_dst & 31) << 0) | ((arm_base & 31) << 5);
        emit_arm64_insn(code_buf, insn_arm);
    }

    /* Step 2: Handle scaled index if present */
    /* Check if index register is valid (not RSP/R12 which is used for SIB base) */
    if (index_reg != 4 && index_reg != 12) {  /* Simplified check */
        /* ARM64 supports shifted register additions: LSL #0/1/2/3 for scale 1/2/4/8 */
        uint8_t shift = scale_bits;  /* 0=LSL#0, 1=LSL#1, 2=LSL#2, 3=LSL#3 */

        /* ADD Xd, Xn, Xm, LSL #shift */
        /* Encoding: 0x8B000000 | Rd | (Rn << 5) | (Rm << 16) | (shift << 22) */
        uint32_t insn_arm = 0x8B000000 | ((arm_dst & 31) << 0) |
                                               ((arm_dst & 31) << 5) |  /* Rd = destination */
                                               ((arm_index & 31) << 16) |  /* Rm = index */
                                               ((shift & 3) << 22);  /* LSL #shift */
        emit_arm64_insn(code_buf, insn_arm);
    }
}

void translate_memory_push(code_buffer_t *code_buf, const void *insn_ptr, uint8_t arm_rd)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;

    /* PUSH - Push onto stack
     * x86: PUSH r64 decrements SP by 8 and stores value
     * ARM64: STR Xd, [SP, #-8]!
     * Encoding: STR (register) with pre-index: 0xF90003E0 | (Rt << 0)
     */

    /* STR Xd, [SP, #-8]! - Store with pre-index decrement */
    /* Pre-indexed store: 0xF90003E0 | (Rt << 0) with bit 24 set for pre-index */
    uint32_t insn_arm = 0xF90003E0 | ((arm_rd & 31) << 0);
    emit_arm64_insn(code_buf, insn_arm);
}

void translate_memory_pop(code_buffer_t *code_buf, const void *insn_ptr, uint8_t arm_rd)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;

    /* POP - Pop from stack
     * x86: POP r64 loads value and increments SP by 8
     * ARM64: LDR Xd, [SP], #8
     * Encoding: LDR (register) with post-index: 0xF94003E0 | (Rt << 0)
     */

    /* LDR Xd, [SP], #8 - Load with post-index increment */
    uint32_t insn_arm = 0xF94003E0 | ((arm_rd & 31) << 0);
    emit_arm64_insn(code_buf, insn_arm);
}

void translate_memory_cmp(code_buffer_t *code_buf, const void *insn_ptr,
                          uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;

    if (x86_is_cmp(insn)) {
        if (insn->modrm & 0xC0) {
            emit_cmp_reg_reg(code_buf, arm_rd, arm_rm);
        } else if (insn->imm32 != 0) {
            emit_cmp_reg_imm32(code_buf, arm_rd, (uint32_t)insn->imm32);
        }
    }
}

void translate_memory_test(code_buffer_t *code_buf, const void *insn_ptr,
                           uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;

    if (!x86_is_test(insn)) {
        return;
    }

    /* TEST - Logical compare (AND without storing result, just flags)
     * x86: TEST r64, r64 sets flags based on AND operation
     * ARM64: TST (alias of ANDS with ZR destination)
     * TST Xn, Xm -> ANDS XZR, Xn, Xm
     */

    /* TST Xn, Xm - Test bits (ANDS with zero register) */
    /* ANDS XZR, Xn, Xm: 0xEA00001F | (Rn << 5) | (Rm << 16) */
    uint32_t insn_arm = 0xEA00001F | ((arm_rd & 31) << 5) | ((arm_rm & 31) << 16);
    emit_arm64_insn(code_buf, insn_arm);
}

void emit_load_imm64(code_buffer_t *code_buf, uint8_t reg, uint64_t imm)
{
    emit_mov_reg_imm64(code_buf, reg, imm);
}

/* ============================================================================
 * PC-Relative Addressing Translation
 * ============================================================================ */

/**
 * translate_adr - Translate ARM64 ADR (PC-relative address) instruction
 * @code_buf: Code buffer for x86_64 emission
 * @insn_ptr: Pointer to ARM64 instruction
 * @arm_rd: Destination ARM64 register
 * @pc: Current program counter
 *
 * ADR computes a PC-relative address and stores it in a register.
 * ADR Rd, label  ->  Rd = PC + offset
 *
 * On x86_64, this is translated to LEA Rip-relative or MOV with relocation.
 */
void translate_adr(code_buffer_t *code_buf, const void *insn_ptr,
                   uint8_t arm_rd, uint64_t pc)
{
    const uint32_t *insn = (const uint32_t *)insn_ptr;
    uint32_t encoding = *insn;

    /* Extract fields from ADR encoding */
    /* ADR: 00010000iiiiiiiiiiiiiiiiiiiiiiiii Rd */
    uint8_t rd = encoding & 0x1F;
    int64_t imm = ((encoding >> 29) & 0x03) << 19 |
                  ((encoding >> 5) & 0x7FFFF);
    /* Sign extend 21-bit immediate */
    imm = (imm << 43) >> 43;

    /* Map ARM64 register to x86_64 */
    uint8_t x86_rd = arm_rd & 0x0F;

    /* Calculate target address */
    uint64_t target = pc + imm;

    /* On x86_64, emit LEA Rip-relative or MOV with immediate */
    /* For now, emit MOV reg, imm64 (will need relocation in real implementation) */

    /* MOV reg64, imm64: 48 B8+rd imm64 */
    uint8_t *code = code_buf->buffer + code_buf->offset;

    /* REX prefix with register extension */
    code[0] = 0x48;
    if (x86_rd >= 8) {
        code[0] |= 0x41;
    }

    code[1] = 0xB8 + (x86_rd & 7);

    /* Emit 64-bit immediate (target address) */
    uint64_t *imm_ptr = (uint64_t *)(code + 2);
    *imm_ptr = target;

    code_buf->offset += 10;  /* 2 bytes prefix + opcode + 8 bytes immediate */

    (void)rd;  /* Suppress unused warning */
}

/**
 * translate_adrp - Translate ARM64 ADRP (PC-relative page address) instruction
 * @code_buf: Code buffer for x86_64 emission
 * @insn_ptr: Pointer to ARM64 instruction
 * @arm_rd: Destination ARM64 register
 * @pc: Current program counter (page-aligned)
 *
 * ADRP computes a PC-relative page address (aligned to 4KB).
 * ADRP Rd, label  ->  Rd = (PC & ~0xFFF) + (offset << 12)
 *
 * The offset is shifted left by 12 bits (page offset).
 */
void translate_adrp(code_buffer_t *code_buf, const void *insn_ptr,
                    uint8_t arm_rd, uint64_t pc)
{
    const uint32_t *insn = (const uint32_t *)insn_ptr;
    uint32_t encoding = *insn;

    /* Extract fields from ADRP encoding */
    /* ADRP: 10010000iiiiiiiiiiiiiiiiiiiiiiiii Rd */
    uint8_t rd = encoding & 0x1F;
    int64_t imm = ((encoding >> 29) & 0x03) << 19 |
                  ((encoding >> 5) & 0x7FFFF);
    /* Sign extend 21-bit immediate */
    imm = (imm << 43) >> 43;
    /* Shift left by 12 for page offset */
    imm = imm << 12;

    /* Map ARM64 register to x86_64 */
    uint8_t x86_rd = arm_rd & 0x0F;

    /* Calculate target page address */
    uint64_t page_base = pc & ~0xFFFull;  /* Clear page offset */
    uint64_t target = page_base + imm;

    /* On x86_64, emit MOV reg, imm64 (will need relocation) */
    uint8_t *code = code_buf->buffer + code_buf->offset;

    /* REX prefix */
    code[0] = 0x48;
    if (x86_rd >= 8) {
        code[0] |= 0x41;
    }

    code[1] = 0xB8 + (x86_rd & 7);

    /* Emit 64-bit immediate */
    uint64_t *imm_ptr = (uint64_t *)(code + 2);
    *imm_ptr = target;

    code_buf->offset += 10;

    (void)rd;  /* Suppress unused warning */
}

/**
 * translate_adr_imm - Translate ADR with immediate offset
 * @code_buf: Code buffer
 * @imm: Immediate offset
 * @arm_rd: Destination register
 * @pc: Current PC
 *
 * Helper function for ADR translation with explicit immediate.
 */
void translate_adr_imm(code_buffer_t *code_buf, int64_t imm,
                       uint8_t arm_rd, uint64_t pc)
{
    uint8_t x86_rd = arm_rd & 0x0F;
    uint64_t target = pc + imm;

    uint8_t *code = code_buf->buffer + code_buf->offset;

    /* REX prefix */
    code[0] = 0x48;
    if (x86_rd >= 8) {
        code[0] |= 0x41;
    }

    code[1] = 0xB8 + (x86_rd & 7);

    uint64_t *imm_ptr = (uint64_t *)(code + 2);
    *imm_ptr = target;

    code_buf->offset += 10;
}

/**
 * translate_pcrel_load - Translate PC-relative load (LDR literal)
 * @code_buf: Code buffer for x86_64 emission
 * @insn_ptr: Pointer to ARM64 instruction
 * @arm_rd: Destination ARM64 register
 * @pc: Current program counter
 *
 * LDR Xt, [PC + offset] loads a value from a PC-relative address.
 * Encoding: 00011000iiiiiiiiiiiiiiiiiiiiiiii Rt
 */
void translate_pcrel_load(code_buffer_t *code_buf, const void *insn_ptr,
                          uint8_t arm_rd, uint64_t pc)
{
    const uint32_t *insn = (const uint32_t *)insn_ptr;
    uint32_t encoding = *insn;

    /* Extract fields */
    uint8_t rt = encoding & 0x1F;
    int32_t imm = (encoding >> 5) & 0xFFFFF;
    /* Sign extend 19-bit immediate */
    imm = (imm << 13) >> 13;
    /* Scale by 4 for word access */
    int32_t offset = imm * 4;

    /* Map to x86_64 */
    uint8_t x86_rt = arm_rd & 0x0F;

    /* Calculate target address */
    uint64_t target = pc + offset;

    /* Emit MOV reg, [Rip-relative] or MOV reg, imm64 + MOV reg, [reg] */
    /* For simplicity, use MOV reg, imm64 then MOV reg, [reg] */

    uint8_t *code = code_buf->buffer + code_buf->offset;

    /* First, load address into temp register (R11 as temp) */
    code[0] = 0x49;  /* REX for R11 */
    code[1] = 0xB3;  /* MOV R11, imm64 */
    uint64_t *addr_ptr = (uint64_t *)(code + 2);
    *addr_ptr = target;
    code += 10;

    /* Then load from address: MOV r64, [r11] */
    code[0] = 0x49;  /* REX.W for R11 */
    code[1] = 0x8B;  /* MOV r64, [r/m64] */
    code[2] = 0xC0 + ((x86_rt & 7) << 3) + (11 & 7);  /* ModRM */

    code_buf->offset += 13;

    (void)rt;  /* Suppress unused warning */
}

/**
 * translate_pcrel_store - Translate PC-relative store (STR literal)
 * @code_buf: Code buffer for x86_64 emission
 * @insn_ptr: Pointer to ARM64 instruction
 * @arm_rd: Source ARM64 register
 * @pc: Current program counter
 *
 * STR Xt, [PC + offset] stores a value to a PC-relative address.
 */
void translate_pcrel_store(code_buffer_t *code_buf, const void *insn_ptr,
                           uint8_t arm_rd, uint64_t pc)
{
    const uint32_t *insn = (const uint32_t *)insn_ptr;
    uint32_t encoding = *insn;

    /* Extract fields */
    uint8_t rt = encoding & 0x1F;
    int32_t imm = (encoding >> 5) & 0xFFFFF;
    /* Sign extend 19-bit immediate */
    imm = (imm << 13) >> 13;
    /* Scale by 4 for word access */
    int32_t offset = imm * 4;

    /* Map to x86_64 */
    uint8_t x86_rt = arm_rd & 0x0F;

    /* Calculate target address */
    uint64_t target = pc + offset;

    /* Emit MOV [Rip-relative], reg or MOV imm64, reg + MOV [imm64], reg */

    uint8_t *code = code_buf->buffer + code_buf->offset;

    /* First, load address into temp register (R11 as temp) */
    code[0] = 0x49;
    code[1] = 0xB3;
    uint64_t *addr_ptr = (uint64_t *)(code + 2);
    *addr_ptr = target;
    code += 10;

    /* Then store to address: MOV [r11], r64 */
    code[0] = 0x49;  /* REX.W for R11 */
    code[1] = 0x89;  /* MOV [r/m64], r64 */
    code[2] = 0xC0 + ((x86_rt & 7) << 3) + (11 & 7);  /* ModRM */

    code_buf->offset += 13;

    (void)rt;  /* Suppress unused warning */
}
