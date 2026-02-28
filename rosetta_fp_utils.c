/* ============================================================================
 * Rosetta Refactored - Floating-Point Utilities Implementation
 * ============================================================================
 *
 * This module implements floating-point utility functions for the
 * Rosetta translation layer.
 * ============================================================================ */

#include "rosetta_fp_utils.h"
#include <string.h>

/* ============================================================================
 * FP Control Register Access
 *
 * Note: On x86_64, these are emulated since x86_64 doesn't have
 * direct equivalents to ARM64 FPCR/FPSR.
 * ============================================================================ */

static u32 g_fpcr = 0;  /* FP Control Register */
static u32 g_fpsr = 0;  /* FP Status Register */

u32 read_fpcr(void)
{
    return g_fpcr;
}

void write_fpcr(u32 val)
{
    g_fpcr = val;
}

u32 read_fpsr(void)
{
    return g_fpsr;
}

void write_fpsr(u32 val)
{
    g_fpsr = val;
}

/* ============================================================================
 * FP State Management
 * ============================================================================ */

void set_fp_registers(u64 fpcr_val, u64 fpsr_val)
{
    write_fpcr((u32)fpcr_val);
    write_fpsr((u32)fpsr_val);
}

vec128_t clear_fp_registers(void)
{
    write_fpcr(0);
    write_fpsr(0);
    vec128_t zero;
    zero.u64[0] = 0;
    zero.u64[1] = 0;
    return zero;
}

vec128_t fp_noop(void)
{
    vec128_t zero;
    zero.u64[0] = 0;
    zero.u64[1] = 0;
    return zero;
}

vec128_t init_fp_state(void)
{
    vec128_t state;
    state.u64[0] = 0;
    state.u64[1] = 0;
    init_fp_context(&state);
    return state;
}

void init_fp_context(vec128_t *ctx)
{
    if (ctx) {
        ctx->u64[0] = 0;
        ctx->u64[1] = 0;
    }
}

/* ============================================================================
 * FP Estimates
 *
 * These emulate ARM64 FRECPE (FP Reciprocal Estimate) and
 * FRSQRTE (FP Reciprocal Square Root Estimate) instructions.
 * ============================================================================ */

u32 fp_recip_estimate(u32 x)
{
    u32 result;
    u64 fpsr_save = read_fpsr();

    /* FRECPE instruction emulation
     * This is a simplified approximation.
     * Real implementation would use proper FP arithmetic.
     */
    result = 0x7FFFF800 - x;  /* Simplified approximation */

    write_fpsr((u32)fpsr_save);
    return result;
}

u32 fp_rsqrt_estimate(u32 x)
{
    u32 result;
    u64 fpsr_save = read_fpsr();

    /* FRSQRTE instruction emulation
     * This is a simplified approximation.
     */
    result = 0x5F800000 - (x >> 1);  /* Simplified approximation */

    write_fpsr((u32)fpsr_save);
    return result;
}

/* ============================================================================
 * Vector Register Operations
 * ============================================================================ */

vec128_t load_vector_reg(const vec128_t *src)
{
    if (!src) {
        vec128_t zero = {0, 0};
        return zero;
    }
    return *src;
}

void store_vector_reg(vec128_t *dest, vec128_t v)
{
    if (dest) {
        *dest = v;
    }
}
