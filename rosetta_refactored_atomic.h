/* ============================================================================
 * Rosetta Refactored - Atomic/Memory Barrier Instruction Header
 * ============================================================================
 *
 * This header defines the interface for ARM64 atomic and memory barrier
 * instruction translation to x86_64.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_ATOMIC_H
#define ROSETTA_REFACTORED_ATOMIC_H

#include "rosetta_refactored.h"
#include "rosetta_emit_x86.h"
#include <stdint.h>

/* NOTE: This module provides code-buffer based atomic/barrier translation.
 * The rosetta_trans_* modules provide ThreadState-based translation.
 * Function names are prefixed with 'refactored_' to avoid symbol conflicts.
 */

/* ============================================================================
 * Memory Barrier Instructions
 * ============================================================================ */

/**
 * refactored_translate_dmb - Translate ARM64 DMB (Data Memory Barrier)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * Returns: 0 on success
 */
int refactored_translate_dmb(uint32_t encoding, code_buf_t *code_buf);

/**
 * refactored_translate_dsb - Translate ARM64 DSB (Data Synchronization Barrier)
 */
int refactored_translate_dsb(uint32_t encoding, code_buf_t *code_buf);

/**
 * refactored_translate_isb - Translate ARM64 ISB (Instruction Synchronization Barrier)
 */
int refactored_translate_isb(uint32_t encoding, code_buf_t *code_buf);

/* ============================================================================
 * Load-Acquire / Store-Release (scalar)
 * ============================================================================ */

/**
 * refactored_translate_ldar - Translate ARM64 LDAR (Load-Acquire Register)
 */
int refactored_translate_ldar(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_stlr - Translate ARM64 STLR (Store-Release Register)
 */
int refactored_translate_stlr(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_ldarb - Translate ARM64 LDARB (Load-Acquire byte)
 */
int refactored_translate_ldarb(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_stlrb - Translate ARM64 STLRB (Store-Release byte)
 */
int refactored_translate_stlrb(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_ldarh - Translate ARM64 LDARH (Load-Acquire halfword)
 */
int refactored_translate_ldarh(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_stlrh - Translate ARM64 STLRH (Store-Release halfword)
 */
int refactored_translate_stlrh(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_ldapr - Translate ARM64 LDAPR (Load-Acquire PC-relative)
 */
int refactored_translate_ldapr(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/* ============================================================================
 * Atomic Load/Store Exclusive
 * ============================================================================ */

/**
 * refactored_translate_ldaxr - Translate ARM64 LDAXR (Load-Acquire Exclusive)
 */
int refactored_translate_ldaxr(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_stlxr - Translate ARM64 STLXR (Store-Release Exclusive)
 */
int refactored_translate_stlxr(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_ldxr - Translate ARM64 LDXR (Load Exclusive)
 */
int refactored_translate_ldxr(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_stxr - Translate ARM64 STXR (Store Exclusive)
 */
int refactored_translate_stxr(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_ldaxrb - Translate ARM64 LDAXRB (byte)
 */
int refactored_translate_ldaxrb(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_stlxrb - Translate ARM64 STLXRB (byte)
 */
int refactored_translate_stlxrb(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_ldaxrh - Translate ARM64 LDAXRH (halfword)
 */
int refactored_translate_ldaxrh(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_stlxrh - Translate ARM64 STLXRH (halfword)
 */
int refactored_translate_stlxrh(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/* ============================================================================
 * Atomic Memory Operations
 * ============================================================================ */

/**
 * refactored_translate_ldadd - Translate ARM64 LDADD (Atomic Add on Memory)
 */
int refactored_translate_ldadd(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_ldclr - Translate ARM64 LDCLR (Atomic Bit Clear on Memory)
 */
int refactored_translate_ldclr(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_ldset - Translate ARM64 LDSET (Atomic Bit Set on Memory)
 */
int refactored_translate_ldset(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_ldumax - Translate ARM64 LDUMAX (Atomic Unsigned Max)
 */
int refactored_translate_ldumax(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_ldumin - Translate ARM64 LDUMIN (Atomic Unsigned Min)
 */
int refactored_translate_ldumin(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_swp - Translate ARM64 SWP (Atomic Swap)
 */
int refactored_translate_swp(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_ldadda - Translate ARM64 LDADDAL (Atomic Add with AL variant)
 */
int refactored_translate_ldadda(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_ldclra - Translate ARM64 LDCLRAL (Atomic Clear with AL variant)
 */
int refactored_translate_ldclra(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_ldseta - Translate ARM64 LDSETAL (Atomic Set with AL variant)
 */
int refactored_translate_ldseta(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/* ============================================================================
 * Compare-And-Swap Operations
 * ============================================================================ */

/**
 * refactored_translate_cas - Translate ARM64 CAS (Compare-And-Swap)
 */
int refactored_translate_cas(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_casa - Translate ARM64 CASAL (Compare-And-Swap with AL variant)
 */
int refactored_translate_casa(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_casl - Translate ARM64 CASL (Compare-And-Swap Load-Acquire)
 */
int refactored_translate_casl(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_casb - Translate ARM64 CASB (byte variant)
 */
int refactored_translate_casb(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_cash - Translate ARM64 CASH (halfword variant)
 */
int refactored_translate_cash(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/**
 * refactored_translate_casp - Translate ARM64 CASP (Compare-And-Swap Pair)
 */
int refactored_translate_casp(uint32_t encoding, code_buf_t *code_buf, uint64_t *x_regs);

/* ============================================================================
 * Dispatch Function
 * ============================================================================ */

/**
 * refactored_translate_atomic_dispatch - Dispatch atomic instruction based on encoding
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @x_regs: General purpose register state
 * Returns: 0 on success, -1 if not an atomic instruction
 */
int refactored_translate_atomic_dispatch(uint32_t encoding, code_buf_t *code_buf,
                              uint64_t *x_regs);

#endif /* ROSETTA_REFACTORED_ATOMIC_H */
