/* ============================================================================
 * Rosetta Translator - Floating Point Implementation
 * ============================================================================ */

#include "rosetta_fp_translate.h"
#include <math.h>

int translate_fmov(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t op = (insn[1] >> 21) & 0x03;
    uint8_t type = (insn[0] >> 22) & 0x03;

    if (op == 0) {
        uint64_t val = state->guest.x[rn];
        if (type == 0) {
            state->guest.v[rd].u64[0] = val & 0xFFFFFFFFULL;
            state->guest.v[rd].u64[1] = 0;
        } else {
            state->guest.v[rd].u64[0] = val;
            state->guest.v[rd].u64[1] = 0;
        }
    } else if (op == 1) {
        if (type == 0) {
            state->guest.x[rd] = state->guest.v[rn].u64[0] & 0xFFFFFFFFULL;
        } else {
            state->guest.x[rd] = state->guest.v[rn].u64[0];
        }
    } else if (op == 2) {
        uint8_t imm8 = ((insn[0] >> 5) & 0x07) | ((insn[1] >> 1) & 0xF8);
        uint64_t val = 0;
        for (int i = 0; i < 8; i++) {
            val |= ((uint64_t)imm8) << (i * 8);
        }
        state->guest.v[rd].u64[0] = val;
        state->guest.v[rd].u64[1] = 0;
    }

    return 0;
}

int translate_fadd(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t type = (insn[0] >> 22) & 0x01;

    if (type == 0) {
        float a = *(float*)&state->guest.v[rn].u64[0];
        float b = *(float*)&state->guest.v[rm].u64[0];
        float result = a + b;
        state->guest.v[rd].u64[0] = *(uint32_t*)&result;
        state->guest.v[rd].u64[1] = 0;
    } else {
        double a = *(double*)&state->guest.v[rn].u64[0];
        double b = *(double*)&state->guest.v[rm].u64[0];
        double result = a + b;
        state->guest.v[rd].u64[0] = *(uint64_t*)&result;
        state->guest.v[rd].u64[1] = 0;
    }

    return 0;
}

int translate_fsub(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t type = (insn[0] >> 22) & 0x01;

    if (type == 0) {
        float a = *(float*)&state->guest.v[rn].u64[0];
        float b = *(float*)&state->guest.v[rm].u64[0];
        float result = a - b;
        state->guest.v[rd].u64[0] = *(uint32_t*)&result;
        state->guest.v[rd].u64[1] = 0;
    } else {
        double a = *(double*)&state->guest.v[rn].u64[0];
        double b = *(double*)&state->guest.v[rm].u64[0];
        double result = a - b;
        state->guest.v[rd].u64[0] = *(uint64_t*)&result;
        state->guest.v[rd].u64[1] = 0;
    }

    return 0;
}

int translate_fmul(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t type = (insn[0] >> 22) & 0x01;

    if (type == 0) {
        float a = *(float*)&state->guest.v[rn].u64[0];
        float b = *(float*)&state->guest.v[rm].u64[0];
        float result = a * b;
        state->guest.v[rd].u64[0] = *(uint32_t*)&result;
        state->guest.v[rd].u64[1] = 0;
    } else {
        double a = *(double*)&state->guest.v[rn].u64[0];
        double b = *(double*)&state->guest.v[rm].u64[0];
        double result = a * b;
        state->guest.v[rd].u64[0] = *(uint64_t*)&result;
        state->guest.v[rd].u64[1] = 0;
    }

    return 0;
}

int translate_fdiv(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t type = (insn[0] >> 22) & 0x01;

    if (type == 0) {
        float a = *(float*)&state->guest.v[rn].u64[0];
        float b = *(float*)&state->guest.v[rm].u64[0];
        if (b == 0.0f) {
            state->guest.v[rd].u64[0] = 0x7F800000ULL;
            state->guest.v[rd].u64[1] = 0;
        } else {
            float result = a / b;
            state->guest.v[rd].u64[0] = *(uint32_t*)&result;
            state->guest.v[rd].u64[1] = 0;
        }
    } else {
        double a = *(double*)&state->guest.v[rn].u64[0];
        double b = *(double*)&state->guest.v[rm].u64[0];
        if (b == 0.0) {
            state->guest.v[rd].u64[0] = 0x7FF0000000000000ULL;
            state->guest.v[rd].u64[1] = 0;
        } else {
            double result = a / b;
            state->guest.v[rd].u64[0] = *(uint64_t*)&result;
            state->guest.v[rd].u64[1] = 0;
        }
    }

    return 0;
}

