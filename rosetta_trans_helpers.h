/* ============================================================================
 * Rosetta Translator - Translation Helpers Header
 * ============================================================================
 *
 * This module provides translation helper functions for the Rosetta
 * binary translation layer, including block translation, caching,
 * and context management.
 * ============================================================================ */

#ifndef ROSETTA_TRANS_HELPERS_H
#define ROSETTA_TRANS_HELPERS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "rosetta_types.h"
#include "rosetta_refactored.h"

/* ============================================================================
 * Translation Block Helpers
 * ============================================================================ */

/**
 * Translate a basic block
 * @param guest_pc Guest PC of block to translate
 * @return Translated block pointer, or NULL on failure
 */
void *helper_block_translate(uint64_t guest_pc);

/**
 * Insert translated block into cache
 * @param guest Guest PC
 * @param host Host PC
 * @param size Block size
 * @return 0 on success, -1 on failure
 */
int helper_block_insert(uint64_t guest, uint64_t host, size_t size);

/**
 * Look up translated block
 * @param guest_pc Guest PC
 * @return Host PC, or NULL if not found
 */
void *helper_block_lookup(uint64_t guest_pc);

/**
 * Remove translated block from cache
 * @param guest_pc Guest PC
 * @return 0 on success, -1 on failure
 */
int helper_block_remove(uint64_t guest_pc);

/**
 * Invalidate all translated blocks
 */
void helper_block_invalidate(void);

/* ============================================================================
 * Context Management Helpers
 * ============================================================================ */

/**
 * Perform context switch
 * @param old_ctx Old context to save
 * @param new_ctx New context to restore
 */
void helper_context_switch(CPUContext *old_ctx, CPUContext *new_ctx);

/**
 * Called on syscall entry
 * @param state Thread state
 * @param nr Syscall number
 */
void helper_syscall_enter(ThreadState *state, int nr);

/**
 * Called on syscall exit
 * @param state Thread state
 * @param result Syscall result
 */
void helper_syscall_exit(ThreadState *state, int64_t result);

/**
 * Handle interrupt
 * @param state Thread state
 * @param vector Interrupt vector
 */
void helper_interrupt(ThreadState *state, int vector);

/* ============================================================================
 * Debug and Performance Helpers
 * ============================================================================ */

/**
 * Debug trace helper
 * @param msg Message to trace
 * @param value Value to trace
 */
void helper_debug_trace(const char *msg, uint64_t value);

/**
 * Performance counter helper
 * @return Current performance counter value
 */
uint64_t helper_perf_counter(void);

/* ============================================================================
 * Switch Case Handlers
 * ============================================================================ */

/**
 * Switch case handler 13
 * @param value Switch value
 * @return Handler result
 */
uint64_t switch_case_handler_13(uint64_t value);

/**
 * Switch case handler 2e
 * @param value Switch value
 * @return Handler result
 */
uint64_t switch_case_handler_2e(uint64_t value);

/* ============================================================================
 * FP Estimate Helpers
 * ============================================================================ */

/**
 * Reciprocal estimate (FRECPE)
 * @param value Input value
 * @return Reciprocal estimate
 */
float fp_recip_estimate(float value);

/**
 * Reciprocal square root estimate (FRSQRTE)
 * @param value Input value
 * @return Reciprocal square root estimate
 */
float fp_rsqrt_estimate(float value);

/* ============================================================================
 * Context Save/Restore Helpers
 * ============================================================================ */

/**
 * Save FP context
 * @param save Array to save FP state (32 x 128-bit registers + fpsr/fpcr)
 */
void save_fp_context(uint64_t *save);

/**
 * Restore FP context
 * @param save Array containing saved FP state
 */
void restore_fp_context(uint64_t *save);

/**
 * Restore full CPU context
 * @param ctx Context to restore
 * @param save_area Saved state area
 */
void restore_cpu_context_full(CPUContext *ctx, const void *save_area);

/* ============================================================================
 * Translation Infrastructure Helpers
 * ============================================================================ */

/**
 * Check if translation block is valid
 * @param guest_pc Guest PC to check
 * @return 1 if valid, 0 if not
 */
int translation_block_is_valid(uint64_t guest_pc);

/**
 * Set flag on translation block
 * @param guest_pc Guest PC of block
 * @param flag Flag to set
 */
void translation_block_set_flag(uint64_t guest_pc, uint32_t flag);

/**
 * Clear flag on translation block
 * @param guest_pc Guest PC of block
 * @param flag Flag to clear
 */
void translation_block_clear_flag(uint64_t guest_pc, uint32_t flag);

/**
 * Get translation cache size
 * @return Size of translation cache
 */
size_t translation_cache_get_size(void);

/**
 * Check if translation cache is full
 * @return true if cache is full, false otherwise
 */
bool translation_cache_is_full(void);

/**
 * Get free code cache space
 * @return Number of bytes free in code cache
 */
size_t code_cache_get_free_space(void);

/**
 * Reset code cache
 */
void code_cache_reset(void);

/* ============================================================================
 * ELF Helper Functions
 * ============================================================================ */

/**
 * Validate ELF magic number
 * @param base Base address of ELF
 * @return 1 if valid ELF magic, 0 otherwise
 */
int elf_validate_magic(const void *base);

/**
 * Get ELF entry point
 * @param base Base address of ELF
 * @return Entry point address
 */
uint64_t elf_get_entry_point(const void *base);

/**
 * Get section offset
 * @param base Base address of ELF
 * @param section_index Section index
 * @return Section offset
 */
uint64_t elf_get_section_offset(const void *base, uint32_t section_index);

/**
 * Get section size
 * @param base Base address of ELF
 * @param section_index Section index
 * @return Section size
 */
uint64_t elf_get_section_size(const void *base, uint32_t section_index);

/**
 * Check if valid 64-bit ELF
 * @param base Base address of ELF
 * @return true if ELF64, false otherwise
 */
bool elf_is_valid_class64(const void *base);

/**
 * Check if AArch64 ELF
 * @param base Base address of ELF
 * @return true if AArch64 ELF, false otherwise
 */
bool elf_is_valid_machine_aarch64(const void *base);

/* ============================================================================
 * Runtime Utility Functions
 * ============================================================================ */

/**
 * Get system page size
 * @return Page size in bytes
 */
int rosetta_getpagesize(void);

/**
 * Get system tick count
 * @return Tick count
 */
uint64_t rosetta_get_tick_count(void);

#endif /* ROSETTA_TRANS_HELPERS_H */
