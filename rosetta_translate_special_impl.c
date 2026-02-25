/* ============================================================================
 * Rosetta Translator - Special Translation Implementation
 * ============================================================================ */

#include "rosetta_codegen.h"
#include "rosetta_translate_special_impl.h"
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

void translate_special_cpuid(code_buffer_t *code_buf, const void *insn_ptr)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    emit_nop(code_buf);
}

void translate_special_rdtsc(code_buffer_t *code_buf, const void *insn_ptr)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    emit_nop(code_buf);
}

void translate_special_shld(code_buffer_t *code_buf, const void *insn_ptr,
                            uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    (void)arm_rd;
    (void)arm_rm;
    emit_nop(code_buf);
}

void translate_special_shrd(code_buffer_t *code_buf, const void *insn_ptr,
                            uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    (void)arm_rd;
    (void)arm_rm;
    emit_nop(code_buf);
}

void translate_special_cqo(code_buffer_t *code_buf, const void *insn_ptr)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    emit_nop(code_buf);
}

void translate_special_cli(code_buffer_t *code_buf, const void *insn_ptr)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    emit_nop(code_buf);
}

void translate_special_sti(code_buffer_t *code_buf, const void *insn_ptr)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    emit_nop(code_buf);
}

void translate_special_nop(code_buffer_t *code_buf, const void *insn_ptr)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    emit_nop(code_buf);
}

void translate_bit_bsf(code_buffer_t *code_buf, const void *insn_ptr,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    emit_bsf_reg(code_buf, arm_rd, arm_rm);
}

void translate_bit_bsr(code_buffer_t *code_buf, const void *insn_ptr,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    emit_bsr_reg(code_buf, arm_rd, arm_rm);
}

void translate_bit_popcnt(code_buffer_t *code_buf, const void *insn_ptr,
                          uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    (void)arm_rd;
    (void)arm_rm;
    emit_nop(code_buf);
}

void translate_bit_bt(code_buffer_t *code_buf, const void *insn_ptr,
                      uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    emit_bt_reg(code_buf, arm_rd, arm_rm, 0);
}

void translate_bit_bts(code_buffer_t *code_buf, const void *insn_ptr,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    emit_bts_reg(code_buf, arm_rd, arm_rm, 0);
}

void translate_bit_btr(code_buffer_t *code_buf, const void *insn_ptr,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    emit_btr_reg(code_buf, arm_rd, arm_rm, 0);
}

void translate_bit_btc(code_buffer_t *code_buf, const void *insn_ptr,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    emit_btc_reg(code_buf, arm_rd, arm_rm, 0);
}

void translate_string_movs(code_buffer_t *code_buf, const void *insn_ptr)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    emit_movs(code_buf, 0);
}

void translate_string_stos(code_buffer_t *code_buf, const void *insn_ptr)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    emit_nop(code_buf);
}

void translate_string_lods(code_buffer_t *code_buf, const void *insn_ptr)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    emit_nop(code_buf);
}

void translate_string_cmps(code_buffer_t *code_buf, const void *insn_ptr)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    emit_cmps(code_buf, 8);
}

void translate_string_scas(code_buffer_t *code_buf, const void *insn_ptr)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;
    emit_nop(code_buf);
}
