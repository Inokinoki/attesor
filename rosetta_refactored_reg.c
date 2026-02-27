/* ============================================================================
 * Rosetta Refactored - Register Access Implementation
 * ============================================================================
 *
 * This module implements register access functions for the Rosetta
 * translation layer.
 * ============================================================================ */

#include "rosetta_refactored_reg.h"
#include "rosetta_refactored_init.h"
#include "rosetta_refactored.h"
#include <stdio.h>

/* External reference to thread state */
extern ThreadState *rosetta_get_state(void);

/* Flag bit positions */
#define FLAG_N_BIT  31  /* Negative */
#define FLAG_Z_BIT  30  /* Zero */
#define FLAG_C_BIT  29  /* Carry */
#define FLAG_V_BIT  28  /* Overflow */

/* Flag masks */
#define FLAG_N_MASK (1ULL << FLAG_N_BIT)
#define FLAG_Z_MASK (1ULL << FLAG_Z_BIT)
#define FLAG_C_MASK (1ULL << FLAG_C_BIT)
#define FLAG_V_MASK (1ULL << FLAG_V_BIT)
#define FLAG_NZCV_MASK (FLAG_N_MASK | FLAG_Z_MASK | FLAG_C_MASK | FLAG_V_MASK)

/* ============================================================================
 * General Purpose Register Access
 * ============================================================================ */

/**
 * rosetta_get_reg - Get ARM64 register value
 * @reg: Register number (0-30)
 * Returns: Register value
 */
uint64_t rosetta_get_reg(uint8_t reg)
{
    ThreadState *state = rosetta_get_state();

    if (reg > 30) {
        return 0;
    }
    return state->guest.x[reg];
}

/**
 * rosetta_set_reg - Set ARM64 register value
 * @reg: Register number (0-30)
 * @value: New value
 */
void rosetta_set_reg(uint8_t reg, uint64_t value)
{
    ThreadState *state = rosetta_get_state();

    if (reg > 30) {
        return;
    }
    state->guest.x[reg] = value;
}

/**
 * rosetta_get_lr - Get link register (LR/X30) value
 * Returns: Link register value
 */
uint64_t rosetta_get_lr(void)
{
    ThreadState *state = rosetta_get_state();
    return state->guest.x[30];
}

/**
 * rosetta_set_lr - Set link register (LR/X30) value
 * @value: New value
 */
void rosetta_set_lr(uint64_t value)
{
    ThreadState *state = rosetta_get_state();
    state->guest.x[30] = value;
}

/* ============================================================================
 * Program Counter and Stack Pointer
 * ============================================================================ */

/**
 * rosetta_get_pc - Get current PC
 * Returns: Program counter value
 */
uint64_t rosetta_get_pc(void)
{
    ThreadState *state = rosetta_get_state();
    return state->guest.pc;
}

/**
 * rosetta_set_pc - Set current PC
 * @pc: New program counter value
 */
void rosetta_set_pc(uint64_t pc)
{
    ThreadState *state = rosetta_get_state();
    state->guest.pc = pc;
}

/**
 * rosetta_get_sp - Get current stack pointer
 * Returns: Stack pointer value
 */
uint64_t rosetta_get_sp(void)
{
    ThreadState *state = rosetta_get_state();
    return state->guest.sp;
}

/**
 * rosetta_set_sp - Set stack pointer
 * @sp: New stack pointer value
 */
void rosetta_set_sp(uint64_t sp)
{
    ThreadState *state = rosetta_get_state();
    state->guest.sp = sp;
}

/* ============================================================================
 * Flag Access
 * ============================================================================ */

/**
 * rosetta_get_flags - Get NZCV flags (PSTATE)
 * Returns: PSTATE flag value
 */
uint64_t rosetta_get_flags(void)
{
    ThreadState *state = rosetta_get_state();
    return state->guest.pstate;
}

/**
 * rosetta_set_flags - Set NZCV flags (PSTATE)
 * @flags: New flag value
 */
void rosetta_set_flags(uint64_t flags)
{
    ThreadState *state = rosetta_get_state();
    state->guest.pstate = flags;
}

/**
 * rosetta_get_n_flag - Get N (Negative) flag
 * Returns: true if set, false otherwise
 */
