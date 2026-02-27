/* ============================================================================
 * Rosetta Execution Engine Header
 * ============================================================================
 *
 * This module provides the execution engine for running translated code.
 * It handles context switching, execution of translated blocks, and
 * coordination between guest and host state.
 * ============================================================================ */

#ifndef ROSETTA_EXEC_H
#define ROSETTA_EXEC_H

#include "rosetta_types.h"
#include "rosetta_refactored.h"

/* ============================================================================
 * Execution Functions
 * ============================================================================ */

/**
 * Execute a translated block
 * @param state Thread state containing guest/host context
 * @param block Pointer to translated code block
 */
void execute_translated(ThreadState *state, void *block);

/**
 * Execute translated block with result
 * @param state Thread state
 * @param block Translated code block
 * @return Execution result (0 = success)
 */
int execute_translated_with_result(ThreadState *state, void *block);

/**
 * Set up execution context for translated code
 * @param state Thread state to set up
 */
void setup_execution_context(ThreadState *state);

/**
 * Tear down execution context after translated code
 * @param state Thread state to clean up
 */
void teardown_execution_context(ThreadState *state);

/**
 * Entry point for translated code execution
 * @param state Thread state
 * @param guest_pc Guest program counter to execute
 */
void execute_at_pc(ThreadState *state, uint64_t guest_pc);

/* ============================================================================
 * Context Switching
 * ============================================================================ */

/**
 * Save host context before executing guest code
 * @param ctx Context to save to
 */
void save_host_context(void *ctx);

/**
 * Restore host context after executing guest code
 * @param ctx Context to restore from
 */
void restore_host_context(const void *ctx);

/**
 * Switch from host to guest context
 * @param old_ctx Host context to save
 * @param new_ctx Guest context to restore
 */
void switch_to_guest(void *old_ctx, const void *new_ctx);

/**
 * Switch from guest to host context
 * @param old_ctx Guest context to save
 * @param new_ctx Host context to restore
 */
void switch_to_host(const void *old_ctx, void *new_ctx);

#endif /* ROSETTA_EXEC_H */
