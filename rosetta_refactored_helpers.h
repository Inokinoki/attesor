/* ============================================================================
 * Rosetta Refactored - Helper Functions Header
 * ============================================================================
 *
 * This module provides helper utilities for hash functions, translation
 * cache management, memory management, and context operations.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_HELPERS_H
#define ROSETTA_REFACTORED_HELPERS_H

#include <stdint.h>
#include <stddef.h>
#include "rosetta_refactored.h"

/* ============================================================================
 * Hash Functions
 * ============================================================================ */

/**
 * Hash a 64-bit address for translation cache lookup
 * @param addr The address to hash
 * @return Hash value (32-bit)
 */
uint32_t hash_address(uint64_t addr);

/**
 * Hash a null-terminated string
 * @param s Null-terminated string to hash
 * @return Hash value (32-bit)
 */
uint32_t hash_string(const char *s);

/**
 * Compute hash over arbitrary data
 * @param data Pointer to data to hash
 * @param len Length of data in bytes
 * @return Hash value (32-bit)
 */
uint32_t hash_compute(const void *data, size_t len);

/* ============================================================================
 * Translation Cache
 * ============================================================================ */

/**
 * Look up a translation in the cache
 * @param guest_pc Guest ARM64 PC to look up
 * @return Host x86_64 PC if found, NULL otherwise
 */
void *translation_lookup(uint64_t guest_pc);

/**
 * Insert a translation into the cache
 * @param guest Guest ARM64 PC
 * @param host Host x86_64 PC
 * @param sz Size of translated block
 * @return 0 on success, -1 on failure
 */
int translation_insert(uint64_t guest, uint64_t host, size_t sz);

/**
 * Invalidate all translations in the cache
 */
void translation_invalidate(void);

/* ============================================================================
 * Memory Management
 * ============================================================================ */

/**
 * Initialize memory management subsystem
 * @return 0 on success, -1 on failure
 */
int memory_init(void);

/**
 * Clean up memory management subsystem
 */
void memory_cleanup(void);

/**
 * Map guest memory region
 * @param guest Guest address
 * @param size Size in bytes
 * @return Pointer to mapped memory, or NULL on failure
 */
void *memory_map_guest(uint64_t guest, uint64_t size);

/**
 * Map guest memory with specific protection
 * @param guest Guest address
 * @param size Size in bytes
 * @param prot Protection flags (PROT_READ, PROT_WRITE, PROT_EXEC)
 * @return Pointer to mapped memory, or NULL on failure
 */
void *memory_map_guest_with_prot(uint64_t guest, uint64_t size, int32_t prot);

/**
 * Unmap guest memory region
 * @param guest Guest address
 * @param size Size in bytes
 * @return 0 on success, -1 on failure
 */
int memory_unmap_guest(uint64_t guest, uint64_t size);

/**
 * Change protection on guest memory region
 * @param guest Guest address
 * @param size Size in bytes
 * @param prot Protection flags
 * @return 0 on success, -1 on failure
 */
int memory_protect_guest(uint64_t guest, uint64_t size, int32_t prot);

/**
 * Translate guest address to host address
 * @param guest Guest address
 * @return Host address, or NULL if not mapped
 */
void *memory_translate_addr(uint64_t guest);

/* ============================================================================
 * Block Helpers
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
 * Context Helpers
 * ============================================================================ */

/**
 * Perform context switch
 * @param old_ctx Old context to save
 * @param new_ctx New context to restore
 */
void helper_context_switch(CPUContext *old_ctx, CPUContext *new_ctx);

/**
 * No-op context function 1 (placeholder)
 */
void context_noop_1(void);

/**
 * No-op context function 2 (placeholder)
 */
void context_noop_2(void);

/* ============================================================================
 * FP Register Helpers
 * ============================================================================ */

/**
 * Read FPCR (Floating Point Control Register)
 * @return FPCR value
 */
uint32_t read_fpcr(void);

/**
 * Write FPCR (Floating Point Control Register)
 * @param val Value to write
 */
void write_fpcr(uint32_t val);

/**
 * Read FPSR (Floating Point Status Register)
 * @return FPSR value
 */
uint32_t read_fpsr(void);

/**
 * Write FPSR (Floating Point Status Register)
 * @param val Value to write
 */
void write_fpsr(uint32_t val);

/**
 * Set FP registers
 * @param fpcr_value FPCR value
 * @param fpsr_value FPSR value
 */
void set_fp_registers(uint64_t fpcr_value, uint64_t fpsr_value);

/**
 * Clear FP registers
 */
void clear_fp_registers(void);

/**
 * FP noop operation
 * @return Vector128 result (zero)
 */
Vector128 fp_noop(void);

#endif /* ROSETTA_REFACTORED_HELPERS_H */
