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

static inline int x86_is_movzx(const x86_insn_t *insn)
{
    return insn->opcode == 0x0FB6 || insn->opcode == 0x0FB7;
}

static inline int x86_is_movsx(const x86_insn_t *insn)
{
    return insn->opcode == 0x0FBE || insn->opcode == 0x0FBF;
}

static inline int x86_is_movsxd(const x86_insn_t *insn)
{
    return insn->opcode == 0x63;
}

static inline int x86_is_lea(const x86_insn_t *insn)
{
    return insn->opcode == 0x8D;
}

static inline int x86_is_push(const x86_insn_t *insn)
{
    return (insn->opcode >= 0x50 && insn->opcode <= 0x57) ||
           (insn->opcode == 0x6A);
}

static inline int x86_is_pop(const x86_insn_t *insn)
{
    return insn->opcode >= 0x58 && insn->opcode <= 0x5F;
}

static inline int x86_is_cmp(const x86_insn_t *insn)
{
    return (insn->opcode >= 0x38 && insn->opcode <= 0x3D) ||
           (insn->opcode == 0x80 && (insn->modrm & 0x38) == 0x08);
}

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
    (void)insn;
    (void)arm_rd;
    (void)arm_rm;
    emit_mov_reg_reg(code_buf, arm_rd, arm_rm);
}

void translate_memory_movsx(code_buffer_t *code_buf, const void *insn_ptr,
                            uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    (void)arm_rd;
    (void)arm_rm;
    emit_mov_reg_reg(code_buf, arm_rd, arm_rm);
}

void translate_memory_movsxd(code_buffer_t *code_buf, const void *insn_ptr,
                             uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    (void)arm_rd;
    (void)arm_rm;
    emit_mov_reg_reg(code_buf, arm_rd, arm_rm);
}

void translate_memory_lea(code_buffer_t *code_buf, const void *insn_ptr, uint8_t arm_rd)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    (void)arm_rd;
}

void translate_memory_push(code_buffer_t *code_buf, const void *insn_ptr, uint8_t arm_rd)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    (void)arm_rd;
}

void translate_memory_pop(code_buffer_t *code_buf, const void *insn_ptr, uint8_t arm_rd)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    (void)arm_rd;
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
    (void)insn;
    (void)arm_rd;
    (void)arm_rm;
    emit_cmp_reg_reg(code_buf, arm_rd, arm_rm);
}

void emit_load_imm64(code_buffer_t *code_buf, uint8_t reg, uint64_t imm)
{
    emit_mov_reg_imm64(code_buf, reg, imm);
}
