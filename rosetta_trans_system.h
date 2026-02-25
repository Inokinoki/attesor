/* ============================================================================
 * Rosetta Translator - System Instruction Translation Header
 * ============================================================================
 *
 * This module provides system instruction translation functions
 * for ARM64 to x86_64 binary translation, including system registers,
 * barriers, and privilege operations.
 * ============================================================================ */

#ifndef ROSETTA_TRANS_SYSTEM_H
#define ROSETTA_TRANS_SYSTEM_H

#include <stdint.h>
#include "rosetta_types.h"

/* ============================================================================
 * System Register Access
 * ============================================================================ */

/**
 * translate_mrs - Translate ARM64 MRS (move from system register)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_mrs(ThreadState *state, const uint8_t *insn);

/**
 * translate_msr - Translate ARM64 MSR (move to system register)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_msr(ThreadState *state, const uint8_t *insn);

/**
 * translate_mrs_impl - Implementation helper for MRS
 * @param state Thread state
 * @param sysreg System register encoding
 * @param rt Target general purpose register
 * @return 0 on success, -1 on failure
 */
int translate_mrs_impl(ThreadState *state, uint16_t sysreg, uint8_t rt);

/**
 * translate_msr_impl - Implementation helper for MSR
 * @param state Thread state
 * @param sysreg System register encoding
 * @param rt Source general purpose register
 * @return 0 on success, -1 on failure
 */
int translate_msr_impl(ThreadState *state, uint16_t sysreg, uint8_t rt);

/* ============================================================================
 * System Control Registers
 * ============================================================================ */

/**
 * translate_mrs_cntvct - Translate MRS CNTVCT_EL0 (counter-timer)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_mrs_cntvct(ThreadState *state, const uint8_t *insn);

/**
 * translate_mrs_cntfrq - Translate MRS CNTFRQ_EL0 (counter frequency)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_mrs_cntfrq(ThreadState *state, const uint8_t *insn);

/**
 * translate_mrs_tpidr - Translate MRS TPIDR_EL0 (thread ID)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_mrs_tpidr(ThreadState *state, const uint8_t *insn);

/**
 * translate_msr_tpidr - Translate MSR TPIDR_EL0 (thread ID)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_msr_tpidr(ThreadState *state, const uint8_t *insn);

/**
 * translate_mrs_fpcr - Translate MRS FPCR (FP control)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_mrs_fpcr(ThreadState *state, const uint8_t *insn);

/**
 * translate_msr_fpcr - Translate MSR FPCR (FP control)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_msr_fpcr(ThreadState *state, const uint8_t *insn);

/**
 * translate_mrs_fpsr - Translate MRS FPSR (FP status)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_mrs_fpsr(ThreadState *state, const uint8_t *insn);

/**
 * translate_msr_fpsr - Translate MSR FPSR (FP status)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_msr_fpsr(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Memory Barriers and Synchronization
 * ============================================================================ */

/**
 * translate_dmb - Translate ARM64 DMB (data memory barrier)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_dmb(ThreadState *state, const uint8_t *insn);

/**
 * translate_dsb - Translate ARM64 DSB (data synchronization barrier)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_dsb(ThreadState *state, const uint8_t *insn);

/**
 * translate_isb - Translate ARM64 ISB (instruction synchronization barrier)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_isb(ThreadState *state, const uint8_t *insn);

/**
 * translate_dmb_impl - Implementation helper for DMB
 * @param state Thread state
 * @param option Barrier option (ISH, OSH, etc.)
 * @return 0 on success, -1 on failure
 */
int translate_dmb_impl(ThreadState *state, uint8_t option);

/**
 * translate_dsb_impl - Implementation helper for DSB
 * @param state Thread state
 * @param option Barrier option
 * @return 0 on success, -1 on failure
 */
int translate_dsb_impl(ThreadState *state, uint8_t option);

/* ============================================================================
 * Cache and TLB Operations
 * ============================================================================ */

/**
 * translate_dc - Translate ARM64 DC (data cache) operations
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_dc(ThreadState *state, const uint8_t *insn);

/**
 * translate_ic - Translate ARM64 IC (instruction cache) operations
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ic(ThreadState *state, const uint8_t *insn);

/**
 * translate_tlbi - Translate ARM64 TLBI (TLB invalidate)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_tlbi(ThreadState *state, const uint8_t *insn);

/**
 * translate_dc_civac - Translate DC CIVAC (clean & invalidate)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_dc_civac(ThreadState *state, const uint8_t *insn);

/**
 * translate_dc_cvac - Translate DC CVAC (clean to PoC)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_dc_cvac(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Exception and Debug
 * ============================================================================ */

/**
 * translate_svc - Translate ARM64 SVC (supervisor call)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_svc(ThreadState *state, const uint8_t *insn);

/**
 * translate_hvc - Translate ARM64 HVC (hypervisor call)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_hvc(ThreadState *state, const uint8_t *insn);

/**
 * translate_smc - Translate ARM64 SMC (secure monitor call)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_smc(ThreadState *state, const uint8_t *insn);

/**
 * translate_brk - Translate ARM64 BRK (breakpoint)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_brk(ThreadState *state, const uint8_t *insn);

/**
 * translate_hlt - Translate ARM64 HLT (halt)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_hlt(ThreadState *state, const uint8_t *insn);

/**
 * translate_udf - Translate ARM64 UDF (undefined)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_udf(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Special Instructions
 * ============================================================================ */

/**
 * translate_clrex - Translate ARM64 CLREX (clear exclusive)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_clrex(ThreadState *state, const uint8_t *insn);

/**
 * translate_ldaxr - Translate ARM64 LDAXR (load acquire)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldaxr(ThreadState *state, const uint8_t *insn);

/**
 * translate_stlxr - Translate ARM64 STLXR (store release exclusive)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_stlxr(ThreadState *state, const uint8_t *insn);

/**
 * translate_ldxr - Translate ARM64 LDXR (load exclusive)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldxr(ThreadState *state, const uint8_t *insn);

/**
 * translate_stxr - Translate ARM64 STXR (store exclusive)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_stxr(ThreadState *state, const uint8_t *insn);

/**
 * translate_wfe - Translate ARM64 WFE (wait for event)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_wfe(ThreadState *state, const uint8_t *insn);

/**
 * translate_wfi - Translate ARM64 WFI (wait for interrupt)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_wfi(ThreadState *state, const uint8_t *insn);

/**
 * translate_yield - Translate ARM64 YIELD
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_yield(ThreadState *state, const uint8_t *insn);

/**
 * translate_nop - Translate ARM64 NOP
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_nop(ThreadState *state, const uint8_t *insn);

/**
 * translate_sev - Translate ARM64 SEV (send event)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sev(ThreadState *state, const uint8_t *insn);

/**
 * translate_sevl - Translate ARM64 SEVL (send event local)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sevl(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Hint Instructions
 * ============================================================================ */

/**
 * translate_hint - Translate ARM64 HINT instructions
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_hint(ThreadState *state, const uint8_t *insn);

/**
 * translate_xpac - Translate ARM64 XPAC (strip pointer auth)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_xpac(ThreadState *state, const uint8_t *insn);

/**
 * translate_paciaz - Translate ARM64 PACIAZ (pointer auth)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_paciaz(ThreadState *state, const uint8_t *insn);

#endif /* ROSETTA_TRANS_SYSTEM_H */
