/* ============================================================================
 * Rosetta Refactored - CPU Context Management Implementation
 * ============================================================================
 *
 * This module implements CPU context save/restore functions for the
 * Rosetta translation layer.
 * ============================================================================ */

#include "rosetta_context_utils.h"
#include "rosetta_fp_utils.h"
#include <string.h>

/* ============================================================================
 * Context Save/Restore - Full Context
 *
 * Context Layout (400 bytes total):
 * - Offset 0-3:   FPSR (4 bytes)
 * - Offset 4-7:   FPCR (4 bytes)
 * - Offset 8-519: V0-V31 (32 registers * 16 bytes = 512 bytes)
 * - Offset 520-759: X0-X29 (30 registers * 8 bytes = 240 bytes)
 * - Offset 760-767: FP (8 bytes)
 * - Offset 768-775: LR (8 bytes)
 * - Offset 776-783: SP (8 bytes)
 * - Offset 784-791: PC (8 bytes)
 * ============================================================================ */

void save_cpu_context_full(cpu_ctx_t *ctx, void *save_area, long flags)
{
    u64 *save = (u64 *)save_area;
    (void)flags;

    if (!ctx || !save_area) {
        return;
    }

    /* Save FP status and control registers */
    ((u32 *)(save_area + 8))[0] = read_fpsr();
    ((u32 *)(save_area + 0xc))[0] = read_fpcr();

    /* Save vector registers V0-V31 */
    for (int i = 0; i < 32; i++) {
        save[2 + i * 2] = ctx->vec.v[i].u64[0];
        save[3 + i * 2] = ctx->vec.v[i].u64[1];
    }

    /* Save general purpose registers X0-X29 */
    for (int i = 0; i < 30; i++) {
        save[66 + i] = ctx->gpr.x[i];
    }

    /* Save FP, LR, SP, PC */
    save[96] = ctx->gpr.fp;
    save[97] = ctx->gpr.lr;
    save[98] = ctx->gpr.sp;
    save[99] = ctx->pc;
}

void restore_cpu_context_full(cpu_ctx_t *ctx, const void *save_area, long flags)
{
    const u64 *save = (const u64 *)save_area;
    (void)flags;

    if (!ctx || !save_area) {
        return;
    }

    /* Restore FP status and control registers */
    write_fpsr(((const u32 *)(save_area + 8))[0]);
    write_fpcr(((const u32 *)(save_area + 0xc))[0]);

    /* Restore vector registers V0-V31 */
    for (int i = 0; i < 32; i++) {
        ctx->vec.v[i].u64[0] = save[2 + i * 2];
        ctx->vec.v[i].u64[1] = save[3 + i * 2];
    }

    /* Restore general purpose registers X0-X29 */
    for (int i = 0; i < 30; i++) {
        ctx->gpr.x[i] = save[66 + i];
    }

    /* Restore FP, LR, SP, PC */
    ctx->gpr.fp = save[96];
    ctx->gpr.lr = save[97];
    ctx->gpr.sp = save[98];
    ctx->pc = save[99];
}

/* ============================================================================
 * Context Save/Restore - Minimal Context
 * ============================================================================ */

void save_cpu_context_minimal(cpu_ctx_t *ctx, void *save_area)
{
    u64 *save = (u64 *)save_area;

    if (!ctx || !save_area) {
        return;
    }

    /* Save only GPRs and PC */
    for (int i = 0; i < 30; i++) {
        save[i] = ctx->gpr.x[i];
    }
    save[30] = ctx->gpr.fp;
    save[31] = ctx->gpr.lr;
    save[32] = ctx->gpr.sp;
    save[33] = ctx->pc;
}

void restore_cpu_context_minimal(cpu_ctx_t *ctx, const void *save_area)
{
    const u64 *save = (const u64 *)save_area;

    if (!ctx || !save_area) {
        return;
    }

    /* Restore only GPRs and PC */
    for (int i = 0; i < 30; i++) {
        ctx->gpr.x[i] = save[i];
    }
    ctx->gpr.fp = save[30];
    ctx->gpr.lr = save[31];
    ctx->gpr.sp = save[32];
    ctx->pc = save[33];
}

/* ============================================================================
 * Context Stubs
 * ============================================================================ */

void context_stub_1(void)
{
    /* Stub function for compatibility */
}

void context_stub_2(void)
{
    /* Stub function for compatibility */
}
