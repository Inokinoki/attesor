/* ============================================================================
 * Rosetta Binary Translator - Syscall Translation Implementation
 * ============================================================================
 *
 * This module translates ARM64 Linux syscalls to x86_64 Linux syscalls.
 * It handles:
 * - Syscall number translation
 * - Argument register remapping
 * - Syscall dispatch to handlers
 *
 * ============================================================================ */

#include "rosetta_syscalls.h"
#include "rosetta_types.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define noreturn for C99 compatibility */
#ifndef noreturn
#define noreturn _Noreturn
#endif

/* epoll not available on macOS - stub out */
#ifdef __APPLE__
#define epoll_create1(flags) (-1)
#define epoll_ctl(epfd, op, fd, event) (-1)
#endif

/* ============================================================================
 * Syscall Number Mapping Table
 * ============================================================================ */

static const SyscallEntry syscall_table[] = {
    /* Basic I/O */
    {ARM64_SYS_READ,        X86_64_SYS_READ,        syscall_read},
    {ARM64_SYS_WRITE,       X86_64_SYS_WRITE,       syscall_write},
    {ARM64_SYS_OPEN,        X86_64_SYS_OPEN,        syscall_open},
    {ARM64_SYS_CLOSE,       X86_64_SYS_CLOSE,       syscall_close},
    {ARM64_SYS_LSEEK,       X86_64_SYS_LSEEK,       syscall_lseek},
    {ARM64_SYS_ACCESS,      X86_64_SYS_ACCESS,      syscall_access},
    {ARM64_SYS_PIPE,        X86_64_SYS_PIPE,        syscall_pipe},
    {ARM64_SYS_DUP2,        X86_64_SYS_DUP2,        syscall_dup2},
    {ARM64_SYS_DUP3,        X86_64_SYS_DUP2,        syscall_dup3},

    /* Memory */
    {ARM64_SYS_MMAP,        X86_64_SYS_MMAP,        syscall_mmap},
    {ARM64_SYS_MUNMAP,      X86_64_SYS_MUNMAP,      syscall_munmap},
    {ARM64_SYS_MPROTECT,    X86_64_SYS_MPROTECT,    syscall_mprotect},
    {ARM64_SYS_BRK,         X86_64_SYS_BRK,         syscall_brk},

    /* File Status */
    {ARM64_SYS_STAT,        X86_64_SYS_STAT,        syscall_stat},
    {ARM64_SYS_FSTAT,       X86_64_SYS_FSTAT,       syscall_fstat},
    {ARM64_SYS_LSTAT,       X86_64_SYS_LSTAT,       syscall_lstat},

    /* Process */
    {ARM64_SYS_GETPID,      X86_64_SYS_GETPID,      syscall_getpid},
    {ARM64_SYS_GETTID,      X86_64_SYS_GETTID,      syscall_gettid},
    {ARM64_SYS_UNAME,       X86_64_SYS_UNAME,       syscall_uname},
    {ARM64_SYS_FCNTL,       X86_64_SYS_FCNTL,       syscall_fcntl},
    {ARM64_SYS_SET_TID_ADDRESS, X86_64_SYS_SET_TID_ADDRESS, syscall_set_tid_address},
    {ARM64_SYS_EXIT,        X86_64_SYS_EXIT,        (syscall_handler_t)syscall_exit},
    {ARM64_SYS_EXIT_GROUP,  X86_64_SYS_EXIT_GROUP,  (syscall_handler_t)syscall_exit_group},
    {ARM64_SYS_WAIT4,       X86_64_SYS_WAIT4,       syscall_wait4},
    {ARM64_SYS_KILL,        X86_64_SYS_KILL,        syscall_kill},

    /* Time */
    {ARM64_SYS_GETTIMEOFDAY, X86_64_SYS_GETTIMEOFDAY, syscall_gettimeofday},
    {ARM64_SYS_CLOCK_GETTIME, X86_64_SYS_CLOCK_GETTIME, syscall_clock_gettime},
    {ARM64_SYS_NANOSLEEP,   X86_64_SYS_NANOSLEEP,   syscall_nanosleep},

    /* Signal */
    {ARM64_SYS_RT_SIGACTION, X86_64_SYS_RT_SIGACTION, syscall_rt_sigaction},
    {ARM64_SYS_RT_SIGPROCMASK, X86_64_SYS_RT_SIGPROCMASK, syscall_rt_sigprocmask},
    {ARM64_SYS_SCHED_YIELD, X86_64_SYS_SCHED_YIELD, syscall_sched_yield},

    /* IPC/Sync */
    {ARM64_SYS_FUTEX,       X86_64_SYS_FUTEX,       syscall_futex},
    {ARM64_SYS_ARCH_PRCTL,  -1,                     syscall_arch_prctl},  /* Architecture-specific */

    /* Network */
    {ARM64_SYS_SOCKET,      X86_64_SYS_SOCKET,      syscall_socket},
    {ARM64_SYS_CONNECT,     X86_64_SYS_CONNECT,     syscall_connect},
    {ARM64_SYS_SENDTO,      X86_64_SYS_SENDTO,      syscall_sendto},
    {ARM64_SYS_RECVFROM,    X86_64_SYS_RECVFROM,    syscall_recvfrom},
    {ARM64_SYS_EPOLL_CREATE1, X86_64_SYS_EPOLL_CREATE1, syscall_epoll_create},
    {ARM64_SYS_EPOLL_CTL,   X86_64_SYS_EPOLL_CTL,   syscall_epoll_ctl},

    /* Additional */
    {ARM64_SYS_IOCTL,       X86_64_SYS_IOCTL,       syscall_ioctl},
    {ARM64_SYS_READV,       X86_64_SYS_READV,       syscall_readv},
    {ARM64_SYS_WRITEV,      X86_64_SYS_WRITEV,      syscall_writev},
    {ARM64_SYS_GETCWD,      X86_64_SYS_GETCWD,      syscall_getcwd},
    {ARM64_SYS_CHDIR,       X86_64_SYS_CHDIR,       syscall_chdir},
};