bool rosetta_get_n_flag(void)
{
    ThreadState *state = rosetta_get_state();
    return (state->guest.pstate & FLAG_N_MASK) != 0;
}

/**
 * rosetta_get_z_flag - Get Z (Zero) flag
 * Returns: true if set, false otherwise
 */
bool rosetta_get_z_flag(void)
{
    ThreadState *state = rosetta_get_state();
    return (state->guest.pstate & FLAG_Z_MASK) != 0;
}

/**
 * rosetta_get_c_flag - Get C (Carry) flag
 * Returns: true if set, false otherwise
 */
bool rosetta_get_c_flag(void)
{
    ThreadState *state = rosetta_get_state();
    return (state->guest.pstate & FLAG_C_MASK) != 0;
}

/**
 * rosetta_get_v_flag - Get V (Overflow) flag
 * Returns: true if set, false otherwise
 */
bool rosetta_get_v_flag(void)
{
    ThreadState *state = rosetta_get_state();
    return (state->guest.pstate & FLAG_V_MASK) != 0;
}

/**
 * rosetta_set_n_flag - Set N (Negative) flag
 * @value: Flag value
 */
void rosetta_set_n_flag(bool value)
{
    ThreadState *state = rosetta_get_state();
    if (value) {
        state->guest.pstate |= FLAG_N_MASK;
    } else {
        state->guest.pstate &= ~FLAG_N_MASK;
    }
}

/**
 * rosetta_set_z_flag - Set Z (Zero) flag
 * @value: Flag value
 */
void rosetta_set_z_flag(bool value)
{
    ThreadState *state = rosetta_get_state();
    if (value) {
        state->guest.pstate |= FLAG_Z_MASK;
    } else {
        state->guest.pstate &= ~FLAG_Z_MASK;
    }
}

/**
 * rosetta_set_c_flag - Set C (Carry) flag
 * @value: Flag value
 */
void rosetta_set_c_flag(bool value)
{
    ThreadState *state = rosetta_get_state();
    if (value) {
        state->guest.pstate |= FLAG_C_MASK;
    } else {
        state->guest.pstate &= ~FLAG_C_MASK;
    }
}

/**
 * rosetta_set_v_flag - Set V (Overflow) flag
 * @value: Flag value
 */
void rosetta_set_v_flag(bool value)
{
    ThreadState *state = rosetta_get_state();
    if (value) {
        state->guest.pstate |= FLAG_V_MASK;
    } else {
        state->guest.pstate &= ~FLAG_V_MASK;
    }
}

/**
 * rosetta_clear_flags - Clear all NZCV flags
 */
void rosetta_clear_flags(void)
{
    ThreadState *state = rosetta_get_state();
    state->guest.pstate &= ~FLAG_NZCV_MASK;
}

/* ============================================================================
 * Vector Register Access
 * ============================================================================ */

/**
 * rosetta_get_vreg - Get vector register value (full 128-bit)
 * @reg: Register number (0-31)
 * @lo: Pointer to store low 64 bits
 * @hi: Pointer to store high 64 bits
 */
void rosetta_get_vreg(uint8_t reg, uint64_t *lo, uint64_t *hi)
{
    ThreadState *state = rosetta_get_state();

    if (reg > 31 || !lo || !hi) {
        return;
    }
    *lo = state->guest.v[reg].u64[0];
    *hi = state->guest.v[reg].u64[1];
}

/**
 * rosetta_set_vreg - Set vector register value (full 128-bit)
 * @reg: Register number (0-31)
 * @lo: Low 64 bits
 * @hi: High 64 bits
 */
void rosetta_set_vreg(uint8_t reg, uint64_t lo, uint64_t hi)
{
    ThreadState *state = rosetta_get_state();

    if (reg > 31) {
        return;
    }
    state->guest.v[reg].u64[0] = lo;
    state->guest.v[reg].u64[1] = hi;
}

/**
 * rosetta_get_vreg_lo - Get vector register low 64 bits
 * @reg: Register number (0-31)
 * Returns: Low 64 bits
 */
uint64_t rosetta_get_vreg_lo(uint8_t reg)
{
    ThreadState *state = rosetta_get_state();

    if (reg > 31) {
        return 0;
    }
    return state->guest.v[reg].u64[0];
}

