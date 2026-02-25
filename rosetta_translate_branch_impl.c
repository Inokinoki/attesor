/* ============================================================================
 * Rosetta Translator - Branch Translation Implementation
 * ============================================================================ */

#include "rosetta_codegen.h"
#include "rosetta_translate_branch_impl.h"
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

static inline int x86_is_jcc(const x86_insn_t *insn)
{
    return (insn->opcode >= 0x70 && insn->opcode <= 0x7F) ||
           (insn->opcode >= 0x0F80 && insn->opcode <= 0x0F8F);
}

static inline int x86_is_jmp(const x86_insn_t *insn)
{
    return insn->opcode == 0xE9 || insn->opcode == 0xEB;
}

static inline int x86_is_call(const x86_insn_t *insn)
{
    return insn->opcode == 0xE8;
}

static inline int x86_is_ret(const x86_insn_t *insn)
{
    return insn->opcode == 0xC3 || insn->opcode == 0xC2;
}

static inline int x86_is_cmov(const x86_insn_t *insn)
{
    return (insn->opcode >= 0x0F40 && insn->opcode <= 0x0F4F);
}

static inline int x86_is_setcc(const x86_insn_t *insn)
{
    return (insn->opcode >= 0x0F90 && insn->opcode <= 0x0F9F);
}

static inline int x86_is_xchg(const x86_insn_t *insn)
{
    return (insn->opcode >= 0x90 && insn->opcode <= 0x97);
}

int translate_branch_jcc(code_buffer_t *code_buf, const void *insn_ptr, uint64_t block_pc)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    (void)block_pc;
    emit_nop(code_buf);
    return 1;
}

int translate_branch_jmp(code_buffer_t *code_buf, const void *insn_ptr, uint64_t block_pc)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;

    if (x86_is_jmp(insn)) {
        emit_b(code_buf, insn->imm32);
        return 1;
    }
    return 0;
}

int translate_branch_call(code_buffer_t *code_buf, const void *insn_ptr, uint64_t block_pc)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    (void)block_pc;
    emit_nop(code_buf);
    return 1;
}

int translate_branch_ret(code_buffer_t *code_buf)
{
    emit_ret(code_buf);
    return 1;
}

void translate_branch_cmov(code_buffer_t *code_buf, const void *insn_ptr,
                           uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    (void)arm_rd;
    (void)arm_rm;
    emit_nop(code_buf);
}

void translate_branch_setcc(code_buffer_t *code_buf, const void *insn_ptr, uint8_t arm_rd)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    (void)arm_rd;
    emit_nop(code_buf);
}

void translate_branch_xchg(code_buffer_t *code_buf, const void *insn_ptr,
                           uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    emit_mov_reg_reg(code_buf, arm_rd, arm_rm);
}