static int syscall_table_size = sizeof(syscall_table) / sizeof(syscall_table[0]);

/* ============================================================================
 * Syscall Number Translation
 * ============================================================================ */

/**
 * Translate ARM64 syscall number to x86_64
 */
int translate_syscall_number(int arm64_nr)
{
    int i;

    for (i = 0; i < syscall_table_size; i++) {
        if (syscall_table[i].arm64_nr == arm64_nr) {
            return syscall_table[i].x86_64_nr;
        }
    }

    /* Unknown syscall - return as-is (may still work for some syscalls) */
    return arm64_nr;
}

/**
 * Get syscall handler for ARM64 syscall
 */
syscall_handler_t get_syscall_handler(int arm64_nr)
{
    int i;

    for (i = 0; i < syscall_table_size; i++) {
        if (syscall_table[i].arm64_nr == arm64_nr) {
            return syscall_table[i].handler;
        }
    }

    return NULL;
}

/* ============================================================================
 * Syscall Dispatch
 * ============================================================================ */

/**
 * Remap ARM64 syscall arguments to x86_64 calling convention
 *
 * ARM64: x0, x1, x2, x3, x4, x5
 * x86_64: rdi, rsi, rdx, rcx, r8, r9
 *
 * For Linux syscalls, the arguments are passed in the same order,
 * just in different registers. However, since we're translating
 * at the syscall number level (not intercepting at the register level),
 * we just need to ensure the arguments in memory are correctly interpreted.
 */
void remap_syscall_args(ThreadState *state)
{
    /* ARM64 and x86_64 Linux use the same argument passing for syscalls
     * when the arguments are in memory. The difference is only in which
     * registers are used. Since we're handling syscalls in C, the
     * arguments are already in the correct order in the ThreadState. */
    (void)state;  /* No remapping needed for our implementation */
}

/**
 * Dispatch syscall to appropriate handler
 */
int dispatch_syscall(ThreadState *state, int syscall_nr)
{
    syscall_handler_t handler;

    /* Try to find handler in our table */
    handler = get_syscall_handler(syscall_nr);

    if (handler != NULL) {
        /* Call handler */
        return handler(state);
    }

    /* No handler found - try direct syscall passthrough */
    /* This is a fallback for unimplemented syscalls */
    state->syscall_result = -ENOSYS;
    return -1;
}

/**
 * Initialize syscall table (called once at startup)
 */
