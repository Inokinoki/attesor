/*
 * Rosetta Daemon (rosettad) - Header File
 *
 * This header provides declarations for the Rosetta daemon, which handles:
 * - System call translation and forwarding
 * - Process lifecycle management
 * - Communication with the host kernel
 * - IPC between translated processes and host
 *
 * The daemon reuses existing Rosetta modules for translation, caching,
 * and execution while adding daemon-specific functionality.
 */

#ifndef ROSETTAD_H
#define ROSETTAD_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sched.h>

/* Include the master Rosetta header - this includes all module headers */
#include "rosetta_refactored.h"

/* Use existing types from rosetta_refactored.h */
/* ThreadState is already defined in rosetta_types.h */
/* translation_entry_t is already defined in rosetta_refactored.h */

/* Typedef for thread_state_t as alias for ThreadState */
typedef ThreadState thread_state_t;

/* ============================================================================
 * TYPE DEFINITIONS
 * ============================================================================ */

/* Basic types for clarity (if not already defined) */
#ifndef _U8_DEF
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;
typedef int64_t   s64;
#define _U8_DEF
#endif

/* Daemon state flags */
#define ROSETTAD_FLAG_RUNNING      0x0001
#define ROSETTAD_FLAG_DEBUG        0x0002
#define ROSETTAD_FLAG_LOGGING      0x0004
#define ROSETTAD_FLAG_SUPERVISOR   0x0008
#define ROSETTAD_FLAG_SIGNALS_INIT 0x0010

/* IPC channel types */
typedef enum {
    IPC_CHANNEL_CMD,      /* Command channel */
    IPC_CHANNEL_STATUS,   /* Status channel */
    IPC_CHANNEL_LOG,      /* Logging channel */
    IPC_CHANNEL_MAX
} rosettad_ipc_channel_t;

/* Daemon configuration */
typedef struct {
    u32     flags;
    u32     log_level;
    s32     log_fd;
    s32     cmd_fd;
    s32     status_fd;
    u64     guest_base;
    u64     host_base;
    size_t  memory_size;
    s32     translation_mode;
} rosettad_config_t;

/* Daemon state */
typedef struct {
    rosettad_config_t  config;
    thread_state_t    *thread_state;
    void              *memory_base;
    size_t             memory_size;
    u64                entry_point;
    s32                process_id;
    s32                status;
    u64                syscalls_handled;
    u64                translations_count;
    void              *ipc_channels[IPC_CHANNEL_MAX];
} rosettad_state_t;

/* Syscall handler function type */
typedef s64 (*rosettad_syscall_handler_t)(rosettad_state_t *state, thread_state_t *ts, s32 nr);

/* Signal handler type */
typedef void (*rosettad_signal_handler_t)(s32 signum, void *info, void *ucontext);

/* ============================================================================
 * DAEMON ENTRY POINT
 * ============================================================================ */

/* Main daemon entry point */
void rosettad_entry(void);

/* Initialize daemon state */
void init_daemon_state(void);

/* Initialize daemon with configuration */
int rosettad_init(rosettad_config_t *config);

/* Run daemon main loop */
int rosettad_run(rosettad_state_t *state);

/* Shutdown daemon */
void rosettad_shutdown(rosettad_state_t *state);

/* ============================================================================
 * PROCESS LIFECYCLE MANAGEMENT
 * ============================================================================ */

/* Initialize process execution environment */
int init_process_environment(rosettad_state_t *state, int argc, char **argv, char **envp);

/* Set up process memory mappings */
int setup_process_memory(rosettad_state_t *state, u64 guest_base, size_t size);

/* Clean up process resources */
void cleanup_process_resources(rosettad_state_t *state);

/* Get current process ID */
s32 rosettad_getpid(rosettad_state_t *state);

/* Get current thread ID */
s32 rosettad_gettid(rosettad_state_t *state);

/* ============================================================================
 * SUPERVISOR MODE / SYSCALL HANDLING
 * ============================================================================ */