/**
 * rosetta_get_vreg_hi - Get vector register high 64 bits
 * @reg: Register number (0-31)
 * Returns: High 64 bits
 */
uint64_t rosetta_get_vreg_hi(uint8_t reg)
{
    ThreadState *state = rosetta_get_state();

    if (reg > 31) {
        return 0;
    }
    return state->guest.v[reg].u64[1];
}

/* ============================================================================
 * FP Register Access
 * ============================================================================ */

/**
 * rosetta_get_fpcr - Read FPCR (Floating Point Control Register)
 * Returns: FPCR value
 */
uint32_t rosetta_get_fpcr(void)
{
    ThreadState *state = rosetta_get_state();
    return state->guest.fpcr;
}

/**
 * rosetta_set_fpcr - Write FPCR (Floating Point Control Register)
 * @val: Value to write
 */
void rosetta_set_fpcr(uint32_t val)
{
    ThreadState *state = rosetta_get_state();
    state->guest.fpcr = val;
}

/**
 * rosetta_get_fpsr - Read FPSR (Floating Point Status Register)
 * Returns: FPSR value
 */
uint32_t rosetta_get_fpsr(void)
{
    ThreadState *state = rosetta_get_state();
    return state->guest.fpsr;
}

/**
 * rosetta_set_fpsr - Write FPSR (Floating Point Status Register)
 * @val: Value to write
 */
void rosetta_set_fpsr(uint32_t val)
{
    ThreadState *state = rosetta_get_state();
    state->guest.fpsr = val;
}

/* ============================================================================
 * Debug and Utility Functions
 * ============================================================================ */

/**
 * rosetta_print_state - Print current thread state (debug)
 */
void rosetta_print_state(void)
{
    ThreadState *state = rosetta_get_state();
    int i;

    printf("=== Rosetta Thread State ===\n");
    printf("PC:   0x%016llx\n", (unsigned long long)state->guest.pc);
    printf("SP:   0x%016llx\n", (unsigned long long)state->guest.sp);
    printf("LR:   0x%016llx\n", (unsigned long long)state->guest.x[30]);
    printf("PSTATE: 0x%08llx\n", (unsigned long long)state->guest.pstate);
    printf("\nGeneral Purpose Registers:\n");
    for (i = 0; i < 16; i++) {
        printf("X%-2d: 0x%016llx  X%-2d: 0x%016llx\n",
               i, (unsigned long long)state->guest.x[i],
               i + 16, (unsigned long long)state->guest.x[i + 16]);
    }
}

/**
 * rosetta_dump_regs - Print register state in formatted output
 */
void rosetta_dump_regs(void)
{
    ThreadState *state = rosetta_get_state();
    int i;

    printf("=== Register Dump ===\n");
    printf("PC   = 0x%016llx  SP   = 0x%016llx  LR   = 0x%016llx\n",
           (unsigned long long)state->guest.pc,
           (unsigned long long)state->guest.sp,
           (unsigned long long)state->guest.x[30]);
    printf("PSTATE = 0x%08llx  (N=%d Z=%d C=%d V=%d)\n",
           (unsigned long long)state->guest.pstate,
           (int)((state->guest.pstate >> 31) & 1),
           (int)((state->guest.pstate >> 30) & 1),
           (int)((state->guest.pstate >> 29) & 1),
           (int)((state->guest.pstate >> 28) & 1));
    printf("\n");

    for (i = 0; i < 30; i += 4) {
        printf("X%-2d: 0x%016llx  X%-2d: 0x%016llx  X%-2d: 0x%016llx  X%-2d: 0x%016llx\n",
               i, (unsigned long long)state->guest.x[i],
               i + 1, (unsigned long long)state->guest.x[i + 1],
               i + 2, (unsigned long long)state->guest.x[i + 2],
               i + 3, (unsigned long long)state->guest.x[i + 3]);
    }
}

/**
 * rosetta_reg_trace - Debug trace helper for register operations
 * @msg: Message to print
 * @value: Associated value
 */
void rosetta_reg_trace(const char *msg, uint64_t value)
{
    printf("[ROSETTA] %s: 0x%016llx\n", msg, (unsigned long long)value);
}
