/*
 * Rosetta Daemon (rosettad) - Refactored Implementation
 *
 * This file implements the Rosetta daemon using the existing modular Rosetta
 * translation infrastructure. The daemon handles:
 * - System call translation and forwarding
 * - Process lifecycle management
 * - Communication with the host kernel
 * - IPC between translated processes and host
 *
 * Architecture:
 * - Reuses rosetta_exec.c for executing translated blocks
 * - Reuses rosetta_syscalls.c for syscall handling
 * - Reuses rosetta_cache.c for translation caching
 * - Reuses rosetta_runtime.c for runtime environment
 * - Adds daemon-specific IPC, signal handling, and process management
 */

#define _XOPEN_SOURCE 700
#define _DARWIN_C_SOURCE

#include "rosettad.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <stdnoreturn.h>

/* ============================================================================
 * Global Daemon State
 * ============================================================================ */

static rosettad_state_t g_daemon_state;
static int g_daemon_initialized = 0;

/* ============================================================================
 * Constants and Data Structures
 * ============================================================================ */

/* Shuffle masks for SIMD alignment handling */
static const uint64_t SHUFFLE_MASK_LO = 0x0c0e0f0d080a0b09ULL;
static const uint64_t SHUFFLE_MASK_HI = 0x0406070500020301ULL;

/* Alignment lookup tables for unaligned access handling */
static const uint64_t ALIGNMENT_TABLE_LO[16] = {
    0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
    0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
    0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
    0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL
};

static const uint64_t ALIGNMENT_TABLE_HI[16] = {
    0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
    0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
    0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
    0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL
};

/* ============================================================================
 * Forward Declarations (Internal functions not exposed in header)
 * ============================================================================ */

static void do_setup_signal_handlers(rosettad_state_t *state);
static void do_enter_supervisor_mode(rosettad_state_t *state);
static void do_crash_handler(rosettad_state_t *state, u64 code, u64 addr);
static int do_init_logging(rosettad_state_t *state, s32 log_fd, u32 log_level);
static int do_init_ipc_channels(rosettad_state_t *state);
static void process_command_line(rosettad_state_t *state, int argc, char **argv, char **envp);
static void *undefined_instruction_exception(void);

/* ============================================================================
 * SIMD Memory Operations (Shared with rosetta)
 * ============================================================================ */

/**
 * SIMD-optimized memory search
 * Reuses the same algorithm from rosetta_string_simd.c
 */
