/* ============================================================================
 * Rosetta Compatibility Layer
 * ============================================================================
 *
 * This module provides compatibility wrappers for the demo programs.
 * It bridges between the old API names used in demos and the actual
 * implementations in the refactored modules.
 * ============================================================================ */

#include "rosetta_types.h"
#include "rosetta_init.h"
#include "rosetta_hash.h"
#include <stdlib.h>
#include <sys/mman.h>

/* External function declarations */
extern void rosetta_cache_init(void);

/* ============================================================================
 * Initialization Compatibility
 * ============================================================================ */

void init_translation_env(void **entry_callback)
{
    /* Initialize translation cache */
    init_translation_cache();

    /* Return entry point callback if requested */
    if (entry_callback != NULL) {
        *entry_callback = NULL;
    }
}

/* ============================================================================
 * Memory Management Compatibility
 * ============================================================================ */

int init_memory_mappings(void)
{
    /* Memory mapping is now handled by the translation cache */
    return 0;
}

void cleanup_translation_env(void)
{
    /* Cleanup is handled by individual module cleanup functions */
    /* For now, this is a no-op */
}

/* ============================================================================
 * Translation Cache Compatibility
 * ============================================================================ */

void init_translation_cache(void)
{
    /* Translation cache is now initialized by JIT context */
    /* For the demo, we just initialize the hash table */
    rosetta_cache_init();
}

/* ============================================================================
 * FP Register Compatibility
 * ============================================================================ */

uint32_t read_fpcr(void)
{
    uint64_t val = 0;
#ifdef __linux__
    __asm__ volatile("mrs %0, fpcr" : "=r"(val));
#endif
    /* On macOS, FPCR access is restricted; return default */
    return (uint32_t)val;
}

void write_fpcr(uint32_t val)
{
#ifdef __linux__
    __asm__ volatile("msr fpcr, %0" :: "r"((uint64_t)val));
#endif
    /* On macOS, FPCR access is restricted; no-op */
}

uint32_t read_fpsr(void)
{
    uint64_t val = 0;
#ifdef __linux__
    __asm__ volatile("mrs %0, fpsr" : "=r"(val));
#endif
    /* On macOS, FPSR access is restricted; return default */
    return (uint32_t)val;
}

void write_fpsr(uint32_t val)
{
#ifdef __linux__
    __asm__ volatile("msr fpsr, %0" :: "r"((uint64_t)val));
#endif
    /* On macOS, FPSR access is restricted; no-op */
}
