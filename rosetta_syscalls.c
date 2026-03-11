/* ============================================================================
 * Rosetta Binary Translator - Syscall Translation Implementation
 * ============================================================================
 *
 * OVERVIEW
 * --------
 * This module translates x86_64 Linux syscalls to ARM64 Linux syscalls,
 * enabling x86_64 binaries to make system calls on ARM64 hosts.
 *
 * TRANSLATION DIRECTION
 * --------------------
 * Guest: x86_64 Linux application
 * Host: ARM64 Linux kernel
 *
 * This matches Apple's Rosetta 2 architecture (x86_64 → ARM64).
 *
 * KEY RESPONSIBILITIES
 * -------------------
 * 1. Syscall Number Translation
 *    - Map x86_64 syscall numbers to ARM64 equivalents
 *    - Handle architecture-specific syscalls (e.g., arch_prctl)
 *    - Return ENOSYS for unsupported syscalls
 *
 * 2. Argument Register Remapping
 *    - x86_64 syscall ABI: RDI, RSI, RDX, R10, R8, R9
 *    - ARM64 syscall ABI:  X0,  X1,  X2,  X3,  X4, X5
 *    - Same order, different registers (handled by ThreadState)
 *
 * 3. Syscall Dispatch
 *    - Lookup handler in syscall table
 *    - Invoke platform-specific implementation
 *    - Return results in guest state
 *
 * SUPPORTED SYSCALL CATEGORIES
 * ----------------------------
 * - Basic I/O: read, write, open, close, lseek, pipe, dup*
 * - Memory: mmap, munmap, mprotect, brk
 * - File Status: stat, fstat, lstat
 * - Process: getpid, gettid, uname, fcntl, exit*, wait4, kill
 * - Time: gettimeofday, clock_gettime, nanosleep
 * - Signal: rt_sigaction, rt_sigprocmask, sched_yield
 * - IPC/Sync: futex, arch_prctl
 * - Network: socket, connect, sendto, recvfrom, epoll*
 * - Additional: ioctl, readv, writev, getcwd, chdir
 *
 * LIMITATIONS
 * -----------
 * - futex: Linux-only (macOS returns ENOSYS)
 * - arch_prctl: x86_64-specific (returns ENOSYS on ARM64)
 * - epoll: Linux-only (macOS returns ENOSYS)
 * - Some socket options may not translate perfectly
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

/* SyscallEntry: maps x86_64 guest syscall to ARM64 host syscall */
typedef struct {
    int x86_64_nr;        /* x86_64 syscall number (guest) */
    int arm64_nr;         /* ARM64 syscall number (host) */
    syscall_handler_t handler;
} SyscallEntryLocal;

