/* ============================================================================
 * Rosetta Binary Translator - Peephole Optimizer Implementation
 * ============================================================================
 *
 * This module implements peephole optimization for translated code sequences.
 * ============================================================================ */

#include "rosetta_peephole.h"
#include <string.h>

/* ============================================================================
 * Initialization
 * ============================================================================ */

/**
 * peephole_init - Initialize peephole optimizer
 */
int peephole_init(peephole_context_t *ctx, u32 flags)
{
    if (!ctx) {
        return -1;
    }

    memset(ctx, 0, sizeof(*ctx));
    ctx->flags = flags ? flags : PEEPHOLE_OPT_ALL;
    return 0;
}

/**
 * peephole_cleanup - Cleanup peephole optimizer
 */
void peephole_cleanup(peephole_context_t *ctx)
{
    if (ctx) {
        memset(ctx, 0, sizeof(*ctx));
    }
}

/* ============================================================================
 * Pattern Matching Implementations
 * ============================================================================ */

/**
 * match_mov_imm_zero - Match MOV reg, 0 pattern
 *
 * Matches:
 * - 31 C0       XOR EAX, EAX
 * - 31 C9       XOR ECX, ECX
 * - B8 00...    MOV EAX, 0
 * - C7 C0 00..  MOV EAX, 0 (alternate encoding)
 */
u32 match_mov_imm_zero(const u8 *code, u8 *reg)
{
    if (!code || !reg) return 0;

    /* XOR reg, reg (3 bytes: 31 C0) */
    if (code[0] == 0x31 && code[1] >= 0xC0 && code[1] <= 0xC7) {
        *reg = code[1] - 0xC0;
        return 2;
    }

    /* XOR reg, reg with REX (4 bytes: 41 31 C0) */
    if (code[0] == 0x41 && code[1] == 0x31 && code[2] >= 0xC0) {
        *reg = code[2] - 0xC0;
        return 3;
    }

    return 0;
}

/**
 * match_mov_reg_reg - Match MOV reg1, reg2 pattern
 *
 * Matches:
 * - 48 89 C0    MOV RAX, RAX
 * - 89 C0       MOV EAX, EAX
 * - 4C 89 C0    MOV RAX, R8
 */
u32 match_mov_reg_reg(const u8 *code, u8 *dst, u8 *src)
{
    if (!code || !dst || !src) return 0;

    const u8 *p = code;
    u8 rex = 0;

    /* Check for REX prefix */
    if (*p >= 0x40 && *p <= 0x4F) {
        rex = *p++;
    }

    /* MOV r/m32, r32 (89 /r) or MOV r/m64, r64 (48 89 /r) */
    if (*p == 0x89) {
        p++;
        u8 modrm = *p;
        if ((modrm & 0xC0) == 0xC0) {  /* reg to reg */
            *dst = (modrm >> 3) & 0x07;
            *src = modrm & 0x07;
            if (rex & 0x04) *dst |= 0x08;  /* R bit */
            if (rex & 0x01) *src |= 0x08;  /* B bit */
            return (u32)(p - code + 1);
        }
    }

    return 0;
}

/**
 * match_add_imm_zero - Match ADD reg, 0 pattern
 */
u32 match_add_imm_zero(const u8 *code, u8 *reg)
{
    if (!code || !reg) return 0;

    /* ADD reg, 0 - typically 83 C0 00 */
    if (code[0] == 0x83 && (code[1] & 0xF8) == 0xC0 && code[2] == 0x00) {
        *reg = code[1] & 0x07;
        return 3;
    }

    return 0;
}

/**
 * match_redundant_mov_chain - Match chain of MOVs
 */
u32 match_redundant_mov_chain(const u8 *code, u8 *first_reg, u8 *last_reg, u32 *chain_len)
{
    u8 dst, src;
    u32 offset = 0;
    u32 len = 0;

    if (!code || !first_reg || !last_reg || !chain_len) return 0;

    *first_reg = 0xFF;
    *last_reg = 0;
    *chain_len = 0;

    /* Match first MOV */
    u32 mov_len = match_mov_reg_reg(code + offset, &dst, &src);
    if (!mov_len) return 0;

    *first_reg = src;
    offset += mov_len;
    len += mov_len;
    (*chain_len)++;

    /* Match subsequent MOVs in chain */
    while (*chain_len < 4) {  /* Max chain length */
        u8 next_dst, next_src;
        mov_len = match_mov_reg_reg(code + offset, &next_dst, &next_src);
        if (!mov_len) break;

        /* Check if this continues the chain */
        if (next_src != dst) break;

        dst = next_dst;
        offset += mov_len;
        len += mov_len;
        (*chain_len)++;
    }

    *last_reg = dst;
    return (*chain_len > 1) ? len : 0;
}

