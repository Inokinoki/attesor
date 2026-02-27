#ifndef ROSETTA_SYSCALLS_H
#define ROSETTA_SYSCALLS_H

/* ============================================================================
 * Rosetta Binary Translator - Syscall Translation Layer
 * ============================================================================
 *
 * This module handles ARM64 to x86_64 syscall translation including:
 * - Syscall number mapping (ARM64 Linux -> x86_64 Linux)
 * - Argument register remapping
 * - Syscall handler dispatch
 *
 * ============================================================================ */

#include "rosetta_types.h"

/* Define noreturn for C99 compatibility */
#ifndef noreturn
#define noreturn _Noreturn
#endif

/* ============================================================================
 * Syscall Number Mapping (ARM64 -> x86_64)
 * ============================================================================ */

/* ARM64 syscall numbers (Linux ABI) */
#define ARM64_SYS_READ          63
#define ARM64_SYS_WRITE         64
#define ARM64_SYS_OPEN          56
#define ARM64_SYS_CLOSE         57
#define ARM64_SYS_STAT          79
#define ARM64_SYS_FSTAT         80
#define ARM64_SYS_LSTAT         81
#define ARM64_SYS_POLL          166
#define ARM64_SYS_LSEEK         62
#define ARM64_SYS_MMAP          222
#define ARM64_SYS_MPROTECT      226
#define ARM64_SYS_MUNMAP        215
#define ARM64_SYS_BRK           214
#define ARM64_SYS_IOCTL         29
#define ARM64_SYS_ACCESS        48
#define ARM64_SYS_PIPE          40
#define ARM64_SYS_DUP           23
#define ARM64_SYS_DUP2          24
#define ARM64_SYS_DUP3          24  /* Same as dup2 on ARM64 */
#define ARM64_SYS_PAUSE         179
#define ARM64_SYS_NANOSLEEP     35
#define ARM64_SYS_GETPID        177
#define ARM64_SYS_GETUID        174
#define ARM64_SYS_GETEUID       175
#define ARM64_SYS_GETGID        176
#define ARM64_SYS_GETEGID       177
#define ARM64_SYS_GETTID        178
#define ARM64_SYS_UNAME         160
#define ARM64_SYS_FCNTL         25
#define ARM64_SYS_FLOCK         73
#define ARM64_SYS_FSYNC         82
#define ARM64_SYS_FDATASYNC     83
#define ARM64_SYS_GETCWD        17
#define ARM64_SYS_CHDIR         49
#define ARM64_SYS_RENAME        82
#define ARM64_SYS_MKDIR         83
#define ARM64_SYS_RMDIR         84
#define ARM64_SYS_CREAT         85
#define ARM64_SYS_LINK          86
#define ARM64_SYS_UNLINK        87
#define ARM64_SYS_SYMLINK       88
#define ARM64_SYS_READLINK      89
#define ARM64_SYS_CHMOD         91
#define ARM64_SYS_FCHMOD        92
#define ARM64_SYS_CHOWN         93
#define ARM64_SYS_FCHOWN        94
#define ARM64_SYS_LCHOWN        95
#define ARM64_SYS_UMASK         96
#define ARM64_SYS_GETTIMEOFDAY  169
#define ARM64_SYS_GETRLIMIT     160
#define ARM64_SYS_GETRUSAGE     165
#define ARM64_SYS_TIMES         153
#define ARM64_SYS_SYSINFO       179
#define ARM64_SYS_EXIT          93
#define ARM64_SYS_EXIT_GROUP    94
#define ARM64_SYS_WAIT4         260
#define ARM64_SYS_KILL          129
#define ARM64_SYS_CLONE         220
#define ARM64_SYS_EXECVE        221
#define ARM64_SYS_MADVISE       233
#define ARM64_SYS_RT_SIGACTION  134
#define ARM64_SYS_RT_SIGPROCMASK 135
#define ARM64_SYS_RT_SIGRETURN  139
#define ARM64_SYS_SET_TID_ADDRESS 96
#define ARM64_SYS_FUTEX         98
#define ARM64_SYS_SET_ROBUST_LIST 99
#define ARM64_SYS_GET_ROBUST_LIST 100
#define ARM64_SYS_CLOCK_GETTIME 228
#define ARM64_SYS_CLOCK_GETRES  113
#define ARM64_SYS_CLOCK_NANOSLEEP 115
#define ARM64_SYS_TIMERFD_CREATE 85
#define ARM64_SYS_EPOLL_CREATE1 24
#define ARM64_SYS_EPOLL_CTL     21
#define ARM64_SYS_EPOLL_PWAIT   69
#define ARM64_SYS_EVENTFD2      19
#define ARM64_SYS_SIGNALFD4     74
#define ARM64_SYS_ACCEPT4       242
#define ARM64_SYS_READV         65
#define ARM64_SYS_WRITEV        66
#define ARM64_SYS_SOCKET        198
#define ARM64_SYS_CONNECT       203
#define ARM64_SYS_SENDTO        207
#define ARM64_SYS_RECVFROM      208
#define ARM64_SYS_GETSOCKOPT    209
#define ARM64_SYS_SETSOCKOPT    210
#define ARM64_SYS_SCHED_YIELD   124
#define ARM64_SYS_ARCH_PRCTL    160

