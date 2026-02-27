/* ============================================================================
 * Rosetta Refactored - Helper Functions Implementation
 * ============================================================================
 *
 * This module implements helper utilities for hash functions, translation
 * cache management, memory management, and context operations.
 * ============================================================================ */

#include "rosetta_refactored_helpers.h"
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>

/* ============================================================================
 * Global State
 * ============================================================================ */

/* Guest memory mapping state */
static void *guest_memory_base = NULL;
static uint64_t guest_memory_size = 0;

/* Translation cache configuration */
#ifndef TRANSLATION_CACHE_SIZE
#define TRANSLATION_CACHE_SIZE  4096
#define TRANSLATION_CACHE_MASK  (TRANSLATION_CACHE_SIZE - 1)
#endif

/* Static translation cache */
static TranslationCacheEntry translation_cache[TRANSLATION_CACHE_SIZE];
static uint32_t cache_insert_index = 0;

/* ============================================================================
 * Hash Functions
 * ============================================================================ */

uint32_t hash_address(uint64_t addr)
{
    /* Golden ratio multiplicative hash */
    uint64_t hash = addr * 2654435761ULL;
    return (uint32_t)(hash >> 32);
}

uint32_t hash_string(const char *s)
{
    uint32_t hash = 5381;
    int c;

    while ((c = *s++) != '\0') {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

uint32_t hash_compute(const void *data, size_t len)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t hash = 0;
    size_t i;

    for (i = 0; i < len; i++) {
        hash = hash * 31 + bytes[i];
    }

    return hash;
}

/* ============================================================================
 * Translation Cache
 * ============================================================================ */

void *translation_lookup(uint64_t guest_pc)
{
    uint32_t hash = hash_address(guest_pc);
    uint32_t index = hash & TRANSLATION_CACHE_MASK;

    /* Check cache entry */
    if (translation_cache[index].guest_addr == guest_pc &&
        translation_cache[index].host_addr != 0) {
        translation_cache[index].refcount++;
        return (void *)translation_cache[index].host_addr;
    }

    return NULL;
}

int translation_insert(uint64_t guest, uint64_t host, size_t sz)
{
    uint32_t hash = hash_address(guest);
    uint32_t index = hash & TRANSLATION_CACHE_MASK;

    /* Insert into cache (simple direct-mapped cache) */
    translation_cache[index].guest_addr = guest;
    translation_cache[index].host_addr = host;
    translation_cache[index].hash = hash;
    translation_cache[index].refcount = 1;
    (void)sz;  /* Size stored for future use */

    return 0;
}

void translation_invalidate(void)
{
    for (uint32_t i = 0; i < TRANSLATION_CACHE_SIZE; i++) {
        translation_cache[i].guest_addr = 0;
        translation_cache[i].host_addr = 0;
        translation_cache[i].hash = 0;
        translation_cache[i].refcount = 0;
    }
    cache_insert_index = 0;
}

/* ============================================================================
 * Memory Management
 * ============================================================================ */

int memory_init(void)
{
    guest_memory_base = NULL;
    guest_memory_size = 0;
    return 0;
}

void memory_cleanup(void)
{
    if (guest_memory_base != NULL) {
        munmap(guest_memory_base, guest_memory_size);
        guest_memory_base = NULL;
        guest_memory_size = 0;
    }
}

void *memory_map_guest(uint64_t guest, uint64_t size)
{
    void *ret = mmap((void *)guest, size, PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (ret == MAP_FAILED) {
        return NULL;
    }
    if (guest_memory_base == NULL) {
        guest_memory_base = ret;
        guest_memory_size = size;
    }
    return ret;
}

void *memory_map_guest_with_prot(uint64_t guest, uint64_t size, int32_t prot)
{
    void *ret = mmap((void *)guest, size, prot,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (ret == MAP_FAILED) {
        return NULL;
    }
    if (guest_memory_base == NULL) {
        guest_memory_base = ret;
        guest_memory_size = size;
    }
    return ret;
}

int memory_unmap_guest(uint64_t guest, uint64_t size)
{
    int ret = munmap((void *)guest, size);
    if (ret < 0) {
        return -1;
    }
    if ((uint64_t)guest == (uint64_t)guest_memory_base) {
        guest_memory_base = NULL;
        guest_memory_size = 0;
    }
    return 0;
}

int memory_protect_guest(uint64_t guest, uint64_t size, int32_t prot)
{
    int ret = mprotect((void *)guest, size, prot);
    if (ret < 0) {
        return -1;
    }
    return 0;
}

void *memory_translate_addr(uint64_t guest)
{
    /* Simple identity mapping for now */
    if (guest_memory_base != NULL &&
        guest >= (uint64_t)guest_memory_base &&
        guest < (uint64_t)guest_memory_base + guest_memory_size) {
        return (void *)guest;
    }
    return NULL;
}

/* ============================================================================
 * Block Helpers
 * ============================================================================ */

void *helper_block_translate(uint64_t guest_pc)
{
    /* Look up in cache first */
    void *cached = translation_lookup(guest_pc);
    if (cached != NULL) {
        return cached;
    }

    /* Would translate block here */
    return NULL;
}

int helper_block_insert(uint64_t guest, uint64_t host, size_t size)
{
    return translation_insert(guest, host, size);
}

void *helper_block_lookup(uint64_t guest_pc)
{
    return translation_lookup(guest_pc);
}

int helper_block_remove(uint64_t guest_pc)
{
    /* Hash the guest PC to find cache entry */
    uint32_t hash = hash_address(guest_pc);
    uint32_t index = hash & (TRANSLATION_CACHE_SIZE - 1);

    /* Check if entry exists and remove it */
    if (translation_cache[index].guest_addr == guest_pc) {
        translation_cache[index].guest_addr = 0;
        translation_cache[index].host_addr = 0;
        translation_cache[index].refcount = 0;
        return 0;
    }

    return -1;  /* Entry not found */
}

void helper_block_invalidate(void)
{
    translation_invalidate();
}

/* ============================================================================
 * Context Helpers
 * ============================================================================ */

void helper_context_switch(CPUContext *old_ctx, CPUContext *new_ctx)
{
    if (old_ctx != NULL) {
        /* Save current context */
        /* In a full implementation, this would save the current CPU state */
    }
    if (new_ctx != NULL) {
        /* Restore new context */
        /* In a full implementation, this would restore the saved CPU state */
        restore_cpu_context_full(new_ctx, new_ctx);
    }
}

void context_noop_1(void)
{
    /* No-op - placeholder for future implementation */
}

void context_noop_2(void)
{
    /* No-op - placeholder for future implementation */
}

/* ============================================================================
 * FP Register Helpers
 * ============================================================================ */

uint32_t read_fpcr(void)
{
    uint32_t val = 0;
#ifdef __linux__
    __asm__ volatile("mrs %w0, fpcr" : "=r"(val));
#endif
    /* On macOS, FPCR access is restricted; return default */
    return val;
}

void write_fpcr(uint32_t val)
{
#ifdef __linux__
    __asm__ volatile("msr fpcr, %w0" :: "r"(val));
#endif
    /* On macOS, FPCR access is restricted; no-op */
}

uint32_t read_fpsr(void)
{
    uint32_t val = 0;
#ifdef __linux__
    __asm__ volatile("mrs %w0, fpsr" : "=r"(val));
#endif
    /* On macOS, FPSR access is restricted; return default */
    return val;
}

void write_fpsr(uint32_t val)
{
#ifdef __linux__
    __asm__ volatile("msr fpsr, %w0" :: "r"(val));
#endif
    /* On macOS, FPSR access is restricted; no-op */
}

void set_fp_registers(uint64_t fpcr_value, uint64_t fpsr_value)
{
    /* Set FPCr and FPSr values */
    /* This would typically be done via assembly instructions */
    (void)fpcr_value;
    (void)fpsr_value;
}

void clear_fp_registers(void)
{
    /* Clear floating point control and status registers */
    /* This would typically be done via assembly instructions */
}

Vector128 fp_noop(void)
{
    /* FP noop operation - returns zero vector */
    Vector128 result;
    result.lo = 0;
    result.hi = 0;
    return result;
}