/* ============================================================================
 * Analysis Helpers
 * ============================================================================ */

/**
 * analyze_insn_bytes - Analyze x86_64 instruction length
 * Simplified decoder for common instructions
 */
u32 analyze_insn_bytes(const u8 *code)
{
    const u8 *p = code;
    u8 rex = 0;
    u8 opcode;

    if (!code) return 0;

    /* REX prefix */
    if (*p >= 0x40 && *p <= 0x4F) {
        rex = *p++;
    }

    /* Two-byte escape */
    if (*p == 0x0F) {
        p++;
        p++;  /* Skip second opcode byte */
        /* Check for ModR/M */
        if (*p >= 0xC0) p++;  /* reg form */
        else p += 2;  /* memory form with displacement */
        return (u32)(p - code);
    }

    /* Single-byte opcodes */
    opcode = *p++;

    switch (opcode) {
        case 0x89:  /* MOV r/m, r */
        case 0x8B:  /* MOV r, r/m */
        case 0x01:  /* ADD r/m, r */
        case 0x03:  /* ADD r, r/m */
        case 0x29:  /* SUB r/m, r */
        case 0x2B:  /* SUB r, r/m */
        case 0x31:  /* XOR r/m, r */
        case 0x33:  /* XOR r, r/m */
        case 0x39:  /* CMP r/m, r */
        case 0x3B:  /* CMP r, r/m */
            if (*p >= 0xC0) p++;  /* reg form */
            else p += 2;  /* memory form */
            break;

        case 0xB0: case 0xB1: case 0xB2: case 0xB3:
        case 0xB4: case 0xB5: case 0xB6: case 0xB7:
            p++;  /* MOV r8, imm8 */
            break;

        case 0xB8: case 0xB9: case 0xBA: case 0xBB:
        case 0xBC: case 0xBD: case 0xBE: case 0xBF:
            p += (rex & 0x08) ? 8 : 4;  /* MOV r32/64, imm */
            break;

        case 0x83:  /* ADD/SUB/etc r/m, imm8 */
            p += 2;  /* ModR/M + imm8 */
            break;

        case 0xC3:  /* RET */
            break;

        case 0x90:  /* NOP */
            break;

        default:
            p++;  /* Unknown, skip */
            break;
    }

    return (u32)(p - code);
}

/**
 * is_insn_redundant - Check if instruction is redundant
 */
int is_insn_redundant(const u8 *code)
{
    u8 dst, src;

    if (!code) return 0;

    /* Check for MOV reg, reg (same register) */
    if (match_mov_reg_reg(code, &dst, &src)) {
        return (dst == src);
    }

    /* Check for ADD reg, 0 */
    u8 reg;
    if (match_add_imm_zero(code, &reg)) {
        return 1;  /* ADD reg, 0 is redundant */
    }

    return 0;
}

/**
 * get_insn_deps - Get instruction dependencies
 */
u32 get_insn_deps(const u8 *code, u64 *reg_mask)
{
    if (!code || !reg_mask) return 0;

    *reg_mask = 0;

    /* Simplified: just identify if instruction reads/writes registers */
    u8 dst, src;
    if (match_mov_reg_reg(code, &dst, &src)) {
        *reg_mask = (1ULL << dst) | (1ULL << (src + 16));
        return 1;
    }

    return 0;
}

/* ============================================================================
 * Optimization Transformers
 * ============================================================================ */

/**
 * optimize_redundant_mov - Eliminate redundant MOV reg, reg
 */
int optimize_redundant_mov(peephole_context_t *ctx)
{
    u8 dst, src;
    u32 len;

    if (!ctx || !ctx->code_current) return 0;

    len = match_mov_reg_reg(ctx->code_current, &dst, &src);
    if (!len) return 0;

    /* MOV reg, reg is redundant - replace with NOP */
    if (dst == src) {
        for (u32 i = 0; i < len; i++) {
            ctx->code_current[i] = 0x90;  /* NOP */
        }
        ctx->bytes_optimized += len;
        ctx->optimizations_done++;
        return 1;
    }

    return 0;
}

/**
 * optimize_nop_fill - Convert NOP sequences to aligned padding
 */
