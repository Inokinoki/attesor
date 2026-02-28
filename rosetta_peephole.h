/* ============================================================================
 * Rosetta Binary Translator - Peephole Optimizer Header
 * ============================================================================
 *
 * This module provides peephole optimization for translated code sequences.
 * It analyzes and optimizes short instruction sequences to produce more
 * efficient x86_64 code.
 *
 * Optimizations include:
 * - Redundant move elimination
 * - Constant folding
 * - Strength reduction
 * - Dead code elimination
 * - Instruction combining
 * ============================================================================ */

#ifndef ROSETTA_PEEPHOLE_H
#define ROSETTA_PEEPHOLE_H

#include "rosetta_types.h"
#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * Optimization Flags
 * ============================================================================ */

#define PEEPHOLE_OPT_NONE           0x0000
#define PEEPHOLE_OPT_REDUNDANT_MOV  0x0001  /* Eliminate redundant moves */
#define PEEPHOLE_OPT_CONST_FOLD     0x0002  /* Constant folding */
#define PEEPHOLE_OPT_STRENGTH_RED   0x0004  /* Strength reduction */
#define PEEPHOLE_OPT_DEAD_CODE      0x0005  /* Dead code elimination */
#define PEEPHOLE_OPT_INSN_COMBINE   0x0008  /* Instruction combining */
#define PEEPHOLE_OPT_ALL            0xFFFF  /* All optimizations */

/* ============================================================================
 * Optimization Context
 * ============================================================================ */

typedef struct {
    u8 *code_start;         /* Start of code buffer */
    u8 *code_end;           /* End of code buffer */
    u8 *code_current;       /* Current position */
    u32 bytes_optimized;    /* Bytes saved by optimization */
    u32 instructions_seen;  /* Instructions analyzed */
    u32 optimizations_done; /* Optimizations applied */
    u32 flags;              /* Optimization flags */
} peephole_context_t;

/* ============================================================================
 * Initialization
 * ============================================================================ */

/**
 * peephole_init - Initialize peephole optimizer
 * @ctx: Peephole context to initialize
 * @flags: Optimization flags
 * Returns: 0 on success
 */
int peephole_init(peephole_context_t *ctx, u32 flags);

/**
 * peephole_cleanup - Cleanup peephole optimizer
 * @ctx: Peephole context to cleanup
 */
void peephole_cleanup(peephole_context_t *ctx);

/* ============================================================================
 * Main Optimization Entry Points
 * ============================================================================ */

/**
 * peephole_optimize - Optimize code in buffer
 * @ctx: Peephole context
 * @code_start: Start of code buffer
 * @code_len: Length of code
 * Returns: Number of bytes optimized (may be less if code shrunk)
 */
u32 peephole_optimize(peephole_context_t *ctx, u8 *code_start, u32 code_len);

/**
 * peephole_optimize_insn - Optimize single instruction at cursor
 * @ctx: Peephole context (cursor set to instruction)
 * Returns: Number of bytes to advance cursor
 */
u32 peephole_optimize_insn(peephole_context_t *ctx);

/* ============================================================================
 * Pattern Matching
 * ============================================================================ */

/**
 * match_mov_imm_zero - Match MOV reg, 0 pattern
 * @code: Code pointer
 * @reg: Output register number
 * Returns: Length of matched sequence (0 = no match)
 */
u32 match_mov_imm_zero(const u8 *code, u8 *reg);

/**
 * match_mov_reg_reg - Match MOV reg1, reg2 pattern
 * @code: Code pointer
 * @dst: Output destination register
 * @src: Output source register
 * Returns: Length of matched sequence (0 = no match)
 */
u32 match_mov_reg_reg(const u8 *code, u8 *dst, u8 *src);

/**
 * match_add_imm_zero - Match ADD reg, 0 pattern
 * @code: Code pointer
 * @reg: Output register number
 * Returns: Length of matched sequence (0 = no match)
 */
u32 match_add_imm_zero(const u8 *code, u8 *reg);

/**
 * match_redundant_mov_chain - Match chain of MOVs (A->B, B->C)
 * @code: Code pointer
 * @first_reg: First register in chain
 * @last_reg: Last register in chain
 * @chain_len: Length of chain (number of MOVs)
 * Returns: Total bytes in chain (0 = no match)
 */
u32 match_redundant_mov_chain(const u8 *code, u8 *first_reg, u8 *last_reg, u32 *chain_len);

/* ============================================================================
 * Optimization Transformers
 * ============================================================================ */

/**
 * optimize_redundant_mov - Eliminate redundant MOV reg, reg
 * @ctx: Peephole context
 * Returns: 1 if optimized, 0 otherwise
 */
int optimize_redundant_mov(peephole_context_t *ctx);

/**
 * optimize_nop_fill - Convert NOP sequences to aligned padding
 * @ctx: Peephole context
 * Returns: 1 if optimized, 0 otherwise
 */
int optimize_nop_fill(peephole_context_t *ctx);

/**
 * optimize_imm_zero - Replace operations with zero immediate
 * @ctx: Peephole context
 * Returns: 1 if optimized, 0 otherwise
 */
int optimize_imm_zero(peephole_context_t *ctx);

/**
 * optimize_lea_arith - Convert ADD/MUL to LEA where beneficial
 * @ctx: Peephole context
 * Returns: 1 if optimized, 0 otherwise
 */
int optimize_lea_arith(peephole_context_t *ctx);

/* ============================================================================
 * Code Emission Helpers
 * ============================================================================ */

/**
 * emit_nop_bytes - Emit NOP padding
 * @code: Code buffer
 * @count: Number of NOP bytes to emit
 */
void emit_nop_bytes(u8 *code, int count);

/**
 * patch_code_region - Patch code region (for shrinkage)
 * @dest: Destination buffer
 * @src: Source buffer
 * @len: Length to copy
 */
void patch_code_region(u8 *dest, const u8 *src, u32 len);

/* ============================================================================
 * Analysis Helpers
 * ============================================================================ */

/**
 * analyze_insn_bytes - Analyze x86_64 instruction length
 * @code: Code pointer
 * Returns: Instruction length in bytes
 */
u32 analyze_insn_bytes(const u8 *code);

/**
 * is_insn_redundant - Check if instruction is redundant
 * @code: Code pointer
 * Returns: 1 if redundant, 0 otherwise
 */
int is_insn_redundant(const u8 *code);

/**
 * get_insn_deps - Get instruction dependencies
 * @code: Code pointer
 * @reg_mask: Output register dependency mask
 * Returns: Dependency flags
 */
u32 get_insn_deps(const u8 *code, u64 *reg_mask);

#endif /* ROSETTA_PEEPHOLE_H */