static const SyscallEntryLocal syscall_table[] = {
    /* Basic I/O */
    {X86_64_SYS_READ,         ARM64_SYS_READ,         syscall_read},
    {X86_64_SYS_WRITE,        ARM64_SYS_WRITE,        syscall_write},
    {X86_64_SYS_OPEN,         ARM64_SYS_OPEN,         syscall_open},
    {X86_64_SYS_CLOSE,        ARM64_SYS_CLOSE,        syscall_close},
    {X86_64_SYS_LSEEK,        ARM64_SYS_LSEEK,        syscall_lseek},
    {X86_64_SYS_ACCESS,       ARM64_SYS_ACCESS,       syscall_access},
    {X86_64_SYS_PIPE,         ARM64_SYS_PIPE,         syscall_pipe},
    {X86_64_SYS_DUP,          ARM64_SYS_DUP,          syscall_dup},
    {X86_64_SYS_DUP2,         ARM64_SYS_DUP2,         syscall_dup2},
    {X86_64_SYS_DUP3,         ARM64_SYS_DUP3,         syscall_dup3},

    /* Memory */
    {X86_64_SYS_MMAP,         ARM64_SYS_MMAP,         syscall_mmap},
    {X86_64_SYS_MUNMAP,       ARM64_SYS_MUNMAP,       syscall_munmap},
    {X86_64_SYS_MPROTECT,     ARM64_SYS_MPROTECT,     syscall_mprotect},
    {X86_64_SYS_BRK,          ARM64_SYS_BRK,          syscall_brk},

    /* File Status */
    {X86_64_SYS_STAT,         ARM64_SYS_STAT,         syscall_stat},
    {X86_64_SYS_FSTAT,        ARM64_SYS_FSTAT,        syscall_fstat},
    {X86_64_SYS_LSTAT,        ARM64_SYS_LSTAT,        syscall_lstat},

    /* Process */
    {X86_64_SYS_GETPID,       ARM64_SYS_GETPID,       syscall_getpid},
    {X86_64_SYS_GETTID,       ARM64_SYS_GETTID,       syscall_gettid},
    {X86_64_SYS_UNAME,        ARM64_SYS_UNAME,        syscall_uname},
    {X86_64_SYS_FCNTL,        ARM64_SYS_FCNTL,        syscall_fcntl},
    {X86_64_SYS_SET_TID_ADDRESS, ARM64_SYS_SET_TID_ADDRESS, syscall_set_tid_address},
    {X86_64_SYS_EXIT,         ARM64_SYS_EXIT,         (syscall_handler_t)syscall_exit},
    {X86_64_SYS_EXIT_GROUP,   ARM64_SYS_EXIT_GROUP,   (syscall_handler_t)syscall_exit_group},
    {X86_64_SYS_WAIT4,        ARM64_SYS_WAIT4,        syscall_wait4},
    {X86_64_SYS_KILL,         ARM64_SYS_KILL,         syscall_kill},

    /* Time */
    {X86_64_SYS_GETTIMEOFDAY, ARM64_SYS_GETTIMEOFDAY, syscall_gettimeofday},
    {X86_64_SYS_CLOCK_GETTIME, ARM64_SYS_CLOCK_GETTIME, syscall_clock_gettime},
    {X86_64_SYS_NANOSLEEP,    ARM64_SYS_NANOSLEEP,    syscall_nanosleep},

    /* Signal */
    {X86_64_SYS_RT_SIGACTION, ARM64_SYS_RT_SIGACTION, syscall_rt_sigaction},
    {X86_64_SYS_RT_SIGPROCMASK, ARM64_SYS_RT_SIGPROCMASK, syscall_rt_sigprocmask},
    {X86_64_SYS_SCHED_YIELD,  ARM64_SYS_SCHED_YIELD,  syscall_sched_yield},

    /* IPC/Sync */
    {X86_64_SYS_FUTEX,        ARM64_SYS_FUTEX,        syscall_futex},
    {X86_64_SYS_ARCH_PRCTL,   -1,                     syscall_arch_prctl},  /* Architecture-specific */

    /* Network */
    {X86_64_SYS_SOCKET,       ARM64_SYS_SOCKET,       syscall_socket},
    {X86_64_SYS_CONNECT,      ARM64_SYS_CONNECT,      syscall_connect},
    {X86_64_SYS_SENDTO,       ARM64_SYS_SENDTO,       syscall_sendto},
    {X86_64_SYS_RECVFROM,     ARM64_SYS_RECVFROM,     syscall_recvfrom},
    {X86_64_SYS_EPOLL_CREATE1, ARM64_SYS_EPOLL_CREATE1, syscall_epoll_create},
    {X86_64_SYS_EPOLL_CTL,    ARM64_SYS_EPOLL_CTL,    syscall_epoll_ctl},

    /* Additional */
    {X86_64_SYS_IOCTL,        ARM64_SYS_IOCTL,        syscall_ioctl},
    {X86_64_SYS_READV,        ARM64_SYS_READV,        syscall_readv},
    {X86_64_SYS_WRITEV,       ARM64_SYS_WRITEV,       syscall_writev},
    {X86_64_SYS_GETCWD,       ARM64_SYS_GETCWD,       syscall_getcwd},
    {X86_64_SYS_CHDIR,        ARM64_SYS_CHDIR,        syscall_chdir},
};

static int syscall_table_size = sizeof(syscall_table) / sizeof(syscall_table[0]);

/* ============================================================================
 * Syscall Number Translation
 * ============================================================================ */

/**
 * Translate x86_64 syscall number to ARM64
 */
int translate_syscall_number(int x86_64_nr)
{
    int i;

    for (i = 0; i < syscall_table_size; i++) {
        if (syscall_table[i].x86_64_nr == x86_64_nr) {
            return syscall_table[i].arm64_nr;
        }
    }

    /* Unknown syscall - return -1 (not supported) */
    return -1;
}

/**
 * Get syscall handler for x86_64 syscall
 */
syscall_handler_t get_syscall_handler(int x86_64_nr)
{
    int i;

    for (i = 0; i < syscall_table_size; i++) {
        if (syscall_table[i].x86_64_nr == x86_64_nr) {
            return syscall_table[i].handler;
        }
    }

    return NULL;
}

/* ============================================================================
 * Syscall Dispatch
 * ============================================================================ */

/**
 * Remap x86_64 syscall arguments to ARM64 calling convention
 *
 * x86_64: rdi, rsi, rdx, r10, r8, r9  (syscall convention)
 * ARM64:  x0, x1, x2, x3, x4, x5
 *
 * Both use the same order for the first 6 arguments, just in different registers.
 */
