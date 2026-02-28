/* ============================================================================
 * Rosetta Refactored - Syscall Handlers Implementation
 * ============================================================================
 *
 * This module implements syscall handlers for the Rosetta translation layer.
 * These handlers translate ARM64 Linux syscalls to x86_64/macOS equivalents.
 * ============================================================================ */

#include "rosetta_refactored_syscall_handlers.h"
#include "rosetta_refactored.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>

/* ============================================================================
 * Syscall Infrastructure
 * ============================================================================ */

/**
 * syscall_get_nr - Get syscall number from thread state
 * @st: Thread state
 * Returns: Syscall number
 */
s32 syscall_get_nr(ThreadState *st)
{
    if (st == NULL) {
        return -1;
    }
    /* Syscall number is typically in X8 on ARM64 */
    return (s32)st->guest.x[8];
}

/**
 * syscall_set_result - Set syscall result in thread state
 * @st: Thread state
 * @res: Result value
 */
void syscall_set_result(ThreadState *st, s64 res)
{
    if (st == NULL) {
        return;
    }
    /* Result is typically returned in X0 on ARM64 */
    st->guest.x[0] = (u64)res;
}

/**
 * syscall_dispatch - Dispatch syscall to appropriate handler
 * @st: Thread state
 * Returns: Syscall result
 */
s64 syscall_dispatch(ThreadState *st)
{
    s32 nr;
    syscall_handler_func_t handler;
    s64 result;

    if (st == NULL) {
        return -EFAULT;
    }

    /* Get syscall number */
    nr = syscall_get_nr(st);

    /* Validate syscall number */
    if (nr < 0 || nr >= SYSCALL_HANDLER_MAX) {
        return -ENOSYS;
    }

    /* Get handler */
    handler = syscall_handler_table[nr];
    if (handler == NULL) {
        return -ENOSYS;
    }

    /* Call handler */
    result = handler(st);

    return result;
}

/* ============================================================================
 * Syscall Argument Helpers
 * ============================================================================ */

#define GET_ARG0(st) ((st)->guest.x[0])
#define GET_ARG1(st) ((st)->guest.x[1])
#define GET_ARG2(st) ((st)->guest.x[2])
#define GET_ARG3(st) ((st)->guest.x[3])
#define GET_ARG4(st) ((st)->guest.x[4])
#define GET_ARG5(st) ((st)->guest.x[5])

/* ============================================================================
 * Basic Syscall Handlers
 * ============================================================================ */

/**
 * syscall_handler_read_impl - Read from file descriptor
 */
static s64 syscall_handler_read_impl(ThreadState *st)
{
    s32 fd = (s32)GET_ARG0(st);
    void *buf = (void *)GET_ARG1(st);
    size_t count = (size_t)GET_ARG2(st);
    ssize_t result;

    result = read(fd, buf, count);
    if (result == -1) {
        return -errno;
    }
    return result;
}

/**
 * syscall_handler_write_impl - Write to file descriptor
 */
static s64 syscall_handler_write_impl(ThreadState *st)
{
    s32 fd = (s32)GET_ARG0(st);
    const void *buf = (const void *)GET_ARG1(st);
    size_t count = (size_t)GET_ARG2(st);
    ssize_t result;

    result = write(fd, buf, count);
    if (result == -1) {
        return -errno;
    }
    return result;
}

/**
 * syscall_handler_close_impl - Close file descriptor
 */
static s64 syscall_handler_close_impl(ThreadState *st)
{
    s32 fd = (s32)GET_ARG0(st);
    s32 result;

    result = close(fd);
    if (result == -1) {
        return -errno;
    }
    return result;
}

/**
 * syscall_handler_open_impl - Open file
 */
static s64 syscall_handler_open_impl(ThreadState *st)
{
    const char *path = (const char *)GET_ARG0(st);
    int flags = (int)GET_ARG1(st);
    int result;

    result = open(path, flags);
    if (result == -1) {
        return -errno;
    }
    return result;
}

/**
 * syscall_handler_lseek_impl - Seek in file
 */
static s64 syscall_handler_lseek_impl(ThreadState *st)
{
    s32 fd = (s32)GET_ARG0(st);
    s64 offset = (s64)GET_ARG1(st);
    s32 whence = (s32)GET_ARG2(st);
    off_t result;

    result = lseek(fd, offset, whence);
    if (result == -1) {
        return -errno;
    }
    return result;
}

/* ============================================================================
 * Memory Management Syscall Handlers
 * ============================================================================ */

/**
 * syscall_handler_mmap_impl - Map files or devices into memory
 */
static s64 syscall_handler_mmap_impl(ThreadState *st)
{
    u64 addr = GET_ARG0(st);
    size_t len = (size_t)GET_ARG1(st);
    s32 prot = (s32)GET_ARG2(st);
    s32 flags = (s32)GET_ARG3(st);
    s32 fd = (s32)GET_ARG4(st);
    s64 offset = (s64)GET_ARG5(st);
    void *result;

    (void)addr; /* Let kernel choose address */

    result = mmap(NULL, len, prot, flags | MAP_ANONYMOUS, fd, offset);
    if (result == MAP_FAILED) {
        return -errno;
    }
    return (s64)(uintptr_t)result;
}

/**
 * syscall_handler_mprotect_impl - Set protection on memory region
 */
static s64 syscall_handler_mprotect_impl(ThreadState *st)
{
    u64 addr = GET_ARG0(st);
    size_t len = (size_t)GET_ARG1(st);
    s32 prot = (s32)GET_ARG2(st);
    s32 result;

    result = mprotect((void *)(uintptr_t)addr, len, prot);
    if (result == -1) {
        return -errno;
    }
    return 0;
}

/**
 * syscall_handler_munmap_impl - Unmap memory region
 */
static s64 syscall_handler_munmap_impl(ThreadState *st)
{
    u64 addr = GET_ARG0(st);
    size_t len = (size_t)GET_ARG1(st);
    s32 result;

    result = munmap((void *)(uintptr_t)addr, len);
    if (result == -1) {
        return -errno;
    }
    return 0;
}

/**
 * syscall_handler_brk_impl - Change data segment size
 */
