#ifndef ROSETTA_SYSCALLS_H
#define ROSETTA_SYSCALLS_H

/* ============================================================================
 * Rosetta Binary Translator - Syscall Translation Layer
 * ============================================================================
 *
 * This module handles x86_64 to ARM64 syscall translation including:
 * - Syscall number mapping (x86_64 Linux -> ARM64 Linux)
 * - Argument register remapping (x86_64 -> ARM64)
 * - Syscall handler dispatch
 *
 * Translation direction: x86_64 guest binary running on ARM64 host
 * (Same as Apple's Rosetta 2)
 *
 * ============================================================================ */

#include "rosetta_types.h"

/* Define noreturn for C99 compatibility */
#ifndef noreturn
#define noreturn _Noreturn
#endif

/* ============================================================================
 * Syscall Number Mapping (x86_64 -> ARM64)
 * ============================================================================ */

/* x86_64 syscall numbers (Linux ABI) - Guest */
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
#define X86_64_SYS_DUP3         292
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
#define X86_64_SYS_CONNECT      42
#define X86_64_SYS_SENDTO       44
#define X86_64_SYS_RECVFROM     45
#define X86_64_SYS_SOCKET       41
#define X86_64_SYS_GETTIMEOFDAY 96
#define X86_64_SYS_GETRLIMIT    97
#define X86_64_SYS_GETRUSAGE    98
#define X86_64_SYS_TIMES        100
#define X86_64_SYS_SYSINFO      99
#define X86_64_SYS_MADVISE      28
#define X86_64_SYS_EVENTFD2     290
#define X86_64_SYS_SIGNALFD4    289
#define X86_64_SYS_ACCEPT4      288
#define X86_64_SYS_GETSOCKOPT   55
#define X86_64_SYS_SETSOCKOPT   54
#define X86_64_SYS_GETEGID      108
#define X86_64_SYS_GETEUID      107
#define X86_64_SYS_GETGID       104
#define X86_64_SYS_FLOCK        73
#define X86_64_SYS_FSYNC        74
#define X86_64_SYS_FDATASYNC    75
#define X86_64_SYS_RENAME       82
#define X86_64_SYS_MKDIR        83
#define X86_64_SYS_RMDIR        84
#define X86_64_SYS_CREAT        85
#define X86_64_SYS_LINK         86
#define X86_64_SYS_UNLINK       87
#define X86_64_SYS_SYMLINK      88
#define X86_64_SYS_READLINK     89
#define X86_64_SYS_CHMOD        90
#define X86_64_SYS_FCHMOD       91
#define X86_64_SYS_CHOWN        92
#define X86_64_SYS_FCHOWN       93
#define X86_64_SYS_LCHOWN       94
#define X86_64_SYS_UMASK        95
#define X86_64_SYS_GETPGID      121
#define X86_64_SYS_GETSID       124
#define X86_64_SYS_SETSID       112
#define X86_64_SYS_GETGROUPS    115
#define X86_64_SYS_SETGROUPS    116
#define X86_64_SYS_SETHOSTNAME  170
#define X86_64_SYS_SETDOMAINNAME 171
#define X86_64_SYS_PRCTL        157
#define X86_64_SYS_STATFS       137
#define X86_64_SYS_FSTATFS      138

/* *at family syscalls (directory FD variants) */
#define X86_64_SYS_OPENAT       257
#define X86_64_SYS_MKDIRAT      258
#define X86_64_SYS_MKNODAT      259
#define X86_64_SYS_FCHOWNAT     260
#define X86_64_SYS_FUTIMESAT    261
#define X86_64_SYS_NEWFSTATAT   262
#define X86_64_SYS_UNLINKAT     263
#define X86_64_SYS_RENAMEAT     264
#define X86_64_SYS_LINKAT       265
#define X86_64_SYS_SYMLINKAT    266
#define X86_64_SYS_READLINKAT   267
#define X86_64_SYS_FCHMODAT     268
#define X86_64_SYS_FACCESSAT    269
#define X86_64_SYS_UTIMENSAT    280

#define X86_64_SYS_SET_ROBUST_LIST 273
#define X86_64_SYS_GET_ROBUST_LIST 274
#define X86_64_SYS_CLOCK_GETRES 229
#define X86_64_SYS_CLOCK_NANOSLEEP 287
#define X86_64_SYS_TIMERFD_CREATE 283
#define X86_64_SYS_PAUSE         34
#define X86_64_SYS_GETUID        102
#define X86_64_SYS_GETEUID       107
#define X86_64_SYS_GETGID        104
#define X86_64_SYS_GETEGID       108
#define X86_64_SYS_GETRLIMIT     97
#define X86_64_SYS_GETRUSAGE     98
#define X86_64_SYS_TIMES         100
#define X86_64_SYS_SYSINFO       99
#define X86_64_SYS_MINCORE       232
#define X86_64_SYS_MLOCK         149
#define X86_64_SYS_MUNLOCK       150
#define X86_64_SYS_MSYNC         26
#define X86_64_SYS_PRLIMIT       309
#define X86_64_SYS_GETCPU        204