int translate_fsqrt(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t type = (insn[0] >> 22) & 0x01;

    if (type == 0) {
        float a = *(float*)&state->guest.v[rn].u64[0];
        float result = sqrtf(a);
        state->guest.v[rd].u64[0] = *(uint32_t*)&result;
        state->guest.v[rd].u64[1] = 0;
    } else {
        double a = *(double*)&state->guest.v[rn].u64[0];
        double result = sqrt(a);
        state->guest.v[rd].u64[0] = *(uint64_t*)&result;
        state->guest.v[rd].u64[1] = 0;
    }

    return 0;
}

int translate_fcmp(ThreadState *state, const uint8_t *insn)
{
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t type = (insn[0] >> 22) & 0x01;

    state->guest.pstate = 0;

    if (type == 0) {
        float a = *(float*)&state->guest.v[rn].u64[0];
        float b = *(float*)&state->guest.v[rm].u64[0];
        if (a < b) state->guest.pstate |= (1 << 31);
        else if (a > b) state->guest.pstate |= (1 << 29);
        else state->guest.pstate |= (1 << 30);
    } else {
        double a = *(double*)&state->guest.v[rn].u64[0];
        double b = *(double*)&state->guest.v[rm].u64[0];
        if (a < b) state->guest.pstate |= (1 << 31);
        else if (a > b) state->guest.pstate |= (1 << 29);
        else state->guest.pstate |= (1 << 30);
    }

    return 0;
}

int translate_fcvt(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t op = (insn[0] >> 22) & 0x01;

    if (op == 0) {
        /* Double to single */
        double a = *(double*)&state->guest.v[rn].u64[0];
        float result = (float)a;
        state->guest.v[rd].u64[0] = *(uint32_t*)&result;
    } else {
        /* Single to double */
        float a = *(float*)&state->guest.v[rn].u64[0];
        double result = (double)a;
        state->guest.v[rd].u64[0] = *(uint64_t*)&result;
    }

    return 0;
}

int translate_fcsel(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t cond = (insn[0] >> 12) & 0x0F;

    uint8_t N = (state->guest.pstate >> 31) & 1;
    uint8_t Z = (state->guest.pstate >> 30) & 1;

    bool cond_true = (cond == 0) ? (Z == 1) : (Z == 0);

    if (cond_true) {
        state->guest.v[rd] = state->guest.v[rn];
    } else {
        state->guest.v[rd] = state->guest.v[rm];
    }

    return 0;
}

int translate_ld1(ThreadState *state, const uint8_t *insn)
{
    uint8_t vd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint64_t addr = state->guest.x[rn];
    void *host_addr = (void *)(uintptr_t)addr;

    state->guest.v[vd].u64[0] = *(uint64_t *)host_addr;
    state->guest.v[vd].u64[1] = *((uint64_t *)host_addr + 1);

    return 0;
}

int translate_st1(ThreadState *state, const uint8_t *insn)
{
    uint8_t vd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint64_t addr = state->guest.x[rn];
    void *host_addr = (void *)(uintptr_t)addr;

    *(uint64_t *)host_addr = state->guest.v[vd].u64[0];
    *((uint64_t *)host_addr + 1) = state->guest.v[vd].u64[1];

    return 0;
}

int translate_ld2(ThreadState *state, const uint8_t *insn)
{
    uint8_t vd = (insn[0] >> 0) & 0x1F;
    uint8_t vt = (insn[1] >> 0) & 0x1F;
    uint8_t rn = (insn[2] >> 16) & 0x1F;

    uint64_t addr = state->guest.x[rn];
    void *host_addr = (void *)(uintptr_t)addr;

    state->guest.v[vd].u64[0] = *(uint64_t *)host_addr;
    state->guest.v[vt].u64[0] = *((uint64_t *)host_addr + 1);

    return 0;
}

int translate_st2(ThreadState *state, const uint8_t *insn)
{
    uint8_t vd = (insn[0] >> 0) & 0x1F;
    uint8_t vt = (insn[1] >> 0) & 0x1F;
    uint8_t rn = (insn[2] >> 16) & 0x1F;

    uint64_t addr = state->guest.x[rn];
    void *host_addr = (void *)(uintptr_t)addr;

    *(uint64_t *)host_addr = state->guest.v[vd].u64[0];
    *((uint64_t *)host_addr + 1) = state->guest.v[vt].u64[0];

    return 0;
}

/* ============================================================================
 * NEON Multi-Structure Load/Store Operations
 * ============================================================================ */

/**
 * translate_ld3 - NEON Load Structure (3 elements)
 * Loads 3 consecutive registers from memory with interleaving
 */