static s64 syscall_handler_brk_impl(ThreadState *st)
{
    u64 addr = GET_ARG0(st);
    void *result;

    if (addr == 0) {
        /* Return current break */
        return (s64)(uintptr_t)sbrk(0);
    }

    result = sbrk((intptr_t)addr - (intptr_t)sbrk(0));
    if (result == MAP_FAILED) {
        return -errno;
    }
    return addr;
}

/* ============================================================================
 * Process/Signal Syscall Handlers
 * ============================================================================ */

/**
 * syscall_handler_exit_impl - Exit process
 */
static s64 syscall_handler_exit_impl(ThreadState *st)
{
    s32 status = (s32)GET_ARG0(st);
    exit(status);
    return 0; /* Never reached */
}

/**
 * syscall_handler_getpid_impl - Get process ID
 */
static s64 syscall_handler_getpid_impl(ThreadState *st)
{
    (void)st;
    return getpid();
}

/**
 * syscall_handler_getuid_impl - Get user ID
 */
static s64 syscall_handler_getuid_impl(ThreadState *st)
{
    (void)st;
    return getuid();
}

/**
 * syscall_handler_geteuid_impl - Get effective user ID
 */
static s64 syscall_handler_geteuid_impl(ThreadState *st)
{
    (void)st;
    return geteuid();
}

/**
 * syscall_handler_getgid_impl - Get group ID
 */
static s64 syscall_handler_getgid_impl(ThreadState *st)
{
    (void)st;
    return getgid();
}

/**
 * syscall_handler_getegid_impl - Get effective group ID
 */
static s64 syscall_handler_getegid_impl(ThreadState *st)
{
    (void)st;
    return getegid();
}

/**
 * syscall_handler_sigaction_impl - Examine and change signal action
 */
static s64 syscall_handler_sigaction_impl(ThreadState *st)
{
    s32 signum = (s32)GET_ARG0(st);
    void *act = (void *)GET_ARG1(st);
    void *oldact = (void *)GET_ARG2(st);

    (void)act;
    (void)oldact;

    /* Simplified - just return success */
    if (signum <= 0 || signum >= NSIG) {
        return -EINVAL;
    }
    return 0;
}

/* ============================================================================
 * File Stat Syscall Handlers
 * ============================================================================ */

/**
 * syscall_handler_fstat_impl - Get file status by fd
 */
static s64 syscall_handler_fstat_impl(ThreadState *st)
{
    s32 fd = (s32)GET_ARG0(st);
    void *statbuf = (void *)GET_ARG1(st);
    s32 result;

    result = fstat(fd, (struct stat *)statbuf);
    if (result == -1) {
        return -errno;
    }
    return 0;
}

/**
 * syscall_handler_stat_impl - Get file status by path
 */
static s64 syscall_handler_stat_impl(ThreadState *st)
{
    const char *path = (const char *)GET_ARG0(st);
    void *statbuf = (void *)GET_ARG1(st);
    s32 result;

    result = stat(path, (struct stat *)statbuf);
    if (result == -1) {
        return -errno;
    }
    return 0;
}

/**
 * syscall_handler_access_impl - Check file accessibility
 */
static s64 syscall_handler_access_impl(ThreadState *st)
{
    const char *path = (const char *)GET_ARG0(st);
    s32 mode = (s32)GET_ARG1(st);
    s32 result;

    result = access(path, mode);
    if (result == -1) {
        return -errno;
    }
    return 0;
}

/* ============================================================================
 * Poll/Select Syscall Handlers
 * ============================================================================ */

/**
 * syscall_handler_poll_impl - Wait for events on file descriptors
 */
static s64 syscall_handler_poll_impl(ThreadState *st)
{
    void *fds = (void *)GET_ARG0(st);
    s32 nfds = (s32)GET_ARG1(st);
    s32 timeout = (s32)GET_ARG2(st);
    s32 result;

    (void)timeout;

#ifdef __APPLE__
    result = poll((struct pollfd *)fds, nfds, timeout);
#else
    result = poll((struct pollfd *)fds, nfds, timeout);
#endif
    if (result == -1) {
        return -errno;
    }
    return result;
}

/**
 * syscall_handler_select_impl - Synchronous I/O multiplexing
 */
static s64 syscall_handler_select_impl(ThreadState *st)
{
    s32 nfds = (s32)GET_ARG0(st);
    void *readfds = (void *)GET_ARG1(st);
    void *writefds = (void *)GET_ARG2(st);
    void *exceptfds = (void *)GET_ARG3(st);
    void *timeout = (void *)GET_ARG4(st);
    s32 result;

    result = select(nfds, (fd_set *)readfds, (fd_set *)writefds,
                    (fd_set *)exceptfds, (struct timeval *)timeout);
    if (result == -1) {
        return -errno;
    }
    return result;
}

/* ============================================================================
 * Syscall Handler Table
 * ============================================================================ */

