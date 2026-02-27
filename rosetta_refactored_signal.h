/* ============================================================================
 * Rosetta Refactored - Signal Handling Header
 * ============================================================================
 *
 * This module provides signal handling functions for the Rosetta
 * translation layer.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_SIGNAL_H
#define ROSETTA_REFACTORED_SIGNAL_H

#include <stdint.h>
#include <stddef.h>
#include <signal.h>

/* ============================================================================
 * Signal Handler Types
 * ============================================================================ */

/* Signal handler function type */
typedef void (*rosetta_signal_handler_t)(int);

/* Extended signal handler with info and context */
typedef void (*rosetta_siginfo_handler_t)(int, siginfo_t *, void *);

/* ============================================================================
 * Signal Handler Registration
 * ============================================================================ */

/**
 * rosetta_install_signal_handler - Install signal handler for translated code
 * @signo: Signal number
 * @handler: Signal handler function
 * Returns: 0 on success, -1 on error
 */
int rosetta_install_signal_handler(int signo, rosetta_signal_handler_t handler);

/**
 * rosetta_install_siginfo_handler - Install extended signal handler
 * @signo: Signal number
 * @handler: Extended signal handler with info/context
 * Returns: 0 on success, -1 on error
 */
int rosetta_install_siginfo_handler(int signo, rosetta_siginfo_handler_t handler);

/**
 * rosetta_handle_signal - Signal handler for translated code
 * @signo: Signal number
 * @info: Signal information
 * @context: Machine context
 */
void rosetta_handle_signal(int signo, siginfo_t *info, void *context);

/**
 * rosetta_handle_fault - Handle memory fault in translated code
 * @signo: Signal number
 * @info: Signal information
 * @context: Machine context
 */
void rosetta_handle_fault(int signo, siginfo_t *info, void *context);

/**
 * rosetta_signal_handler_fault - Signal handler for segmentation faults
 * @sig: Signal number
 * @info: Signal information
 * @context: Machine context
 */
void rosetta_signal_handler_fault(int sig, siginfo_t *info, void *context);

/* ============================================================================
 * Signal Setup and Cleanup
 * ============================================================================ */

/**
 * rosetta_init_signal_handlers - Initialize all signal handlers
 * Returns: 0 on success, -1 on error
 */
int rosetta_init_signal_handlers(void);

/**
 * rosetta_cleanup_signal_handlers - Cleanup signal handlers
 */
void rosetta_cleanup_signal_handlers(void);

/**
 * rosetta_setup_signal_trampoline - Setup signal trampoline for translated code
 * @handler: Handler address
 * Returns: 0 on success, -1 on error
 */
int rosetta_setup_signal_trampoline(void *handler);

/**
 * init_signal_handlers - Initialize default signal handlers
 */
void init_signal_handlers(void);

/* ============================================================================
 * Signal Masking
 * ============================================================================ */

/**
 * rosetta_signal_block - Block a signal
 * @sig: Signal number
 * @oldmask: Previous signal mask (can be NULL)
 * Returns: 0 on success, -1 on error
 */
int rosetta_signal_block(int sig, sigset_t *oldmask);

/**
 * rosetta_signal_unblock - Unblock a signal
 * @sig: Signal number
 * Returns: 0 on success, -1 on error
 */
int rosetta_signal_unblock(int sig);

/**
 * rosetta_signal_set_mask - Set signal mask
 * @mask: New signal mask
 * Returns: 0 on success, -1 on error
 */
int rosetta_signal_set_mask(const sigset_t *mask);

/**
 * rosetta_signal_get_mask - Get current signal mask
 * @mask: Pointer to store current mask
 * Returns: 0 on success, -1 on error
 */
int rosetta_signal_get_mask(sigset_t *mask);

#endif /* ROSETTA_REFACTORED_SIGNAL_H */