/* ARM64 syscall numbers (Linux ABI) - Host */
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
#define ARM64_SYS_DUP3          24
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
#define ARM64_SYS_GETPGID       155
#define ARM64_SYS_GETSID        156
#define ARM64_SYS_SETSID        157
#define ARM64_SYS_GETGROUPS     158
#define ARM64_SYS_SETGROUPS     159
#define ARM64_SYS_SETHOSTNAME   161
#define ARM64_SYS_SETDOMAINNAME 162
#define ARM64_SYS_PRCTL         167
#define ARM64_SYS_STATFS        43
#define ARM64_SYS_FSTATFS       44

/* *at family syscalls (directory FD variants) */
#define ARM64_SYS_OPENAT        56
#define ARM64_SYS_MKDIRAT       34
#define ARM64_SYS_MKNODAT       33
#define ARM64_SYS_FCHOWNAT      204
#define ARM64_SYS_NEWFSTATAT    79
#define ARM64_SYS_UNLINKAT      35
#define ARM64_SYS_RENAMEAT      35
#define ARM64_SYS_LINKAT        37
#define ARM64_SYS_SYMLINKAT     38
#define ARM64_SYS_READLINKAT    78
#define ARM64_SYS_FCHMODAT      53
#define ARM64_SYS_FACCESSAT     48
#define ARM64_SYS_UTIMENSAT     88

#define ARM64_SYS_MINCORE       232
#define ARM64_SYS_MLOCK         228
#define ARM64_SYS_MUNLOCK       229
#define ARM64_SYS_MSYNC         227
#define ARM64_SYS_PRLIMIT       261
#define ARM64_SYS_GETCPU        168

/* ============================================================================
 * Syscall Handler Function Types
 * ============================================================================ */

/**
 * Syscall handler function type
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
typedef int (*syscall_handler_t)(ThreadState *state);

/* Forward declaration for syscall handler function */
int syscall_dup(ThreadState *state);

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

/* File Operations (Priority 1) */
int syscall_rename(ThreadState *state);
int syscall_mkdir(ThreadState *state);
int syscall_rmdir(ThreadState *state);
int syscall_unlink(ThreadState *state);
int syscall_symlink(ThreadState *state);
int syscall_readlink(ThreadState *state);
int syscall_chmod(ThreadState *state);
int syscall_fchmod(ThreadState *state);
int syscall_chown(ThreadState *state);
int syscall_fchown(ThreadState *state);
int syscall_lchown(ThreadState *state);
int syscall_creat(ThreadState *state);
int syscall_link(ThreadState *state);
int syscall_umask(ThreadState *state);
int syscall_flock(ThreadState *state);
int syscall_fsync(ThreadState *state);
int syscall_fdatasync(ThreadState *state);
int syscall_poll(ThreadState *state);
int syscall_dup(ThreadState *state);

/* Filesystem Statistics */
int syscall_statfs(ThreadState *state);
int syscall_fstatfs(ThreadState *state);

/* *at Family Syscalls (directory FD variants) */
int syscall_openat(ThreadState *state);
int syscall_mkdirat(ThreadState *state);
int syscall_mknodat(ThreadState *state);
int syscall_fchownat(ThreadState *state);
int syscall_futimesat(ThreadState *state);
int syscall_newfstatat(ThreadState *state);
int syscall_unlinkat(ThreadState *state);
int syscall_renameat(ThreadState *state);
int syscall_linkat(ThreadState *state);
int syscall_symlinkat(ThreadState *state);
int syscall_readlinkat(ThreadState *state);
int syscall_fchmodat(ThreadState *state);
int syscall_faccessat(ThreadState *state);
int syscall_utimensat(ThreadState *state);

/* Process Management (Priority 2) */
int syscall_getuid(ThreadState *state);
int syscall_geteuid(ThreadState *state);
int syscall_getgid(ThreadState *state);
int syscall_getegid(ThreadState *state);
int syscall_setpgid(ThreadState *state);
int syscall_getpgid(ThreadState *state);
int syscall_setsid(ThreadState *state);
int syscall_getsid(ThreadState *state);
int syscall_getgroups(ThreadState *state);
int syscall_setgroups(ThreadState *state);
int syscall_sethostname(ThreadState *state);
int syscall_setdomainname(ThreadState *state);
int syscall_prctl(ThreadState *state);

/* Memory Operations (Priority 3) */
int syscall_madvise(ThreadState *state);
int syscall_mincore(ThreadState *state);
int syscall_mlock(ThreadState *state);
int syscall_munlock(ThreadState *state);
int syscall_msync(ThreadState *state);
int syscall_prlimit(ThreadState *state);
int syscall_getrlimit(ThreadState *state);
int syscall_getrusage(ThreadState *state);
int syscall_times(ThreadState *state);
int syscall_sysinfo(ThreadState *state);

/* Additional Syscalls */
int syscall_pause(ThreadState *state);
int syscall_getcpu(ThreadState *state);
int syscall_clock_getres(ThreadState *state);
int syscall_set_robust_list(ThreadState *state);
int syscall_get_robust_list(ThreadState *state);
int syscall_eventfd2(ThreadState *state);
int syscall_signalfd4(ThreadState *state);
int syscall_accept4(ThreadState *state);
int syscall_getsockopt(ThreadState *state);
int syscall_setsockopt(ThreadState *state);
int syscall_timerfd_create(ThreadState *state);
int syscall_clock_nanosleep(ThreadState *state);
int syscall_clone(ThreadState *state);
int syscall_execve(ThreadState *state);
int syscall_epoll_pwait(ThreadState *state);

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
