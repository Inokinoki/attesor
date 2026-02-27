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
    (void)insn_ptr;
    /* CPUID - CPU Identification
     * x86: CPUID returns processor information in EAX/EBX/ECX/EDX
     * ARM64: Emulate by setting up registers with CPUID data
     */
    emit_cpuid(code_buf);
}

void translate_special_rdtsc(code_buffer_t *code_buf, const void *insn_ptr)
{
    (void)insn_ptr;
    /* RDTSC - Read Time-Stamp Counter
     * x86: RDTSC returns 64-bit timestamp in RDX:RAX
     * ARM64: Use CNTVCT_EL0 to read virtual count
     */
    emit_rdtsc(code_buf);
}

void translate_special_shld(code_buffer_t *code_buf, const void *insn_ptr,
                            uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;

    /* SHLD - Shift Left Double
     * SHLD r64, r64, imm8 shifts destination left, filling from source
     */
    uint8_t shift = insn->imm32 & 0x3F;
    emit_shld(code_buf, arm_rd, arm_rm, shift);
}

void translate_special_shrd(code_buffer_t *code_buf, const void *insn_ptr,
                            uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;

    /* SHRD - Shift Right Double
     * SHRD r64, r64, imm8 shifts destination right, filling from source
     */
    uint8_t shift = insn->imm32 & 0x3F;
    emit_shrd(code_buf, arm_rd, arm_rm, shift);
}

void translate_special_cqo(code_buffer_t *code_buf, const void *insn_ptr)
{
    (void)insn_ptr;
    /* CQO - Convert Quadword to Octword
     * Sign-extend RAX into RDX:RAX (128-bit result)
     * ARM64: SAR RDX, RAX, #63 (arithmetic shift right by 63)
     */
    emit_cqo(code_buf);
}

void translate_special_cli(code_buffer_t *code_buf, const void *insn_ptr)
{
    (void)insn_ptr;
    /* CLI - Clear Interrupt Flag (disable interrupts) */
    emit_cli(code_buf);
}

void translate_special_sti(code_buffer_t *code_buf, const void *insn_ptr)
{
    (void)insn_ptr;
    /* STI - Set Interrupt Flag (enable interrupts) */
    emit_sti(code_buf);
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
    (void)insn_ptr;
    /* POPCNT - Population Count (count set bits)
     * ARM64: CNT (count bits) followed by ADDV (sum across vector)
     * Or use simpler: RBIT + CLZ approach for scalar
     */
    emit_popcnt_reg(code_buf, arm_rd, arm_rm);
}

void translate_bit_bt(code_buffer_t *code_buf, const void *insn_ptr,
                      uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    uint8_t bit = insn->imm32 & 0x3F;
    emit_bt_reg(code_buf, arm_rd, arm_rm, bit);
}

void translate_bit_bts(code_buffer_t *code_buf, const void *insn_ptr,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    uint8_t bit = insn->imm32 & 0x3F;
    emit_bts_reg(code_buf, arm_rd, arm_rm, bit);
}

void translate_bit_btr(code_buffer_t *code_buf, const void *insn_ptr,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    uint8_t bit = insn->imm32 & 0x3F;
    emit_btr_reg(code_buf, arm_rd, arm_rm, bit);
}

void translate_bit_btc(code_buffer_t *code_buf, const void *insn_ptr,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    uint8_t bit = insn->imm32 & 0x3F;
    emit_btc_reg(code_buf, arm_rd, arm_rm, bit);
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
    /* STOS - Store String
     * STOSB/STOSW/STOSD/STOSQ store AL/AX/EAX/RAX to [RDI]
     */
    int size = 8; /* Default to 64-bit */
    if (insn->opcode == 0xAA) size = 1;      /* STOSB */
    else if (insn->opcode == 0xAB) size = 4; /* STOSD */
    else if (insn->opcode == 0x48AB) size = 8; /* STOSQ */
    emit_stos(code_buf, size);
}

void translate_string_lods(code_buffer_t *code_buf, const void *insn_ptr)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    /* LODS - Load String
     * LODSB/LODSW/LODSD/LODSQ load from [RSI] to AL/AX/EAX/RAX
     */
    int size = 8; /* Default to 64-bit */
    if (insn->opcode == 0xAC) size = 1;      /* LODSB */
    else if (insn->opcode == 0xAD) size = 4; /* LODSD */
    else if (insn->opcode == 0x48AD) size = 8; /* LODSQ */
    emit_lods(code_buf, size);
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
    /* SCAS - Scan String
     * SCASB/SCASW/SCASD/SCASQ compare AL/AX/EAX/RAX with [RDI]
     */
    int size = 8; /* Default to 64-bit */
    if (insn->opcode == 0xAE) size = 1;      /* SCASB */
    else if (insn->opcode == 0xAF) size = 4; /* SCASD */
    else if (insn->opcode == 0x48AF) size = 8; /* SCASQ */
    emit_scas(code_buf, size);
}
