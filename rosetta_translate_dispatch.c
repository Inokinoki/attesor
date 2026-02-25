/* ============================================================================
 * Rosetta Translation Dispatcher - Implementation
 * ============================================================================
 *
 * This module handles dispatching of decoded x86_64 instructions to the
 * appropriate translation handlers based on instruction type.
 * ============================================================================ */

#include "rosetta_translate_dispatch.h"
#include "rosetta_translate_alu.h"
#include "rosetta_translate_memory.h"
#include "rosetta_translate_branch.h"
#include "rosetta_translate_bit.h"
#include "rosetta_translate_string.h"
#include "rosetta_translate_special.h"

/* ============================================================================
 * Instruction Classification
 * ============================================================================ */

InsnCategory dispatch_classify_insn(const x86_insn_t *insn)
{
    /* ALU operations */
    if (x86_is_add(insn) || x86_is_sub(insn) ||
        x86_is_and(insn) || x86_is_or(insn) || x86_is_xor(insn) ||
        x86_is_mul(insn) || x86_is_div(insn) ||
        x86_is_inc(insn) || x86_is_dec(insn) ||
        x86_is_neg(insn) || x86_is_not(insn) ||
        x86_is_shl(insn) || x86_is_shr(insn) || x86_is_sar(insn) ||
        x86_is_rol(insn) || x86_is_ror(insn)) {
        return INSN_ALU;
    }

    /* Memory operations */
    if (x86_is_mov(insn) || x86_is_mov_imm64(insn) ||
        x86_is_movzx(insn) || x86_is_movsx(insn) || x86_is_movsxd(insn) ||
        x86_is_lea(insn) || x86_is_push(insn) || x86_is_pop(insn) ||
        x86_is_cmp(insn) || x86_is_test(insn)) {
        return INSN_MEMORY;
    }

    /* Branch/control flow operations */
    if (x86_is_jcc(insn) || x86_is_jmp(insn) || x86_is_call(insn) ||
        x86_is_ret(insn) || x86_is_cmov(insn) || x86_is_setcc(insn) ||
        x86_is_xchg(insn)) {
        return INSN_BRANCH;
    }

    /* Bit manipulation operations */
    if (x86_is_bsf(insn) || x86_is_bsr(insn) || x86_is_popcnt(insn) ||
        x86_is_bt(insn) || x86_is_bts(insn) || x86_is_btr(insn) ||
        x86_is_btc(insn)) {
        return INSN_BIT;
    }

    /* String operations */
    if (x86_is_movs(insn) || x86_is_stos(insn) || x86_is_lods(insn) ||
        x86_is_cmps(insn) || x86_is_scas(insn)) {
        return INSN_STRING;
    }

    /* Special instructions */
    if (x86_is_cpuid(insn) || x86_is_rdtsc(insn) || x86_is_shld(insn) ||
        x86_is_shrd(insn) || x86_is_cwd(insn) || x86_is_cqo(insn) ||
        x86_is_cli(insn) || x86_is_sti(insn) || x86_is_nop(insn)) {
        return INSN_SPECIAL;
    }

    return INSN_UNKNOWN;
}

const char *dispatch_category_name(InsnCategory category)
{
    switch (category) {
        case INSN_ALU:      return "ALU";
        case INSN_MEMORY:   return "MEMORY";
        case INSN_BRANCH:   return "BRANCH";
        case INSN_BIT:      return "BIT";
        case INSN_STRING:   return "STRING";
        case INSN_SPECIAL:  return "SPECIAL";
        case INSN_FP:       return "FP";
        case INSN_SIMD:     return "SIMD";
        default:            return "UNKNOWN";
    }
}

/* ============================================================================
 * Instruction Translation Dispatch
 * ============================================================================ */

