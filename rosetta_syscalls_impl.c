/* ============================================================================
 * Rosetta Translator - Syscall Implementation
 * ============================================================================
 *
 * This module implements syscall handling functions for the Rosetta
 * binary translation layer. It translates x86_64 Linux syscalls to
 * host OS (macOS/ARM64) equivalents.
 * ============================================================================ */

#define _GNU_SOURCE
#define _DEFAULT_SOURCE

#include "rosetta_syscalls_impl.h"
#include "rosetta_refactored_helpers.h"
#include "rosetta_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#ifdef __linux__
#include <sys/epoll.h>
#include <sys/prctl.h>
#include <linux/sysctl.h>
#endif
#if defined(__APPLE__)
#include <mach/mach_time.h>
#include <sys/sysctl.h>
#endif

/* x86_64 syscall argument registers */
#define GUEST_ARG0(st) ((st)->guest.r[X86_RDI])
#define GUEST_ARG1(st) ((st)->guest.r[X86_RSI])
#define GUEST_ARG2(st) ((st)->guest.r[X86_RDX])
#define GUEST_ARG3(st) ((st)->guest.r[X86_R10])
#define GUEST_ARG4(st) ((st)->guest.r[X86_R8])
#define GUEST_ARG5(st) ((st)->guest.r[X86_R9])

/* ============================================================================
 * Basic I/O Syscalls
 * ============================================================================ */

/**
 * syscall_read - Read from file descriptor
 */
