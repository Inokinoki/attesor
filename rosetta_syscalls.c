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

#define _GNU_SOURCE
#define _DEFAULT_SOURCE

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
#ifdef __linux__
#include <sys/epoll.h>
#include <sys/prctl.h>
#endif

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

    /* File Operations (Priority 1) */
    {X86_64_SYS_RENAME,       ARM64_SYS_RENAME,       syscall_rename},
    {X86_64_SYS_MKDIR,        ARM64_SYS_MKDIR,        syscall_mkdir},
    {X86_64_SYS_RMDIR,        ARM64_SYS_RMDIR,        syscall_rmdir},
    {X86_64_SYS_UNLINK,       ARM64_SYS_UNLINK,       syscall_unlink},
    {X86_64_SYS_SYMLINK,      ARM64_SYS_SYMLINK,      syscall_symlink},
    {X86_64_SYS_READLINK,     ARM64_SYS_READLINK,     syscall_readlink},
    {X86_64_SYS_CHMOD,        ARM64_SYS_CHMOD,        syscall_chmod},
    {X86_64_SYS_FCHMOD,       ARM64_SYS_FCHMOD,       syscall_fchmod},
    {X86_64_SYS_CHOWN,        ARM64_SYS_CHOWN,        syscall_chown},
    {X86_64_SYS_FCHOWN,       ARM64_SYS_FCHOWN,       syscall_fchown},
    {X86_64_SYS_LCHOWN,       ARM64_SYS_LCHOWN,       syscall_lchown},
    {X86_64_SYS_CREAT,        ARM64_SYS_CREAT,        syscall_creat},

    /* Process Management (Priority 2) */
    {X86_64_SYS_GETPGID,      ARM64_SYS_GETPGID,      syscall_getpgid},
    {X86_64_SYS_GETSID,       ARM64_SYS_GETSID,       syscall_getsid},
    {X86_64_SYS_SETSID,       ARM64_SYS_SETSID,       syscall_setsid},
    {X86_64_SYS_GETGROUPS,    ARM64_SYS_GETGROUPS,    syscall_getgroups},
    {X86_64_SYS_SETGROUPS,    ARM64_SYS_SETGROUPS,    syscall_setgroups},
    {X86_64_SYS_SETHOSTNAME,  ARM64_SYS_SETHOSTNAME,  syscall_sethostname},
    {X86_64_SYS_SETDOMAINNAME, ARM64_SYS_SETDOMAINNAME, syscall_setdomainname},
    {X86_64_SYS_PRCTL,        -1,                     syscall_prctl},

    /* Memory Operations (Priority 3) */
    {X86_64_SYS_MADVISE,      ARM64_SYS_MADVISE,      syscall_madvise},

    /* Signal and Time */
    {X86_64_SYS_SET_TID_ADDRESS, ARM64_SYS_SET_TID_ADDRESS, syscall_set_tid_address},
    {X86_64_SYS_CLOCK_GETRES,  ARM64_SYS_CLOCK_GETRES, syscall_clock_getres},

    /* Futex and Robust List */
    {X86_64_SYS_SET_ROBUST_LIST, ARM64_SYS_SET_ROBUST_LIST, syscall_set_robust_list},
    {X86_64_SYS_GET_ROBUST_LIST, ARM64_SYS_GET_ROBUST_LIST, syscall_get_robust_list},

    /* Additional File */
    {X86_64_SYS_POLL,         ARM64_SYS_POLL,         syscall_poll},

    /* Clone and Exec */
    {X86_64_SYS_CLONE,        ARM64_SYS_CLONE,        syscall_clone},
    {X86_64_SYS_EXECVE,       ARM64_SYS_EXECVE,       syscall_execve},

    /* Memory Advanced */
    {X86_64_SYS_MINCORE,      -1,                     syscall_mincore},
    {X86_64_SYS_MLOCK,        -1,                     syscall_mlock},
    {X86_64_SYS_MUNLOCK,      -1,                     syscall_munlock},
    {X86_64_SYS_MSYNC,        -1,                     syscall_msync},
    {X86_64_SYS_PRLIMIT,      -1,                     syscall_prlimit},

    /* Additional */
    {X86_64_SYS_GETCPU,       -1,                     syscall_getcpu},
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
 * x86_64 Linux syscall ABI: RDI, RSI, RDX, R10, R8, R9
 * ARM64 Linux syscall ABI: X0,  X1,  X2,  X3,  X4, X5
 *
 * Both architectures use the same argument order, but in different registers.
 * This function ensures guest x86_64 register values are accessible
 * through the standard GUEST_ARG0..GUEST_ARG5 macros.
 *
 * In our current implementation, the ThreadState already maps
 * guest x86_64 registers to these macros, so this function is primarily
 * for documentation and potential future optimizations.
 */
void remap_syscall_args(ThreadState *state)
{
    /* The GUEST_ARG macros already provide the correct mapping:
     * GUEST_ARG0(state) -> state->guest.r[X86_RDI] (arg1)
     * GUEST_ARG1(state) -> state->guest.r[X86_RSI] (arg2)
     * GUEST_ARG2(state) -> state->guest.r[X86_RDX] (arg3)
     * GUEST_ARG3(state) -> state->guest.r[X86_R10] (arg4)
     * GUEST_ARG4(state) -> state->guest.r[X86_R8]  (arg5)
     * GUEST_ARG5(state) -> state->guest.r[X86_R9]  (arg6)
     *
     * This matches the x86_64 syscall ABI where arguments are passed in
     * RDI, RSI, RDX, R10, R8, R9 (note: RCX is used for return address,
     * so R10 is used for the 4th argument instead of RCX).
     *
     * For ARM64 syscall ABI, the mapping would be:
     * X0 (arg1), X1 (arg2), X2 (arg3), X3 (arg4), X4 (arg5), X5 (arg6)
     *
     * No remapping is needed in our implementation since we extract
     * arguments directly from the guest x86_64 state.
     */
    (void)state;
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
