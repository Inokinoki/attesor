/* ============================================================================
 * Rosetta Refactored - Register Access Header
 * ============================================================================
 *
 * This module provides register access functions for the Rosetta
 * translation layer.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_REG_H
#define ROSETTA_REFACTORED_REG_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * General Purpose Register Access
 * ============================================================================ */

/**
 * Get ARM64 register value
 * @reg: Register number (0-30)
 * Returns: Register value
 */
uint64_t rosetta_get_reg(uint8_t reg);

/**
 * Set ARM64 register value
 * @reg: Register number (0-30)
 * @value: New value
 */
void rosetta_set_reg(uint8_t reg, uint64_t value);

/**
 * Get link register (LR/X30) value
 * Returns: Link register value
 */
uint64_t rosetta_get_lr(void);

/**
 * Set link register (LR/X30) value
 * @value: New value
 */
void rosetta_set_lr(uint64_t value);

/* ============================================================================
 * Program Counter and Stack Pointer
 * ============================================================================ */

/**
 * Get current PC
 * Returns: Program counter value
 */
uint64_t rosetta_get_pc(void);

/**
 * Set current PC
 * @pc: New program counter value
 */
void rosetta_set_pc(uint64_t pc);

/**
 * Get current stack pointer
 * Returns: Stack pointer value
 */
uint64_t rosetta_get_sp(void);

/**
 * Set stack pointer
 * @sp: New stack pointer value
 */
void rosetta_set_sp(uint64_t sp);

/* ============================================================================
 * Flag Access
 * ============================================================================ */

/**
 * Get NZCV flags (PSTATE)
 * Returns: PSTATE flag value
 */
uint64_t rosetta_get_flags(void);

/**
 * Set NZCV flags (PSTATE)
 * @flags: New flag value
 */
void rosetta_set_flags(uint64_t flags);

/**
 * Get N (Negative) flag
 * Returns: true if set, false otherwise
 */
bool rosetta_get_n_flag(void);

/**
 * Get Z (Zero) flag
 * Returns: true if set, false otherwise
 */
bool rosetta_get_z_flag(void);

/**
 * Get C (Carry) flag
 * Returns: true if set, false otherwise
 */
bool rosetta_get_c_flag(void);

/**
 * Get V (Overflow) flag
 * Returns: true if set, false otherwise
 */
bool rosetta_get_v_flag(void);

/**
 * Set N (Negative) flag
 * @value: Flag value
 */
void rosetta_set_n_flag(bool value);

/**
 * Set Z (Zero) flag
 * @value: Flag value
 */
void rosetta_set_z_flag(bool value);

/**
 * Set C (Carry) flag
 * @value: Flag value
 */
void rosetta_set_c_flag(bool value);

/**
 * Set V (Overflow) flag
 * @value: Flag value
 */
void rosetta_set_v_flag(bool value);

/**
 * Clear all NZCV flags
 */
void rosetta_clear_flags(void);

/* ============================================================================
 * Vector Register Access
 * ============================================================================ */

/**
 * Get vector register value (full 128-bit)
 * @reg: Register number (0-31)
 * @lo: Pointer to store low 64 bits
 * @hi: Pointer to store high 64 bits
 */
void rosetta_get_vreg(uint8_t reg, uint64_t *lo, uint64_t *hi);

/**
 * Set vector register value (full 128-bit)
 * @reg: Register number (0-31)
 * @lo: Low 64 bits
 * @hi: High 64 bits
 */
void rosetta_set_vreg(uint8_t reg, uint64_t lo, uint64_t hi);

/**
 * Get vector register low 64 bits
 * @reg: Register number (0-31)
 * Returns: Low 64 bits
 */
uint64_t rosetta_get_vreg_lo(uint8_t reg);

/**
 * Get vector register high 64 bits
 * @reg: Register number (0-31)
 * Returns: High 64 bits
 */
uint64_t rosetta_get_vreg_hi(uint8_t reg);

/* ============================================================================
 * FP Register Access
 * ============================================================================ */

/**
 * Read FPCR (Floating Point Control Register)
 * Returns: FPCR value
 */
uint32_t rosetta_get_fpcr(void);

/**
 * Write FPCR (Floating Point Control Register)
 * @val: Value to write
 */
void rosetta_set_fpcr(uint32_t val);

/**
 * Read FPSR (Floating Point Status Register)
 * Returns: FPSR value
 */
uint32_t rosetta_get_fpsr(void);

/**
 * Write FPSR (Floating Point Status Register)
 * @val: Value to write
 */
void rosetta_set_fpsr(uint32_t val);

/* ============================================================================
 * Debug and Utility Functions
 * ============================================================================ */

/**
 * Print current thread state (debug)
 */
void rosetta_print_state(void);

/**
 * Print register state in formatted output
 * @brief Dumps all registers to stdout
 */
void rosetta_dump_regs(void);

/**
 * Debug trace helper for register operations
 * @msg: Message to print
 * @value: Associated value
 */
void rosetta_reg_trace(const char *msg, uint64_t value);

#endif /* ROSETTA_REFACTORED_REG_H */
