/* ============================================================================
 * Rosetta Translator - Translation Dispatch Implementation
 * ============================================================================
 *
 * Main block translation dispatch and instruction routing.
 * ============================================================================ */

#include "rosetta_types.h"
#include "rosetta_codegen.h"
#include "rosetta_trans_dispatch.h"
#include "rosetta_translate_alu_impl.h"
#include "rosetta_translate_memory_impl.h"
#include "rosetta_translate_branch_impl.h"
#include "rosetta_translate_special_impl.h"
#include "rosetta_x86_insns.h"
#include <string.h>

/* Forward declarations for cache functions */
void *translation_lookup(uint64_t guest_pc);
void translation_insert(uint64_t guest_pc, uint64_t host_pc, size_t size);
void *code_cache_alloc(size_t size);

/* Register mapping: x86_64 -> ARM64 */
static inline uint8_t map_x86_to_arm(uint8_t x86_reg)
{
    return x86_reg & 0x0F;  /* RAX-R15 -> X0-X15 */
}

/* ============================================================================
 * Instruction Dispatch Functions
 * ============================================================================ */

void dispatch_alu_insn(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm)
{
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
}

void dispatch_memory_insn(code_buffer_t *code_buf, const x86_insn_t *insn,
                          uint8_t arm_rd, uint8_t arm_rm)
{
    if (x86_is_mov_imm64(insn)) {
        translate_memory_mov(code_buf, insn, arm_rd, arm_rm);
    } else if (x86_is_mov(insn)) {
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
}

int dispatch_branch_insn(code_buffer_t *code_buf, const x86_insn_t *insn,
                         uint64_t block_pc, uint8_t arm_rd, uint8_t arm_rm)
{
    if (x86_is_jcc(insn)) {
        return translate_branch_jcc(code_buf, insn, block_pc);
    } else if (x86_is_jmp(insn)) {
        return translate_branch_jmp(code_buf, insn, block_pc);
    } else if (x86_is_call(insn)) {
        return translate_branch_call(code_buf, insn, block_pc);
    } else if (x86_is_ret(insn)) {
        return translate_branch_ret(code_buf);
    } else if (x86_is_cmov(insn)) {
        translate_branch_cmov(code_buf, insn, arm_rd, arm_rm);
    } else if (x86_is_setcc(insn)) {
        translate_branch_setcc(code_buf, insn, arm_rd);
    } else if (x86_is_xchg(insn)) {
        translate_branch_xchg(code_buf, insn, arm_rd, arm_rm);
    }
    return 0;
}

void dispatch_bit_insn(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm)
{
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
}

void dispatch_string_insn(code_buffer_t *code_buf, const x86_insn_t *insn)
{
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
}

void dispatch_special_insn(code_buffer_t *code_buf, const x86_insn_t *insn,
                           uint8_t arm_rd, uint8_t arm_rm)
{
    if (x86_is_cpuid(insn)) {
        translate_special_cpuid(code_buf, insn);
    } else if (x86_is_rdtsc(insn)) {
        translate_special_rdtsc(code_buf, insn);
    } else if (x86_is_shld(insn)) {
        translate_special_shld(code_buf, insn, arm_rd, arm_rm);
    } else if (x86_is_shrd(insn)) {
        translate_special_shrd(code_buf, insn, arm_rd, arm_rm);
    } else if (x86_is_cqo(insn)) {
        translate_special_cqo(code_buf, insn);
    } else if (x86_is_cli(insn)) {
        translate_special_cli(code_buf, insn);
    } else if (x86_is_sti(insn)) {
        translate_special_sti(code_buf, insn);
    } else if (x86_is_nop(insn)) {
        translate_special_nop(code_buf, insn);
    }
}

/* ============================================================================
 * Block Translation Functions
 * ============================================================================ */

void *translate_block(uint64_t guest_pc)
{
    /* Look up in translation cache first */
    void *cached = translation_lookup(guest_pc);
    if (cached != NULL) {
        return cached;
    }

    /* Initialize code buffer for JIT emission */
    code_buffer_t code_buf;
    code_buffer_init(&code_buf, NULL, 65536);  /* 64KB per block max */

    /* Translate instructions in the basic block */
    uint64_t block_pc = guest_pc;
    int block_size = 0;
    int is_block_end = 0;

    while (!is_block_end && block_size < MAX_BLOCK_INSTRUCTIONS) {
        /* Decode x86_64 instruction at current PC */
        const uint8_t *insn_ptr = (const uint8_t *)block_pc;
        x86_insn_t insn;
        decode_x86_insn(insn_ptr, &insn);

        /* Map x86_64 registers to ARM64 */
        uint8_t arm_rd = map_x86_to_arm(insn.reg);
        uint8_t arm_rm = map_x86_to_arm(insn.rm);

        /* Translate based on instruction type using dispatch functions */
        if (x86_is_add(&insn) || x86_is_sub(&insn) ||
            x86_is_and(&insn) || x86_is_or(&insn) || x86_is_xor(&insn) ||
            x86_is_mul(&insn) || x86_is_div(&insn) ||
            x86_is_inc(&insn) || x86_is_dec(&insn) ||
            x86_is_neg(&insn) || x86_is_not(&insn) ||
            x86_is_shl(&insn) || x86_is_shr(&insn) || x86_is_sar(&insn) ||
            x86_is_rol(&insn) || x86_is_ror(&insn)) {
            dispatch_alu_insn(&code_buf, &insn, arm_rd, arm_rm);
        } else if (x86_is_mov(&insn) || x86_is_mov_imm64(&insn) ||
                   x86_is_movzx(&insn) || x86_is_movsx(&insn) || x86_is_movsxd(&insn) ||
                   x86_is_lea(&insn) || x86_is_push(&insn) || x86_is_pop(&insn) ||
                   x86_is_cmp(&insn) || x86_is_test(&insn)) {
            dispatch_memory_insn(&code_buf, &insn, arm_rd, arm_rm);
        } else if (x86_is_jcc(&insn) || x86_is_jmp(&insn) || x86_is_call(&insn) ||
                   x86_is_ret(&insn) || x86_is_cmov(&insn) || x86_is_setcc(&insn) ||
                   x86_is_xchg(&insn)) {
            is_block_end = dispatch_branch_insn(&code_buf, &insn, block_pc, arm_rd, arm_rm);
        } else if (x86_is_bsf(&insn) || x86_is_bsr(&insn) || x86_is_popcnt(&insn) ||
                   x86_is_bt(&insn) || x86_is_bts(&insn) || x86_is_btr(&insn) ||
                   x86_is_btc(&insn)) {
            dispatch_bit_insn(&code_buf, &insn, arm_rd, arm_rm);
        } else if (x86_is_movs(&insn) || x86_is_stos(&insn) || x86_is_lods(&insn) ||
                   x86_is_cmps(&insn) || x86_is_scas(&insn)) {
            dispatch_string_insn(&code_buf, &insn);
        } else if (x86_is_cpuid(&insn) || x86_is_rdtsc(&insn) || x86_is_shld(&insn) ||
                   x86_is_shrd(&insn) || x86_is_cqo(&insn) || x86_is_cli(&insn) ||
                   x86_is_sti(&insn) || x86_is_nop(&insn)) {
            dispatch_special_insn(&code_buf, &insn, arm_rd, arm_rm);
        } else {
            /* Unknown instruction - emit NOP */
            emit_nop(&code_buf);
        }

        /* Advance to next instruction */
        block_pc += insn.length;
        block_size++;

        /* Check for block end conditions */
        if (is_block_end || insn.length == 0) {
            break;
        }
    }

    /* Ensure block ends with RET if not already */
    if (!is_block_end) {
        emit_ret(&code_buf);
    }

    /* Check for errors */
    if (code_buf.error) {
        return NULL;
    }

    /* Allocate code cache memory and copy generated code */
    size_t code_size = code_buffer_get_size(&code_buf);
    void *code_cache = code_cache_alloc(code_size);
    if (!code_cache) {
        return NULL;
    }

    /* Copy generated code to executable memory */
    memcpy(code_cache, code_buf.buffer, code_size);

    /* Insert into translation cache */
    translation_insert(guest_pc, (uint64_t)code_cache, code_size);

    return code_cache;
}

void *translate_block_fast(uint64_t guest_pc)
{
    /* Check cache with faster lookup */
    void *cached = translation_lookup(guest_pc);
    if (cached != NULL) {
        return cached;
    }

    /* Fall back to standard translation */
    return translate_block(guest_pc);
}

void execute_translated(ThreadState *state, void *block)
{
    if (block == NULL || state == NULL) {
        return;
    }

    /* Cast block to function pointer and execute */
    typedef void (*translated_func_t)(void);
    ((translated_func_t)block)();
}
