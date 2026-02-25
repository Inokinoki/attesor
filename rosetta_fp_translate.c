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
        uint64_t val = state->cpu.gpr.x[rn];
        if (type == 0) {
            state->cpu.vec.v[rd].lo = val & 0xFFFFFFFFULL;
            state->cpu.vec.v[rd].hi = 0;
        } else {
            state->cpu.vec.v[rd].lo = val;
            state->cpu.vec.v[rd].hi = 0;
        }
    } else if (op == 1) {
        if (type == 0) {
            state->cpu.gpr.x[rd] = state->cpu.vec.v[rn].lo & 0xFFFFFFFFULL;
        } else {
            state->cpu.gpr.x[rd] = state->cpu.vec.v[rn].lo;
        }
    } else if (op == 2) {
        uint8_t imm8 = ((insn[0] >> 5) & 0x07) | ((insn[1] >> 1) & 0xF8);
        uint64_t val = 0;
        for (int i = 0; i < 8; i++) {
            val |= ((uint64_t)imm8) << (i * 8);
        }
        state->cpu.vec.v[rd].lo = val;
        state->cpu.vec.v[rd].hi = 0;
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
        float a = *(float*)&state->cpu.vec.v[rn].lo;
        float b = *(float*)&state->cpu.vec.v[rm].lo;
        float result = a + b;
        state->cpu.vec.v[rd].lo = *(uint32_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
    } else {
        double a = *(double*)&state->cpu.vec.v[rn].lo;
        double b = *(double*)&state->cpu.vec.v[rm].lo;
        double result = a + b;
        state->cpu.vec.v[rd].lo = *(uint64_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
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
        float a = *(float*)&state->cpu.vec.v[rn].lo;
        float b = *(float*)&state->cpu.vec.v[rm].lo;
        float result = a - b;
        state->cpu.vec.v[rd].lo = *(uint32_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
    } else {
        double a = *(double*)&state->cpu.vec.v[rn].lo;
        double b = *(double*)&state->cpu.vec.v[rm].lo;
        double result = a - b;
        state->cpu.vec.v[rd].lo = *(uint64_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
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
        float a = *(float*)&state->cpu.vec.v[rn].lo;
        float b = *(float*)&state->cpu.vec.v[rm].lo;
        float result = a * b;
        state->cpu.vec.v[rd].lo = *(uint32_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
    } else {
        double a = *(double*)&state->cpu.vec.v[rn].lo;
        double b = *(double*)&state->cpu.vec.v[rm].lo;
        double result = a * b;
        state->cpu.vec.v[rd].lo = *(uint64_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
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
        float a = *(float*)&state->cpu.vec.v[rn].lo;
        float b = *(float*)&state->cpu.vec.v[rm].lo;
        if (b == 0.0f) {
            state->cpu.vec.v[rd].lo = 0x7F800000ULL;
            state->cpu.vec.v[rd].hi = 0;
        } else {
            float result = a / b;
            state->cpu.vec.v[rd].lo = *(uint32_t*)&result;
            state->cpu.vec.v[rd].hi = 0;
        }
    } else {
        double a = *(double*)&state->cpu.vec.v[rn].lo;
        double b = *(double*)&state->cpu.vec.v[rm].lo;
        if (b == 0.0) {
            state->cpu.vec.v[rd].lo = 0x7FF0000000000000ULL;
            state->cpu.vec.v[rd].hi = 0;
        } else {
            double result = a / b;
            state->cpu.vec.v[rd].lo = *(uint64_t*)&result;
            state->cpu.vec.v[rd].hi = 0;
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
        float a = *(float*)&state->cpu.vec.v[rn].lo;
        float result = sqrtf(a);
        state->cpu.vec.v[rd].lo = *(uint32_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
    } else {
        double a = *(double*)&state->cpu.vec.v[rn].lo;
        double result = sqrt(a);
        state->cpu.vec.v[rd].lo = *(uint64_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
    }

    return 0;
}

int translate_fcmp(ThreadState *state, const uint8_t *insn)
{
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t type = (insn[0] >> 22) & 0x01;

    state->cpu.gpr.nzcv = 0;

    if (type == 0) {
        float a = *(float*)&state->cpu.vec.v[rn].lo;
        float b = *(float*)&state->cpu.vec.v[rm].lo;
        if (a < b) state->cpu.gpr.nzcv |= (1 << 31);
        else if (a > b) state->cpu.gpr.nzcv |= (1 << 29);
        else state->cpu.gpr.nzcv |= (1 << 30);
    } else {
        double a = *(double*)&state->cpu.vec.v[rn].lo;
        double b = *(double*)&state->cpu.vec.v[rm].lo;
        if (a < b) state->cpu.gpr.nzcv |= (1 << 31);
        else if (a > b) state->cpu.gpr.nzcv |= (1 << 29);
        else state->cpu.gpr.nzcv |= (1 << 30);
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
        double a = *(double*)&state->cpu.vec.v[rn].lo;
        float result = (float)a;
        state->cpu.vec.v[rd].lo = *(uint32_t*)&result;
    } else {
        /* Single to double */
        float a = *(float*)&state->cpu.vec.v[rn].lo;
        double result = (double)a;
        state->cpu.vec.v[rd].lo = *(uint64_t*)&result;
    }

    return 0;
}

int translate_fcsel(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t cond = (insn[0] >> 12) & 0x0F;

    uint8_t N = (state->cpu.gpr.nzcv >> 31) & 1;
    uint8_t Z = (state->cpu.gpr.nzcv >> 30) & 1;

    bool cond_true = (cond == 0) ? (Z == 1) : (Z == 0);

    if (cond_true) {
        state->cpu.vec.v[rd] = state->cpu.vec.v[rn];
    } else {
        state->cpu.vec.v[rd] = state->cpu.vec.v[rm];
    }

    return 0;
}

int translate_ld1(ThreadState *state, const uint8_t *insn)
{
    uint8_t vd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint64_t addr = state->cpu.gpr.x[rn];
    void *host_addr = (void *)(uintptr_t)addr;

    state->cpu.vec.v[vd].lo = *(uint64_t *)host_addr;
    state->cpu.vec.v[vd].hi = *((uint64_t *)host_addr + 1);

    return 0;
}

int translate_st1(ThreadState *state, const uint8_t *insn)
{
    uint8_t vd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint64_t addr = state->cpu.gpr.x[rn];
    void *host_addr = (void *)(uintptr_t)addr;

    *(uint64_t *)host_addr = state->cpu.vec.v[vd].lo;
    *((uint64_t *)host_addr + 1) = state->cpu.vec.v[vd].hi;

    return 0;
}

int translate_ld2(ThreadState *state, const uint8_t *insn)
{
    uint8_t vd = (insn[0] >> 0) & 0x1F;
    uint8_t vt = (insn[1] >> 0) & 0x1F;
    uint8_t rn = (insn[2] >> 16) & 0x1F;

    uint64_t addr = state->cpu.gpr.x[rn];
    void *host_addr = (void *)(uintptr_t)addr;

    state->cpu.vec.v[vd].lo = *(uint64_t *)host_addr;
    state->cpu.vec.v[vt].lo = *((uint64_t *)host_addr + 1);

    return 0;
}

int translate_st2(ThreadState *state, const uint8_t *insn)
{
    uint8_t vd = (insn[0] >> 0) & 0x1F;
    uint8_t vt = (insn[1] >> 0) & 0x1F;
    uint8_t rn = (insn[2] >> 16) & 0x1F;

    uint64_t addr = state->cpu.gpr.x[rn];
    void *host_addr = (void *)(uintptr_t)addr;

    *(uint64_t *)host_addr = state->cpu.vec.v[vd].lo;
    *((uint64_t *)host_addr + 1) = state->cpu.vec.v[vt].lo;

    return 0;
}

/* Stub implementations for remaining functions */
int translate_ld3(ThreadState *state, const uint8_t *insn) { (void)state; (void)insn; return 0; }
int translate_st3(ThreadState *state, const uint8_t *insn) { (void)state; (void)insn; return 0; }
int translate_ld4(ThreadState *state, const uint8_t *insn) { (void)state; (void)insn; return 0; }
int translate_st4(ThreadState *state, const uint8_t *insn) { (void)state; (void)insn; return 0; }
int translate_dup(ThreadState *state, const uint8_t *insn) { (void)state; (void)insn; return 0; }
int translate_ext(ThreadState *state, const uint8_t *insn) { (void)state; (void)insn; return 0; }
int translate_tbl(ThreadState *state, const uint8_t *insn) { (void)state; (void)insn; return 0; }
int translate_tbx(ThreadState *state, const uint8_t *insn) { (void)state; (void)insn; return 0; }
int translate_ushr(ThreadState *state, const uint8_t *insn) { (void)state; (void)insn; return 0; }
int translate_sshr(ThreadState *state, const uint8_t *insn) { (void)state; (void)insn; return 0; }
int translate_shl(ThreadState *state, const uint8_t *insn) { (void)state; (void)insn; return 0; }
