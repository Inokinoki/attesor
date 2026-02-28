/* ============================================================================
 * Rosetta Refactored - CPU Context Management Header
 * ============================================================================
 *
 * This header declares CPU context save/restore functions for the
 * Rosetta translation layer.
 * ============================================================================ */

#ifndef ROSETTA_CONTEXT_UTILS_H
#define ROSETTA_CONTEXT_UTILS_H

#include "rosetta_types.h"
#include <stdint.h>

/* ============================================================================
 * CPU Context Structure (local definition to avoid redefinition)
 * ============================================================================ */

/* GPR context - General Purpose Registers */
typedef struct {
    u64 x[30];  /* X0-X29 */
    u64 fp;     /* X29 - Frame Pointer */
    u64 lr;     /* X30 - Link Register */
    u64 sp;     /* Stack Pointer */
} gpr_ctx_t;

/* Vector context - NEON/FP registers */
typedef struct {
    vec128_t v[32];
} vec_ctx_t;

/* FP context - Floating point control registers */
typedef struct {
    u32 fpsr;
    u32 fpcr;
    u32 pad[14];
} fp_ctx_t;

/* Full CPU Context */
typedef struct {
    gpr_ctx_t   gpr;      /* General purpose registers */
    vec_ctx_t   vec;      /* Vector/NEON registers */
    fp_ctx_t    fp;       /* Floating point control */
    u64         pc;       /* Program counter */
    u64         pstate;   /* Processor state */
} cpu_ctx_t;

/* ============================================================================
 * Context Save/Restore
 * ============================================================================ */

/**
 * save_cpu_context_full - Save complete CPU context to memory
 * @ctx: CPU context to save
 * @save_area: Destination buffer (must be at least 400 bytes)
 * @flags: Save options (reserved)
 */
void save_cpu_context_full(cpu_ctx_t *ctx, void *save_area, long flags);

/**
 * restore_cpu_context_full - Restore complete CPU context from memory
 * @ctx: CPU context to restore
 * @save_area: Source buffer
 * @flags: Restore options (reserved)
 */
void restore_cpu_context_full(cpu_ctx_t *ctx, const void *save_area, long flags);

/**
 * save_cpu_context_minimal - Save minimal CPU context (GPRs + PC)
 * @ctx: CPU context to save
 * @save_area: Destination buffer
 */
void save_cpu_context_minimal(cpu_ctx_t *ctx, void *save_area);

/**
 * restore_cpu_context_minimal - Restore minimal CPU context
 * @ctx: CPU context to restore
 * @save_area: Source buffer
 */
void restore_cpu_context_minimal(cpu_ctx_t *ctx, const void *save_area);

/* ============================================================================
 * Context Stubs (for compatibility)
 * ============================================================================ */

void context_stub_1(void);
void context_stub_2(void);

/* ============================================================================
 * Context Size Constants
 * ============================================================================ */

#define CPU_CONTEXT_FULL_SIZE       400
#define CPU_CONTEXT_MINIMAL_SIZE    256

#endif /* ROSETTA_CONTEXT_UTILS_H */