void *rosettad_memchr_simd(const void *ptr, long max_len)
{
    const uint64_t *words;
    uint64_t w0, w1;
    uint64_t shuffle_lo, shuffle_hi;
    uint8_t bytevec[16];
    uint8_t min_result;

    const uint64_t mask_lo = SHUFFLE_MASK_LO;
    const uint64_t mask_hi = SHUFFLE_MASK_HI;

    uint8_t m[16];
    m[0]  = (uint8_t)mask_lo;
    m[1]  = (uint8_t)(mask_lo >> 8);
    m[2]  = (uint8_t)(mask_lo >> 16);
    m[3]  = (uint8_t)(mask_lo >> 24);
    m[4]  = (uint8_t)(mask_lo >> 32);
    m[5]  = (uint8_t)(mask_lo >> 40);
    m[6]  = (uint8_t)(mask_lo >> 48);
    m[7]  = (uint8_t)(mask_lo >> 56);
    m[8]  = (uint8_t)mask_hi;
    m[9]  = (uint8_t)(mask_hi >> 8);
    m[10] = (uint8_t)(mask_hi >> 16);
    m[11] = (uint8_t)(mask_hi >> 24);
    m[12] = (uint8_t)(mask_hi >> 32);
    m[13] = (uint8_t)(mask_hi >> 40);
    m[14] = (uint8_t)(mask_hi >> 48);
    m[15] = (uint8_t)(mask_hi >> 56);

    if (max_len < 0) {
        words = (const uint64_t *)((uint64_t)ptr & ~0xfULL);
        w1 = words[1];
        w0 = words[0];

        size_t offset = (size_t)ptr & 0xf;
        shuffle_hi = ALIGNMENT_TABLE_HI[offset];
        shuffle_lo = ALIGNMENT_TABLE_LO[offset];

        bytevec[0]  = (uint8_t)w0 | (uint8_t)shuffle_lo;
        bytevec[1]  = (uint8_t)(w0 >> 8) | (uint8_t)(shuffle_lo >> 8);
        bytevec[2]  = (uint8_t)(w0 >> 16) | (uint8_t)(shuffle_lo >> 16);
        bytevec[3]  = (uint8_t)(w0 >> 24) | (uint8_t)(shuffle_lo >> 24);
        bytevec[4]  = (uint8_t)(w0 >> 32) | (uint8_t)(shuffle_lo >> 32);
        bytevec[5]  = (uint8_t)(w0 >> 40) | (uint8_t)(shuffle_lo >> 40);
        bytevec[6]  = (uint8_t)(w0 >> 48) | (uint8_t)(shuffle_lo >> 48);
        bytevec[7]  = (uint8_t)(w0 >> 56) | (uint8_t)(shuffle_lo >> 56);
        bytevec[8]  = (uint8_t)w1 | (uint8_t)shuffle_hi;
        bytevec[9]  = (uint8_t)(w1 >> 8) | (uint8_t)(shuffle_hi >> 8);
        bytevec[10] = (uint8_t)(w1 >> 16) | (uint8_t)(shuffle_hi >> 16);
        bytevec[11] = (uint8_t)(w1 >> 24) | (uint8_t)(shuffle_hi >> 24);
        bytevec[12] = (uint8_t)(w1 >> 32) | (uint8_t)(shuffle_hi >> 32);
        bytevec[13] = (uint8_t)(w1 >> 40) | (uint8_t)(shuffle_hi >> 40);
        bytevec[14] = (uint8_t)(w1 >> 48) | (uint8_t)(shuffle_hi >> 48);
        bytevec[15] = (uint8_t)(w1 >> 56) | (uint8_t)(shuffle_hi >> 56);

        while (1) {
            min_result = neon_uminv(bytevec);
            if (min_result == 0)
                break;

            w1 = words[3];
            w0 = words[2];

            bytevec[0]  = (uint8_t)w0;
            bytevec[1]  = (uint8_t)(w0 >> 8);
            bytevec[2]  = (uint8_t)(w0 >> 16);
            bytevec[3]  = (uint8_t)(w0 >> 24);
            bytevec[4]  = (uint8_t)(w0 >> 32);
            bytevec[5]  = (uint8_t)(w0 >> 40);
            bytevec[6]  = (uint8_t)(w0 >> 48);
            bytevec[7]  = (uint8_t)(w0 >> 56);
            bytevec[8]  = (uint8_t)w1;
            bytevec[9]  = (uint8_t)(w1 >> 8);
            bytevec[10] = (uint8_t)(w1 >> 16);
            bytevec[11] = (uint8_t)(w1 >> 24);
            bytevec[12] = (uint8_t)(w1 >> 32);
            bytevec[13] = (uint8_t)(w1 >> 40);
            bytevec[14] = (uint8_t)(w1 >> 48);
            bytevec[15] = (uint8_t)(w1 >> 56);

            words += 2;
        }

        for (int i = 0; i < 16; i++) {
            bytevec[i] = (bytevec[i] != 0 ? 0xff : 0) | m[i];
        }

        min_result = neon_uminv(bytevec);
        return (void *)((uint64_t)words + (min_result - (uint64_t)ptr));
    }

    if (max_len != 0) {
        words = (const uint64_t *)((uint64_t)ptr & ~0xfULL);
        w1 = words[1];
        w0 = words[0];

        size_t ptr_offset = (size_t)ptr & 0xf;
        shuffle_hi = ALIGNMENT_TABLE_HI[ptr_offset];
        shuffle_lo = ALIGNMENT_TABLE_LO[ptr_offset];

        bytevec[0]  = (uint8_t)w0 | (uint8_t)shuffle_lo;
        bytevec[1]  = (uint8_t)(w0 >> 8) | (uint8_t)(shuffle_lo >> 8);
        bytevec[2]  = (uint8_t)(w0 >> 16) | (uint8_t)(shuffle_lo >> 16);
        bytevec[3]  = (uint8_t)(w0 >> 24) | (uint8_t)(shuffle_lo >> 24);
        bytevec[4]  = (uint8_t)(w0 >> 32) | (uint8_t)(shuffle_lo >> 32);
        bytevec[5]  = (uint8_t)(w0 >> 40) | (uint8_t)(shuffle_lo >> 40);
        bytevec[6]  = (uint8_t)(w0 >> 48) | (uint8_t)(shuffle_lo >> 48);
        bytevec[7]  = (uint8_t)(w0 >> 56) | (uint8_t)(shuffle_lo >> 56);
        bytevec[8]  = (uint8_t)w1 | (uint8_t)shuffle_hi;
        bytevec[9]  = (uint8_t)(w1 >> 8) | (uint8_t)(shuffle_hi >> 8);
        bytevec[10] = (uint8_t)(w1 >> 16) | (uint8_t)(shuffle_hi >> 16);
        bytevec[11] = (uint8_t)(w1 >> 24) | (uint8_t)(shuffle_hi >> 24);
        bytevec[12] = (uint8_t)(w1 >> 32) | (uint8_t)(shuffle_hi >> 32);
        bytevec[13] = (uint8_t)(w1 >> 40) | (uint8_t)(shuffle_hi >> 40);
        bytevec[14] = (uint8_t)(w1 >> 48) | (uint8_t)(shuffle_hi >> 48);
        bytevec[15] = (uint8_t)(w1 >> 56) | (uint8_t)(shuffle_hi >> 56);

        size_t offset = (size_t)max_len + ptr_offset;

        while (1) {
            min_result = neon_uminv(bytevec);

            if (min_result == 0) {
                for (int i = 0; i < 16; i++) {
                    bytevec[i] = (bytevec[i] != 0 ? 0xff : 0) | m[i];
                }

                min_result = neon_uminv(bytevec);
                if (min_result <= offset) {
                    offset = min_result;
                }
                return (void *)((uint64_t)words + (offset - (uint64_t)ptr));
            }

            if (offset < 16 || offset == 16)
                break;

            w1 = words[3];
            w0 = words[2];
            offset -= 16;
            words += 2;
        }

        return (void *)((uint64_t)words + (offset - (uint64_t)ptr));
    }

    return NULL;
}

