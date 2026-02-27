/* ============================================================================
 * Rosetta Refactored - Execution Header
 * ============================================================================
 *
 * This module provides execution functions for running translated code
 * in the Rosetta translation layer.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_EXEC_H
#define ROSETTA_REFACTORED_EXEC_H

#include <stdint.h>
#include "rosetta_types.h"

/* ============================================================================
 * Translation Entry Points
 * ============================================================================ */

/**
 * Translate a basic block of ARM64 instructions
 * @guest_pc: Starting program counter
 * Returns: Pointer to translated code, or NULL on error
 */
void *translate_block(uint64_t guest_pc);

/**
 * Fast path translation (lookup only)
 * @guest_pc: Guest ARM64 program counter
 * Returns: Pointer to cached translation or NULL if not found
 */
void *translate_block_fast(uint64_t guest_pc);

/**
 * Main translation entry point
 * @guest_pc: Guest ARM64 program counter
 * Returns: Pointer to translated x86_64 code
 */
void *rosetta_translate(uint64_t guest_pc);

/* ============================================================================
 * Execution Functions
 * ============================================================================ */

/**
 * Execute translated code
 * @host_code: Pointer to translated x86_64 code
 * Updates guest state after execution.
 */
void rosetta_execute(void *host_code);

/**
 * Run ARM64 code at given address
 * @guest_pc: Guest ARM64 program counter
 * Translates and executes ARM64 code.
 */
void rosetta_run(uint64_t guest_pc);

/**
 * Execute translated code with state
 * @state: Thread state
 * @block: Pointer to translated block
 */
void execute_translated(ThreadState *state, void *block);

/* ============================================================================
 * Interpreter Mode
 * ============================================================================ */

/**
 * Interpret a single ARM64 instruction
 * @insn: ARM64 instruction encoding
 * @state: Thread state to update
 * @pc: Current program counter
 * Returns: Number of bytes consumed (always 4 for ARM64)
 */
uint64_t rosetta_interpret(uint32_t insn, ThreadState *state, uint64_t pc);

/**
 * Run ARM64 code in interpreter mode
 * @guest_pc: Guest ARM64 program counter
 * @max_insns: Maximum number of instructions to execute
 * Interprets ARM64 code without translation.
 */
void rosetta_run_interpreter(uint64_t guest_pc, int max_insns);

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/**
 * Get system call handler
 * @nr: System call number
 * Returns: Pointer to syscall handler function
 */
void *get_syscall_handler(int nr);

/**
 * Handle system call
 * @state: Thread state
 * @nr: System call number
 * Returns: System call result
 */
int64_t handle_syscall(ThreadState *state, int nr);

/**
 * Helper for syscall entry
 * @state: Thread state
 * @nr: System call number
 */
void helper_syscall_enter(ThreadState *state, int nr);

/**
 * Helper for syscall exit
 * @state: Thread state
 * @result: System call result
 */
void helper_syscall_exit(ThreadState *state, int64_t result);

/**
 * Helper for interrupt handling
 * @state: Thread state
 * @vector: Interrupt vector
 */
void helper_interrupt(ThreadState *state, int vector);

#endif /* ROSETTA_REFACTORED_EXEC_H */