TranslateResult dispatch_translate_insn(CodeBuffer *code_buf,
                                        const x86_insn_t *insn,
                                        uint8_t arm_rd,
                                        uint8_t arm_rm,
                                        uint64_t block_pc)
{
    TranslateResult result = {
        .success = false,
        .is_block_end = false,
        .insn_length = insn->length
    };

    InsnCategory category = dispatch_classify_insn(insn);

    switch (category) {
        case INSN_ALU:
            /* ALU operations */
            if (x86_is_add(insn)) {
                translate_alu_add(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_sub(insn)) {
                translate_alu_sub(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_and(insn)) {
                translate_alu_and(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_or(insn)) {
                translate_alu_or(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_xor(insn)) {
                translate_alu_xor(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_mul(insn)) {
                translate_alu_mul(code_buf, insn, arm_rm);
            } else if (x86_is_div(insn)) {
                translate_alu_div(code_buf, insn, arm_rm);
            } else if (x86_is_inc(insn)) {
                translate_alu_inc(code_buf, insn, arm_rd);
            } else if (x86_is_dec(insn)) {
                translate_alu_dec(code_buf, insn, arm_rd);
            } else if (x86_is_neg(insn)) {
                translate_alu_neg(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_not(insn)) {
                translate_alu_not(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_shl(insn) || x86_is_shr(insn) || x86_is_sar(insn) ||
                       x86_is_rol(insn) || x86_is_ror(insn)) {
                translate_alu_shift(code_buf, insn, arm_rd, arm_rm);
            }
            result.success = true;
            break;

        case INSN_MEMORY:
            /* Memory operations */
            if (x86_is_mov_imm64(insn) || x86_is_mov(insn)) {
                translate_memory_mov(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_movzx(insn)) {
                translate_memory_movzx(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_movsx(insn)) {
                translate_memory_movsx(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_movsxd(insn)) {
                translate_memory_movsxd(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_lea(insn)) {
                translate_memory_lea(code_buf, insn, arm_rd);
            } else if (x86_is_push(insn)) {
                translate_memory_push(code_buf, insn, arm_rd);
            } else if (x86_is_pop(insn)) {
                translate_memory_pop(code_buf, insn, arm_rd);
            } else if (x86_is_cmp(insn)) {
                translate_memory_cmp(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_test(insn)) {
                translate_memory_test(code_buf, insn, arm_rd, arm_rm);
            }
            result.success = true;
            break;

        case INSN_BRANCH:
            /* Branch/control flow operations */
            if (x86_is_jcc(insn)) {
                result.is_block_end = translate_branch_jcc(code_buf, insn, block_pc);
            } else if (x86_is_jmp(insn)) {
                result.is_block_end = translate_branch_jmp(code_buf, insn, block_pc);
            } else if (x86_is_call(insn)) {
                result.is_block_end = translate_branch_call(code_buf, insn, block_pc);
            } else if (x86_is_ret(insn)) {
                result.is_block_end = translate_branch_ret(code_buf);
            } else if (x86_is_cmov(insn)) {
                translate_branch_cmov(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_setcc(insn)) {
                translate_branch_setcc(code_buf, insn, arm_rd);
            } else if (x86_is_xchg(insn)) {
                translate_branch_xchg(code_buf, insn, arm_rd, arm_rm);
            }
            result.success = true;
            break;

        case INSN_BIT:
            /* Bit manipulation operations */
            if (x86_is_bsf(insn)) {
                translate_bit_bsf(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_bsr(insn)) {
                translate_bit_bsr(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_popcnt(insn)) {
                translate_bit_popcnt(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_bt(insn)) {
                translate_bit_bt(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_bts(insn)) {
                translate_bit_bts(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_btr(insn)) {
                translate_bit_btr(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_btc(insn)) {
                translate_bit_btc(code_buf, insn, arm_rd, arm_rm);
            }
            result.success = true;
            break;

        case INSN_STRING:
            /* String operations */
            if (x86_is_movs(insn)) {
                translate_string_movs(code_buf, insn);
            } else if (x86_is_stos(insn)) {
                translate_string_stos(code_buf, insn);
            } else if (x86_is_lods(insn)) {
                translate_string_lods(code_buf, insn);
            } else if (x86_is_cmps(insn)) {
                translate_string_cmps(code_buf, insn);
            } else if (x86_is_scas(insn)) {
                translate_string_scas(code_buf, insn);
            }
            result.success = true;
            break;

        case INSN_SPECIAL:
            /* Special instructions */
            if (x86_is_cpuid(insn)) {
                translate_special_cpuid(code_buf, insn);
            } else if (x86_is_rdtsc(insn)) {
                translate_special_rdtsc(code_buf, insn);
            } else if (x86_is_shld(insn)) {
                translate_special_shld(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_shrd(insn)) {
                translate_special_shrd(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_cwd(insn) || x86_is_cqo(insn)) {
                translate_special_cqo(code_buf, insn);
            } else if (x86_is_cli(insn)) {
                translate_special_cli(code_buf, insn);
            } else if (x86_is_sti(insn)) {
                translate_special_sti(code_buf, insn);
            } else if (x86_is_nop(insn)) {
                translate_special_nop(code_buf, insn);
            }
            result.success = true;
            break;

        case INSN_UNKNOWN:
        default:
            /* Unknown instruction - emit NOP as placeholder */
            emit_nop(code_buf);
            result.success = false;
            break;
    }

    return result;
}

/* End of rosetta_translate_dispatch.c */
