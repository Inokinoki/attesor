/* ============================================================================
 * Rosetta CPU Context Module
 * ============================================================================
 *
 * This module handles CPU context save/restore operations including
 * general purpose registers, vector registers, and FP control registers.
 * ============================================================================ */

#include "rosetta_context.h"
#include <string.h>

/* Forward declarations for FP register access - defined in rosetta_refactored.c */
extern uint32_t read_fpsr(void);
extern uint32_t read_fpcr(void);
extern void write_fpsr(uint32_t val);
extern void write_fpcr(uint32_t val);

/* ============================================================================
 * Context Save/Restore Functions
 * ============================================================================ */

void save_cpu_context_full(CPUContext *ctx, void *save_area, long flags)
{
    uint64_t *save = (uint64_t *)save_area;
    uint32_t fpsr_val;
    uint32_t fpcr_val;

    (void)flags;  /* Flags currently unused */

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

void save_fp_context(uint64_t *save)
{
    /* Save all 32 SIMD/FP registers (V0-V31) */
    /* Each register is 128-bit = 2 x 64-bit */
    /* Note: In a full implementation, this would read actual V registers */
    memset(save, 0, 64 * sizeof(uint64_t));

    /* Save FPSR and FPCR */
    save[64] = read_fpsr();
    save[65] = read_fpcr();
}

void restore_fp_context(uint64_t *save)
{
    /* Restore all 32 SIMD/FP registers (V0-V31) */
    /* Each register is 128-bit = 2 x 64-bit */
    /* Note: In a full implementation, this would write to actual V registers */
    (void)save;

    /* Restore FPSR and FPCR */
    write_fpsr((uint32_t)save[64]);
    write_fpcr((uint32_t)save[65]);
}

/* ============================================================================
 * FP Context Management
 * ============================================================================ */

Vector128 load_vector_register(const Vector128 *ctx)
{
    return *ctx;
}

void set_fp_registers(uint64_t fpcr_value, uint64_t fpsr_value)
{
    write_fpcr(fpcr_value);
    write_fpsr(fpsr_value);
}

void clear_fp_registers(void)
{
    write_fpcr(0);
    write_fpsr(0);
}

Vector128 fp_noop(void)
{
    return (Vector128){0, 0};
}

/* End of rosetta_context.c */