/**
 * SIMD string compare (strcmp implementation)
 */
int rosettad_strcmp_simd(const char *s1, const char *s2)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    if (((uint64_t)s1 & 0xf) == 0 && ((uint64_t)s2 & 0xf) == 0) {
        const uint64_t *w1 = (const uint64_t *)p1;
        const uint64_t *w2 = (const uint64_t *)p2;
        uint64_t word1, word2;
        uint8_t cmp_mask[16];

        do {
            word2 = w2[1];
            word1 = w1[1];

            uint64_t diff = word1 ^ word2;
            cmp_mask[0]  = ((diff) & 0xff);
            cmp_mask[1]  = ((diff >> 8) & 0xff);
            cmp_mask[2]  = ((diff >> 16) & 0xff);
            cmp_mask[3]  = ((diff >> 24) & 0xff);
            cmp_mask[4]  = ((diff >> 32) & 0xff);
            cmp_mask[5]  = ((diff >> 40) & 0xff);
            cmp_mask[6]  = ((diff >> 48) & 0xff);
            cmp_mask[7]  = ((diff >> 56) & 0xff);

            diff = w1[0] ^ w2[0];
            cmp_mask[8]  = ((diff) & 0xff);
            cmp_mask[9]  = ((diff >> 8) & 0xff);
            cmp_mask[10] = ((diff >> 16) & 0xff);
            cmp_mask[11] = ((diff >> 24) & 0xff);
            cmp_mask[12] = ((diff >> 32) & 0xff);
            cmp_mask[13] = ((diff >> 40) & 0xff);
            cmp_mask[14] = ((diff >> 48) & 0xff);
            cmp_mask[15] = ((diff >> 56) & 0xff);

            p1 += 16;
            p2 += 16;
            w1 += 2;
            w2 += 2;
        } while (neon_uminv(cmp_mask) == 0);

        p1 -= 16;
        p2 -= 16;
        while (*p1 == *p2 && *p1 != 0) {
            p1++;
            p2++;
        }

        return *p1 - *p2;
    }

    while (*p1 == *p2 && *p1 != 0) {
        p1++;
        p2++;
    }

    return *p1 - *p2;
}

/**
 * Byte-by-byte string comparison fallback
 */
int rosettad_strcmp_bytewise(const char *s1, const char *s2)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    while (*p1 == *p2 && *p1 != 0) {
        p1++;
        p2++;
    }

    return *p1 - *p2;
}

/* ============================================================================
 * Daemon Entry Point and Initialization
 * ============================================================================ */

