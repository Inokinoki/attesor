/* ============================================================================
 * Rosetta Refactored - Initialization Header
 * ============================================================================
 *
 * This module provides initialization and cleanup functions for the
 * Rosetta translation layer.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_INIT_H
#define ROSETTA_REFACTORED_INIT_H

#include <stdint.h>
#include <stdbool.h>
#include "rosetta_types.h"

/* ============================================================================
 * Initialization and Cleanup
 * ============================================================================ */

/**
 * Initialize the Rosetta translation environment
 * Sets up thread state, initializes code cache, and prepares for translation.
 * Returns: 0 on success, -1 on error
 */
int rosetta_init(void);

/**
 * Cleanup Rosetta translation environment
 * Frees all allocated resources and resets state.
 */
void rosetta_cleanup(void);

/**
 * Get current thread state
 * Returns: Pointer to thread state
 */
ThreadState *rosetta_get_state(void);

/**
 * Check if Rosetta is initialized
 * Returns: true if initialized, false otherwise
 */
bool rosetta_is_initialized(void);

/**
 * Initialize translation environment
 * Sets up entry callback for translation.
 * @param entry_callback Pointer to store entry callback function
 */
void init_translation_env(void **entry_callback);

/**
 * Initialize translation cache
 * Sets up the translation cache data structures.
 */
void init_translation_cache(void);

/**
 * Initialize syscall table
 * Sets up the syscall handler table.
 */
void init_syscall_table(void);

/**
 * Initialize signal handlers
 * Sets up signal handlers for fault handling.
 */
void init_signal_handlers(void);

/**
 * Setup signal trampoline
 * Sets up the signal trampoline for signal handling.
 */
void setup_signal_tramp(void);

/**
 * Setup signal trampoline with handler
 * @param handler Signal handler function pointer
 * Returns: 0 on success, -1 on error
 */
int setup_signal_trampoline(void *handler);

/**
 * Cleanup signal handlers
 * Resets signal handlers to default.
 */
void cleanup_signal_handlers(void);

/**
 * Rosetta entry point
 * Main entry point for Rosetta initialization.
 */
void rosetta_entry(void);

/**
 * Daemon entry point
 * Entry point for Rosetta daemon mode.
 */
void rosettad_entry(void);

/**
 * Initialize daemon state
 * Sets up daemon-specific state and configuration.
 */
void init_daemon_state(void);

#endif /* ROSETTA_REFACTORED_INIT_H */
