/* ============================================================================
 * Rosetta Refactored - Signal Handling Implementation
 * ============================================================================
 *
 * This module implements signal handling functions for the Rosetta
 * translation layer, translating host signals to guest exceptions.
 *
 * Architecture: x86_64 guest -> ARM64 host
 * Signals must be translated from host ARM64 to guest x86_64 context
 * ============================================================================ */

#include "rosetta_refactored_signal.h"
#include "rosetta_refactored_reg.h"
#include "rosetta_refactored_debug.h"
#include "rosetta_refactored_exception.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ============================================================================
 * Signal-to-Exception Translation
 * ============================================================================ */

/**
 * Translate host signal to guest exception and handle appropriately
 * This ensures x86_64 guest programs receive proper exceptions
 */
static void rosetta_signal_to_exception_handler(int sig, siginfo_t *info, void *context)
{
    ros_exception_info_t exc_info;
    memset(&exc_info, 0, sizeof(exc_info));

    /* Translate signal to exception type */
    exc_info.type = rosetta_signal_to_exception(sig, info ? info->si_code : 0);
    exc_info.sig = sig;
    exc_info.code = info ? info->si_code : 0;
    exc_info.addr = (uint64_t)(info ? info->si_addr : 0);

#ifdef DEBUG
    printf("[ROSETTA] Signal %d at address %p (code=%d)\n",
           sig, info ? info->si_addr : NULL, info ? info->si_code : 0);
#endif

    /* Try to handle the exception */
    int result = rosetta_handle_exception(&exc_info, context);

    if (result < 0) {
        /* Fatal exception - cannot continue */
        fprintf(stderr, "[ROSETTA] Fatal exception: signal %d\n", sig);

        /* Restore default signal handler and re-raise */
        signal(sig, SIG_DFL);
        raise(sig);
    }
}

/* ============================================================================
 * Signal Handler Installation
 * ============================================================================ */

/**
 * rosetta_install_signal_handler - Install signal handler for translated code
 */
int rosetta_install_signal_handler(int signo, rosetta_signal_handler_t handler)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    return sigaction(signo, &sa, NULL);
}

/**
 * rosetta_install_siginfo_handler - Install extended signal handler
 */
int rosetta_install_siginfo_handler(int signo, rosetta_siginfo_handler_t handler)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART;

    return sigaction(signo, &sa, NULL);
}

/**
 * rosetta_handle_signal - Signal handler for translated code
 * Translates host signals to guest exceptions and handles appropriately
 */
void rosetta_handle_signal(int signo, siginfo_t *info, void *context)
{
#ifdef DEBUG
    printf("[ROSETTA SIGNAL] Received signal %d\n", signo);
    if (info) {
        printf("[ROSETTA SIGNAL] Address: %p, Code: %d\n",
               info->si_addr, info->si_code);
    }
#endif

    /* Translate signal to exception and handle */
    rosetta_signal_to_exception_handler(signo, info, context);
}

/**
 * rosetta_handle_fault - Handle memory fault in translated code
 * Specialized handler for SIGSEGV and SIGBUS
 */
void rosetta_handle_fault(int signo, siginfo_t *info, void *context)
{
    const char *fault_type = (signo == SIGSEGV) ? "Segmentation fault" : "Bus error";

    fprintf(stderr, "[ROSETTA] %s at address %p\n",
            fault_type, info ? info->si_addr : NULL);

    if (info) {
        fprintf(stderr, "[ROSETTA] Fault code: %d\n", info->si_code);

        /* Provide helpful diagnostics based on signal type */
        if (signo == SIGSEGV) {
            switch (info->si_code) {
            case SEGV_MAPERR:
                fprintf(stderr, "[ROSETTA] Address not mapped to object\n");
                break;
            case SEGV_ACCERR:
                fprintf(stderr, "[ROSETTA] Invalid permissions for mapped object\n");
                break;
            default:
                fprintf(stderr, "[ROSETTA] Unknown segmentation fault\n");
                break;
            }
        } else if (signo == SIGBUS) {
            switch (info->si_code) {
            case BUS_ADRALN:
                fprintf(stderr, "[ROSETTA] Invalid address alignment\n");
                break;
            case BUS_ADRERR:
                fprintf(stderr, "[ROSETTA] Non-existent physical address\n");
                break;
            default:
                fprintf(stderr, "[ROSETTA] Unknown bus error\n");
                break;
            }
        }
    }

    /* Try to handle as exception first */
    rosetta_signal_to_exception_handler(signo, info, context);
}