/* Syscall handler table - maps ARM64 Linux syscall numbers to handlers */
syscall_handler_func_t syscall_handler_table[SYSCALL_HANDLER_MAX] = {
    [0]      = syscall_handler_io_setup,       /* io_setup */
    [1]      = syscall_handler_io_destroy,     /* io_destroy */
    [2]      = syscall_handler_io_submit,      /* io_submit */
    [3]      = syscall_handler_io_cancel,      /* io_cancel */
    [4]      = syscall_handler_io_getevents,   /* io_getevents */
    [5]      = syscall_handler_setxattr,       /* setxattr */
    [6]      = syscall_handler_lsetxattr,      /* lsetxattr */
    [7]      = syscall_handler_fsetxattr,      /* fsetxattr */
    [8]      = syscall_handler_getxattr,       /* getxattr */
    [9]      = syscall_handler_lgetxattr,      /* lgetxattr */
    [10]     = syscall_handler_fgetxattr,      /* fgetxattr */
    [11]     = syscall_handler_listxattr,      /* listxattr */
    [12]     = syscall_handler_llistxattr,     /* llistxattr */
    [13]     = syscall_handler_flistxattr,     /* flistxattr */
    [14]     = syscall_handler_removexattr,    /* removexattr */
    [15]     = syscall_handler_lremovexattr,   /* lremovexattr */
    [16]     = syscall_handler_fremovexattr,   /* fremovexattr */
    [17]     = syscall_handler_getcwd,         /* getcwd */
    [18]     = syscall_handler_lookup_dcookie, /* lookup_dcookie */
    [19]     = syscall_handler_eventfd2,       /* eventfd2 */
    [20]     = syscall_handler_epoll_create1,  /* epoll_create1 */
    [21]     = syscall_handler_epoll_ctl,      /* epoll_ctl */
    [22]     = syscall_handler_epoll_pwait,    /* epoll_pwait */
    [23]     = syscall_handler_dup,            /* dup */
    [24]     = syscall_handler_dup3,           /* dup3 */
    [25]     = syscall_handler_fcntl,          /* fcntl */
    [26]     = syscall_handler_inotify_init1,  /* inotify_init1 */
    [27]     = syscall_handler_inotify_add_watch, /* inotify_add_watch */
    [28]     = syscall_handler_inotify_rm_watch, /* inotify_rm_watch */
    [29]     = syscall_handler_ioctl,          /* ioctl */
    [30]     = syscall_handler_ioprio_set,     /* ioprio_set */
    [31]     = syscall_handler_ioprio_get,     /* ioprio_get */
    [32]     = syscall_handler_flock,          /* flock */
    [33]     = syscall_handler_mknodat,        /* mknodat */
    [34]     = syscall_handler_mkdirat,        /* mkdirat */
    [35]     = syscall_handler_unlinkat,       /* unlinkat */
    [36]     = syscall_handler_symlinkat,      /* symlinkat */
    [37]     = syscall_handler_linkat,         /* linkat */
    [38]     = syscall_handler_umount2,        /* umount2 */
    [39]     = syscall_handler_mount,          /* mount */
    [40]     = syscall_handler_pivot_root,     /* pivot_root */
    [41]     = syscall_handler_nfsservctl,     /* nfsservctl */
    [42]     = syscall_handler_statfs,         /* statfs */
    [43]     = syscall_handler_fstatfs,        /* fstatfs */
    [44]     = syscall_handler_truncate,       /* truncate */
    [45]     = syscall_handler_ftruncate,      /* ftruncate */
    [46]     = syscall_handler_fallocate,      /* fallocate */
    [47]     = syscall_handler_faccessat,      /* faccessat */
    [48]     = syscall_handler_chdir,          /* chdir */
    [49]     = syscall_handler_fchdir,         /* fchdir */
    [50]     = syscall_handler_chroot,         /* chroot */
    [51]     = syscall_handler_fchmod,         /* fchmod */
    [52]     = syscall_handler_fchmodat,       /* fchmodat */
    [53]     = syscall_handler_fchownat,       /* fchownat */
    [54]     = syscall_handler_fchown,         /* fchown */
    [55]     = syscall_handler_openat,         /* openat */
    [56]     = syscall_handler_close,          /* close */
    [57]     = syscall_handler_vmsplice,       /* vmsplice */
    [58]     = syscall_handler_splice,         /* splice */
    [59]     = syscall_handler_tee,            /* tee */
    [60]     = syscall_handler_readlinkat,     /* readlinkat */
    [61]     = syscall_handler_fstatat,        /* fstatat */
    [62]     = syscall_handler_fstat,          /* fstat */
    [63]     = syscall_handler_sync,           /* sync */
    [64]     = syscall_handler_fsync,          /* fsync */
    [65]     = syscall_handler_fdatasync,      /* fdatasync */
    [66]     = syscall_handler_sync_file_range, /* sync_file_range */
    [67]     = syscall_handler_timerfd_create, /* timerfd_create */
    [68]     = syscall_handler_timerfd_settime, /* timerfd_settime */
    [69]     = syscall_handler_timerfd_gettime, /* timerfd_gettime */
    [70]     = syscall_handler_utimensat,      /* utimensat */
    [71]     = syscall_handler_acct,           /* acct */
    [72]     = syscall_handler_capget,         /* capget */
    [73]     = syscall_handler_capset,         /* capset */
    [74]     = syscall_handler_personality,    /* personality */
    [75]     = syscall_handler_exit,           /* exit */
    [76]     = syscall_handler_exit_group,     /* exit_group */
    [77]     = syscall_handler_waitid,         /* waitid */
    [78]     = syscall_handler_set_tid_address, /* set_tid_address */
    [79]     = syscall_handler_unshare,        /* unshare */
    [80]     = syscall_handler_futex,          /* futex */
    [81]     = syscall_handler_set_robust_list, /* set_robust_list */
    [82]     = syscall_handler_get_robust_list, /* get_robust_list */
    [83]     = syscall_handler_nanosleep,      /* nanosleep */
    [84]     = syscall_handler_getitimer,      /* getitimer */
    [85]     = syscall_handler_setitimer,      /* setitimer */
    [86]     = syscall_handler_kexec_load,     /* kexec_load */
    [87]     = syscall_handler_init_module,    /* init_module */
    [88]     = syscall_handler_delete_module,  /* delete_module */
    [89]     = syscall_handler_timer_create,   /* timer_create */
    [90]     = syscall_handler_timer_gettime,  /* timer_gettime */
    [91]     = syscall_handler_timer_getoverrun, /* timer_getoverrun */
    [92]     = syscall_handler_timer_settime,  /* timer_settime */
    [93]     = syscall_handler_timer_delete,   /* timer_delete */
    [94]     = syscall_handler_clock_settime,  /* clock_settime */
    [95]     = syscall_handler_clock_gettime,  /* clock_gettime */
    [96]     = syscall_handler_clock_getres,   /* clock_getres */
    [97]     = syscall_handler_clock_nanosleep, /* clock_nanosleep */
    [98]     = syscall_handler_syslog,         /* syslog */
    [99]     = syscall_handler_ptrace,         /* ptrace */
    [100]    = syscall_handler_sched_setparam, /* sched_setparam */
    [101]    = syscall_handler_sched_setscheduler, /* sched_setscheduler */
    [102]    = syscall_handler_sched_getscheduler, /* sched_getscheduler */
    [103]    = syscall_handler_sched_getparam, /* sched_getparam */
    [104]    = syscall_handler_sched_setaffinity, /* sched_setaffinity */
    [105]    = syscall_handler_sched_getaffinity, /* sched_getaffinity */
    [106]    = syscall_handler_sched_yield,    /* sched_yield */
    [107]    = syscall_handler_sched_get_priority_max, /* sched_get_priority_max */
    [108]    = syscall_handler_sched_get_priority_min, /* sched_get_priority_min */
    [109]    = syscall_handler_sched_rr_get_interval, /* sched_rr_get_interval */
    [110]    = syscall_handler_restart_syscall, /* restart_syscall */
    [111]    = syscall_handler_kill,           /* kill */
    [112]    = syscall_handler_tkill,          /* tkill */
    [113]    = syscall_handler_tgkill,         /* tgkill */
    [114]    = syscall_handler_sigaltstack,    /* sigaltstack */
    [115]    = syscall_handler_rt_sigsuspend,  /* rt_sigsuspend */
    [116]    = syscall_handler_rt_sigaction,   /* rt_sigaction */
    [117]    = syscall_handler_rt_sigprocmask, /* rt_sigprocmask */
    [118]    = syscall_handler_rt_sigpending,  /* rt_sigpending */
    [119]    = syscall_handler_rt_sigtimedwait, /* rt_sigtimedwait */
    [120]    = syscall_handler_rt_sigqueueinfo, /* rt_sigqueueinfo */
    [121]    = syscall_handler_rt_sigreturn,   /* rt_sigreturn */
    [122]    = syscall_handler_setpriority,    /* setpriority */
    [123]    = syscall_handler_getpriority,    /* getpriority */
    [124]    = syscall_handler_reboot,         /* reboot */
    [125]    = syscall_handler_setregid,       /* setregid */
    [126]    = syscall_handler_setgid,         /* setgid */
    [127]    = syscall_handler_setreuid,       /* setreuid */
    [128]    = syscall_handler_setuid,         /* setuid */
    [129]    = syscall_handler_setresuid,      /* setresuid */
    [130]    = syscall_handler_getresuid,      /* getresuid */
    [131]    = syscall_handler_setresgid,      /* setresgid */
    [132]    = syscall_handler_getresgid,      /* getresgid */
    [133]    = syscall_handler_setfsuid,       /* setfsuid */
    [134]    = syscall_handler_setfsgid,       /* setfsgid */
    [135]    = syscall_handler_times,          /* times */
    [136]    = syscall_handler_setpgid,        /* setpgid */
    [137]    = syscall_handler_getpgid,        /* getpgid */
    [138]    = syscall_handler_getsid,         /* getsid */
    [139]    = syscall_handler_setsid,         /* setsid */
    [140]    = syscall_handler_getgroups,      /* getgroups */
    [141]    = syscall_handler_setgroups,      /* setgroups */
    [142]    = syscall_handler_uname,          /* uname */
    [143]    = syscall_handler_sethostname,    /* sethostname */
    [144]    = syscall_handler_setdomainname,  /* setdomainname */
    [145]    = syscall_handler_getrlimit,      /* getrlimit */
    [146]    = syscall_handler_setrlimit,      /* setrlimit */
    [147]    = syscall_handler_getrusage,      /* getrusage */
    [148]    = syscall_handler_umask,          /* umask */
    [149]    = syscall_handler_prctl,          /* prctl */
    [150]    = syscall_handler_getcpu,         /* getcpu */
    [151]    = syscall_handler_gettimeofday,   /* gettimeofday */
    [152]    = syscall_handler_settimeofday,   /* settimeofday */
    [153]    = syscall_handler_adjtimex,       /* adjtimex */
    [154]    = syscall_handler_getpid,         /* getpid */
    [155]    = syscall_handler_getppid,        /* getppid */
    [156]    = syscall_handler_getuid,         /* getuid */
    [157]    = syscall_handler_geteuid,        /* geteuid */
    [158]    = syscall_handler_getgid,         /* getgid */
    [159]    = syscall_handler_getegid,        /* getegid */
    [160]    = syscall_handler_gettid,         /* gettid */
    [161]    = syscall_handler_sysinfo,        /* sysinfo */
    [162]    = syscall_handler_mq_open,        /* mq_open */
    [163]    = syscall_handler_mq_unlink,      /* mq_unlink */
    [164]    = syscall_handler_mq_timedsend,   /* mq_timedsend */
    [165]    = syscall_handler_mq_timedreceive, /* mq_timedreceive */
    [166]    = syscall_handler_mq_notify,      /* mq_notify */
    [167]    = syscall_handler_mq_getsetattr,  /* mq_getsetattr */
    [168]    = syscall_handler_msgget,         /* msgget */
    [169]    = syscall_handler_msgctl,         /* msgctl */
    [170]    = syscall_handler_msgrcv,         /* msgrcv */
    [171]    = syscall_handler_msgsnd,         /* msgsnd */
    [172]    = syscall_handler_semget,         /* semget */
    [173]    = syscall_handler_semctl,         /* semctl */
    [174]    = syscall_handler_semtimedop,     /* semtimedop */
    [175]    = syscall_handler_semop,          /* semop */
    [176]    = syscall_handler_shmget,         /* shmget */
    [177]    = syscall_handler_shmctl,         /* shmctl */
    [178]    = syscall_handler_shmat,          /* shmat */
    [179]    = syscall_handler_shmdt,          /* shmdt */
    [180]    = syscall_handler_socket,         /* socket */
    [181]    = syscall_handler_socketpair,     /* socketpair */
    [182]    = syscall_handler_bind,           /* bind */
    [183]    = syscall_handler_listen,         /* listen */
    [184]    = syscall_handler_accept,         /* accept */
    [185]    = syscall_handler_connect,        /* connect */
    [186]    = syscall_handler_getsockname,    /* getsockname */
    [187]    = syscall_handler_getpeername,    /* getpeername */
    [188]    = syscall_handler_sendto,         /* sendto */
    [189]    = syscall_handler_recvfrom,       /* recvfrom */
    [190]    = syscall_handler_setsockopt,     /* setsockopt */
    [191]    = syscall_handler_getsockopt,     /* getsockopt */
    [192]    = syscall_handler_shutdown,       /* shutdown */
    [193]    = syscall_handler_sendmsg,        /* sendmsg */
    [194]    = syscall_handler_recvmsg,        /* recvmsg */
    [195]    = syscall_handler_readahead,      /* readahead */
    [196]    = syscall_handler_brk,            /* brk */
    [197]    = syscall_handler_munmap,         /* munmap */
    [198]    = syscall_handler_mremap,         /* mremap */
    [199]    = syscall_handler_add_key,        /* add_key */
    [200]    = syscall_handler_request_key,    /* request_key */
    [201]    = syscall_handler_keyctl,         /* keyctl */
    [202]    = syscall_handler_clone,          /* clone */
    [203]    = syscall_handler_execve,         /* execve */
    [204]    = syscall_handler_mmap,           /* mmap */
    [205]    = syscall_handler_fadvise64,      /* fadvise64 */
    [206]    = syscall_handler_swapon,         /* swapon */
    [207]    = syscall_handler_swapoff,        /* swapoff */
    [208]    = syscall_handler_mprotect,       /* mprotect */
    [209]    = syscall_handler_msync,          /* msync */
    [210]    = syscall_handler_mlock,          /* mlock */
    [211]    = syscall_handler_munlock,        /* munlock */
    [212]    = syscall_handler_mlockall,       /* mlockall */
    [213]    = syscall_handler_munlockall,     /* munlockall */
    [214]    = syscall_handler_mincore,        /* mincore */
    [215]    = syscall_handler_madvise,        /* madvise */
    [216]    = syscall_handler_remap_file_pages, /* remap_file_pages */
    [217]    = syscall_handler_mbind,          /* mbind */
    [218]    = syscall_handler_get_mempolicy,  /* get_mempolicy */
    [219]    = syscall_handler_set_mempolicy,  /* set_mempolicy */
    [220]    = syscall_handler_migrate_pages,  /* migrate_pages */
    [221]    = syscall_handler_move_pages,     /* move_pages */
    [222]    = syscall_handler_rt_tgsigqueueinfo, /* rt_tgsigqueueinfo */
    [223]    = syscall_handler_perf_event_open, /* perf_event_open */
    [224]    = syscall_handler_accept4,        /* accept4 */
    [225]    = syscall_handler_wait4,          /* wait4 */
    [226]    = syscall_handler_prlimit64,      /* prlimit64 */
    [227]    = syscall_handler_fanotify_init,  /* fanotify_init */
    [228]    = syscall_handler_fanotify_mark,  /* fanotify_mark */
    [229]    = syscall_handler_name_to_handle_at, /* name_to_handle_at */
    [230]    = syscall_handler_open_by_handle_at, /* open_by_handle_at */
    [231]    = syscall_handler_clock_adjtime,  /* clock_adjtime */
    [232]    = syscall_handler_syncfs,         /* syncfs */
    [233]    = syscall_handler_setns,          /* setns */
    [234]    = syscall_handler_sendmmsg,       /* sendmmsg */
    [235]    = syscall_handler_process_vm_readv, /* process_vm_readv */
    [236]    = syscall_handler_process_vm_writev, /* process_vm_writev */
    [237]    = syscall_handler_kcmp,           /* kcmp */
    [238]    = syscall_handler_finit_module,   /* finit_module */
    [239]    = syscall_handler_sched_setattr,  /* sched_setattr */
    [240]    = syscall_handler_sched_getattr,  /* sched_getattr */
    [241]    = syscall_handler_renameat2,      /* renameat2 */
    [242]    = syscall_handler_seccomp,        /* seccomp */
    [243]    = syscall_handler_getrandom,      /* getrandom */
    [244]    = syscall_handler_memfd_create,   /* memfd_create */
    [245]    = syscall_handler_kexec_file_load, /* kexec_file_load */
    [246]    = syscall_handler_bpf,            /* bpf */
    [247]    = syscall_handler_execveat,       /* execveat */
    [248]    = syscall_handler_userfaultfd,    /* userfaultfd */
    [249]    = syscall_handler_membarrier,     /* membarrier */
    [250]    = syscall_handler_mlock2,         /* mlock2 */
    [251]    = syscall_handler_copy_file_range, /* copy_file_range */
    [252]    = syscall_handler_preadv2,        /* preadv2 */
    [253]    = syscall_handler_pwritev2,       /* pwritev2 */
    [254]    = syscall_handler_pkey_mprotect,  /* pkey_mprotect */
    [255]    = syscall_handler_pkey_alloc,     /* pkey_alloc */
    [256]    = syscall_handler_pkey_free2,      /* pkey_free */
    [257]    = syscall_handler_statx,          /* statx */
    [258]    = syscall_handler_io_pgetevents,  /* io_pgetevents */
    [259]    = syscall_handler_rseq,           /* rseq */
    [260]    = syscall_handler_kexec_load,     /* kexec_load (2) */
    [261]    = syscall_handler_pkey_free2,     /* pkey_free (2) */
    [262]    = syscall_handler_io_uring_setup, /* io_uring_setup */
    [263]    = syscall_handler_io_uring_enter, /* io_uring_enter */
    [264]    = syscall_handler_io_uring_register, /* io_uring_register */
    [265]    = syscall_handler_openat2,        /* openat2 */
    [266]    = syscall_handler_close_range,    /* close_range */
    [267]    = syscall_handler_copy_file_range2, /* copy_file_range (2) */
    [268]    = syscall_handler_statx2,         /* statx (2) */
    [269]    = syscall_handler_mount_setattr,  /* mount_setattr */
    [270]    = syscall_handler_quotactl_fd,    /* quotactl_fd */
    [271]    = syscall_handler_landlock_create_ruleset, /* landlock_create_ruleset */
    [272]    = syscall_handler_landlock_add_rule, /* landlock_add_rule */
    [273]    = syscall_handler_landlock_restrict_self, /* landlock_restrict_self */
    [274]    = syscall_handler_process_mrelease, /* process_mrelease */
};

