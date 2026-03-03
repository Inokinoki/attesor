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
#include "rosetta_translate_simd.h"

/* ============================================================================
 * Instruction Classification
 * ============================================================================ */

InsnCategory dispatch_classify_insn(const x86_insn_t *insn)
{
    /* SIMD/Floating Point operations (check first due to opcode2) */
    if (insn->opcode2 >= 0x10 && insn->opcode2 <= 0x7F) {
        /* This is an SSE/SSE2/SSE4 instruction */
        return INSN_SIMD;
    }

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

TranslateResult dispatch_translate_insn(code_buffer_t *code_buf,
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

        case INSN_SIMD:
            /* SIMD/ Floating Point operations (SSE, SSE2, SSE4) */
            /* MOV instructions */
            if (x86_is_movaps(insn) || x86_is_movups(insn) ||
                x86_is_movapd(insn) || x86_is_movupd(insn)) {
                translate_simd_mov(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_movss(insn)) {
                translate_simd_movss(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_movsd(insn)) {
                translate_simd_movsd(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_movhlps(insn) || x86_is_movlhps(insn)) {
                translate_simd_movhl(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_movhps(insn) || x86_is_movhpd(insn)) {
                translate_simd_movh(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_movlps(insn) || x86_is_movlpd(insn)) {
                translate_simd_movl(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_movdqu(insn)) {
                translate_simd_movdqu(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_movdqa(insn)) {
                translate_simd_movdqa(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_movq(insn)) {
                translate_simd_movq(code_buf, insn, arm_rd, arm_rm);
            }
            /* Arithmetic operations */
            else if (x86_is_addps(insn)) {
                translate_simd_addps(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_addpd(insn)) {
                translate_simd_addpd(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_addss(insn)) {
                translate_simd_addss(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_addsd(insn)) {
                translate_simd_addsd(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_subps(insn)) {
                translate_simd_subps(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_subpd(insn)) {
                translate_simd_subpd(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_subss(insn)) {
                translate_simd_subss(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_subsd(insn)) {
                translate_simd_subsd(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_mulps(insn)) {
                translate_simd_mulps(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_mulpd(insn)) {
                translate_simd_mulpd(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_mulss(insn)) {
                translate_simd_mulss(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_mulsd(insn)) {
                translate_simd_mulsd(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_divps(insn)) {
                translate_simd_divps(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_divpd(insn)) {
                translate_simd_divpd(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_divss(insn)) {
                translate_simd_divss(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_divsd(insn)) {
                translate_simd_divsd(code_buf, insn, arm_rd, arm_rm);
            }
            /* Logical operations */
            else if (x86_is_andps(insn) || x86_is_andpd(insn)) {
                translate_simd_and(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_andnps(insn) || x86_is_andnpd(insn)) {
                translate_simd_andn(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_orps(insn) || x86_is_orpd(insn)) {
                translate_simd_or(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_xorps(insn) || x86_is_xorpd(insn)) {
                translate_simd_xor(code_buf, insn, arm_rd, arm_rm);
            }
            /* Comparison operations */
            else if (x86_is_cmpps(insn)) {
                translate_simd_cmpps(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_cmppd(insn)) {
                translate_simd_cmppd(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_cmpss(insn)) {
                translate_simd_cmpss(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_cmpsd(insn)) {
                translate_simd_cmpsd(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_ucomiss(insn)) {
                translate_simd_ucomiss(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_ucomisd(insn)) {
                translate_simd_ucomisd(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_comiss(insn)) {
                translate_simd_comiss(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_comisd(insn)) {
                translate_simd_comisd(code_buf, insn, arm_rd, arm_rm);
            }
            /* Conversion operations */
            else if (x86_is_cvtps2pd(insn)) {
                translate_simd_cvtps2pd(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_cvtpd2ps(insn)) {
                translate_simd_cvtpd2ps(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_cvtss2sd(insn)) {
                translate_simd_cvtss2sd(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_cvtsd2ss(insn)) {
                translate_simd_cvtsd2ss(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_cvtdq2ps(insn)) {
                translate_simd_cvtdq2ps(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_cvtps2dq(insn)) {
                translate_simd_cvtps2dq(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_cvttps2dq(insn)) {
                translate_simd_cvttps2dq(code_buf, insn, arm_rd, arm_rm);
            }
            /* Square root */
            else if (x86_is_sqrtps(insn)) {
                translate_simd_sqrtps(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_sqrtpd(insn)) {
                translate_simd_sqrtpd(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_sqrtss(insn)) {
                translate_simd_sqrtss(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_sqrtsd(insn)) {
                translate_simd_sqrtsd(code_buf, insn, arm_rd, arm_rm);
            }
            /* Shuffle/unpack */
            else if (x86_is_pshufd(insn)) {
                translate_simd_pshufd(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_pshuflw(insn)) {
                translate_simd_pshuflw(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_pshufhw(insn)) {
                translate_simd_pshufhw(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_punpckldq(insn)) {
                translate_simd_punpckldq(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_punpckhdq(insn)) {
                translate_simd_punpckhdq(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_punpcklqdq(insn)) {
                translate_simd_punpcklqdq(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_punpckhqdq(insn)) {
                translate_simd_punpckhqdq(code_buf, insn, arm_rd, arm_rm);
            }
            /* Integer SIMD */
            else if (x86_is_paddb(insn)) {
                translate_simd_paddb(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_paddw(insn)) {
                translate_simd_paddw(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_paddd(insn)) {
                translate_simd_paddd(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_paddq(insn)) {
                translate_simd_paddq(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_psubb(insn)) {
                translate_simd_psubb(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_psubw(insn)) {
                translate_simd_psubw(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_psubd(insn)) {
                translate_simd_psubd(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_psubq(insn)) {
                translate_simd_psubq(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_pand(insn)) {
                translate_simd_pand(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_pandn(insn)) {
                translate_simd_pandn(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_por(insn)) {
                translate_simd_por(code_buf, insn, arm_rd, arm_rm);
            } else if (x86_is_pxor(insn)) {
                translate_simd_pxor(code_buf, insn, arm_rd, arm_rm);
            }
            /* Default: emit NOP for unhandled SIMD */
            else {
                emit_nop(code_buf);
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