/**
 * Rosetta Daemon Entry Point
 */
void rosettad_entry(void)
{
    rosettad_state_t *state = &g_daemon_state;

    /* Zero-initialize state */
    memset(state, 0, sizeof(rosettad_state_t));

    /* Initialize daemon state */
    init_daemon_state();

    /* Enter supervisor mode - handles syscall trapping */
    do_enter_supervisor_mode(state);

    /* Should not reach here - but handle exceptions */
    do_crash_handler(state, 0, 0x225064);

    /* Unreachable - but satisfy type system */
    ((void (*)())undefined_instruction_exception())();
}

/**
 * Initialize daemon state
 */
void init_daemon_state(void)
{
    rosettad_state_t *state = &g_daemon_state;

    /* Set default configuration */
    state->config.flags = ROSETTAD_FLAG_RUNNING;
    state->config.log_level = LOG_LEVEL_INFO;
    state->config.log_fd = -1;
    state->config.cmd_fd = -1;
    state->config.status_fd = -1;
    state->config.guest_base = 0x0;
    state->config.host_base = 0x0;
    state->config.memory_size = 0x100000000ULL; /* 4GB default */
    state->config.translation_mode = 0;

    /* Set up signal handlers */
    do_setup_signal_handlers(state);

    /* Initialize logging */
    do_init_logging(state, STDERR_FILENO, LOG_LEVEL_INFO);

    /* Initialize IPC channels */
    do_init_ipc_channels(state);

    /* Initialize runtime environment */
    syscall_handler_init();

    g_daemon_initialized = 1;
}

/**
 * Initialize daemon with configuration
 */
int rosettad_init(rosettad_config_t *config)
{
    rosettad_state_t *state = &g_daemon_state;

    if (g_daemon_initialized) {
        return ROSETTAD_ERR_ALREADY_RUNNING;
    }

    /* Copy configuration */
    if (config) {
        memcpy(&state->config, config, sizeof(rosettad_config_t));
    } else {
        /* Use defaults */
        memset(&state->config, 0, sizeof(rosettad_config_t));
        state->config.flags = ROSETTAD_FLAG_RUNNING;
        state->config.log_level = LOG_LEVEL_INFO;
        state->config.memory_size = 0x100000000ULL;
    }

    /* Initialize subsystems */
    init_daemon_state();

    return ROSETTAD_OK;
}

/**
 * Run daemon main loop
 */
int rosettad_run(rosettad_state_t *state)
{
    if (!state) {
        state = &g_daemon_state;
    }

    if (!g_daemon_initialized) {
        return ROSETTAD_ERR_NOT_RUNNING;
    }

    /* Enter supervisor mode */
    enter_supervisor_mode(state);

    return ROSETTAD_OK; /* Never reached */
}

/**
 * Shutdown daemon
 */
void rosettad_shutdown(rosettad_state_t *state)
{
    if (!state) {
        state = &g_daemon_state;
    }

    LOG_INFO(state, "Shutting down Rosetta daemon");

    /* Close IPC channels */
    close_ipc_channels(state);

    /* Close logging */
    close_logging(state);

    /* Clean up process resources */
    cleanup_process_resources(state);

    /* Clear flags */
    state->config.flags = 0;
    g_daemon_initialized = 0;
}

/* ============================================================================
 * Signal Handling
 * ============================================================================ */

/**
 * Set up daemon signal handlers (internal implementation)
 */
static void do_setup_signal_handlers(rosettad_state_t *state)
{
    struct sigaction sa;

    (void)state; /* Unused in minimal impl */

    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handle_sigsegv;
    sigaction(SIGSEGV, &sa, NULL);

    sa.sa_sigaction = handle_sigill;
    sigaction(SIGILL, &sa, NULL);

    sa.sa_sigaction = handle_sigbus;
    sigaction(SIGBUS, &sa, NULL);

    sa.sa_sigaction = handle_sigabrt;
    sigaction(SIGABRT, &sa, NULL);

    ROSETTAD_FLAG_SET_FLAG(state, ROSETTAD_FLAG_SIGNALS_INIT);
}

/**
 * Handle SIGSEGV - memory access violations
 */
void handle_sigsegv(s32 signum, siginfo_t *info, void *ucontext)
{
    (void)signum;
    (void)ucontext;

    rosettad_state_t *state = &g_daemon_state;
    LOG_ERROR(state, "SIGSEGV at address %p", info->si_addr);
    do_crash_handler(state, 1, (u64)info->si_addr);
}

