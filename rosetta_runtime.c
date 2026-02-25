/* ============================================================================
 * Rosetta Translator - Runtime and Entry Point
 * ============================================================================
 *
 * This module provides the runtime environment including:
 * - Entry point initialization
 * - FP register management
 * - CPU context save/restore
 * ============================================================================ */

#include "rosetta_refactored_types.h"
#include <stdint.h>

/* External function declarations */
extern uint32_t read_fpcr(void);
extern void write_fpcr(uint32_t val);
extern uint32_t read_fpsr(void);
extern void write_fpsr(uint32_t val);
extern void init_translation_env(void **entry_callback);

typedef void (*entry_point_t)(long, long, long, long, long,
                               uint64_t, long, long, long);

/* ============================================================================
 * Rosetta Entry Point
 * ============================================================================ */

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
    if ((*(byte *)(cpu_flags + 0x138) >> 1 & 1) != 0) {
        cpu_flags = 6;  /* Enable FZ16 mode */
    }

    /* Transfer control to translated entry point */
    ((entry_point_t)main_callback)(0, 0, 0, 0, 0, cpu_flags, 0, 0, 0);
}

/* ============================================================================
 * FP Context Management
 * ============================================================================ */

/**
 * Load vector register from memory
 *
 * @param ctx Pointer to 16-byte register data
 * @return Vector128 value
 */
Vector128 load_vector_register(const Vector128 *ctx)
{
    return *ctx;
}

/**
 * Set FP control and status registers
 *
 * @param fpcr_value New FPCR value
 * @param fpsr_value New FPSR value
 */
void set_fp_registers(uint64_t fpcr_value, uint64_t fpsr_value)
{
    write_fpcr((uint32_t)fpcr_value);
    write_fpsr((uint32_t)fpsr_value);
}

/**
 * Clear FP control and status registers
 */
void clear_fp_registers(void)
{
    write_fpcr(0);
    write_fpsr(0);
}

/**
 * No-operation FP operation
 * Returns zero vector
 */
Vector128 fp_noop(void)
{
    return (Vector128){0, 0};
}

/* ============================================================================
 * CPU Context Save/Restore
 * ============================================================================ */

/**
 * Save CPU context to memory structure
 *
 * Saves all general purpose registers, vector registers,
 * and FP control registers to a contiguous memory area.
 *
 * @param ctx Source CPU context
 * @param save_area Destination memory area (0x210 bytes)
 * @param flags Save flags
 */
void save_cpu_context_full(CPUContext *ctx, void *save_area, long flags)
{
    uint64_t *save = (uint64_t *)save_area;
    uint32_t fpsr_val;
    uint32_t fpcr_val;

    (void)flags;  /* Unused for now */

    /* Save FP status */
    fpsr_val = read_fpsr();
    ((uint32_t *)(save_area + 8))[0] = fpsr_val;

    /* Save FP control */
    fpcr_val = read_fpcr();
    ((uint32_t *)(save_area + 0xc))[0] = fpcr_val;

    /* Save vector registers V0-V31 */
    for (int i = 0; i < 32; i++) {
        save[2 + i * 2] = ctx->vec.v[i].lo;
        save[3 + i * 2] = ctx->vec.v[i].hi;
    }

    /* Save general purpose registers */
    for (int i = 0; i < 30; i++) {
        save[66 + i] = ctx->gpr.x[i];
    }
    save[96] = ctx->gpr.lr;
    save[97] = ctx->gpr.sp;
    save[98] = ctx->gpr.pc;
    save[99] = ctx->gpr.nzcv;
}

/**
 * Restore CPU context from memory structure
 *
 * @param ctx Destination CPU context
 * @param save_area Source memory area
 */
void restore_cpu_context_full(CPUContext *ctx, const void *save_area)
{
    const uint64_t *save = (const uint64_t *)save_area;

    /* Restore vector registers V0-V31 */
    for (int i = 0; i < 32; i++) {
        ctx->vec.v[i].lo = save[2 + i * 2];
        ctx->vec.v[i].hi = save[3 + i * 2];
    }

    /* Restore general purpose registers */
    for (int i = 0; i < 30; i++) {
        ctx->gpr.x[i] = save[66 + i];
    }
    ctx->gpr.lr = save[96];
    ctx->gpr.sp = save[97];
    ctx->gpr.pc = save[98];
    ctx->gpr.nzcv = save[99];
}