/* Enter supervisor mode - main syscall handling loop */
noreturn void enter_supervisor_mode(rosettad_state_t *state);

/* Handle trapped syscall from guest */
s64 handle_guest_syscall(rosettad_state_t *state, ThreadState *ts, s32 nr);

/* Translate guest syscall to host */
s32 translate_syscall_to_host(s32 guest_nr, s32 *host_nr);

/* Execute host syscall */
s64 execute_host_syscall(s32 host_nr, u64 *args);

/* Return syscall result to guest */
void return_syscall_to_guest(ThreadState *ts, s64 result);

/* ============================================================================
 * SIGNAL HANDLING
 * ============================================================================ */

/* Set up daemon signal handlers */
int setup_signal_handlers(rosettad_state_t *state);

/* Handle SIGSEGV - memory access violations */
void handle_sigsegv(s32 signum, siginfo_t *info, void *ucontext);

/* Handle SIGILL - illegal instructions */
void handle_sigill(s32 signum, siginfo_t *info, void *ucontext);

/* Handle SIGBUS - bus errors */
void handle_sigbus(s32 signum, siginfo_t *info, void *ucontext);

/* Handle SIGABRT - aborts */
void handle_sigabrt(s32 signum, siginfo_t *info, void *ucontext);

/* Send signal to guest process */
int send_signal_to_guest(rosettad_state_t *state, s32 signum);

/* ============================================================================
 * IPC COMMUNICATION
 * ============================================================================ */

/* Initialize IPC channels */
int init_ipc_channels(rosettad_state_t *state);

/* Create shared memory region for IPC */
void *create_ipc_shared_memory(size_t size, s32 *fd);

/* Send message via IPC */
int ipc_send(rosettad_state_t *state, rosettad_ipc_channel_t channel, const void *msg, size_t len);

/* Receive message via IPC */
int ipc_recv(rosettad_state_t *state, rosettad_ipc_channel_t channel, void *msg, size_t *len);

/* Close IPC channels */
void close_ipc_channels(rosettad_state_t *state);

/* ============================================================================
 * LOGGING
 * ============================================================================ */

/* Initialize logging */
int init_logging(rosettad_state_t *state, s32 log_fd, u32 log_level);

/* Log message */
void rosettad_log(rosettad_state_t *state, u32 level, const char *fmt, ...);

/* Log syscall */
void log_syscall(rosettad_state_t *state, s32 nr, u64 *args);

/* Log translation event */
void log_translation(rosettad_state_t *state, u64 guest_pc, u64 host_pc, u32 size);

/* Close logging */
void close_logging(rosettad_state_t *state);

/* ============================================================================
 * MEMORY MANAGEMENT
 * ============================================================================ */

/* Allocate daemon memory */
void *rosettad_malloc(size_t size);

/* Free daemon memory */
void rosettad_free(void *ptr);

/* Map guest memory */
void *map_guest_memory(rosettad_state_t *state, u64 guest_addr, size_t size, s32 prot);

/* Unmap guest memory */
int unmap_guest_memory(rosettad_state_t *state, u64 guest_addr, size_t size);

/* Translate guest address to host */
void *translate_guest_addr(rosettad_state_t *state, u64 guest_addr);

/* ============================================================================
 * CRASH HANDLING
 * ============================================================================ */

/* Handle crashes and exceptions */
noreturn void crash_handler(rosettad_state_t *state, u64 crash_code, u64 crash_addr);

/* Generate crash report */
int generate_crash_report(rosettad_state_t *state, void *report_buf, size_t buf_size);

/* Clean up after crash */
void cleanup_after_crash(rosettad_state_t *state);

/* ============================================================================
 * SIMD STRING/MEMORY OPERATIONS (Shared with rosetta)
 * ============================================================================ */

/* SIMD-optimized memchr */
void *rosettad_memchr_simd(const void *ptr, long max_len);