/* x86_64 syscall numbers (Linux ABI) */
#define X86_64_SYS_READ         0
#define X86_64_SYS_WRITE        1
#define X86_64_SYS_OPEN         2
#define X86_64_SYS_CLOSE        3
#define X86_64_SYS_STAT         4
#define X86_64_SYS_FSTAT        5
#define X86_64_SYS_LSTAT        6
#define X86_64_SYS_POLL         7
#define X86_64_SYS_LSEEK        8
#define X86_64_SYS_MMAP         9
#define X86_64_SYS_MPROTECT     10
#define X86_64_SYS_MUNMAP       11
#define X86_64_SYS_BRK          12
#define X86_64_SYS_IOCTL        16
#define X86_64_SYS_ACCESS       21
#define X86_64_SYS_PIPE         22
#define X86_64_SYS_DUP          32
#define X86_64_SYS_DUP2         33
#define X86_64_SYS_NANOSLEEP    35
#define X86_64_SYS_GETPID       39
#define X86_64_SYS_UNAME        63
#define X86_64_SYS_FCNTL        72
#define X86_64_SYS_GETCWD       79
#define X86_64_SYS_CHDIR        80
#define X86_64_SYS_GETUID       102
#define X86_64_SYS_GETTID       186
#define X86_64_SYS_EXIT         60
#define X86_64_SYS_EXIT_GROUP   231
#define X86_64_SYS_WAIT4        61
#define X86_64_SYS_KILL         62
#define X86_64_SYS_CLONE        56
#define X86_64_SYS_EXECVE       59
#define X86_64_SYS_RT_SIGACTION 13
#define X86_64_SYS_RT_SIGPROCMASK 14
#define X86_64_SYS_SET_TID_ADDRESS 218
#define X86_64_SYS_FUTEX        202
#define X86_64_SYS_CLOCK_GETTIME 228
#define X86_64_SYS_EPOLL_CREATE1 291
#define X86_64_SYS_EPOLL_CTL    233
#define X86_64_SYS_EPOLL_PWAIT  281
#define X86_64_SYS_READV        19
#define X86_64_SYS_WRITEV       20
#define X86_64_SYS_SCHED_YIELD  24
#define X86_64_SYS_ARCH_PRCTL   158
#define X86_64_SYS_SOCKET       41
#define X86_64_SYS_CONNECT      42
#define X86_64_SYS_SENDTO       44
#define X86_64_SYS_RECVFROM     45
#define X86_64_SYS_GETTIMEOFDAY 78

/* ============================================================================
 * Syscall Handler Function Types
 * ============================================================================ */

/**
 * Syscall handler function type
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
typedef int (*syscall_handler_t)(ThreadState *state);

/* ============================================================================
 * Syscall Mapping Structure
 * ============================================================================ */

/**
 * Syscall mapping entry
 */
typedef struct {
    int arm64_nr;           /* ARM64 syscall number */
    int x86_64_nr;          /* x86_64 syscall number */
    syscall_handler_t handler;  /* Handler function */
} SyscallEntry;

