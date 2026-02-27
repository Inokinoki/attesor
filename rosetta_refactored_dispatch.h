/* ============================================================================
 * Rosetta Refactored - Instruction Dispatch Header
 * ============================================================================
 *
 * This module provides instruction dispatch functions for the Rosetta
 * translation layer, routing ARM64 instructions to appropriate translators.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_DISPATCH_H
#define ROSETTA_REFACTORED_DISPATCH_H

#include <stdint.h>
#include <stdbool.h>
#include "rosetta_types.h"
#include "rosetta_emit_x86.h"

/* ============================================================================
 * Instruction Dispatch
 * ============================================================================ */

/**
 * rosetta_decode_and_dispatch - Decode ARM64 instruction and dispatch to handler
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @state: Guest thread state
 * @pc: Current program counter
 * @terminated: Output flag for block termination
 * Returns: 0 on success, -1 on unknown instruction
 */
int rosetta_decode_and_dispatch(uint32_t encoding, code_buf_t *code_buf,
                                 ThreadState *state, uint64_t pc, int *terminated);

/**
 * rosetta_translate_instruction - Translate single instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @state: Guest thread state
 * @pc: Current program counter
 * Returns: 0 on success, -1 on unknown instruction
 */
int rosetta_translate_instruction(uint32_t encoding, code_buf_t *code_buf,
                                   ThreadState *state, uint64_t pc);

/**
 * rosetta_dispatch_alu - Dispatch ALU instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer
 * @regs: Guest registers
 * Returns: 0 on success, -1 on unknown instruction
 */
int rosetta_dispatch_alu(uint32_t encoding, code_buf_t *code_buf, uint64_t *regs);

/**
 * rosetta_dispatch_memory - Dispatch memory instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer
 * @regs: Guest registers
 * Returns: 0 on success, -1 on unknown instruction
 */
int rosetta_dispatch_memory(uint32_t encoding, code_buf_t *code_buf, uint64_t *regs);

/**
 * rosetta_dispatch_branch - Dispatch branch instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer
 * @regs: Guest registers
 * @pc: Current program counter
 * @terminated: Output flag for block termination
 * Returns: 0 on success, -1 on unknown instruction
 */
int rosetta_dispatch_branch(uint32_t encoding, code_buf_t *code_buf,
                             uint64_t *regs, uint64_t pc, int *terminated);

/**
 * rosetta_dispatch_system - Dispatch system instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer
 * @regs: Guest registers
 * Returns: 0 on success, -1 on unknown instruction
 */
int rosetta_dispatch_system(uint32_t encoding, code_buf_t *code_buf, uint64_t *regs);

/* ============================================================================
 * Module Registration (for future extensibility)
 * ============================================================================ */

/* Translation module descriptor */
typedef struct {
    const char *name;
    int (*init)(void);
    void (*cleanup)(void);
    void *dispatch;  /* Generic dispatch function pointer */
} rosetta_module_t;

/**
 * rosetta_register_module - Register a translation module
 * @module: Module descriptor
 * Returns: 0 on success, -1 on error
 */
int rosetta_register_module(rosetta_module_t *module);

/**
 * rosetta_init_all_modules - Initialize all registered modules
 * Returns: 0 on success, -1 on error
 */
int rosetta_init_all_modules(void);

/**
 * rosetta_cleanup_all_modules - Cleanup all registered modules
 */
void rosetta_cleanup_all_modules(void);

#endif /* ROSETTA_REFACTORED_DISPATCH_H */