/**
 * Handle SIGILL - illegal instructions
 */
void handle_sigill(s32 signum, siginfo_t *info, void *ucontext)
{
    (void)signum;
    (void)ucontext;

    rosettad_state_t *state = &g_daemon_state;
    LOG_ERROR(state, "SIGILL at address %p", info->si_addr);
    do_crash_handler(state, 2, (u64)info->si_addr);
}

/**
 * Handle SIGBUS - bus errors
 */
void handle_sigbus(s32 signum, siginfo_t *info, void *ucontext)
{
    (void)signum;
    (void)ucontext;

    rosettad_state_t *state = &g_daemon_state;
    LOG_ERROR(state, "SIGBUS at address %p", info->si_addr);
    do_crash_handler(state, 3, (u64)info->si_addr);
}

/**
 * Handle SIGABRT - aborts
 */
void handle_sigabrt(s32 signum, siginfo_t *info, void *ucontext)
{
    (void)signum;
    (void)info;
    (void)ucontext;

    rosettad_state_t *state = &g_daemon_state;
    LOG_ERROR(state, "SIGABRT received");
    do_crash_handler(state, 4, 0);
}

/* ============================================================================
 * Supervisor Mode / Syscall Handling
 * ============================================================================ */

/**
 * Enter supervisor mode - main syscall handling loop (internal implementation)
 */
static noreturn void do_enter_supervisor_mode(rosettad_state_t *state)
{
    LOG_INFO(state, "Entering supervisor mode");
    state->config.flags |= ROSETTAD_FLAG_SUPERVISOR;

    /* Main syscall handling loop */
    while (1) {
        ThreadState *ts = state->thread_state;

        if (ts) {
            /* Wait for syscall trap */
            /* Read guest syscall number and arguments */
            s32 guest_nr = syscall_get_nr(ts);

            /* Log syscall */
            log_syscall(state, guest_nr, NULL);

            /* Execute host syscall and return result */
            /* This is handled by the existing syscall_dispatch */
            s64 result = syscall_dispatch(ts);
            return_syscall_to_guest(ts, result);

            state->syscalls_handled++;
        }

        /* Small yield to prevent busy-waiting */
        sched_yield();
    }
}

/**
 * Handle trapped syscall from guest
 */
s64 handle_guest_syscall(rosettad_state_t *state, ThreadState *ts, s32 nr)
{
    (void)state;
    (void)nr;

    if (!ts) {
        return -EINVAL;
    }

    return syscall_dispatch(ts);
}

/**
 * Translate guest syscall to host
 * Maps ARM64 Linux syscall numbers to host (macOS/Darwin) syscall numbers
 */
s32 translate_syscall_to_host(s32 guest_nr, s32 *host_nr)
{
    if (!host_nr) {
        return -EINVAL;
    }

    /* Simple identity mapping for common syscalls */
    /* In a full implementation, this would map Linux ARM64 syscall */
    /* numbers to macOS/Darwin syscall numbers */

    /* Common syscall mappings (Linux ARM64 -> macOS) */
    switch (guest_nr) {
    case 0:  /* io_setup */
        *host_nr = SYS_open;
        break;
    case 63: /* read */
        *host_nr = SYS_read;
        break;
    case 64: /* write */
        *host_nr = SYS_write;
        break;
    case 80: /* open */
        *host_nr = SYS_open;
        break;
    case 57: /* close */
        *host_nr = SYS_close;
        break;
    case 169: /* stat */
        *host_nr = SYS_stat;
        break;
    case 170: /* fstat */
        *host_nr = SYS_fstat;
        break;
    case 39: /* getpid */
        *host_nr = SYS_getpid;
        break;
    case 177: /* gettid */
        *host_nr = SYS_gettid;
        break;
    case 93: /* exit */
        *host_nr = SYS_exit;
        break;
    default:
        /* Pass through for unknown syscalls */
        *host_nr = guest_nr;
        break;
    }

    return 0;
}

/**
 * Execute host syscall
 */