/* SIMD-optimized strcmp */
int rosettad_strcmp_simd(const char *s1, const char *s2);

/* SIMD-optimized strncmp */
int rosettad_strncmp_simd(const char *s1, const char *s2, size_t n);

/* SIMD-optimized memcmp */
int rosettad_memcmp_simd(const void *s1, const void *s2, size_t n);

/* SIMD-optimized memset */
void *rosettad_memset_simd(void *s, int c, size_t n);

/* SIMD-optimized memcpy */
void *rosettad_memcpy_simd(void *dest, const void *src, size_t n);

/* Byte-by-byte strcmp fallback */
int rosettad_strcmp_bytewise(const char *s1, const char *s2);

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

/* Initialize random number generator */
int init_random(void);

/* Get random bytes */
int get_random_bytes(void *buf, size_t len);

/* Get current time */
u64 get_current_time(void);

/* Get CPU count */
s32 get_cpu_count(void);

/* Check if running under Rosetta */
bool is_running_under_rosetta(void);

/* ============================================================================
 * EXTERNAL DECLARATIONS (from rosetta modules)
 * NOTE: Most extern declarations are already in the included headers
 * (rosetta_exec.h, rosetta_syscalls.h, rosetta_cache.h, etc.)
 * ============================================================================ */

/* From rosetta_syscalls.c - already declared in rosetta_syscalls.h */
extern void syscall_handler_init(void);
extern s64 syscall_dispatch(ThreadState *st);
extern s32 syscall_get_nr(ThreadState *st);

/* From rosetta_runtime.c */
extern void init_runtime_environment(u64 *entry_point, int argc, long argv_envp, long *auxv, long **out_argv);

/* From rosetta_refactored_init.c */
extern void init_translation_env(void **entry_callback);

/* From rosetta_vector.c */
extern u8 neon_uminv(const u8 vec[16]);

/* ============================================================================
 * ERROR CODES
 * ============================================================================ */

#define ROSETTAD_OK                     0
#define ROSETTAD_ERR_INVALID_ARG       -1
#define ROSETTAD_ERR_NO_MEMORY         -2
#define ROSETTAD_ERR_SYSCALL_FAIL      -3
#define ROSETTAD_ERR_IPC_FAIL          -4
#define ROSETTAD_ERR_SIGNAL_FAIL       -5
#define ROSETTAD_ERR_MEMORY_FAIL       -6
#define ROSETTAD_ERR_TRANSLATION_FAIL  -7
#define ROSETTAD_ERR_ALREADY_RUNNING   -8
#define ROSETTAD_ERR_NOT_RUNNING       -9

/* ============================================================================
 * LOG LEVELS
 * ============================================================================ */

#define LOG_LEVEL_ERROR     0
#define LOG_LEVEL_WARNING   1
#define LOG_LEVEL_INFO      2
#define LOG_LEVEL_DEBUG     3
#define LOG_LEVEL_TRACE     4

/* ============================================================================
 * MACROS
 * ============================================================================ */

/* Logging macros */
#define LOG_ERROR(state, fmt, ...)  rosettad_log(state, LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define LOG_WARN(state, fmt, ...)   rosettad_log(state, LOG_LEVEL_WARNING, fmt, ##__VA_ARGS__)
#define LOG_INFO(state, fmt, ...)   rosettad_log(state, LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(state, fmt, ...)  rosettad_log(state, LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_TRACE(state, fmt, ...)  rosettad_log(state, LOG_LEVEL_TRACE, fmt, ##__VA_ARGS__)

/* Check if flag is set */
#define ROSETTAD_FLAG_SET(state, flag)  ((state)->config.flags & (flag))

/* Set flag */
#define ROSETTAD_FLAG_SET_FLAG(state, flag)  do { (state)->config.flags |= (flag); } while (0)

/* Clear flag */
#define ROSETTAD_FLAG_CLEAR(state, flag)  do { (state)->config.flags &= ~(flag); } while (0)

#endif /* ROSETTAD_H */