void remap_syscall_args(ThreadState *state)
{
    /* x86_64 and ARM64 Linux use the same argument passing for syscalls
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

/* Helper macros for x86_64 guest syscall arguments */
#define GUEST_RDI(state) ((state)->guest.r[X86_RDI])
#define GUEST_RSI(state) ((state)->guest.r[X86_RSI])
#define GUEST_RDX(state) ((state)->guest.r[X86_RDX])
#define GUEST_R10(state) ((state)->guest.r[X86_R10])
#define GUEST_R8(state)  ((state)->guest.r[X86_R8])
#define GUEST_R9(state)  ((state)->guest.r[X86_R9])
#define GUEST_RAX(state) ((state)->guest.r[X86_RAX])

/**
 * syscall_read - Read from file descriptor
 * x86_64: rdi=fd, rsi=buf, rdx=count
 */
int syscall_read(ThreadState *state)
{
    int fd = (int)GUEST_RDI(state);
    void *buf = (void *)(uintptr_t)GUEST_RSI(state);
    size_t count = (size_t)GUEST_RDX(state);

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
 * x86_64: rdi=fd, rsi=buf, rdx=count
 */
int syscall_write(ThreadState *state)
{
    int fd = (int)GUEST_RDI(state);
    const void *buf = (const void *)(uintptr_t)GUEST_RSI(state);
    size_t count = (size_t)GUEST_RDX(state);

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
    const char *pathname = (const char *)(uintptr_t)GUEST_RDI(state);
    int flags = (int)GUEST_RSI(state);
    mode_t mode = (mode_t)GUEST_RDX(state);

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
    int fd = (int)GUEST_RDI(state);

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
    int fd = (int)GUEST_RDI(state);
    off_t offset = (off_t)GUEST_RSI(state);
    int whence = (int)GUEST_RDX(state);

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
    const char *pathname = (const char *)(uintptr_t)GUEST_RDI(state);
    int mode = (int)GUEST_RSI(state);

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
    GUEST_RDI(state) = pipefd[0];
    GUEST_RSI(state) = pipefd[1];
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_dup - Duplicate file descriptor
 * x86_64: rdi=fd
 */
int syscall_dup(ThreadState *state)
{
    int oldfd = (int)GUEST_RDI(state);

    int ret = dup(oldfd);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_dup2 - Duplicate file descriptor
 * x86_64: rdi=oldfd, rsi=newfd
 */
int syscall_dup2(ThreadState *state)
{
    int oldfd = (int)GUEST_RDI(state);
    int newfd = (int)GUEST_RSI(state);

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
 * x86_64: rdi=oldfd, rsi=newfd, rdx=flags
 */
int syscall_dup3(ThreadState *state)
{
    int oldfd = (int)GUEST_RDI(state);
    int newfd = (int)GUEST_RSI(state);
    int flags = (int)GUEST_RDX(state);

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
    void *addr = (void *)(uintptr_t)GUEST_RDI(state);
    size_t length = (size_t)GUEST_RSI(state);
    int prot = (int)GUEST_RDX(state);
    int flags = (int)GUEST_R10(state);
    int fd = (int)GUEST_R8(state);
    off_t offset = (off_t)GUEST_R9(state);

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
    void *addr = (void *)(uintptr_t)GUEST_RDI(state);
    size_t length = (size_t)GUEST_RSI(state);

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
    void *addr = (void *)(uintptr_t)GUEST_RDI(state);
    size_t length = (size_t)GUEST_RSI(state);
    int prot = (int)GUEST_RDX(state);

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
    void *addr = (void *)(uintptr_t)GUEST_RDI(state);

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
    const char *pathname = (const char *)(uintptr_t)GUEST_RDI(state);
    struct stat *statbuf = (struct stat *)(uintptr_t)GUEST_RSI(state);

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
    int fd = (int)GUEST_RDI(state);
    struct stat *statbuf = (struct stat *)(uintptr_t)GUEST_RSI(state);

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
    const char *pathname = (const char *)(uintptr_t)GUEST_RDI(state);
    struct stat *statbuf = (struct stat *)(uintptr_t)GUEST_RSI(state);

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
    struct utsname *buf = (struct utsname *)(uintptr_t)GUEST_RDI(state);

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
    int fd = (int)GUEST_RDI(state);
    int cmd = (int)GUEST_RSI(state);
    long arg = (long)GUEST_RDX(state);

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
    int *tidptr = (int *)(uintptr_t)GUEST_RDI(state);

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
    int status = (int)GUEST_RDI(state);
    exit(status);
}

/**
 * syscall_exit_group - Terminate all threads in process
 * Note: Declared as returning int for syscall_handler_t compatibility,
 *       but this function never returns.
 */
noreturn int syscall_exit_group(ThreadState *state)
{
    int status = (int)GUEST_RDI(state);
    _exit(status);
}

/**
 * syscall_wait4 - Wait for process to change state
 */
int syscall_wait4(ThreadState *state)
{
    pid_t pid = (pid_t)GUEST_RDI(state);
    int *wstatus = (int *)(uintptr_t)GUEST_RSI(state);
    int options = (int)GUEST_RDX(state);
    struct rusage *rusage = (struct rusage *)(uintptr_t)GUEST_R10(state);

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
    pid_t pid = (pid_t)GUEST_RDI(state);
    int sig = (int)GUEST_RSI(state);

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
    struct timeval *tv = (struct timeval *)(uintptr_t)GUEST_RDI(state);
    struct timezone *tz = (struct timezone *)(uintptr_t)GUEST_RSI(state);

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
    clockid_t clk_id = (clockid_t)GUEST_RDI(state);
    struct timespec *tp = (struct timespec *)(uintptr_t)GUEST_RSI(state);

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
    const struct timespec *req = (const struct timespec *)(uintptr_t)GUEST_RDI(state);
    struct timespec *rem = (struct timespec *)(uintptr_t)GUEST_RSI(state);

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
    int signum = (int)GUEST_RDI(state);
    const struct sigaction *act = (const struct sigaction *)(uintptr_t)GUEST_RSI(state);
    struct sigaction *oldact = (struct sigaction *)(uintptr_t)GUEST_RDX(state);
    size_t sigsetsize = (size_t)GUEST_R10(state);

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
    int how = (int)GUEST_RDI(state);
    const sigset_t *set = (const sigset_t *)(uintptr_t)GUEST_RSI(state);
    sigset_t *oldset = (sigset_t *)(uintptr_t)GUEST_RDX(state);
    size_t sigsetsize = (size_t)GUEST_R10(state);

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
    uint32_t *uaddr = (uint32_t *)(uintptr_t)GUEST_RDI(state);
    int futex_op = (int)GUEST_RSI(state);
    uint32_t val = (uint32_t)GUEST_RDX(state);
    void *timeout = (void *)(uintptr_t)GUEST_R10(state);
    uint32_t *uaddr2 = (uint32_t *)(uintptr_t)GUEST_R8(state);
    uint32_t val3 = (uint32_t)GUEST_R9(state);

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
    int code = (int)GUEST_RDI(state);
    unsigned long addr = (unsigned long)GUEST_RSI(state);

#if defined(__linux__) && defined(SYS_arch_prctl) && !defined(__aarch64__)
    /* SYS_arch_prctl only exists on x86_64 Linux */
    long ret = syscall(SYS_arch_prctl, code, addr);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
#else
    /* macOS and ARM64 Linux don't have arch_prctl */
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
    int domain = (int)GUEST_RDI(state);
    int type = (int)GUEST_RSI(state);
    int protocol = (int)GUEST_RDX(state);

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
    int sockfd = (int)GUEST_RDI(state);
    const struct sockaddr *addr = (const struct sockaddr *)(uintptr_t)GUEST_RSI(state);
    socklen_t addrlen = (socklen_t)GUEST_RDX(state);

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
    int sockfd = (int)GUEST_RDI(state);
    const void *buf = (const void *)(uintptr_t)GUEST_RSI(state);
    size_t len = (size_t)GUEST_RDX(state);
    int flags = (int)GUEST_R10(state);
    const struct sockaddr *dest_addr = (const struct sockaddr *)(uintptr_t)GUEST_R8(state);
    socklen_t addrlen = (socklen_t)GUEST_R9(state);

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
    int sockfd = (int)GUEST_RDI(state);
    void *buf = (void *)(uintptr_t)GUEST_RSI(state);
    size_t len = (size_t)GUEST_RDX(state);
    int flags = (int)GUEST_R10(state);
    struct sockaddr *src_addr = (struct sockaddr *)(uintptr_t)GUEST_R8(state);
    socklen_t *addrlen = (socklen_t *)(uintptr_t)GUEST_R9(state);

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
    int flags = (int)GUEST_RDI(state);

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
    int epfd = (int)GUEST_RDI(state);
    int op = (int)GUEST_RSI(state);
    int fd = (int)GUEST_RDX(state);
    struct epoll_event *event = (struct epoll_event *)(uintptr_t)GUEST_R10(state);

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
    int fd = (int)GUEST_RDI(state);
    unsigned long request = (unsigned long)GUEST_RSI(state);
    void *arg = (void *)(uintptr_t)GUEST_RDX(state);

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
    int fd = (int)GUEST_RDI(state);
    const struct iovec *iov = (const struct iovec *)(uintptr_t)GUEST_RSI(state);
    int iovcnt = (int)GUEST_RDX(state);

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
    int fd = (int)GUEST_RDI(state);
    const struct iovec *iov = (const struct iovec *)(uintptr_t)GUEST_RSI(state);
    int iovcnt = (int)GUEST_RDX(state);

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
    char *buf = (char *)(uintptr_t)GUEST_RDI(state);
    size_t size = (size_t)GUEST_RSI(state);

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
    const char *path = (const char *)(uintptr_t)GUEST_RDI(state);

    int ret = chdir(path);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}