/* Stub implementations for unimplemented syscalls */
static s64 syscall_handler_unimplemented(ThreadState *st)
{
    (void)st;
    return -ENOSYS;
}

/* Define all stub handlers */
#define STUB_HANDLER(name) s64 name(ThreadState *st) { (void)st; return -ENOSYS; }

STUB_HANDLER(syscall_handler_io_setup)
STUB_HANDLER(syscall_handler_io_destroy)
STUB_HANDLER(syscall_handler_io_submit)
STUB_HANDLER(syscall_handler_io_cancel)
STUB_HANDLER(syscall_handler_io_getevents)
STUB_HANDLER(syscall_handler_setxattr)
STUB_HANDLER(syscall_handler_lsetxattr)
STUB_HANDLER(syscall_handler_fsetxattr)
STUB_HANDLER(syscall_handler_getxattr)
STUB_HANDLER(syscall_handler_lgetxattr)
STUB_HANDLER(syscall_handler_fgetxattr)
STUB_HANDLER(syscall_handler_listxattr)
STUB_HANDLER(syscall_handler_llistxattr)
STUB_HANDLER(syscall_handler_flistxattr)
STUB_HANDLER(syscall_handler_removexattr)
STUB_HANDLER(syscall_handler_lremovexattr)
STUB_HANDLER(syscall_handler_fremovexattr)
STUB_HANDLER(syscall_handler_getcwd)
STUB_HANDLER(syscall_handler_lookup_dcookie)
STUB_HANDLER(syscall_handler_eventfd2)
STUB_HANDLER(syscall_handler_epoll_create1)
STUB_HANDLER(syscall_handler_epoll_ctl)
STUB_HANDLER(syscall_handler_epoll_pwait)
STUB_HANDLER(syscall_handler_dup)
STUB_HANDLER(syscall_handler_dup3)
STUB_HANDLER(syscall_handler_fcntl)
STUB_HANDLER(syscall_handler_inotify_init1)
STUB_HANDLER(syscall_handler_inotify_add_watch)
STUB_HANDLER(syscall_handler_inotify_rm_watch)
STUB_HANDLER(syscall_handler_ioprio_set)
STUB_HANDLER(syscall_handler_ioprio_get)
STUB_HANDLER(syscall_handler_flock)
STUB_HANDLER(syscall_handler_mknodat)
STUB_HANDLER(syscall_handler_mkdirat)
STUB_HANDLER(syscall_handler_unlinkat)
STUB_HANDLER(syscall_handler_symlinkat)
STUB_HANDLER(syscall_handler_linkat)
STUB_HANDLER(syscall_handler_umount2)
STUB_HANDLER(syscall_handler_mount)
STUB_HANDLER(syscall_handler_pivot_root)
STUB_HANDLER(syscall_handler_nfsservctl)
STUB_HANDLER(syscall_handler_statfs)
STUB_HANDLER(syscall_handler_fstatfs)
STUB_HANDLER(syscall_handler_truncate)
STUB_HANDLER(syscall_handler_ftruncate)
STUB_HANDLER(syscall_handler_fallocate)
STUB_HANDLER(syscall_handler_faccessat)
STUB_HANDLER(syscall_handler_chdir)
STUB_HANDLER(syscall_handler_fchdir)
STUB_HANDLER(syscall_handler_chroot)
STUB_HANDLER(syscall_handler_fchmod)
STUB_HANDLER(syscall_handler_fchmodat)
STUB_HANDLER(syscall_handler_fchownat)
STUB_HANDLER(syscall_handler_fchown)
STUB_HANDLER(syscall_handler_openat)
/* syscall_handler_close - defined below with implementation */
STUB_HANDLER(syscall_handler_vmsplice)
STUB_HANDLER(syscall_handler_splice)
STUB_HANDLER(syscall_handler_tee)
STUB_HANDLER(syscall_handler_readlinkat)
STUB_HANDLER(syscall_handler_fstatat)
STUB_HANDLER(syscall_handler_sync)
STUB_HANDLER(syscall_handler_fsync)
STUB_HANDLER(syscall_handler_fdatasync)
STUB_HANDLER(syscall_handler_sync_file_range)
STUB_HANDLER(syscall_handler_timerfd_create)
STUB_HANDLER(syscall_handler_timerfd_settime)
STUB_HANDLER(syscall_handler_timerfd_gettime)
STUB_HANDLER(syscall_handler_utimensat)
STUB_HANDLER(syscall_handler_acct)
STUB_HANDLER(syscall_handler_capget)
STUB_HANDLER(syscall_handler_capset)
STUB_HANDLER(syscall_handler_personality)
STUB_HANDLER(syscall_handler_exit_group)
STUB_HANDLER(syscall_handler_waitid)
STUB_HANDLER(syscall_handler_set_tid_address)
STUB_HANDLER(syscall_handler_unshare)
STUB_HANDLER(syscall_handler_futex)
STUB_HANDLER(syscall_handler_set_robust_list)
STUB_HANDLER(syscall_handler_get_robust_list)
STUB_HANDLER(syscall_handler_nanosleep)
STUB_HANDLER(syscall_handler_getitimer)
STUB_HANDLER(syscall_handler_setitimer)
STUB_HANDLER(syscall_handler_init_module)
STUB_HANDLER(syscall_handler_delete_module)
STUB_HANDLER(syscall_handler_timer_create)
STUB_HANDLER(syscall_handler_timer_gettime)
STUB_HANDLER(syscall_handler_timer_getoverrun)
STUB_HANDLER(syscall_handler_timer_settime)
STUB_HANDLER(syscall_handler_timer_delete)
STUB_HANDLER(syscall_handler_clock_settime)
STUB_HANDLER(syscall_handler_clock_gettime)
STUB_HANDLER(syscall_handler_clock_getres)
STUB_HANDLER(syscall_handler_clock_nanosleep)
STUB_HANDLER(syscall_handler_syslog)
STUB_HANDLER(syscall_handler_ptrace)
STUB_HANDLER(syscall_handler_sched_setparam)
STUB_HANDLER(syscall_handler_sched_setscheduler)
STUB_HANDLER(syscall_handler_sched_getscheduler)
STUB_HANDLER(syscall_handler_sched_getparam)
STUB_HANDLER(syscall_handler_sched_setaffinity)
STUB_HANDLER(syscall_handler_sched_getaffinity)
STUB_HANDLER(syscall_handler_sched_get_priority_max)
STUB_HANDLER(syscall_handler_sched_get_priority_min)
STUB_HANDLER(syscall_handler_sched_rr_get_interval)
STUB_HANDLER(syscall_handler_restart_syscall)
STUB_HANDLER(syscall_handler_kill)
STUB_HANDLER(syscall_handler_tkill)
STUB_HANDLER(syscall_handler_tgkill)
STUB_HANDLER(syscall_handler_sigaltstack)
STUB_HANDLER(syscall_handler_rt_sigsuspend)
STUB_HANDLER(syscall_handler_rt_sigprocmask)
STUB_HANDLER(syscall_handler_rt_sigpending)
STUB_HANDLER(syscall_handler_rt_sigtimedwait)
STUB_HANDLER(syscall_handler_rt_sigqueueinfo)
STUB_HANDLER(syscall_handler_rt_sigreturn)
STUB_HANDLER(syscall_handler_setpriority)
STUB_HANDLER(syscall_handler_getpriority)
STUB_HANDLER(syscall_handler_reboot)
STUB_HANDLER(syscall_handler_setregid)
STUB_HANDLER(syscall_handler_setgid)
STUB_HANDLER(syscall_handler_setreuid)
STUB_HANDLER(syscall_handler_setuid)
STUB_HANDLER(syscall_handler_setresuid)
STUB_HANDLER(syscall_handler_getresuid)
STUB_HANDLER(syscall_handler_setresgid)
STUB_HANDLER(syscall_handler_getresgid)
STUB_HANDLER(syscall_handler_setfsuid)
STUB_HANDLER(syscall_handler_setfsgid)
STUB_HANDLER(syscall_handler_times)
STUB_HANDLER(syscall_handler_setpgid)
STUB_HANDLER(syscall_handler_getpgid)
STUB_HANDLER(syscall_handler_getsid)
STUB_HANDLER(syscall_handler_setsid)
STUB_HANDLER(syscall_handler_getgroups)
STUB_HANDLER(syscall_handler_setgroups)
STUB_HANDLER(syscall_handler_uname)
STUB_HANDLER(syscall_handler_sethostname)
STUB_HANDLER(syscall_handler_setdomainname)
STUB_HANDLER(syscall_handler_getrlimit)
STUB_HANDLER(syscall_handler_setrlimit)
STUB_HANDLER(syscall_handler_getrusage)
STUB_HANDLER(syscall_handler_umask)
STUB_HANDLER(syscall_handler_prctl)
STUB_HANDLER(syscall_handler_getcpu)
STUB_HANDLER(syscall_handler_gettimeofday)
STUB_HANDLER(syscall_handler_settimeofday)
STUB_HANDLER(syscall_handler_adjtimex)
STUB_HANDLER(syscall_handler_getppid)
STUB_HANDLER(syscall_handler_gettid)
STUB_HANDLER(syscall_handler_sysinfo)
STUB_HANDLER(syscall_handler_mq_open)
STUB_HANDLER(syscall_handler_mq_unlink)
STUB_HANDLER(syscall_handler_mq_timedsend)
STUB_HANDLER(syscall_handler_mq_timedreceive)
STUB_HANDLER(syscall_handler_mq_notify)
STUB_HANDLER(syscall_handler_mq_getsetattr)
STUB_HANDLER(syscall_handler_msgget)
STUB_HANDLER(syscall_handler_msgctl)
STUB_HANDLER(syscall_handler_msgrcv)
STUB_HANDLER(syscall_handler_msgsnd)
STUB_HANDLER(syscall_handler_semget)
STUB_HANDLER(syscall_handler_semctl)
STUB_HANDLER(syscall_handler_semtimedop)
STUB_HANDLER(syscall_handler_semop)
STUB_HANDLER(syscall_handler_shmget)
STUB_HANDLER(syscall_handler_shmctl)
STUB_HANDLER(syscall_handler_shmat)
STUB_HANDLER(syscall_handler_shmdt)
STUB_HANDLER(syscall_handler_socket)
STUB_HANDLER(syscall_handler_socketpair)
STUB_HANDLER(syscall_handler_bind)
STUB_HANDLER(syscall_handler_listen)
STUB_HANDLER(syscall_handler_accept)
STUB_HANDLER(syscall_handler_connect)
STUB_HANDLER(syscall_handler_getsockname)
STUB_HANDLER(syscall_handler_getpeername)
STUB_HANDLER(syscall_handler_sendto)
STUB_HANDLER(syscall_handler_recvfrom)
STUB_HANDLER(syscall_handler_setsockopt)
STUB_HANDLER(syscall_handler_getsockopt)
STUB_HANDLER(syscall_handler_shutdown)
STUB_HANDLER(syscall_handler_sendmsg)
STUB_HANDLER(syscall_handler_recvmsg)
STUB_HANDLER(syscall_handler_readahead)
STUB_HANDLER(syscall_handler_mremap)
STUB_HANDLER(syscall_handler_add_key)
STUB_HANDLER(syscall_handler_request_key)
STUB_HANDLER(syscall_handler_keyctl)
STUB_HANDLER(syscall_handler_clone)
STUB_HANDLER(syscall_handler_execve)
STUB_HANDLER(syscall_handler_fadvise64)
STUB_HANDLER(syscall_handler_swapon)
STUB_HANDLER(syscall_handler_swapoff)
STUB_HANDLER(syscall_handler_msync)
STUB_HANDLER(syscall_handler_mlock)
STUB_HANDLER(syscall_handler_munlock)
STUB_HANDLER(syscall_handler_mlockall)
STUB_HANDLER(syscall_handler_munlockall)
STUB_HANDLER(syscall_handler_mincore)
STUB_HANDLER(syscall_handler_madvise)
STUB_HANDLER(syscall_handler_remap_file_pages)
STUB_HANDLER(syscall_handler_mbind)
STUB_HANDLER(syscall_handler_get_mempolicy)
STUB_HANDLER(syscall_handler_set_mempolicy)
STUB_HANDLER(syscall_handler_migrate_pages)
STUB_HANDLER(syscall_handler_move_pages)
STUB_HANDLER(syscall_handler_rt_tgsigqueueinfo)
STUB_HANDLER(syscall_handler_perf_event_open)
STUB_HANDLER(syscall_handler_accept4)
STUB_HANDLER(syscall_handler_wait4)
STUB_HANDLER(syscall_handler_prlimit64)
STUB_HANDLER(syscall_handler_fanotify_init)
STUB_HANDLER(syscall_handler_fanotify_mark)
STUB_HANDLER(syscall_handler_name_to_handle_at)
STUB_HANDLER(syscall_handler_open_by_handle_at)
STUB_HANDLER(syscall_handler_clock_adjtime)
STUB_HANDLER(syscall_handler_syncfs)
STUB_HANDLER(syscall_handler_setns)
STUB_HANDLER(syscall_handler_sendmmsg)
STUB_HANDLER(syscall_handler_process_vm_readv)
STUB_HANDLER(syscall_handler_process_vm_writev)
STUB_HANDLER(syscall_handler_kcmp)
STUB_HANDLER(syscall_handler_finit_module)
STUB_HANDLER(syscall_handler_sched_setattr)
STUB_HANDLER(syscall_handler_sched_getattr)
STUB_HANDLER(syscall_handler_renameat2)
STUB_HANDLER(syscall_handler_seccomp)
STUB_HANDLER(syscall_handler_getrandom)
STUB_HANDLER(syscall_handler_memfd_create)
STUB_HANDLER(syscall_handler_kexec_file_load)
STUB_HANDLER(syscall_handler_bpf)
STUB_HANDLER(syscall_handler_execveat)
STUB_HANDLER(syscall_handler_userfaultfd)
STUB_HANDLER(syscall_handler_membarrier)
STUB_HANDLER(syscall_handler_mlock2)
STUB_HANDLER(syscall_handler_copy_file_range)
STUB_HANDLER(syscall_handler_preadv2)
STUB_HANDLER(syscall_handler_pwritev2)
STUB_HANDLER(syscall_handler_pkey_mprotect)
STUB_HANDLER(syscall_handler_pkey_alloc)
STUB_HANDLER(syscall_handler_pkey_free2)
STUB_HANDLER(syscall_handler_statx)
STUB_HANDLER(syscall_handler_io_pgetevents)
STUB_HANDLER(syscall_handler_rseq)
STUB_HANDLER(syscall_handler_io_uring_setup)
STUB_HANDLER(syscall_handler_io_uring_enter)
STUB_HANDLER(syscall_handler_io_uring_register)
STUB_HANDLER(syscall_handler_openat2)
STUB_HANDLER(syscall_handler_close_range)
STUB_HANDLER(syscall_handler_copy_file_range2)
STUB_HANDLER(syscall_handler_statx2)
STUB_HANDLER(syscall_handler_mount_setattr)
STUB_HANDLER(syscall_handler_quotactl_fd)
STUB_HANDLER(syscall_handler_landlock_create_ruleset)
STUB_HANDLER(syscall_handler_landlock_add_rule)
STUB_HANDLER(syscall_handler_landlock_restrict_self)
STUB_HANDLER(syscall_handler_process_mrelease)