/* ============================================================================
 * Basic I/O Syscall Handlers
 * ============================================================================ */

int syscall_read(ThreadState *state);
int syscall_write(ThreadState *state);
int syscall_open(ThreadState *state);
int syscall_close(ThreadState *state);
int syscall_lseek(ThreadState *state);
int syscall_access(ThreadState *state);
int syscall_pipe(ThreadState *state);
int syscall_dup2(ThreadState *state);
int syscall_dup3(ThreadState *state);

/* ============================================================================
 * Memory Syscall Handlers
 * ============================================================================ */

int syscall_mmap(ThreadState *state);
int syscall_munmap(ThreadState *state);
int syscall_mprotect(ThreadState *state);
int syscall_brk(ThreadState *state);

/* ============================================================================
 * File Status Syscall Handlers
 * ============================================================================ */

int syscall_stat(ThreadState *state);
int syscall_fstat(ThreadState *state);
int syscall_lstat(ThreadState *state);

/* ============================================================================
 * Process Syscall Handlers
 * ============================================================================ */

int syscall_getpid(ThreadState *state);
int syscall_gettid(ThreadState *state);
int syscall_uname(ThreadState *state);
int syscall_fcntl(ThreadState *state);
int syscall_set_tid_address(ThreadState *state);
noreturn int syscall_exit(ThreadState *state);
noreturn int syscall_exit_group(ThreadState *state);

/* ============================================================================
 * Time Syscall Handlers
 * ============================================================================ */

int syscall_gettimeofday(ThreadState *state);
int syscall_clock_gettime(ThreadState *state);
int syscall_nanosleep(ThreadState *state);

/* ============================================================================
 * Signal Syscall Handlers
 * ============================================================================ */

int syscall_rt_sigaction(ThreadState *state);
int syscall_rt_sigprocmask(ThreadState *state);
int syscall_sched_yield(ThreadState *state);

/* ============================================================================
 * IPC/Sync Syscall Handlers
 * ============================================================================ */

int syscall_futex(ThreadState *state);
int syscall_arch_prctl(ThreadState *state);

/* ============================================================================
 * Network Syscall Handlers
 * ============================================================================ */

int syscall_socket(ThreadState *state);
int syscall_connect(ThreadState *state);
int syscall_sendto(ThreadState *state);
int syscall_recvfrom(ThreadState *state);
int syscall_epoll_create(ThreadState *state);
int syscall_epoll_ctl(ThreadState *state);
int syscall_epoll_wait(ThreadState *state);

/* ============================================================================
 * Additional Syscall Handlers
 * ============================================================================ */

int syscall_ioctl(ThreadState *state);
int syscall_readv(ThreadState *state);
int syscall_writev(ThreadState *state);
int syscall_getcwd(ThreadState *state);
int syscall_chdir(ThreadState *state);
int syscall_kill(ThreadState *state);
int syscall_wait4(ThreadState *state);

/* ============================================================================
 * Syscall Translation Functions
 * ============================================================================ */

/**
 * Translate ARM64 syscall number to x86_64
 * @param arm64_nr ARM64 syscall number
 * @return x86_64 syscall number, or -1 if unknown
 */
int translate_syscall_number(int arm64_nr);

/**
 * Get syscall handler for ARM64 syscall
 * @param arm64_nr ARM64 syscall number
 * @return Handler function, or NULL if no handler
 */
syscall_handler_t get_syscall_handler(int arm64_nr);

/**
 * Dispatch syscall to appropriate handler
 * @param state Thread state
 * @param syscall_nr ARM64 syscall number
 * @return 0 on success, negative errno on failure
 */
int dispatch_syscall(ThreadState *state, int syscall_nr);

/**
 * Initialize syscall table
 */
void init_syscall_table(void);

/**
 * Remap ARM64 syscall arguments to x86_64 calling convention
 * @param state Thread state
 *
 * ARM64: x0, x1, x2, x3, x4, x5
 * x86_64: rdi, rsi, rdx, rcx, r8, r9
 */
void remap_syscall_args(ThreadState *state);

#endif /* ROSETTA_SYSCALLS_H */