s64 execute_host_syscall(s32 host_nr, u64 *args)
{
    /* Use the host's syscall mechanism */
#ifdef __APPLE__
    /* macOS syscall convention */
    return syscall(host_nr,
                   (void *)(args ? args[0] : 0),
                   (void *)(args ? args[1] : 0),
                   (void *)(args ? args[2] : 0),
                   (void *)(args ? args[3] : 0),
                   (void *)(args ? args[4] : 0),
                   (void *)(args ? args[5] : 0));
#else
    /* Linux syscall convention */
    return syscall(host_nr,
                   args ? args[0] : 0,
                   args ? args[1] : 0,
                   args ? args[2] : 0,
                   args ? args[3] : 0,
                   args ? args[4] : 0,
                   args ? args[5] : 0);
#endif
}

/**
 * Return syscall result to guest
 */
void return_syscall_to_guest(ThreadState *ts, s64 result)
{
    if (ts) {
        /* Set return value in X0 register */
        ts->guest.x[0] = (u64)result;
    }
}

/* ============================================================================
 * Process Lifecycle Management
 * ============================================================================ */

/**
 * Initialize process execution environment
 */
int init_process_environment(rosettad_state_t *state, int argc, char **argv, char **envp)
{
    (void)argc;
    (void)argv;
    (void)envp;

    if (!state) {
        return ROSETTAD_ERR_INVALID_ARG;
    }

    /* Initialize translation environment */
    void *entry_callback;
    init_translation_env(&entry_callback);

    LOG_INFO(state, "Process environment initialized");
    return ROSETTAD_OK;
}

/**
 * Set up process memory mappings
 */
int setup_process_memory(rosettad_state_t *state, u64 guest_base, size_t size)
{
    if (!state) {
        return ROSETTAD_ERR_INVALID_ARG;
    }

    /* Map memory for guest */
    void *mem = mmap(NULL, size,
                     PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (mem == MAP_FAILED) {
        LOG_ERROR(state, "Failed to map process memory");
        return ROSETTAD_ERR_MEMORY_FAIL;
    }

    state->memory_base = mem;
    state->memory_size = size;
    state->config.guest_base = guest_base;

    LOG_INFO(state, "Process memory mapped: %zu bytes at %p", size, mem);
    return ROSETTAD_OK;
}

/**
 * Clean up process resources
 */
void cleanup_process_resources(rosettad_state_t *state)
{
    if (!state) {
        return;
    }

    /* Unmap memory */
    if (state->memory_base && state->memory_size > 0) {
        munmap(state->memory_base, state->memory_size);
        state->memory_base = NULL;
        state->memory_size = 0;
    }

    /* Free thread state */
    if (state->thread_state) {
        free(state->thread_state);
        state->thread_state = NULL;
    }

    LOG_INFO(state, "Process resources cleaned up");
}

/**
 * Get current process ID
 */
s32 rosettad_getpid(rosettad_state_t *state)
{
    (void)state;
    return getpid();
}

/**
 * Get current thread ID
 */
s32 rosettad_gettid(rosettad_state_t *state)
{
    (void)state;
#ifdef __APPLE__
    u64 tid;
    syscall(SYS_thread_selfid, &tid);
    return (s32)tid;
#else
    return syscall(SYS_gettid);
#endif
}

/* ============================================================================
 * Logging
 * ============================================================================ */

/**
 * Initialize logging (internal implementation)
 */
static int do_init_logging(rosettad_state_t *state, s32 log_fd, u32 log_level)
{
    if (!state) {
        return ROSETTAD_ERR_INVALID_ARG;
    }

    state->config.log_fd = log_fd;
    state->config.log_level = log_level;

    LOG_INFO(state, "Logging initialized (fd=%d, level=%u)", log_fd, log_level);
    return ROSETTAD_OK;
}

/**
 * Log message
 */
void rosettad_log(rosettad_state_t *state, u32 level, const char *fmt, ...)
{
    if (!state || !fmt) {
        return;
    }

    /* Check log level */
    if (level > state->config.log_level) {
        return;
    }

    /* Format and write log message */
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (len > 0 && state->config.log_fd >= 0) {
        write(state->config.log_fd, buf, (size_t)len);
        write(state->config.log_fd, "\n", 1);
    }
}

/**
 * Log syscall
 */
void log_syscall(rosettad_state_t *state, s32 nr, u64 *args)
{
    if (!state) {
        return;
    }

    if (state->config.log_level >= LOG_LEVEL_DEBUG) {
        if (args) {
            LOG_DEBUG(state, "Syscall %d: args[%016lx %016lx %016lx]",
                     nr, args[0], args[1], args[2]);
        } else {
            LOG_DEBUG(state, "Syscall %d", nr);
        }
    }
}

/**
 * Log translation event
 */
void log_translation(rosettad_state_t *state, u64 guest_pc, u64 host_pc, u32 size)
{
    if (!state) {
        return;
    }

    if (state->config.log_level >= LOG_LEVEL_DEBUG) {
        LOG_DEBUG(state, "Translation: guest=0x%016lx -> host=0x%016lx (size=%u)",
                 guest_pc, host_pc, size);
        state->translations_count++;
    }
}

/**
 * Close logging
 */
void close_logging(rosettad_state_t *state)
{
    if (state && state->config.log_fd >= 0 && state->config.log_fd != STDERR_FILENO) {
        close(state->config.log_fd);
        state->config.log_fd = -1;
    }
}

/* ============================================================================
 * IPC Communication
 * ============================================================================ */

/**
 * Initialize IPC channels (internal implementation)
 */
static int do_init_ipc_channels(rosettad_state_t *state)
{
    if (!state) {
        return ROSETTAD_ERR_INVALID_ARG;
    }

    /* Create IPC channels using Unix domain sockets or shared memory */
    /* For now, use simple file descriptors */

    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0) {
        state->config.cmd_fd = sv[0];
        state->config.status_fd = sv[1];
    }

    LOG_INFO(state, "IPC channels initialized");
    return ROSETTAD_OK;
}