int translate_ld3(ThreadState *state, const uint8_t *insn)
{
    /* Decode instruction: LD3 {Vt.8B, Vt+1.8B, Vt+2.8B}, [Xn] */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint64_t base_addr = state->guest.x[rn];

    /* Load 3 registers with de-interleaving */
    uint8_t *mem = (uint8_t *)(uintptr_t)base_addr;
    for (int i = 0; i < 16; i += 3) {
        state->guest.v[rt].u64[0]     |= ((uint64_t)mem[i]) << (8 * (i/3));
        state->guest.v[rt + 1].u64[0] |= ((uint64_t)mem[i + 1]) << (8 * (i/3));
        state->guest.v[rt + 2].u64[0] |= ((uint64_t)mem[i + 2]) << (8 * (i/3));
    }

    return 0;
}

/**
 * translate_st3 - NEON Store Structure (3 elements)
 * Stores 3 consecutive registers to memory with interleaving
 */
int translate_st3(ThreadState *state, const uint8_t *insn)
{
    /* Decode instruction: ST3 {Vt.8B, Vt+1.8B, Vt+2.8B}, [Xn] */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint64_t base_addr = state->guest.x[rn];

    /* Store 3 registers with interleaving */
    uint8_t *mem = (uint8_t *)(uintptr_t)base_addr;
    for (int i = 0; i < 16; i += 3) {
        mem[i]     = (state->guest.v[rt].u64[0]     >> (8 * (i/3))) & 0xFF;
        mem[i + 1] = (state->guest.v[rt + 1].u64[0] >> (8 * (i/3))) & 0xFF;
        mem[i + 2] = (state->guest.v[rt + 2].u64[0] >> (8 * (i/3))) & 0xFF;
    }

    return 0;
}

/**
 * translate_ld4 - NEON Load Structure (4 elements)
 * Loads 4 consecutive registers from memory with interleaving
 */
int translate_ld4(ThreadState *state, const uint8_t *insn)
{
    /* Decode instruction: LD4 {Vt.8B, Vt+1.8B, Vt+2.8B, Vt+3.8B}, [Xn] */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint64_t base_addr = state->guest.x[rn];

    /* Load 4 registers with de-interleaving */
    uint8_t *mem = (uint8_t *)(uintptr_t)base_addr;
    for (int i = 0; i < 16; i += 4) {
        state->guest.v[rt].u64[0]     |= ((uint64_t)mem[i])     << (8 * (i/4));
        state->guest.v[rt + 1].u64[0] |= ((uint64_t)mem[i + 1]) << (8 * (i/4));
        state->guest.v[rt + 2].u64[0] |= ((uint64_t)mem[i + 2]) << (8 * (i/4));
        state->guest.v[rt + 3].u64[0] |= ((uint64_t)mem[i + 3]) << (8 * (i/4));
    }

    return 0;
}

/**
 * translate_st4 - NEON Store Structure (4 elements)
 * Stores 4 consecutive registers to memory with interleaving
 */
int translate_st4(ThreadState *state, const uint8_t *insn)
{
    /* Decode instruction: ST4 {Vt.8B, Vt+1.8B, Vt+2.8B, Vt+3.8B}, [Xn] */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint64_t base_addr = state->guest.x[rn];

    /* Store 4 registers with interleaving */
    uint8_t *mem = (uint8_t *)(uintptr_t)base_addr;
    for (int i = 0; i < 16; i += 4) {
        mem[i]     = (state->guest.v[rt].u64[0]     >> (8 * (i/4))) & 0xFF;
        mem[i + 1] = (state->guest.v[rt + 1].u64[0] >> (8 * (i/4))) & 0xFF;
        mem[i + 2] = (state->guest.v[rt + 2].u64[0] >> (8 * (i/4))) & 0xFF;
        mem[i + 3] = (state->guest.v[rt + 3].u64[0] >> (8 * (i/4))) & 0xFF;
    }

    return 0;
}

/* ============================================================================
 * NEON Vector Manipulation Operations
 * ============================================================================ */

/**
 * translate_dup - NEON Duplicate
 * Duplicates a general-purpose register to a SIMD vector
 */
int translate_dup(ThreadState *state, const uint8_t *insn)
{
    /* Decode instruction: DUP Vd.2D, Xn */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    /* Duplicate 64-bit value to both lanes */
    uint64_t val = state->guest.x[rn];
    state->guest.v[rd].u64[0] = val;
    state->guest.v[rd].u64[1] = val;

    return 0;
}

/**
 * translate_ext - NEON Extract
 * Extracts elements from two registers
 */
int translate_ext(ThreadState *state, const uint8_t *insn)
{
    /* Decode instruction: EXT Vd.16B, Vn.16B, Vm.16B, #imm */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t imm = (insn[3] >> 4) & 0x0F;

    vec128_t *vn = &state->guest.v[rn];
    vec128_t *vm = &state->guest.v[rm];
    vec128_t *vd = &state->guest.v[rd];

    /* Extract bytes from position imm */
    uint8_t *src = (uint8_t *)vn;
    uint8_t *src2 = (uint8_t *)vm;
    uint8_t *dst = (uint8_t *)vd;

    for (int i = 0; i < 16; i++) {
        int idx = imm + i;
        if (idx < 16) {
            dst[i] = src[idx];
        } else {
            dst[i] = src2[idx - 16];
        }
    }

    return 0;
}