int optimize_nop_fill(peephole_context_t *ctx)
{
    u32 nop_count = 0;
    u8 *p = ctx->code_current;

    if (!ctx || !p) return 0;

    /* Count consecutive NOPs */
    while (p < ctx->code_end && *p == 0x90) {
        nop_count++;
        p++;
    }

    if (nop_count >= 2) {
        /* Already NOPs, could be optimized for alignment */
        /* For now, just count them */
        ctx->instructions_seen++;
        return 1;
    }

    return 0;
}

/**
 * optimize_imm_zero - Replace operations with zero immediate
 */
int optimize_imm_zero(peephole_context_t *ctx)
{
    u8 reg;
    u32 len;

    if (!ctx || !ctx->code_current) return 0;

    len = match_add_imm_zero(ctx->code_current, &reg);
    if (!len) return 0;

    /* Replace ADD reg, 0 with NOPs */
    for (u32 i = 0; i < len; i++) {
        ctx->code_current[i] = 0x90;
    }
    ctx->bytes_optimized += len;
    ctx->optimizations_done++;
    return 1;
}

/**
 * optimize_lea_arith - Convert ADD/MUL to LEA where beneficial
 */
int optimize_lea_arith(peephole_context_t *ctx)
{
    /* Placeholder for LEA optimization */
    /* This would convert sequences like:
     *   ADD RAX, RBX
     *   SHL RAX, 2
     * Into:
     *   LEA RAX, [RBX*4]
     */
    (void)ctx;
    return 0;
}

/* ============================================================================
 * Main Optimization Entry Points
 * ============================================================================ */

/**
 * peephole_optimize - Optimize code in buffer
 */
u32 peephole_optimize(peephole_context_t *ctx, u8 *code_start, u32 code_len)
{
    u8 *code_end;

    if (!ctx || !code_start || !code_len) return code_len;

    ctx->code_start = code_start;
    ctx->code_end = code_start + code_len;
    ctx->code_current = code_start;
    ctx->bytes_optimized = 0;
    ctx->instructions_seen = 0;
    ctx->optimizations_done = 0;

    code_end = ctx->code_end;

    /* Single-pass peephole optimization */
    while (ctx->code_current < code_end) {
        u32 insn_len = analyze_insn_bytes(ctx->code_current);
        if (insn_len == 0) {
            ctx->code_current++;
            continue;
        }

        ctx->instructions_seen++;

        /* Apply optimizations based on flags */
        if (ctx->flags & PEEPHOLE_OPT_REDUNDANT_MOV) {
            optimize_redundant_mov(ctx);
        }

        if (ctx->flags & PEEPHOLE_OPT_CONST_FOLD) {
            optimize_imm_zero(ctx);
        }

        if (ctx->flags & PEEPHOLE_OPT_INSN_COMBINE) {
            optimize_lea_arith(ctx);
        }

        ctx->code_current += insn_len;
    }

    return code_len - ctx->bytes_optimized;
}

/**
 * peephole_optimize_insn - Optimize single instruction at cursor
 */
u32 peephole_optimize_insn(peephole_context_t *ctx)
{
    u32 insn_len;

    if (!ctx || !ctx->code_current) return 0;

    insn_len = analyze_insn_bytes(ctx->code_current);
    if (insn_len == 0) return 1;

    /* Apply optimizations */
    if (ctx->flags & PEEPHOLE_OPT_REDUNDANT_MOV) {
        if (!optimize_redundant_mov(ctx)) {
            if (ctx->flags & PEEPHOLE_OPT_CONST_FOLD) {
                optimize_imm_zero(ctx);
            }
        }
    }

    return insn_len;
}

/* ============================================================================
 * Code Emission Helpers
 * ============================================================================ */

/**
 * emit_nop_bytes - Emit NOP padding
 */
void emit_nop_bytes(u8 *code, int count)
{
    static const u8 nop_sequences[9][9] = {
        { 0x90 },
        { 0x66, 0x90 },
        { 0x0F, 0x1F, 0x00 },
        { 0x0F, 0x1F, 0x40, 0x00 },
        { 0x0F, 0x1F, 0x44, 0x00, 0x00 },
        { 0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00 },
        { 0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00 },
        { 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 },
        { 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 }
    };

    while (count > 0) {
        int seq_len = (count > 9) ? 9 : count;
        memcpy(code, nop_sequences[seq_len - 1], seq_len);
        code += seq_len;
        count -= seq_len;
    }
}

/**
 * patch_code_region - Patch code region (for shrinkage)
 */
void patch_code_region(u8 *dest, const u8 *src, u32 len)
{
    if (dest && src && len) {
        memmove(dest, src, len);
    }
}
