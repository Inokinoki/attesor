/* ============================================================================
 * Rosetta Initialization Implementation
 * ============================================================================
 *
 * This module implements initialization and teardown functions for the
 * Rosetta binary translation environment.
 * ============================================================================ */

#include "rosetta_init.h"
#include "rosetta_refactored.h"
#include <stdint.h>
#include <string.h>

/* External function declarations */
extern uint32_t read_fpcr_internal(void);
extern void write_fpcr_internal(uint32_t val);
extern uint32_t read_fpsr_internal(void);
extern void write_fpsr_internal(uint32_t val);

/* ============================================================================
 * FP Register Access Implementation
 * ============================================================================ */

/**
 * Read FP Control Register
 * Note: FPCR is not directly accessible on macOS from user space.
 * This is a stub that returns default value.
 */
uint32_t read_fpcr(void)
{
    uint32_t val = 0;
#ifdef __linux__
    __asm__ volatile("mrs %w0, fpcr" : "=r"(val));
#endif
    /* On macOS, FPCR access is restricted; return default */
    return val;
}

/**
 * Write FP Control Register
 * Note: FPCR is not directly accessible on macOS from user space.
 * This is a stub.
 */
void write_fpcr(uint32_t val)
{
#ifdef __linux__
    __asm__ volatile("msr fpcr, %w0" :: "r"(val));
#endif
    /* On macOS, FPCR access is restricted; no-op */
}

/**
 * Read FP Status Register
 * Note: FPSR is not directly accessible on macOS from user space.
 * This is a stub that returns default value.
 */
uint32_t read_fpsr(void)
{
    uint32_t val = 0;
#ifdef __linux__
    __asm__ volatile("mrs %w0, fpsr" : "=r"(val));
#endif
    /* On macOS, FPSR access is restricted; return default */
    return val;
}

/**
 * Write FP Status Register
 * Note: FPSR is not directly accessible on macOS from user space.
 * This is a stub.
 */
void write_fpsr(uint32_t val)
{
#ifdef __linux__
    __asm__ volatile("msr fpsr, %w0" :: "r"(val));
#endif
    /* On macOS, FPSR access is restricted; no-op */
}

/**
 * Set FP registers from values
 */
void set_fp_registers(uint64_t fpcr_value, uint64_t fpsr_value)
{
    write_fpcr((uint32_t)fpcr_value);
    write_fpsr((uint32_t)fpsr_value);
}

/**
 * Clear FP registers to default values
 */
void clear_fp_registers(void)
{
    write_fpcr(0);
    write_fpsr(0);
}

/* ============================================================================
 * Initialization Implementation
 * ============================================================================ */

/**
 * Initialize translation cache
 */
void init_translation_cache(void)
{
    /* External function from rosetta_transcache.c or rosetta_jit.c */
    extern void rosetta_cache_init(void);
    rosetta_cache_init();
}

/**
 * Initialize JIT compiler
 */
int init_jit_compiler(void)
{
    /* External function from rosetta_jit.c */
    extern int jit_init(void *ctx, uint32_t cache_size);
    return jit_init(NULL, 0);
}

/**
 * Initialize memory mappings for guest code
 */
int init_memory_mappings(void)
{
    /* External function from rosetta_memmgmt.c */
    extern int memory_init(void);
    return memory_init();
}

/**
 * Initialize the translation environment
 *
 * Sets up the translation cache, JIT compiler, and memory mappings.
 */
void init_translation_env(void **entry_callback)
{
    /* Initialize translation cache */
    init_translation_cache();

    /* Set up JIT compiler */
    init_jit_compiler();

    /* Map guest memory regions */
    init_memory_mappings();

    /* Install syscall handlers */
    /* In a full implementation, this would set up syscall trampolines */

    /* Return entry point callback if requested */
    if (entry_callback != NULL) {
        /* The entry point would be the address of translated code */
        /* For now, return a placeholder */
        *entry_callback = NULL;
    }
}

/**
 * Clean up translation environment
 */
void cleanup_translation_env(void)
{
    /* External function from rosetta_memmgmt.c */
    extern void memory_cleanup(void);
    memory_cleanup();

    /* Clear FP registers */
    clear_fp_registers();
}

/* ============================================================================
 * Rosetta Entry Point Implementation
 * ============================================================================ */

/* Typedef for entry point function */
typedef void (*entry_point_t)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t,
                              uint64_t, uint64_t, uint64_t, uint64_t);

/**
 * Rosetta Entry Point
 *
 * This is the main entry point for the Rosetta translator. It:
 * 1. Parses command-line arguments from the stack
 * 2. Initializes the translation environment
 * 3. Sets up the CPU context
 * 4. Transfers control to the translated code
 */
void rosetta_entry(void)
{
    long *arg_ptr;
    long current_arg;
    long initial_sp;
    void *main_callback;
    uint64_t cpu_flags;

    /* Parse argc/argv from stack */
    arg_ptr = (long *)(&initial_sp + (initial_sp + 1) * 8);
    do {
        current_arg = *arg_ptr;
        arg_ptr = arg_ptr + 1;
    } while (current_arg != 0);

    /* Initialize translation environment */
    init_translation_env(&main_callback);

    /* Set default FP control register */
    cpu_flags = 0;

    /* Check if FZ16 (flush-to-zero for half precision) is enabled */
    if ((*(uint8_t *)(cpu_flags + 0x138) >> 1 & 1) != 0) {
        cpu_flags = 6;  /* Enable FZ16 mode */
    }

    /* Transfer control to translated entry point */
    if (main_callback != NULL) {
        ((entry_point_t)main_callback)(0, 0, 0, 0, 0, cpu_flags, 0, 0, 0);
    }
}