/**
 * Create shared memory region for IPC
 */
void *create_ipc_shared_memory(size_t size, s32 *fd)
{
    /* Create shared memory using shm_open or mmap */
    void *mem = mmap(NULL, size,
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (mem != MAP_FAILED && fd) {
        *fd = -1; /* Anonymous mapping */
    }

    return mem;
}

/**
 * Send message via IPC
 */
int ipc_send(rosettad_state_t *state, rosettad_ipc_channel_t channel, const void *msg, size_t len)
{
    if (!state || !msg) {
        return ROSETTAD_ERR_INVALID_ARG;
    }

    s32 fd = state->config.cmd_fd;
    if (channel == IPC_CHANNEL_STATUS) {
        fd = state->config.status_fd;
    }

    if (fd < 0) {
        return ROSETTAD_ERR_IPC_FAIL;
    }

    ssize_t sent = write(fd, msg, len);
    return (sent == (ssize_t)len) ? ROSETTAD_OK : ROSETTAD_ERR_IPC_FAIL;
}

/**
 * Receive message via IPC
 */
int ipc_recv(rosettad_state_t *state, rosettad_ipc_channel_t channel, void *msg, size_t *len)
{
    if (!state || !msg || !len) {
        return ROSETTAD_ERR_INVALID_ARG;
    }

    s32 fd = state->config.cmd_fd;
    if (channel == IPC_CHANNEL_STATUS) {
        fd = state->config.status_fd;
    }

    if (fd < 0) {
        return ROSETTAD_ERR_IPC_FAIL;
    }

    ssize_t received = read(fd, msg, *len);
    if (received >= 0) {
        *len = (size_t)received;
        return ROSETTAD_OK;
    }

    return ROSETTAD_ERR_IPC_FAIL;
}

/**
 * Close IPC channels
 */
void close_ipc_channels(rosettad_state_t *state)
{
    if (!state) {
        return;
    }

    if (state->config.cmd_fd >= 0) {
        close(state->config.cmd_fd);
        state->config.cmd_fd = -1;
    }

    if (state->config.status_fd >= 0) {
        close(state->config.status_fd);
        state->config.status_fd = -1;
    }
}

/* ============================================================================
 * Memory Management
 * ============================================================================ */

/**
 * Allocate daemon memory
 */
void *rosettad_malloc(size_t size)
{
    return mmap(NULL, size,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

/**
 * Free daemon memory
 */
void rosettad_free(void *ptr)
{
    if (ptr) {
        munmap(ptr, 0); /* Note: size not tracked */
    }
}

/**
 * Map guest memory
 */
void *map_guest_memory(rosettad_state_t *state, u64 guest_addr, size_t size, s32 prot)
{
    (void)state;
    (void)guest_addr;

    return mmap(NULL, size, prot, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

/**
 * Unmap guest memory
 */
int unmap_guest_memory(rosettad_state_t *state, u64 guest_addr, size_t size)
{
    (void)state;
    (void)guest_addr;

    return munmap((void *)guest_addr, size);
}

/**
 * Translate guest address to host
 */
void *translate_guest_addr(rosettad_state_t *state, u64 guest_addr)
{
    if (!state || !state->memory_base) {
        return NULL;
    }

    /* Simple offset-based translation */
    u64 offset = guest_addr - state->config.guest_base;
    return (void *)((u8 *)state->memory_base + offset);
}

/* ============================================================================
 * Crash Handling
 * ============================================================================ */

/**
 * Handle crashes and exceptions (internal implementation)
 */
static noreturn void do_crash_handler(rosettad_state_t *state, u64 crash_code, u64 crash_addr)
{
    LOG_ERROR(state, "Crash handler invoked: code=%lu, addr=0x%lx",
             (unsigned long)crash_code, (unsigned long)crash_addr);

    /* Generate crash report */
    char report[4096];
    generate_crash_report(state, report, sizeof(report));

    /* Write report to stderr */
    write(STDERR_FILENO, report, strlen(report));

    /* Clean up and exit */
    cleanup_after_crash(state);

    /* Infinite loop - should not return */
    for (;;);
}

/**
 * Generate crash report
 */
int generate_crash_report(rosettad_state_t *state, void *report_buf, size_t buf_size)
{
    char *buf = (char *)report_buf;
    size_t offset = 0;

    offset += snprintf(buf + offset, buf_size - offset,
                       "=== Rosetta Daemon Crash Report ===\n");
    offset += snprintf(buf + offset, buf_size - offset,
                       "State: %s\n", g_daemon_initialized ? "initialized" : "not initialized");
    offset += snprintf(buf + offset, buf_size - offset,
                       "Flags: 0x%x\n", state ? state->config.flags : 0);
    offset += snprintf(buf + offset, buf_size - offset,
                       "Syscalls handled: %lu\n",
                       state ? (unsigned long)state->syscalls_handled : 0);
    offset += snprintf(buf + offset, buf_size - offset,
                       "Translations: %lu\n",
                       state ? (unsigned long)state->translations_count : 0);

    return (int)offset;
}

/**
 * Clean up after crash
 */
void cleanup_after_crash(rosettad_state_t *state)
{
    rosettad_shutdown(state);
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * Initialize random number generator
 */
int init_random(void)
{
    return 0; /* Not needed on modern systems */
}

/**
 * Get random bytes
 */
int get_random_bytes(void *buf, size_t len)
{
#ifdef __APPLE__
    return getentropy(buf, len);
#else
    return getrandom(buf, len, 0);
#endif
}

/**
 * Get current time
 */
u64 get_current_time(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (u64)ts.tv_sec * 1000000000ULL + (u64)ts.tv_nsec;
}

/**
 * Get CPU count
 */
s32 get_cpu_count(void)
{
    return (s32)sysconf(_SC_NPROCESSORS_ONLN);
}

/**
 * Check if running under Rosetta
 */
bool is_running_under_rosetta(void)
{
#if defined(__x86_64__)
    /* On x86_64, check if we're translated */
    return false;
#else
    /* On ARM64, we're native */
    return false;
#endif
}

/* ============================================================================
 * Initialization stubs
 * ============================================================================ */

static void *undefined_instruction_exception(void)
{
    return (void *)0;
}

/* ============================================================================
 * Non-static wrappers for functions declared in header
 * ============================================================================ */

/* Wrapper for crash_handler (calls internal implementation) */
noreturn void crash_handler(rosettad_state_t *state, u64 crash_code, u64 crash_addr)
{
    do_crash_handler(state, crash_code, crash_addr);
}

/* Wrapper for setup_signal_handlers (calls static implementation) */
int setup_signal_handlers(rosettad_state_t *state)
{
    do_setup_signal_handlers(state);
    return ROSETTAD_OK;
}

/* Wrapper for enter_supervisor_mode (calls static implementation) */
noreturn void enter_supervisor_mode(rosettad_state_t *state)
{
    do_enter_supervisor_mode(state);
}

/* Wrapper for init_logging (calls static implementation) */
int init_logging(rosettad_state_t *state, s32 log_fd, u32 log_level)
{
    return do_init_logging(state, log_fd, log_level);
}

/* Wrapper for init_ipc_channels (calls static implementation) */
int init_ipc_channels(rosettad_state_t *state)
{
    return do_init_ipc_channels(state);
}