int syscall_read(ThreadState *state)
{
    int fd = GUEST_ARG0(state);
    void *buf = (void *)GUEST_ARG1(state);
    size_t count = GUEST_ARG2(state);

    ssize_t ret = read(fd, buf, count);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_write - Write to file descriptor
 */
int syscall_write(ThreadState *state)
{
    int fd = GUEST_ARG0(state);
    const void *buf = (const void *)GUEST_ARG1(state);
    size_t count = GUEST_ARG2(state);

    ssize_t ret = write(fd, buf, count);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_open - Open a file
 */
int syscall_open(ThreadState *state)
{
    const char *pathname = (const char *)GUEST_ARG0(state);
    int flags = GUEST_ARG1(state);
    mode_t mode = GUEST_ARG2(state);

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
    int fd = GUEST_ARG0(state);

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
    int fd = GUEST_ARG0(state);
    off_t offset = GUEST_ARG1(state);
    int whence = GUEST_ARG2(state);

    off_t ret = lseek(fd, offset, whence);
    if (ret == (off_t)-1) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_access - Check real user's permissions for a file
 */
int syscall_access(ThreadState *state)
{
    const char *pathname = (const char *)GUEST_ARG0(state);
    int mode = GUEST_ARG1(state);

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
    GUEST_ARG0(state) = pipefd[0];
    GUEST_ARG1(state) = pipefd[1];
    state->syscall_result = 0;
    return 0;
}

/* ============================================================================
 * Memory Management Syscalls
 * ============================================================================ */

/**
 * syscall_mmap - Map files or devices into memory
 */
int syscall_mmap(ThreadState *state)
{
    void *addr = (void *)GUEST_ARG0(state);
    size_t length = GUEST_ARG1(state);
    int prot = GUEST_ARG2(state);
    int flags = GUEST_ARG3(state);
    int fd = GUEST_ARG4(state);
    off_t offset = GUEST_ARG5(state);

    void *ret = mmap(addr, length, prot, flags, fd, offset);
    if (ret == MAP_FAILED) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = (uint64_t)ret;
    return 0;
}

/**
 * syscall_munmap - Unmap memory region
 */
int syscall_munmap(ThreadState *state)
{
    void *addr = (void *)GUEST_ARG0(state);
    size_t length = GUEST_ARG1(state);

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
    void *addr = (void *)GUEST_ARG0(state);
    size_t length = GUEST_ARG1(state);
    int prot = GUEST_ARG2(state);

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
 */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
int syscall_brk(ThreadState *state)
{
    void *addr = (void *)GUEST_ARG0(state);

    void *ret = sbrk(0);
    if (addr != NULL) {
        (void)addr;  /* Simple implementation - just return current break */
    }
    state->syscall_result = (uint64_t)ret;
    return 0;
}
#pragma clang diagnostic pop

/* ============================================================================
 * File Status Syscalls
 * ============================================================================ */

/**
 * syscall_stat - Get file status
 */
int syscall_stat(ThreadState *state)
{
    const char *pathname = (const char *)GUEST_ARG0(state);
    struct stat *statbuf = (struct stat *)GUEST_ARG1(state);

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
    int fd = GUEST_ARG0(state);
    struct stat *statbuf = (struct stat *)GUEST_ARG1(state);

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
    const char *pathname = (const char *)GUEST_ARG0(state);
    struct stat *statbuf = (struct stat *)GUEST_ARG1(state);

    int ret = lstat(pathname, statbuf);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/* ============================================================================
 * Process Management Syscalls
 * ============================================================================ */

/**
 * syscall_getpid - Get process ID
 */
int syscall_getpid(ThreadState *state)
{
    pid_t ret = getpid();
    if (ret == (pid_t)-1) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_gettid - Get thread ID
 */
int syscall_gettid(ThreadState *state)
{
    state->syscall_result = (int64_t)getpid();  /* Simplified for macOS */
    return 0;
}

/**
 * syscall_uname - Get system information
 */
int syscall_uname(ThreadState *state)
{
    struct utsname *buf = (struct utsname *)GUEST_ARG0(state);

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
    int fd = GUEST_ARG0(state);
    int cmd = GUEST_ARG1(state);
    long arg = GUEST_ARG2(state);

    int ret = fcntl(fd, cmd, arg);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_exit - Terminate the calling process
 * Note: Declared as returning int for syscall_handler_t compatibility,
 *       but this function never returns.
 */
noreturn int syscall_exit(ThreadState *state)
{
    int status = GUEST_ARG0(state);
    _exit(status);
}

/**
 * syscall_exit_group - Terminate all threads in process
 * Note: Declared as returning int for syscall_handler_t compatibility,
 *       but this function never returns.
 */
noreturn int syscall_exit_group(ThreadState *state)
{
    int status = GUEST_ARG0(state);
    _exit(status);
}

/**
 * syscall_set_tid_address - Set pointer to thread ID
 */
int syscall_set_tid_address(ThreadState *state)
{
    int *tidptr = (int *)GUEST_ARG0(state);
    (void)tidptr;
    state->syscall_result = (int64_t)getpid();
    return 0;
}

/**
 * syscall_getcpu - Get current CPU and NUMA node
 */
int syscall_getcpu(ThreadState *state)
{
    unsigned *cpu = (unsigned *)GUEST_ARG0(state);
    unsigned *node = (unsigned *)GUEST_ARG1(state);

#if defined(__linux__) && defined(SYS_getcpu)
    int ret = syscall(SYS_getcpu, cpu, node, NULL);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
#else
    (void)cpu;
    (void)node;
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
#endif
}

/**
 * syscall_setpgid - Set process group ID
 */
int syscall_setpgid(ThreadState *state)
{
    pid_t pid = (pid_t)GUEST_ARG0(state);
    pid_t pgid = (pid_t)GUEST_ARG1(state);

    int ret = setpgid(pid, pgid);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_setsid - Create session and set process group ID
 */
int syscall_setsid(ThreadState *state)
{
    pid_t ret = setsid();
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_getpgid - Get process group ID
 */
int syscall_getpgid(ThreadState *state)
{
    pid_t pid = (pid_t)GUEST_ARG0(state);

    pid_t ret = getpgid(pid);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_getsid - Get session ID
 */
int syscall_getsid(ThreadState *state)
{
    pid_t pid = (pid_t)GUEST_ARG0(state);

#ifdef __linux__
    pid_t ret = getsid(pid);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
#else
    (void)pid;
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
#endif
}

/**
 * syscall_getgroups - Get supplementary group IDs
 */
int syscall_getgroups(ThreadState *state)
{
    int size = (int)GUEST_ARG0(state);
    gid_t *list = (gid_t *)GUEST_ARG1(state);

    int ret = getgroups(size, list);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_setgroups - Set supplementary group IDs
 */
int syscall_setgroups(ThreadState *state)
{
    size_t size = (size_t)GUEST_ARG0(state);
    const gid_t *list = (const gid_t *)GUEST_ARG1(state);

#ifdef __linux__
    int ret = setgroups(size, list);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
#else
    (void)size;
    (void)list;
    state->syscall_result = -1;  /* EPERM - not permitted on non-Linux */
    return -1;
#endif
}

/**
 * syscall_sethostname - Set hostname
 */
int syscall_sethostname(ThreadState *state)
{
    const char *name = (const char *)GUEST_ARG0(state);
    size_t len = (size_t)GUEST_ARG1(state);

#ifdef __linux__
    int ret = sethostname(name, len);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
#else
    (void)name;
    (void)len;
    state->syscall_result = -1;  /* EPERM - requires root */
    return -1;
#endif
}

/**
 * syscall_setdomainname - Set domain name
 */
int syscall_setdomainname(ThreadState *state)
{
    const char *name = (const char *)GUEST_ARG0(state);
    size_t len = (size_t)GUEST_ARG1(state);

#ifdef __linux__
    int ret = setdomainname(name, len);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
#else
    (void)name;
    (void)len;
    state->syscall_result = -1;  /* EPERM - requires root */
    return -1;
#endif
}

/**
 * syscall_prctl - Operations on a process
 */
int syscall_prctl(ThreadState *state)
{
    int option = (int)GUEST_ARG0(state);
    unsigned long arg2 = GUEST_ARG1(state);
    unsigned long arg3 = GUEST_ARG2(state);
    unsigned long arg4 = GUEST_ARG3(state);
    unsigned long arg5 = GUEST_ARG4(state);

#ifdef __linux__
    long ret = prctl(option, arg2, arg3, arg4, arg5);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
#else
    (void)option;
    (void)arg2;
    (void)arg3;
    (void)arg4;
    (void)arg5;
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
#endif
}

/* ============================================================================
 * Time-related Syscalls
 * ============================================================================ */

/**
 * syscall_gettimeofday - Get time of day
 */
int syscall_gettimeofday(ThreadState *state)
{
    struct timeval *tv = (struct timeval *)GUEST_ARG0(state);
    struct timezone *tz = (struct timezone *)GUEST_ARG1(state);

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
    clockid_t clk_id = GUEST_ARG0(state);
    struct timespec *tp = (struct timespec *)GUEST_ARG1(state);

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
    const struct timespec *req = (const struct timespec *)GUEST_ARG0(state);
    struct timespec *rem = (struct timespec *)GUEST_ARG1(state);

    int ret = nanosleep(req, rem);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_clock_getres - Get clock resolution
 */
int syscall_clock_getres(ThreadState *state)
{
    clockid_t clk_id = GUEST_ARG0(state);
    struct timespec *tp = (struct timespec *)GUEST_ARG1(state);

    int ret = clock_getres(clk_id, tp);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_settimeofday - Set time of day
 */
int syscall_settimeofday(ThreadState *state)
{
    const struct timeval *tv = (const struct timeval *)GUEST_ARG0(state);
    const struct timezone *tz = (const struct timezone *)GUEST_ARG1(state);

    int ret = settimeofday(tv, tz);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/* ============================================================================
 * Signal Syscalls
 * ============================================================================ */

/**
 * syscall_rt_sigaction - Examine and change a signal handler
 */
int syscall_rt_sigaction(ThreadState *state)
{
    int signum = GUEST_ARG0(state);
    const struct sigaction *act = (const struct sigaction *)GUEST_ARG1(state);
    struct sigaction *oact = (struct sigaction *)GUEST_ARG2(state);
    size_t sigsetsize = GUEST_ARG3(state);

    int ret = sigaction(signum, act, oact);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_rt_sigprocmask - Examine and change blocked signals
 */
int syscall_rt_sigprocmask(ThreadState *state)
{
    int how = GUEST_ARG0(state);
    const sigset_t *set = (const sigset_t *)GUEST_ARG1(state);
    sigset_t *oldset = (sigset_t *)GUEST_ARG2(state);
    size_t sigsetsize = GUEST_ARG3(state);

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

/**
 * syscall_kill - Send signal to process
 */
int syscall_kill(ThreadState *state)
{
    pid_t pid = (pid_t)GUEST_ARG0(state);
    int sig = GUEST_ARG1(state);

    int ret = kill(pid, sig);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/* ============================================================================
 * IPC and Synchronization Syscalls
 * ============================================================================ */

/**
 * syscall_futex - Fast userspace mutex
 */
int syscall_futex(ThreadState *state)
{
    uint32_t *uaddr = (uint32_t *)GUEST_ARG0(state);
    int futex_op = GUEST_ARG1(state);
    uint32_t val = GUEST_ARG2(state);
    struct timespec *timeout = (struct timespec *)GUEST_ARG3(state);
    uint32_t *uaddr2 = (uint32_t *)GUEST_ARG4(state);
    uint32_t val3 = GUEST_ARG5(state);

    (void)uaddr;
    (void)futex_op;
    (void)val;
    (void)timeout;
    (void)uaddr2;
    (void)val3;
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
}

/**
 * syscall_arch_prctl - Set architecture-specific thread state
 */
int syscall_arch_prctl(ThreadState *state)
{
    int code = GUEST_ARG0(state);
    unsigned long addr = GUEST_ARG1(state);

    (void)code;
    (void)addr;
    state->syscall_result = 0;  /* Success - handled by runtime */
    return 0;
}

/* ============================================================================
 * Extended I/O Syscalls
 * ============================================================================ */

/**
 * syscall_ioctl - Manipulate file descriptor
 */
int syscall_ioctl(ThreadState *state)
{
    int fd = GUEST_ARG0(state);
    unsigned long request = GUEST_ARG1(state);
    void *arg = (void *)GUEST_ARG2(state);

    int ret = ioctl(fd, request, arg);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_dup2 - Duplicate file descriptor
 */
int syscall_dup2(ThreadState *state)
{
    int oldfd = GUEST_ARG0(state);
    int newfd = GUEST_ARG1(state);

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
    int oldfd = GUEST_ARG0(state);
    int newfd = GUEST_ARG1(state);
    int flags = GUEST_ARG2(state);

#ifdef __linux__
    int ret = dup3(oldfd, newfd, flags);
#else
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

/**
 * syscall_poll - Wait for events on file descriptors
 */
int syscall_poll(ThreadState *state)
{
    struct pollfd *fds = (struct pollfd *)GUEST_ARG0(state);
    nfds_t nfds = GUEST_ARG1(state);
    int timeout = GUEST_ARG2(state);

    int ret = poll(fds, nfds, timeout);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_select - Monitor multiple file descriptors
 */
int syscall_select(ThreadState *state)
{
    int nfds = GUEST_ARG0(state);
    fd_set *readfds = (fd_set *)GUEST_ARG1(state);
    fd_set *writefds = (fd_set *)GUEST_ARG2(state);
    fd_set *exceptfds = (fd_set *)GUEST_ARG3(state);
    struct timeval *timeout = (struct timeval *)GUEST_ARG4(state);

    int ret = select(nfds, readfds, writefds, exceptfds, timeout);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_readv - Read data from file descriptor into multiple buffers
 */
int syscall_readv(ThreadState *state)
{
    int fd = GUEST_ARG0(state);
    const struct iovec *iov = (const struct iovec *)GUEST_ARG1(state);
    int iovcnt = GUEST_ARG2(state);

    ssize_t ret = readv(fd, iov, iovcnt);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_writev - Write data from multiple buffers to file descriptor
 */
int syscall_writev(ThreadState *state)
{
    int fd = GUEST_ARG0(state);
    const struct iovec *iov = (const struct iovec *)GUEST_ARG1(state);
    int iovcnt = GUEST_ARG2(state);

    ssize_t ret = writev(fd, iov, iovcnt);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/* ============================================================================
 * Filesystem Syscalls
 * ============================================================================ */

/**
 * syscall_getcwd - Get current working directory
 */
int syscall_getcwd(ThreadState *state)
{
    char *buf = (char *)GUEST_ARG0(state);
    size_t size = GUEST_ARG1(state);

    char *ret = getcwd(buf, size);
    if (ret == NULL) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_chdir - Change current working directory
 */
int syscall_chdir(ThreadState *state)
{
    const char *path = (const char *)GUEST_ARG0(state);

    int ret = chdir(path);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_rename - Rename file or directory
 */
int syscall_rename(ThreadState *state)
{
    const char *oldpath = (const char *)GUEST_ARG0(state);
    const char *newpath = (const char *)GUEST_ARG1(state);

    int ret = rename(oldpath, newpath);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_mkdir - Create directory
 */
int syscall_mkdir(ThreadState *state)
{
    const char *pathname = (const char *)GUEST_ARG0(state);
    mode_t mode = GUEST_ARG1(state);

    int ret = mkdir(pathname, mode);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_rmdir - Remove directory
 */
int syscall_rmdir(ThreadState *state)
{
    const char *pathname = (const char *)GUEST_ARG0(state);

    int ret = rmdir(pathname);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_unlink - Delete file
 */
int syscall_unlink(ThreadState *state)
{
    const char *pathname = (const char *)GUEST_ARG0(state);

    int ret = unlink(pathname);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_symlink - Create symbolic link
 */
int syscall_symlink(ThreadState *state)
{
    const char *target = (const char *)GUEST_ARG0(state);
    const char *linkpath = (const char *)GUEST_ARG1(state);

    int ret = symlink(target, linkpath);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_readlink - Read symbolic link
 */
int syscall_readlink(ThreadState *state)
{
    const char *pathname = (const char *)GUEST_ARG0(state);
    char *buf = (char *)GUEST_ARG1(state);
    size_t bufsize = GUEST_ARG2(state);

    ssize_t ret = readlink(pathname, buf, bufsize);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_chmod - Change file permissions
 */
int syscall_chmod(ThreadState *state)
{
    const char *pathname = (const char *)GUEST_ARG0(state);
    mode_t mode = GUEST_ARG1(state);

    int ret = chmod(pathname, mode);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_lchown - Change owner of symbolic link
 */
int syscall_lchown(ThreadState *state)
{
    const char *pathname = (const char *)GUEST_ARG0(state);
    uid_t owner = GUEST_ARG1(state);
    gid_t group = GUEST_ARG2(state);

    int ret = lchown(pathname, owner, group);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_creat - Create file and open it for writing
 */
int syscall_creat(ThreadState *state)
{
    const char *pathname = (const char *)GUEST_ARG0(state);
    mode_t mode = GUEST_ARG1(state);

    /* creat is equivalent to open(pathname, O_WRONLY|O_CREAT|O_TRUNC, mode) */
    int fd = creat(pathname, mode);
    if (fd < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = fd;
    return 0;
}

/**
 * syscall_chown - Change owner and group of file
 */
int syscall_chown(ThreadState *state)
{
    const char *pathname = (const char *)GUEST_ARG0(state);
    uid_t owner = GUEST_ARG1(state);
    gid_t group = GUEST_ARG2(state);

    int ret = chown(pathname, owner, group);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_fchmod - Change file permissions by file descriptor
 */
int syscall_fchmod(ThreadState *state)
{
    int fd = GUEST_ARG0(state);
    mode_t mode = GUEST_ARG1(state);

    int ret = fchmod(fd, mode);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_fchown - Change owner and group by file descriptor
 */
int syscall_fchown(ThreadState *state)
{
    int fd = GUEST_ARG0(state);
    uid_t owner = GUEST_ARG1(state);
    gid_t group = GUEST_ARG2(state);

    int ret = fchown(fd, owner, group);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_getdents - Get directory entries
 */
int syscall_getdents(ThreadState *state)
{
    int fd = GUEST_ARG0(state);
    void *dirp = (void *)GUEST_ARG1(state);
    size_t count = GUEST_ARG2(state);

#if defined(__linux__) && defined(SYS_getdents)
    int ret = syscall(SYS_getdents, fd, dirp, count);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
#else
    (void)fd;
    (void)dirp;
    (void)count;
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
#endif
}

/* ============================================================================
 * Process and Thread Syscalls
 * ============================================================================ */

/**
 * syscall_wait4 - Wait for process to change state
 */
int syscall_wait4(ThreadState *state)
{
    pid_t pid = (pid_t)GUEST_ARG0(state);
    int *wstatus = (int *)GUEST_ARG1(state);
    int options = GUEST_ARG2(state);
    struct rusage *rusage = (struct rusage *)GUEST_ARG3(state);

    pid_t ret = wait4(pid, wstatus, options, rusage);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_capget - Get process capabilities
 */
int syscall_capget(ThreadState *state)
{
    state->syscall_result = -38;  /* ENOSYS - Linux only */
    return -1;
}

/**
 * syscall_capset - Set process capabilities
 */
int syscall_capset(ThreadState *state)
{
    state->syscall_result = -38;  /* ENOSYS - Linux only */
    return -1;
}

/**
 * syscall_mincore - Determine whether pages are in core
 */
int syscall_mincore(ThreadState *state)
{
    void *addr = (void *)GUEST_ARG0(state);
    size_t length = GUEST_ARG1(state);
    unsigned char *vec = (unsigned char *)GUEST_ARG2(state);

#ifdef __linux__
    int ret = mincore(addr, length, vec);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
#else
    (void)addr;
    (void)length;
    (void)vec;
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
#endif
}

/**
 * syscall_prlimit - Get/set resource limits
 */
int syscall_prlimit(ThreadState *state)
{
    pid_t pid = (pid_t)GUEST_ARG0(state);
    int resource = GUEST_ARG1(state);
    const void *new_limit = (const void *)GUEST_ARG2(state);
    void *old_limit = (void *)GUEST_ARG3(state);

#if defined(__linux__) && defined(SYS_prlimit)
    int ret = syscall(SYS_prlimit, pid, resource, new_limit, old_limit);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
#else
    (void)pid;
    (void)resource;
    (void)new_limit;
    (void)old_limit;
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
#endif
}

/**
 * syscall_madvise - Give advice about use of memory
 */
int syscall_madvise(ThreadState *state)
{
    void *addr = (void *)GUEST_ARG0(state);
    size_t length = GUEST_ARG1(state);
    int advice = GUEST_ARG2(state);

#ifdef __linux__
    int ret = madvise(addr, length, advice);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
#else
    (void)addr;
    (void)length;
    (void)advice;
    state->syscall_result = 0;  /* Silently ignore on non-Linux */
    return 0;
#endif
}

/**
 * syscall_mlock - Lock memory in RAM
 */
int syscall_mlock(ThreadState *state)
{
    const void *addr = (const void *)GUEST_ARG0(state);
    size_t len = GUEST_ARG1(state);

#ifdef __linux__
    int ret = mlock(addr, len);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
#else
    (void)addr;
    (void)len;
    state->syscall_result = 0;  /* Silently ignore on non-Linux */
    return 0;
#endif
}

/**
 * syscall_munlock - Unlock memory
 */
int syscall_munlock(ThreadState *state)
{
    const void *addr = (const void *)GUEST_ARG0(state);
    size_t len = GUEST_ARG1(state);

#ifdef __linux__
    int ret = munlock(addr, len);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
#else
    (void)addr;
    (void)len;
    state->syscall_result = 0;  /* Silently ignore on non-Linux */
    return 0;
#endif
}

/**
 * syscall_msync - Synchronize a file with a memory map
 */
int syscall_msync(ThreadState *state)
{
    void *addr = (void *)GUEST_ARG0(state);
    size_t length = GUEST_ARG1(state);
    int flags = GUEST_ARG2(state);

    int ret = msync(addr, length, flags);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_clone - Create child process
 */
int syscall_clone(ThreadState *state)
{
    (void)state;
    state->syscall_result = -38;  /* ENOSYS - Linux only */
    return -1;
}

/**
 * syscall_execve - Execute program
 */
int syscall_execve(ThreadState *state)
{
    const char *pathname = (const char *)GUEST_ARG0(state);
    char *const *argv = (char *const *)GUEST_ARG1(state);
    char *const *envp = (char *const *)GUEST_ARG2(state);

    int ret = execve(pathname, argv, envp);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_set_robust_list - Set robust list
 */
int syscall_set_robust_list(ThreadState *state)
{
    (void)state;
    state->syscall_result = -38;  /* ENOSYS - Linux only */
    return -1;
}

/**
 * syscall_get_robust_list - Get robust list
 */
int syscall_get_robust_list(ThreadState *state)
{
    (void)state;
    state->syscall_result = -38;  /* ENOSYS - Linux only */
    return -1;
}

/* ============================================================================
 * Network Syscalls
 * ============================================================================ */

/**
 * syscall_socket - Create a socket
 */
int syscall_socket(ThreadState *state)
{
    int domain = GUEST_ARG0(state);
    int type = GUEST_ARG1(state);
    int protocol = GUEST_ARG2(state);

    int ret = socket(domain, type, protocol);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_connect - Connect to a socket
 */
int syscall_connect(ThreadState *state)
{
    int sockfd = GUEST_ARG0(state);
    const struct sockaddr *addr = (const struct sockaddr *)GUEST_ARG1(state);
    socklen_t addrlen = GUEST_ARG2(state);

    int ret = connect(sockfd, addr, addrlen);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_sendto - Send data on a socket
 */
int syscall_sendto(ThreadState *state)
{
    int sockfd = GUEST_ARG0(state);
    const void *buf = (const void *)GUEST_ARG1(state);
    size_t len = GUEST_ARG2(state);
    int flags = GUEST_ARG3(state);
    const struct sockaddr *dest_addr = (const struct sockaddr *)GUEST_ARG4(state);
    socklen_t addrlen = GUEST_ARG5(state);

    ssize_t ret = sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_recvfrom - Receive data from a socket
 */
int syscall_recvfrom(ThreadState *state)
{
    int sockfd = GUEST_ARG0(state);
    void *buf = (void *)GUEST_ARG1(state);
    size_t len = GUEST_ARG2(state);
    int flags = GUEST_ARG3(state);
    struct sockaddr *src_addr = (struct sockaddr *)GUEST_ARG4(state);
    socklen_t *addrlen = (socklen_t *)GUEST_ARG5(state);

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
    int size = GUEST_ARG0(state);

#ifdef __linux__
    int ret = epoll_create(size);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
#else
    (void)size;
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
#endif
}

/**
 * syscall_epoll_ctl - Control interface for epoll
 */
int syscall_epoll_ctl(ThreadState *state)
{
    int epfd = GUEST_ARG0(state);
    int op = GUEST_ARG1(state);
    int fd = GUEST_ARG2(state);
    struct epoll_event *event = (struct epoll_event *)GUEST_ARG3(state);

#ifdef __linux__
    int ret = epoll_ctl(epfd, op, fd, event);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
#else
    (void)epfd;
    (void)op;
    (void)fd;
    (void)event;
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
#endif
}

/**
 * syscall_epoll_wait - Wait for events on an epoll instance
 */
int syscall_epoll_wait(ThreadState *state)
{
    int epfd = GUEST_ARG0(state);
    struct epoll_event *events = (struct epoll_event *)GUEST_ARG1(state);
    int maxevents = GUEST_ARG2(state);
    int timeout = GUEST_ARG3(state);

#ifdef __linux__
    int ret = epoll_wait(epfd, events, maxevents, timeout);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
#else
    (void)epfd;
    (void)events;
    (void)maxevents;
    (void)timeout;
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
#endif
}

/* ============================================================================
 * Memory Management Helpers
 * ============================================================================ */

/* Guest memory mapping state */
static void *guest_memory_base = NULL;
static size_t guest_memory_size = 0;

/**
 * memory_map_guest - Map guest memory region
 */
void *memory_map_guest(uint64_t guest, uint64_t size)
{
    void *ret = mmap((void *)guest, size, PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (ret == MAP_FAILED) {
        return NULL;
    }
    if (guest_memory_base == NULL) {
        guest_memory_base = ret;
        guest_memory_size = size;
    }
    return ret;
}

/**
 * memory_unmap_guest - Unmap guest memory region
 */
int memory_unmap_guest(uint64_t guest, uint64_t size)
{
    int ret = munmap((void *)guest, size);
    if (ret < 0) {
        return -1;
    }
    if ((uint64_t)guest == (uint64_t)guest_memory_base) {
        guest_memory_base = NULL;
        guest_memory_size = 0;
    }
    return 0;
}

/**
 * memory_protect_guest - Change protection on guest memory region
 */
int memory_protect_guest(uint64_t guest, uint64_t size, int32_t prot)
{
    int ret = mprotect((void *)guest, size, prot);
    if (ret < 0) {
        return -1;
    }
    return 0;
}

/**
 * memory_init - Initialize memory management subsystem
 */
int memory_init(void)
{
    guest_memory_base = NULL;
    guest_memory_size = 0;
    return 0;
}

/**
 * memory_cleanup - Clean up memory management subsystem
 */
void memory_cleanup(void)
{
    if (guest_memory_base != NULL) {
        munmap(guest_memory_base, guest_memory_size);
        guest_memory_base = NULL;
        guest_memory_size = 0;
    }
}

/**
 * memory_map_guest_with_prot - Map guest memory with specific protection
 */
void *memory_map_guest_with_prot(uint64_t guest, uint64_t size, int32_t prot)
{
    void *ret = mmap((void *)guest, size, prot,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (ret == MAP_FAILED) {
        return NULL;
    }
    if (guest_memory_base == NULL) {
        guest_memory_base = ret;
        guest_memory_size = size;
    }
    return ret;
}

/**
 * memory_translate_addr - Translate guest address to host address
 */
void *memory_translate_addr(uint64_t guest)
{
    if (guest_memory_base != NULL &&
        guest >= (uint64_t)guest_memory_base &&
        guest < (uint64_t)guest_memory_base + guest_memory_size) {
        return (void *)guest;
    }
    return NULL;
}

/* ============================================================================
 * *at Family Syscall Implementations (directory FD variants)
 * ============================================================================ */

/**
 * syscall_openat - Open file relative to directory file descriptor
 *
 * Linux syscall: int openat(int dirfd, const char *pathname, int flags, mode_t mode);
 *
 * *at variant allows opening files relative to a directory file descriptor,
 * providing race-free path resolution and better security.
 */
int syscall_openat(ThreadState *state)
{
    int dirfd = GUEST_ARG0(state);
    const char *pathname = (const char *)GUEST_ARG1(state);
    int flags = GUEST_ARG2(state);
    mode_t mode = GUEST_ARG3(state);

    /* Call the actual syscall */
    int ret = openat(dirfd, pathname, flags, mode);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_mkdirat - Create directory relative to directory FD
 *
 * Linux syscall: int mkdirat(int dirfd, const char *pathname, mode_t mode);
 */
int syscall_mkdirat(ThreadState *state)
{
    int dirfd = GUEST_ARG0(state);
    const char *pathname = (const char *)GUEST_ARG1(state);
    mode_t mode = GUEST_ARG2(state);

    int ret = mkdirat(dirfd, pathname, mode);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_mknodat - Create special file relative to directory FD
 *
 * Linux syscall: int mknodat(int dirfd, const char *pathname,
 *                            mode_t mode, dev_t dev);
 */
int syscall_mknodat(ThreadState *state)
{
    int dirfd = GUEST_ARG0(state);
    const char *pathname = (const char *)GUEST_ARG1(state);
    mode_t mode = GUEST_ARG2(state);
    dev_t dev = (dev_t)GUEST_ARG3(state);

    int ret = mknodat(dirfd, pathname, mode, dev);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_fchownat - Change file ownership relative to directory FD
 *
 * Linux syscall: int fchownat(int dirfd, const char *pathname,
 *                             uid_t owner, gid_t group, int flags);
 */
int syscall_fchownat(ThreadState *state)
{
    int dirfd = GUEST_ARG0(state);
    const char *pathname = (const char *)GUEST_ARG1(state);
    uid_t owner = GUEST_ARG2(state);
    gid_t group = GUEST_ARG3(state);
    int flags = GUEST_ARG4(state);

    int ret = fchownat(dirfd, pathname, owner, group, flags);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_futimesat - Change file timestamps relative to directory FD
 *
 * Linux syscall: int futimesat(int dirfd, const char *pathname,
 *                              const struct timeval times[2]);
 */
int syscall_futimesat(ThreadState *state)
{
    int dirfd = GUEST_ARG0(state);
    const char *pathname = (const char *)GUEST_ARG1(state);
    const struct timeval *times = (const struct timeval *)GUEST_ARG2(state);

    int ret = futimesat(dirfd, pathname, times);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_newfstatat - Get file status relative to directory FD
 *
 * Linux syscall: int newfstatat(int dirfd, const char *pathname,
 *                               struct stat *statbuf, int flags);
 */
int syscall_newfstatat(ThreadState *state)
{
    int dirfd = GUEST_ARG0(state);
    const char *pathname = (const char *)GUEST_ARG1(state);
    struct stat *statbuf = (struct stat *)GUEST_ARG2(state);
    int flags = GUEST_ARG3(state);

    int ret = fstatat(dirfd, pathname, statbuf, flags);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_unlinkat - Unlink file relative to directory FD
 *
 * Linux syscall: int unlinkat(int dirfd, const char *pathname, int flags);
 */
int syscall_unlinkat(ThreadState *state)
{
    int dirfd = GUEST_ARG0(state);
    const char *pathname = (const char *)GUEST_ARG1(state);
    int flags = GUEST_ARG2(state);

    int ret = unlinkat(dirfd, pathname, flags);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_renameat - Rename file relative to directory FD
 *
 * Linux syscall: int renameat(int olddirfd, const char *oldpath,
 *                              int newdirfd, const char *newpath);
 */
int syscall_renameat(ThreadState *state)
{
    int olddirfd = GUEST_ARG0(state);
    const char *oldpath = (const char *)GUEST_ARG1(state);
    int newdirfd = GUEST_ARG2(state);
    const char *newpath = (const char *)GUEST_ARG3(state);

    int ret = renameat(olddirfd, oldpath, newdirfd, newpath);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_linkat - Create hard link relative to directory FD
 *
 * Linux syscall: int linkat(int olddirfd, const char *oldpath,
 *                          int newdirfd, const char *newpath, int flags);
 */
int syscall_linkat(ThreadState *state)
{
    int olddirfd = GUEST_ARG0(state);
    const char *oldpath = (const char *)GUEST_ARG1(state);
    int newdirfd = GUEST_ARG2(state);
    const char *newpath = (const char *)GUEST_ARG3(state);
    int flags = GUEST_ARG4(state);

    int ret = linkat(olddirfd, oldpath, newdirfd, newpath, flags);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_symlinkat - Create symlink relative to directory FD
 *
 * Linux syscall: int symlinkat(const char *oldpath, int newdirfd,
 *                              const char *newpath);
 */
int syscall_symlinkat(ThreadState *state)
{
    const char *oldpath = (const char *)GUEST_ARG0(state);
    int newdirfd = GUEST_ARG1(state);
    const char *newpath = (const char *)GUEST_ARG2(state);

    int ret = symlinkat(oldpath, newdirfd, newpath);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_readlinkat - Read symlink value relative to directory FD
 *
 * Linux syscall: ssize_t readlinkat(int dirfd, const char *pathname,
 *                                   char *buf, size_t bufsiz);
 */
int syscall_readlinkat(ThreadState *state)
{
    int dirfd = GUEST_ARG0(state);
    const char *pathname = (const char *)GUEST_ARG1(state);
    char *buf = (char *)GUEST_ARG2(state);
    size_t bufsiz = GUEST_ARG3(state);

    ssize_t ret = readlinkat(dirfd, pathname, buf, bufsiz);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_fchmodat - Change file mode relative to directory FD
 *
 * Linux syscall: int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);
 */
int syscall_fchmodat(ThreadState *state)
{
    int dirfd = GUEST_ARG0(state);
    const char *pathname = (const char *)GUEST_ARG1(state);
    mode_t mode = GUEST_ARG2(state);
    int flags = GUEST_ARG3(state);

    int ret = fchmodat(dirfd, pathname, mode, flags);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_faccessat - Check file access relative to directory FD
 *
 * Linux syscall: int faccessat(int dirfd, const char *pathname, int mode, int flags);
 */
int syscall_faccessat(ThreadState *state)
{
    int dirfd = GUEST_ARG0(state);
    const char *pathname = (const char *)GUEST_ARG1(state);
    int mode = GUEST_ARG2(state);
    int flags = GUEST_ARG3(state);

    int ret = faccessat(dirfd, pathname, mode, flags);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_utimensat - Change file timestamps relative to directory FD
 *
 * Linux syscall: int utimensat(int dirfd, const char *pathname,
 *                              const struct timespec times[2], int flags);
 */
int syscall_utimensat(ThreadState *state)
{
    int dirfd = GUEST_ARG0(state);
    const char *pathname = (const char *)GUEST_ARG1(state);
    const struct timespec *times = (const struct timespec *)GUEST_ARG2(state);
    int flags = GUEST_ARG3(state);

    int ret = utimensat(dirfd, pathname, times, flags);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/* ============================================================================
 * Helper Utilities
 * ============================================================================ */

/**
 * helper_debug_trace - Debug trace helper
 */
void helper_debug_trace(const char *msg, uint64_t value)
{
    (void)msg;
    (void)value;
    /* Debug tracing - compiled out in release */
}

/**
 * helper_perf_counter - Performance counter helper
 */
uint64_t helper_perf_counter(void)
{
    uint64_t count;
#ifdef __x86_64__
    unsigned int lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    count = ((uint64_t)hi << 32) | lo;
#else
    count = 0;
#endif
    return count;
}

/**
 * helper_block_translate - Block translation helper
 */
void *helper_block_translate(uint64_t guest_pc)
{
    (void)guest_pc;
    return NULL;
}

/**
 * helper_block_insert - Block insert helper
 */
int helper_block_insert(uint64_t guest, uint64_t host, size_t size)
{
    (void)guest;
    (void)host;
    (void)size;
    return 0;
}

/**
 * helper_block_remove - Block remove helper
 */
int helper_block_remove(uint64_t guest_pc)
{
    (void)guest_pc;
    return 0;
}

/**
 * helper_block_invalidate - Block invalidate helper
 */
void helper_block_invalidate(void)
{
    /* Invalidate all cached translations */
}

/**
 * helper_context_switch - Context switch helper
 */
void helper_context_switch(CPUContext *old_ctx, CPUContext *new_ctx)
{
    if (old_ctx && new_ctx) {
        *old_ctx = *new_ctx;
    }
}

/**
 * helper_syscall_enter - Syscall enter hook
 */
void helper_syscall_enter(ThreadState *state, int nr)
{
    (void)state;
    (void)nr;
    /* Hook for syscall tracing/debugging */
}

/**
 * helper_syscall_exit - Syscall exit hook
 */
void helper_syscall_exit(ThreadState *state, int64_t result)
{
    (void)state;
    (void)result;
    /* Hook for syscall tracing/debugging */
}

/**
 * helper_interrupt - Interrupt handler
 */
void helper_interrupt(ThreadState *state, int vector)
{
    if (state == NULL) {
        return;
    }
    (void)vector;
    /* Handle hardware/software interrupts */
}