/**
 * rosetta_signal_handler_fault - Signal handler for segmentation faults
 * Installed for SIGSEGV to catch NULL pointer accesses, etc.
 */
void rosetta_signal_handler_fault(int sig, siginfo_t *info, void *context)
{
    fprintf(stderr, "\n[ROSETTA] Segmentation fault detected!\n");
    fprintf(stderr, "[ROSETTA] Fault address: %p\n", info ? info->si_addr : NULL);

    /* Try to handle the fault gracefully */
    rosetta_handle_fault(sig, info, context);
}

/* ============================================================================
 * Signal Setup and Cleanup
 * ============================================================================ */

/**
 * rosetta_init_signal_handlers - Initialize all signal handlers
 * Sets up handlers for critical signals that translated code may encounter
 */
int rosetta_init_signal_handlers(void)
{
    struct sigaction sa;
    int result = 0;

    /* Initialize exception handling subsystem */
    rosetta_exception_init();

    /* Setup signal handlers with SA_SIGINFO for detailed fault info */

    /* SIGSEGV - Segmentation fault (NULL pointer, invalid access) */
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = rosetta_handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    if (sigaction(SIGSEGV, &sa, NULL) < 0) {
        perror("sigaction(SIGSEGV)");
        result = -1;
    }

    /* SIGBUS - Bus error (alignment faults, etc.) */
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = rosetta_handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    if (sigaction(SIGBUS, &sa, NULL) < 0) {
        perror("sigaction(SIGBUS)");
        result = -1;
    }

    /* SIGILL - Illegal instruction (undefined instructions) */
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = rosetta_handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    if (sigaction(SIGILL, &sa, NULL) < 0) {
        perror("sigaction(SIGILL)");
        result = -1;
    }

    /* SIGFPE - Floating point exception (divide by zero, etc.) */
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = rosetta_handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    if (sigaction(SIGFPE, &sa, NULL) < 0) {
        perror("sigaction(SIGFPE)");
        result = -1;
    }

    /* SIGTRAP - Trace trap (debug breakpoints) */
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = rosetta_handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    if (sigaction(SIGTRAP, &sa, NULL) < 0) {
        perror("sigaction(SIGTRAP)");
        result = -1;
    }

#ifdef DEBUG
    if (result == 0) {
        printf("[ROSETTA] Signal handlers initialized\n");
    }
#endif

    return result;
}

/**
 * rosetta_cleanup_signal_handlers - Cleanup signal handlers
 * Resets all signal handlers to default behavior
 */
void rosetta_cleanup_signal_handlers(void)
{
    /* Reset to default handlers */
    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGTRAP, SIG_DFL);

    /* Cleanup exception handling */
    rosetta_exception_cleanup();

#ifdef DEBUG
    printf("[ROSETTA] Signal handlers cleaned up\n");
#endif
}

/**
 * rosetta_setup_signal_trampoline - Setup signal trampoline for translated code
 */
int rosetta_setup_signal_trampoline(void *handler)
{
    (void)handler;  /* Placeholder for future implementation */
    return 0;
}

/**
 * init_signal_handlers - Initialize default signal handlers
 */
void init_signal_handlers(void)
{
    rosetta_init_signal_handlers();
}

/* ============================================================================
 * Signal Masking
 * ============================================================================ */

/**
 * rosetta_signal_block - Block a signal
 */
int rosetta_signal_block(int sig, sigset_t *oldmask)
{
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, sig);

    return sigprocmask(SIG_BLOCK, &mask, oldmask);
}

/**
 * rosetta_signal_unblock - Unblock a signal
 */
int rosetta_signal_unblock(int sig)
{
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, sig);

    return sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

/**
 * rosetta_signal_set_mask - Set signal mask
 */
int rosetta_signal_set_mask(const sigset_t *mask)
{
    return sigprocmask(SIG_SETMASK, mask, NULL);
}

/**
 * rosetta_signal_get_mask - Get current signal mask
 */
int rosetta_signal_get_mask(sigset_t *mask)
{
    return sigprocmask(SIG_SETMASK, NULL, mask);
}