/**
 * translate_tbl - NEON Table Lookup
 * Performs table lookup using index vector
 */
int translate_tbl(ThreadState *state, const uint8_t *insn)
{
    /* Decode instruction: TBL Vd.16B, {Vn.16B}, Vm.16B */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    vec128_t *table = &state->guest.v[rn];
    vec128_t *index = &state->guest.v[rm];
    vec128_t *result = &state->guest.v[rd];

    uint8_t *tbl = (uint8_t *)table;
    uint8_t *idx = (uint8_t *)index;
    uint8_t *res = (uint8_t *)result;

    /* Table lookup with zero for out-of-range indices */
    for (int i = 0; i < 16; i++) {
        if (idx[i] < 16) {
            res[i] = tbl[idx[i]];
        } else {
            res[i] = 0;
        }
    }

    return 0;
}

/**
 * translate_tbx - NEON Table Lookup Extension
 * Performs table lookup, preserving original values for out-of-range
 */
int translate_tbx(ThreadState *state, const uint8_t *insn)
{
    /* Decode instruction: TBX Vd.16B, {Vn.16B}, Vm.16B */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    vec128_t *table = &state->guest.v[rn];
    vec128_t *index = &state->guest.v[rm];
    vec128_t *result = &state->guest.v[rd];

    uint8_t *tbl = (uint8_t *)table;
    uint8_t *idx = (uint8_t *)index;
    uint8_t *res = (uint8_t *)result;
    uint8_t *orig = (uint8_t *)result;

    /* Table lookup preserving original for out-of-range indices */
    for (int i = 0; i < 16; i++) {
        if (idx[i] < 16) {
            res[i] = tbl[idx[i]];
        }
        /* else keep original value */
    }

    return 0;
}

/* ============================================================================
 * NEON Shift Operations
 * ============================================================================ */

/**
 * translate_ushr - NEON Unsigned Shift Right
 * Shifts each element right by immediate amount (unsigned)
 */
int translate_ushr(ThreadState *state, const uint8_t *insn)
{
    /* Decode instruction: USHR Vd.2D, Vn.2D, #imm */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t shift = 64 - ((insn[3] >> 2) & 0x3F);

    uint64_t *src_lo = (uint64_t *)&state->guest.v[rn].u64[0];
    uint64_t *src_hi = (uint64_t *)&state->guest.v[rn].u64[1];
    uint64_t *dst_lo = (uint64_t *)&state->guest.v[rd].u64[0];
    uint64_t *dst_hi = (uint64_t *)&state->guest.v[rd].u64[1];

    /* Unsigned shift right */
    *dst_lo = *src_lo >> shift;
    *dst_hi = *src_hi >> shift;

    return 0;
}

/**
 * translate_sshr - NEON Signed Shift Right
 * Shifts each element right by immediate amount (signed)
 */
int translate_sshr(ThreadState *state, const uint8_t *insn)
{
    /* Decode instruction: SSHR Vd.2D, Vn.2D, #imm */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t shift = 64 - ((insn[3] >> 2) & 0x3F);

    int64_t *src_lo = (int64_t *)&state->guest.v[rn].u64[0];
    int64_t *src_hi = (int64_t *)&state->guest.v[rn].u64[1];
    int64_t *dst_lo = (int64_t *)&state->guest.v[rd].u64[0];
    int64_t *dst_hi = (int64_t *)&state->guest.v[rd].u64[1];

    /* Signed shift right (arithmetic) */
    *dst_lo = *src_lo >> shift;
    *dst_hi = *src_hi >> shift;

    return 0;
}

/**
 * translate_shl - NEON Shift Left
 * Shifts each element left by immediate amount
 */
int translate_shl(ThreadState *state, const uint8_t *insn)
{
    /* Decode instruction: SHL Vd.2D, Vn.2D, #imm */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t shift = ((insn[3] >> 2) & 0x3F) - 64;

    uint64_t *src_lo = (uint64_t *)&state->guest.v[rn].u64[0];
    uint64_t *src_hi = (uint64_t *)&state->guest.v[rn].u64[1];
    uint64_t *dst_lo = (uint64_t *)&state->guest.v[rd].u64[0];
    uint64_t *dst_hi = (uint64_t *)&state->guest.v[rd].u64[1];

    /* Shift left */
    *dst_lo = *src_lo << shift;
    *dst_hi = *src_hi << shift;

    return 0;
}