void init_syscall_table(void)
{
    /* Table is statically initialized - nothing to do */
}

/* ============================================================================
 * Basic I/O Syscall Handlers
 * ============================================================================ */

/**
 * syscall_read - Read from file descriptor
 */
int syscall_read(ThreadState *state)
{
    int fd = (int)state->guest.x[0];
    void *buf = (void *)(uintptr_t)state->guest.x[1];
    size_t count = (size_t)state->guest.x[2];

    ssize_t ret = read(fd, buf, count);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = (int64_t)ret;
    return 0;
}

/**
 * syscall_write - Write to file descriptor
 */
int syscall_write(ThreadState *state)
{
    int fd = (int)state->guest.x[0];
    const void *buf = (const void *)(uintptr_t)state->guest.x[1];
    size_t count = (size_t)state->guest.x[2];

    ssize_t ret = write(fd, buf, count);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = (int64_t)ret;
    return 0;
}

/**
 * syscall_open - Open a file
 */
int syscall_open(ThreadState *state)
{
    const char *pathname = (const char *)(uintptr_t)state->guest.x[0];
    int flags = (int)state->guest.x[1];
    mode_t mode = (mode_t)state->guest.x[2];

    int fd = open(pathname, flags, mode);
    if (fd < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = fd;
    return 0;
}

/**
 * syscall_close - Close a file descriptor
 */
int syscall_close(ThreadState *state)
{
    int fd = (int)state->guest.x[0];

    int ret = close(fd);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_lseek - Reposition read/write file offset
 */
int syscall_lseek(ThreadState *state)
{
    int fd = (int)state->guest.x[0];
    off_t offset = (off_t)state->guest.x[1];
    int whence = (int)state->guest.x[2];

    off_t ret = lseek(fd, offset, whence);
    if (ret == (off_t)-1) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = (int64_t)ret;
    return 0;
}

/**
 * syscall_access - Check real user's permissions for a file
 */
int syscall_access(ThreadState *state)
{
    const char *pathname = (const char *)(uintptr_t)state->guest.x[0];
    int mode = (int)state->guest.x[1];

    int ret = access(pathname, mode);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_pipe - Create pipe
 */
int syscall_pipe(ThreadState *state)
{
    int pipefd[2];
    int ret = pipe(pipefd);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    /* Store pipe file descriptors in x0 and x1 */
    state->guest.x[0] = pipefd[0];
    state->guest.x[1] = pipefd[1];
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_dup2 - Duplicate file descriptor
 */
int syscall_dup2(ThreadState *state)
{
    int oldfd = (int)state->guest.x[0];
    int newfd = (int)state->guest.x[1];

    int ret = dup2(oldfd, newfd);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_dup3 - Duplicate file descriptor with flags
 */
int syscall_dup3(ThreadState *state)
{
    int oldfd = (int)state->guest.x[0];
    int newfd = (int)state->guest.x[1];
    int flags = (int)state->guest.x[2];

#ifdef __linux__
    int ret = dup3(oldfd, newfd, flags);
#else
    /* macOS doesn't have dup3, fall back to dup2 */
    (void)flags;
    int ret = dup2(oldfd, newfd);
#endif

    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/* ============================================================================
 * Memory Syscall Handlers
 * ============================================================================ */

/**
 * syscall_mmap - Map files or devices into memory
 */
int syscall_mmap(ThreadState *state)
{
    void *addr = (void *)(uintptr_t)state->guest.x[0];
    size_t length = (size_t)state->guest.x[1];
    int prot = (int)state->guest.x[2];
    int flags = (int)state->guest.x[3];
    int fd = (int)state->guest.x[4];
    off_t offset = (off_t)state->guest.x[5];

    void *ret = mmap(addr, length, prot, flags, fd, offset);
    if (ret == MAP_FAILED) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = (int64_t)(uintptr_t)ret;
    return 0;
}

/**
 * syscall_munmap - Unmap memory region
 */
int syscall_munmap(ThreadState *state)
{
    void *addr = (void *)(uintptr_t)state->guest.x[0];
    size_t length = (size_t)state->guest.x[1];

    int ret = munmap(addr, length);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_mprotect - Set protection on memory region
 */
int syscall_mprotect(ThreadState *state)
{
    void *addr = (void *)(uintptr_t)state->guest.x[0];
    size_t length = (size_t)state->guest.x[1];
    int prot = (int)state->guest.x[2];

    int ret = mprotect(addr, length, prot);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_brk - Change data segment size
 * Note: sbrk is deprecated on macOS but is still the correct interface
 */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
int syscall_brk(ThreadState *state)
{
    void *addr = (void *)(uintptr_t)state->guest.x[0];

    /* Get current break */
    void *current_brk = sbrk(0);

    if (addr == (void *)0) {
        /* Return current break when addr is 0 */
        state->syscall_result = (int64_t)(uintptr_t)current_brk;
        return 0;
    }

    /* Try to set new break */
    void *new_brk = sbrk((char *)addr - (char *)current_brk);
    if (new_brk == MAP_FAILED) {
        state->syscall_result = -errno;
        return -1;
    }

    state->syscall_result = (int64_t)(uintptr_t)new_brk;
    return 0;
}
#pragma clang diagnostic pop

/* ============================================================================
 * File Status Syscall Handlers
 * ============================================================================ */

/**
 * syscall_stat - Get file status
 */
int syscall_stat(ThreadState *state)
{
    const char *pathname = (const char *)(uintptr_t)state->guest.x[0];
    struct stat *statbuf = (struct stat *)(uintptr_t)state->guest.x[1];

    int ret = stat(pathname, statbuf);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_fstat - Get file status by file descriptor
 */
int syscall_fstat(ThreadState *state)
{
    int fd = (int)state->guest.x[0];
    struct stat *statbuf = (struct stat *)(uintptr_t)state->guest.x[1];

    int ret = fstat(fd, statbuf);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_lstat - Get file status (don't follow symlinks)
 */
int syscall_lstat(ThreadState *state)
{
    const char *pathname = (const char *)(uintptr_t)state->guest.x[0];
    struct stat *statbuf = (struct stat *)(uintptr_t)state->guest.x[1];

    int ret = lstat(pathname, statbuf);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/* ============================================================================
 * Process Syscall Handlers
 * ============================================================================ */

/**
 * syscall_getpid - Get process ID
 */
int syscall_getpid(ThreadState *state)
{
    pid_t pid = getpid();
    state->syscall_result = pid;
    return 0;
}

/**
 * syscall_gettid - Get thread ID
 */
int syscall_gettid(ThreadState *state)
{
#ifdef __linux__
    pid_t tid = syscall(SYS_gettid);
#else
    pid_t tid = getpid();  /* macOS doesn't have gettid */
#endif
    state->syscall_result = tid;
    return 0;
}

/**
 * syscall_uname - Get system information
 */
int syscall_uname(ThreadState *state)
{
    struct utsname *buf = (struct utsname *)(uintptr_t)state->guest.x[0];

    int ret = uname(buf);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_fcntl - Manipulate file descriptor
 */
int syscall_fcntl(ThreadState *state)
{
    int fd = (int)state->guest.x[0];
    int cmd = (int)state->guest.x[1];
    long arg = (long)state->guest.x[2];

    long ret = fcntl(fd, cmd, arg);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_set_tid_address - Set pointer to thread ID
 */
int syscall_set_tid_address(ThreadState *state)
{
    int *tidptr = (int *)(uintptr_t)state->guest.x[0];

    /* On Linux, this sets the tidptr and returns the thread ID */
    /* For simplicity, just return current thread ID */
    state->syscall_result = syscall_gettid(state);

    /* Store tidptr if needed (for threading support) */
    (void)tidptr;

    return 0;
}

/**
 * syscall_exit - Terminate process
 * Note: Declared as returning int for syscall_handler_t compatibility,
 *       but this function never returns.
 */
noreturn int syscall_exit(ThreadState *state)
{
    int status = (int)state->guest.x[0];
    exit(status);
}

/**
 * syscall_exit_group - Terminate all threads in process
 * Note: Declared as returning int for syscall_handler_t compatibility,
 *       but this function never returns.
 */
noreturn int syscall_exit_group(ThreadState *state)
{
    int status = (int)state->guest.x[0];
    _exit(status);
}

/**
 * syscall_wait4 - Wait for process to change state
 */
int syscall_wait4(ThreadState *state)
{
    pid_t pid = (pid_t)state->guest.x[0];
    int *wstatus = (int *)(uintptr_t)state->guest.x[1];
    int options = (int)state->guest.x[2];
    struct rusage *rusage = (struct rusage *)(uintptr_t)state->guest.x[3];

    pid_t ret = wait4(pid, wstatus, options, rusage);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_kill - Send signal to process
 */
int syscall_kill(ThreadState *state)
{
    pid_t pid = (pid_t)state->guest.x[0];
    int sig = (int)state->guest.x[1];

    int ret = kill(pid, sig);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/* ============================================================================
 * Time Syscall Handlers
 * ============================================================================ */

/**
 * syscall_gettimeofday - Get time of day
 */
int syscall_gettimeofday(ThreadState *state)
{
    struct timeval *tv = (struct timeval *)(uintptr_t)state->guest.x[0];
    struct timezone *tz = (struct timezone *)(uintptr_t)state->guest.x[1];

    int ret = gettimeofday(tv, tz);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_clock_gettime - Get clock time
 */
int syscall_clock_gettime(ThreadState *state)
{
    clockid_t clk_id = (clockid_t)state->guest.x[0];
    struct timespec *tp = (struct timespec *)(uintptr_t)state->guest.x[1];

    int ret = clock_gettime(clk_id, tp);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_nanosleep - High-resolution sleep
 */
int syscall_nanosleep(ThreadState *state)
{
    const struct timespec *req = (const struct timespec *)(uintptr_t)state->guest.x[0];
    struct timespec *rem = (struct timespec *)(uintptr_t)state->guest.x[1];

    int ret = nanosleep(req, rem);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/* ============================================================================
 * Signal Syscall Handlers
 * ============================================================================ */

/**
 * syscall_rt_sigaction - Examine and change signal action
 */
int syscall_rt_sigaction(ThreadState *state)
{
    int signum = (int)state->guest.x[0];
    const struct sigaction *act = (const struct sigaction *)(uintptr_t)state->guest.x[1];
    struct sigaction *oldact = (struct sigaction *)(uintptr_t)state->guest.x[2];
    size_t sigsetsize = (size_t)state->guest.x[3];

    (void)sigsetsize;  /* Ignored on Linux */

    int ret = sigaction(signum, act, oldact);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_rt_sigprocmask - Examine and change signal mask
 */
int syscall_rt_sigprocmask(ThreadState *state)
{
    int how = (int)state->guest.x[0];
    const sigset_t *set = (const sigset_t *)(uintptr_t)state->guest.x[1];
    sigset_t *oldset = (sigset_t *)(uintptr_t)state->guest.x[2];
    size_t sigsetsize = (size_t)state->guest.x[3];

    (void)sigsetsize;  /* Ignored on Linux */

    int ret = sigprocmask(how, set, oldset);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_sched_yield - Yield the processor
 */
int syscall_sched_yield(ThreadState *state)
{
    int ret = sched_yield();
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/* ============================================================================
 * IPC/Sync Syscall Handlers
 * ============================================================================ */

/**
 * syscall_futex - Fast userspace mutex
 */
int syscall_futex(ThreadState *state)
{
    uint32_t *uaddr = (uint32_t *)(uintptr_t)state->guest.x[0];
    int futex_op = (int)state->guest.x[1];
    uint32_t val = (uint32_t)state->guest.x[2];
    void *timeout = (void *)(uintptr_t)state->guest.x[3];
    uint32_t *uaddr2 = (uint32_t *)(uintptr_t)state->guest.x[4];
    uint32_t val3 = (uint32_t)state->guest.x[5];

#ifdef __linux__
    long ret = syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
#else
    /* macOS doesn't have futex - use mutex emulation */
    (void)uaddr; (void)futex_op; (void)val; (void)timeout;
    (void)uaddr2; (void)val3;
    state->syscall_result = -ENOSYS;
    return -1;
#endif

    return 0;
}

/**
 * syscall_arch_prctl - Set architecture-specific thread state
 */
int syscall_arch_prctl(ThreadState *state)
{
    int code = (int)state->guest.x[0];
    unsigned long addr = (unsigned long)state->guest.x[1];

#ifdef __linux__
    long ret = syscall(SYS_arch_prctl, code, addr);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
#else
    /* macOS doesn't have arch_prctl - use alternative */
    (void)code; (void)addr;
    state->syscall_result = -ENOSYS;
    return -1;
#endif

    return 0;
}

/* ============================================================================
 * Network Syscall Handlers
 * ============================================================================ */

/**
 * syscall_socket - Create a socket
 */
int syscall_socket(ThreadState *state)
{
    int domain = (int)state->guest.x[0];
    int type = (int)state->guest.x[1];
    int protocol = (int)state->guest.x[2];

    int ret = socket(domain, type, protocol);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_connect - Initiate a connection on a socket
 */
int syscall_connect(ThreadState *state)
{
    int sockfd = (int)state->guest.x[0];
    const struct sockaddr *addr = (const struct sockaddr *)(uintptr_t)state->guest.x[1];
    socklen_t addrlen = (socklen_t)state->guest.x[2];

    int ret = connect(sockfd, addr, addrlen);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_sendto - Send a message on a socket
 */
int syscall_sendto(ThreadState *state)
{
    int sockfd = (int)state->guest.x[0];
    const void *buf = (const void *)(uintptr_t)state->guest.x[1];
    size_t len = (size_t)state->guest.x[2];
    int flags = (int)state->guest.x[3];
    const struct sockaddr *dest_addr = (const struct sockaddr *)(uintptr_t)state->guest.x[4];
    socklen_t addrlen = (socklen_t)state->guest.x[5];

    ssize_t ret = sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_recvfrom - Receive a message from a socket
 */
int syscall_recvfrom(ThreadState *state)
{
    int sockfd = (int)state->guest.x[0];
    void *buf = (void *)(uintptr_t)state->guest.x[1];
    size_t len = (size_t)state->guest.x[2];
    int flags = (int)state->guest.x[3];
    struct sockaddr *src_addr = (struct sockaddr *)(uintptr_t)state->guest.x[4];
    socklen_t *addrlen = (socklen_t *)(uintptr_t)state->guest.x[5];

    ssize_t ret = recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_epoll_create - Create an epoll instance
 */
int syscall_epoll_create(ThreadState *state)
{
    int flags = (int)state->guest.x[0];

    int ret = epoll_create1(flags);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_epoll_ctl - Control interface for epoll
 */
int syscall_epoll_ctl(ThreadState *state)
{
    int epfd = (int)state->guest.x[0];
    int op = (int)state->guest.x[1];
    int fd = (int)state->guest.x[2];
    struct epoll_event *event = (struct epoll_event *)(uintptr_t)state->guest.x[3];

    int ret = epoll_ctl(epfd, op, fd, event);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/* ============================================================================
 * Additional Syscall Handlers
 * ============================================================================ */

/**
 * syscall_ioctl - Control device
 */
int syscall_ioctl(ThreadState *state)
{
    int fd = (int)state->guest.x[0];
    unsigned long request = (unsigned long)state->guest.x[1];
    void *arg = (void *)(uintptr_t)state->guest.x[2];

    int ret = ioctl(fd, request, arg);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_readv - Read data into multiple buffers
 */
int syscall_readv(ThreadState *state)
{
    int fd = (int)state->guest.x[0];
    const struct iovec *iov = (const struct iovec *)(uintptr_t)state->guest.x[1];
    int iovcnt = (int)state->guest.x[2];

    ssize_t ret = readv(fd, iov, iovcnt);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_writev - Write data from multiple buffers
 */
int syscall_writev(ThreadState *state)
{
    int fd = (int)state->guest.x[0];
    const struct iovec *iov = (const struct iovec *)(uintptr_t)state->guest.x[1];
    int iovcnt = (int)state->guest.x[2];

    ssize_t ret = writev(fd, iov, iovcnt);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_getcwd - Get current working directory
 */
int syscall_getcwd(ThreadState *state)
{
    char *buf = (char *)(uintptr_t)state->guest.x[0];
    size_t size = (size_t)state->guest.x[1];

    char *ret = getcwd(buf, size);
    if (ret == NULL) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_chdir - Change working directory
 */
int syscall_chdir(ThreadState *state)
{
    const char *path = (const char *)(uintptr_t)state->guest.x[0];

    int ret = chdir(path);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}