/* Define actual implementations for used syscalls */
#undef STUB_HANDLER
#define STUB_HANDLER(name) s64 name(ThreadState *st) { (void)st; return -ENOSYS; }

/* ============================================================================
 * Public Syscall Handler Wrappers
 * ============================================================================ */

/* Wrapper functions that call the _impl versions */
s64 syscall_handler_read(ThreadState *st) { return syscall_handler_read_impl(st); }
s64 syscall_handler_write(ThreadState *st) { return syscall_handler_write_impl(st); }
s64 syscall_handler_close(ThreadState *st) { return syscall_handler_close_impl(st); }
s64 syscall_handler_open(ThreadState *st) { return syscall_handler_open_impl(st); }
s64 syscall_handler_lseek(ThreadState *st) { return syscall_handler_lseek_impl(st); }
s64 syscall_handler_mmap(ThreadState *st) { return syscall_handler_mmap_impl(st); }
s64 syscall_handler_mprotect(ThreadState *st) { return syscall_handler_mprotect_impl(st); }
s64 syscall_handler_munmap(ThreadState *st) { return syscall_handler_munmap_impl(st); }
s64 syscall_handler_brk(ThreadState *st) { return syscall_handler_brk_impl(st); }
s64 syscall_handler_exit(ThreadState *st) { return syscall_handler_exit_impl(st); }
s64 syscall_handler_getpid(ThreadState *st) { return syscall_handler_getpid_impl(st); }
s64 syscall_handler_getuid(ThreadState *st) { return syscall_handler_getuid_impl(st); }
s64 syscall_handler_geteuid(ThreadState *st) { return syscall_handler_geteuid_impl(st); }
s64 syscall_handler_getgid(ThreadState *st) { return syscall_handler_getgid_impl(st); }
s64 syscall_handler_getegid(ThreadState *st) { return syscall_handler_getegid_impl(st); }
s64 syscall_handler_sigaction(ThreadState *st) { return syscall_handler_sigaction_impl(st); }
s64 syscall_handler_fstat(ThreadState *st) { return syscall_handler_fstat_impl(st); }
s64 syscall_handler_stat(ThreadState *st) { return syscall_handler_stat_impl(st); }
s64 syscall_handler_access(ThreadState *st) { return syscall_handler_access_impl(st); }
s64 syscall_handler_poll(ThreadState *st) { return syscall_handler_poll_impl(st); }
s64 syscall_handler_select(ThreadState *st) { return syscall_handler_select_impl(st); }

/* External declarations for handlers implemented elsewhere */
extern s64 syscall_handler_ioctl(ThreadState *st);
extern s64 syscall_handler_pipe(ThreadState *st);
extern s64 syscall_handler_sched_yield(ThreadState *st);
