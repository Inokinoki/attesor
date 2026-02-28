/* ============================================================================
 * Rosetta Refactored - Syscall Handlers Header
 * ============================================================================
 *
 * This header defines the interface for syscall handlers in the Rosetta
 * translation layer.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_SYSCALL_HANDLERS_H
#define ROSETTA_REFACTORED_SYSCALL_HANDLERS_H

#include "rosetta_types.h"
#include "rosetta_refactored.h"
#include <stdint.h>

/* ThreadState is already defined in rosetta_types.h */

/* ============================================================================
 * Syscall Numbers (ARM64 Linux)
 * ============================================================================ */

#define SYSCALL_HANDLER_MAX 275

/* ============================================================================
 * Syscall Handler Types
 * ============================================================================ */

/**
 * syscall_handler_func_t - Syscall handler function type
 * @st: Thread state with syscall arguments
 * Returns: Syscall result
 */
typedef s64 (*syscall_handler_func_t)(ThreadState *st);

/* ============================================================================
 * Syscall Infrastructure Functions
 * ============================================================================ */

/**
 * syscall_get_nr - Get syscall number from thread state
 * @st: Thread state
 * Returns: Syscall number
 */
s32 syscall_get_nr(ThreadState *st);

/**
 * syscall_set_result - Set syscall result in thread state
 * @st: Thread state
 * @res: Result value
 */
void syscall_set_result(ThreadState *st, s64 res);

/**
 * syscall_dispatch - Dispatch syscall to appropriate handler
 * @st: Thread state
 * Returns: Syscall result
 */
s64 syscall_dispatch(ThreadState *st);

/* ============================================================================
 * Syscall Handler Table
 * ============================================================================ */

/**
 * syscall_handler_table - Array of syscall handlers
 * Maps ARM64 Linux syscall numbers to handler functions
 */
extern syscall_handler_func_t syscall_handler_table[SYSCALL_HANDLER_MAX];

/* ============================================================================
 * Individual Syscall Handlers (forward declarations)
 * ============================================================================ */

/* Basic I/O */
s64 syscall_handler_read(ThreadState *st);
s64 syscall_handler_write(ThreadState *st);
s64 syscall_handler_close(ThreadState *st);
s64 syscall_handler_open(ThreadState *st);
s64 syscall_handler_lseek(ThreadState *st);

/* Memory */
s64 syscall_handler_mmap(ThreadState *st);
s64 syscall_handler_mprotect(ThreadState *st);
s64 syscall_handler_munmap(ThreadState *st);
s64 syscall_handler_brk(ThreadState *st);

/* Process/Signals */
s64 syscall_handler_exit(ThreadState *st);
s64 syscall_handler_getpid(ThreadState *st);
s64 syscall_handler_getuid(ThreadState *st);
s64 syscall_handler_geteuid(ThreadState *st);
s64 syscall_handler_getgid(ThreadState *st);
s64 syscall_handler_getegid(ThreadState *st);
s64 syscall_handler_sigaction(ThreadState *st);

/* File Status */
s64 syscall_handler_fstat(ThreadState *st);
s64 syscall_handler_stat(ThreadState *st);
s64 syscall_handler_access(ThreadState *st);

/* Poll/Select */
s64 syscall_handler_poll(ThreadState *st);
s64 syscall_handler_select(ThreadState *st);

/* Stub handlers for unimplemented syscalls */
#define STUB_HANDLER_DECL(name) s64 name(ThreadState *st);

/* All syscall handler declarations */
STUB_HANDLER_DECL(syscall_handler_io_setup)
STUB_HANDLER_DECL(syscall_handler_io_destroy)
STUB_HANDLER_DECL(syscall_handler_io_submit)
STUB_HANDLER_DECL(syscall_handler_io_cancel)
STUB_HANDLER_DECL(syscall_handler_io_getevents)
STUB_HANDLER_DECL(syscall_handler_setxattr)
STUB_HANDLER_DECL(syscall_handler_lsetxattr)
STUB_HANDLER_DECL(syscall_handler_fsetxattr)
STUB_HANDLER_DECL(syscall_handler_getxattr)
STUB_HANDLER_DECL(syscall_handler_lgetxattr)
STUB_HANDLER_DECL(syscall_handler_fgetxattr)
STUB_HANDLER_DECL(syscall_handler_listxattr)
STUB_HANDLER_DECL(syscall_handler_llistxattr)
STUB_HANDLER_DECL(syscall_handler_flistxattr)
STUB_HANDLER_DECL(syscall_handler_removexattr)
STUB_HANDLER_DECL(syscall_handler_lremovexattr)
STUB_HANDLER_DECL(syscall_handler_fremovexattr)
STUB_HANDLER_DECL(syscall_handler_getcwd)
STUB_HANDLER_DECL(syscall_handler_lookup_dcookie)
STUB_HANDLER_DECL(syscall_handler_eventfd2)
STUB_HANDLER_DECL(syscall_handler_epoll_create1)
STUB_HANDLER_DECL(syscall_handler_epoll_ctl)
STUB_HANDLER_DECL(syscall_handler_epoll_pwait)
STUB_HANDLER_DECL(syscall_handler_dup)
STUB_HANDLER_DECL(syscall_handler_dup3)
STUB_HANDLER_DECL(syscall_handler_fcntl)
STUB_HANDLER_DECL(syscall_handler_inotify_init1)
STUB_HANDLER_DECL(syscall_handler_inotify_add_watch)
STUB_HANDLER_DECL(syscall_handler_inotify_rm_watch)
STUB_HANDLER_DECL(syscall_handler_ioctl)
STUB_HANDLER_DECL(syscall_handler_ioprio_set)
STUB_HANDLER_DECL(syscall_handler_ioprio_get)
STUB_HANDLER_DECL(syscall_handler_flock)
STUB_HANDLER_DECL(syscall_handler_mknodat)
STUB_HANDLER_DECL(syscall_handler_mkdirat)
STUB_HANDLER_DECL(syscall_handler_unlinkat)
STUB_HANDLER_DECL(syscall_handler_symlinkat)
STUB_HANDLER_DECL(syscall_handler_linkat)
STUB_HANDLER_DECL(syscall_handler_umount2)
STUB_HANDLER_DECL(syscall_handler_mount)
STUB_HANDLER_DECL(syscall_handler_pivot_root)
STUB_HANDLER_DECL(syscall_handler_nfsservctl)
STUB_HANDLER_DECL(syscall_handler_statfs)
STUB_HANDLER_DECL(syscall_handler_fstatfs)
STUB_HANDLER_DECL(syscall_handler_truncate)
STUB_HANDLER_DECL(syscall_handler_ftruncate)
STUB_HANDLER_DECL(syscall_handler_fallocate)
STUB_HANDLER_DECL(syscall_handler_faccessat)
STUB_HANDLER_DECL(syscall_handler_chdir)
STUB_HANDLER_DECL(syscall_handler_fchdir)
STUB_HANDLER_DECL(syscall_handler_chroot)
STUB_HANDLER_DECL(syscall_handler_fchmod)
STUB_HANDLER_DECL(syscall_handler_fchmodat)
STUB_HANDLER_DECL(syscall_handler_fchownat)
STUB_HANDLER_DECL(syscall_handler_fchown)
STUB_HANDLER_DECL(syscall_handler_openat)
STUB_HANDLER_DECL(syscall_handler_close)
STUB_HANDLER_DECL(syscall_handler_vmsplice)
STUB_HANDLER_DECL(syscall_handler_splice)
STUB_HANDLER_DECL(syscall_handler_tee)
STUB_HANDLER_DECL(syscall_handler_readlinkat)
STUB_HANDLER_DECL(syscall_handler_fstatat)
STUB_HANDLER_DECL(syscall_handler_sync)
STUB_HANDLER_DECL(syscall_handler_fsync)
STUB_HANDLER_DECL(syscall_handler_fdatasync)
STUB_HANDLER_DECL(syscall_handler_sync_file_range)
STUB_HANDLER_DECL(syscall_handler_timerfd_create)
STUB_HANDLER_DECL(syscall_handler_timerfd_settime)
STUB_HANDLER_DECL(syscall_handler_timerfd_gettime)
STUB_HANDLER_DECL(syscall_handler_utimensat)
STUB_HANDLER_DECL(syscall_handler_acct)
STUB_HANDLER_DECL(syscall_handler_capget)
STUB_HANDLER_DECL(syscall_handler_capset)
STUB_HANDLER_DECL(syscall_handler_personality)
STUB_HANDLER_DECL(syscall_handler_exit_group)
STUB_HANDLER_DECL(syscall_handler_waitid)
STUB_HANDLER_DECL(syscall_handler_set_tid_address)
STUB_HANDLER_DECL(syscall_handler_unshare)
STUB_HANDLER_DECL(syscall_handler_futex)
STUB_HANDLER_DECL(syscall_handler_set_robust_list)
STUB_HANDLER_DECL(syscall_handler_get_robust_list)
STUB_HANDLER_DECL(syscall_handler_nanosleep)
STUB_HANDLER_DECL(syscall_handler_getitimer)
STUB_HANDLER_DECL(syscall_handler_setitimer)
STUB_HANDLER_DECL(syscall_handler_kexec_load)
STUB_HANDLER_DECL(syscall_handler_init_module)
STUB_HANDLER_DECL(syscall_handler_delete_module)
STUB_HANDLER_DECL(syscall_handler_timer_create)
STUB_HANDLER_DECL(syscall_handler_timer_gettime)
STUB_HANDLER_DECL(syscall_handler_timer_getoverrun)
STUB_HANDLER_DECL(syscall_handler_timer_settime)
STUB_HANDLER_DECL(syscall_handler_timer_delete)
STUB_HANDLER_DECL(syscall_handler_clock_settime)
STUB_HANDLER_DECL(syscall_handler_clock_gettime)
STUB_HANDLER_DECL(syscall_handler_clock_getres)
STUB_HANDLER_DECL(syscall_handler_clock_nanosleep)
STUB_HANDLER_DECL(syscall_handler_syslog)
STUB_HANDLER_DECL(syscall_handler_ptrace)
STUB_HANDLER_DECL(syscall_handler_sched_setparam)
STUB_HANDLER_DECL(syscall_handler_sched_setscheduler)
STUB_HANDLER_DECL(syscall_handler_sched_getscheduler)
STUB_HANDLER_DECL(syscall_handler_sched_getparam)
STUB_HANDLER_DECL(syscall_handler_sched_setaffinity)
STUB_HANDLER_DECL(syscall_handler_sched_getaffinity)
STUB_HANDLER_DECL(syscall_handler_sched_yield)
STUB_HANDLER_DECL(syscall_handler_sched_get_priority_max)
STUB_HANDLER_DECL(syscall_handler_sched_get_priority_min)
STUB_HANDLER_DECL(syscall_handler_sched_rr_get_interval)
STUB_HANDLER_DECL(syscall_handler_restart_syscall)
STUB_HANDLER_DECL(syscall_handler_kill)
STUB_HANDLER_DECL(syscall_handler_tkill)
STUB_HANDLER_DECL(syscall_handler_tgkill)
STUB_HANDLER_DECL(syscall_handler_sigaltstack)
STUB_HANDLER_DECL(syscall_handler_rt_sigsuspend)
STUB_HANDLER_DECL(syscall_handler_rt_sigaction)
STUB_HANDLER_DECL(syscall_handler_rt_sigprocmask)
STUB_HANDLER_DECL(syscall_handler_rt_sigpending)
STUB_HANDLER_DECL(syscall_handler_rt_sigtimedwait)
STUB_HANDLER_DECL(syscall_handler_rt_sigqueueinfo)
STUB_HANDLER_DECL(syscall_handler_rt_sigreturn)
STUB_HANDLER_DECL(syscall_handler_setpriority)
STUB_HANDLER_DECL(syscall_handler_getpriority)
STUB_HANDLER_DECL(syscall_handler_reboot)
STUB_HANDLER_DECL(syscall_handler_setregid)
STUB_HANDLER_DECL(syscall_handler_setgid)
STUB_HANDLER_DECL(syscall_handler_setreuid)
STUB_HANDLER_DECL(syscall_handler_setuid)
STUB_HANDLER_DECL(syscall_handler_setresuid)
STUB_HANDLER_DECL(syscall_handler_getresuid)
STUB_HANDLER_DECL(syscall_handler_setresgid)
STUB_HANDLER_DECL(syscall_handler_getresgid)
STUB_HANDLER_DECL(syscall_handler_setfsuid)
STUB_HANDLER_DECL(syscall_handler_setfsgid)
STUB_HANDLER_DECL(syscall_handler_times)
STUB_HANDLER_DECL(syscall_handler_setpgid)
STUB_HANDLER_DECL(syscall_handler_getpgid)
STUB_HANDLER_DECL(syscall_handler_getsid)
STUB_HANDLER_DECL(syscall_handler_setsid)
STUB_HANDLER_DECL(syscall_handler_getgroups)
STUB_HANDLER_DECL(syscall_handler_setgroups)
STUB_HANDLER_DECL(syscall_handler_uname)
STUB_HANDLER_DECL(syscall_handler_sethostname)
STUB_HANDLER_DECL(syscall_handler_setdomainname)
STUB_HANDLER_DECL(syscall_handler_getrlimit)
STUB_HANDLER_DECL(syscall_handler_setrlimit)
STUB_HANDLER_DECL(syscall_handler_getrusage)
STUB_HANDLER_DECL(syscall_handler_umask)
STUB_HANDLER_DECL(syscall_handler_prctl)
STUB_HANDLER_DECL(syscall_handler_getcpu)
STUB_HANDLER_DECL(syscall_handler_gettimeofday)
STUB_HANDLER_DECL(syscall_handler_settimeofday)
STUB_HANDLER_DECL(syscall_handler_adjtimex)
STUB_HANDLER_DECL(syscall_handler_getppid)
STUB_HANDLER_DECL(syscall_handler_gettid)
STUB_HANDLER_DECL(syscall_handler_sysinfo)
STUB_HANDLER_DECL(syscall_handler_mq_open)
STUB_HANDLER_DECL(syscall_handler_mq_unlink)
STUB_HANDLER_DECL(syscall_handler_mq_timedsend)
STUB_HANDLER_DECL(syscall_handler_mq_timedreceive)
STUB_HANDLER_DECL(syscall_handler_mq_notify)
STUB_HANDLER_DECL(syscall_handler_mq_getsetattr)
STUB_HANDLER_DECL(syscall_handler_msgget)
STUB_HANDLER_DECL(syscall_handler_msgctl)
STUB_HANDLER_DECL(syscall_handler_msgrcv)
STUB_HANDLER_DECL(syscall_handler_msgsnd)
STUB_HANDLER_DECL(syscall_handler_semget)
STUB_HANDLER_DECL(syscall_handler_semctl)
STUB_HANDLER_DECL(syscall_handler_semtimedop)
STUB_HANDLER_DECL(syscall_handler_semop)
STUB_HANDLER_DECL(syscall_handler_shmget)
STUB_HANDLER_DECL(syscall_handler_shmctl)
STUB_HANDLER_DECL(syscall_handler_shmat)
STUB_HANDLER_DECL(syscall_handler_shmdt)
STUB_HANDLER_DECL(syscall_handler_socket)
STUB_HANDLER_DECL(syscall_handler_socketpair)
STUB_HANDLER_DECL(syscall_handler_bind)
STUB_HANDLER_DECL(syscall_handler_listen)
STUB_HANDLER_DECL(syscall_handler_accept)
STUB_HANDLER_DECL(syscall_handler_connect)
STUB_HANDLER_DECL(syscall_handler_getsockname)
STUB_HANDLER_DECL(syscall_handler_getpeername)
STUB_HANDLER_DECL(syscall_handler_sendto)
STUB_HANDLER_DECL(syscall_handler_recvfrom)
STUB_HANDLER_DECL(syscall_handler_setsockopt)
STUB_HANDLER_DECL(syscall_handler_getsockopt)
STUB_HANDLER_DECL(syscall_handler_shutdown)
STUB_HANDLER_DECL(syscall_handler_sendmsg)
STUB_HANDLER_DECL(syscall_handler_recvmsg)
STUB_HANDLER_DECL(syscall_handler_readahead)
STUB_HANDLER_DECL(syscall_handler_mremap)
STUB_HANDLER_DECL(syscall_handler_add_key)
STUB_HANDLER_DECL(syscall_handler_request_key)
STUB_HANDLER_DECL(syscall_handler_keyctl)
STUB_HANDLER_DECL(syscall_handler_clone)
STUB_HANDLER_DECL(syscall_handler_execve)
STUB_HANDLER_DECL(syscall_handler_fadvise64)
STUB_HANDLER_DECL(syscall_handler_swapon)
STUB_HANDLER_DECL(syscall_handler_swapoff)
STUB_HANDLER_DECL(syscall_handler_msync)
STUB_HANDLER_DECL(syscall_handler_mlock)
STUB_HANDLER_DECL(syscall_handler_munlock)
STUB_HANDLER_DECL(syscall_handler_mlockall)
STUB_HANDLER_DECL(syscall_handler_munlockall)
STUB_HANDLER_DECL(syscall_handler_mincore)
STUB_HANDLER_DECL(syscall_handler_madvise)
STUB_HANDLER_DECL(syscall_handler_remap_file_pages)
STUB_HANDLER_DECL(syscall_handler_mbind)
STUB_HANDLER_DECL(syscall_handler_get_mempolicy)
STUB_HANDLER_DECL(syscall_handler_set_mempolicy)
STUB_HANDLER_DECL(syscall_handler_migrate_pages)
STUB_HANDLER_DECL(syscall_handler_move_pages)
STUB_HANDLER_DECL(syscall_handler_rt_tgsigqueueinfo)
STUB_HANDLER_DECL(syscall_handler_perf_event_open)
STUB_HANDLER_DECL(syscall_handler_accept4)
STUB_HANDLER_DECL(syscall_handler_wait4)
STUB_HANDLER_DECL(syscall_handler_prlimit64)
STUB_HANDLER_DECL(syscall_handler_fanotify_init)
STUB_HANDLER_DECL(syscall_handler_fanotify_mark)
STUB_HANDLER_DECL(syscall_handler_name_to_handle_at)
STUB_HANDLER_DECL(syscall_handler_open_by_handle_at)
STUB_HANDLER_DECL(syscall_handler_clock_adjtime)
STUB_HANDLER_DECL(syscall_handler_syncfs)
STUB_HANDLER_DECL(syscall_handler_setns)
STUB_HANDLER_DECL(syscall_handler_sendmmsg)
STUB_HANDLER_DECL(syscall_handler_process_vm_readv)
STUB_HANDLER_DECL(syscall_handler_process_vm_writev)
STUB_HANDLER_DECL(syscall_handler_kcmp)
STUB_HANDLER_DECL(syscall_handler_finit_module)
STUB_HANDLER_DECL(syscall_handler_sched_setattr)
STUB_HANDLER_DECL(syscall_handler_sched_getattr)
STUB_HANDLER_DECL(syscall_handler_renameat2)
STUB_HANDLER_DECL(syscall_handler_seccomp)
STUB_HANDLER_DECL(syscall_handler_getrandom)
STUB_HANDLER_DECL(syscall_handler_memfd_create)
STUB_HANDLER_DECL(syscall_handler_kexec_file_load)
STUB_HANDLER_DECL(syscall_handler_bpf)
STUB_HANDLER_DECL(syscall_handler_execveat)
STUB_HANDLER_DECL(syscall_handler_userfaultfd)
STUB_HANDLER_DECL(syscall_handler_membarrier)
STUB_HANDLER_DECL(syscall_handler_mlock2)
STUB_HANDLER_DECL(syscall_handler_copy_file_range)
STUB_HANDLER_DECL(syscall_handler_preadv2)
STUB_HANDLER_DECL(syscall_handler_pwritev2)
STUB_HANDLER_DECL(syscall_handler_pkey_mprotect)
STUB_HANDLER_DECL(syscall_handler_pkey_alloc)
STUB_HANDLER_DECL(syscall_handler_pkey_free2)
STUB_HANDLER_DECL(syscall_handler_statx)
STUB_HANDLER_DECL(syscall_handler_io_pgetevents)
STUB_HANDLER_DECL(syscall_handler_rseq)
STUB_HANDLER_DECL(syscall_handler_io_uring_setup)
STUB_HANDLER_DECL(syscall_handler_io_uring_enter)
STUB_HANDLER_DECL(syscall_handler_io_uring_register)
STUB_HANDLER_DECL(syscall_handler_openat2)
STUB_HANDLER_DECL(syscall_handler_close_range)
STUB_HANDLER_DECL(syscall_handler_copy_file_range2)
STUB_HANDLER_DECL(syscall_handler_statx2)
STUB_HANDLER_DECL(syscall_handler_mount_setattr)
STUB_HANDLER_DECL(syscall_handler_quotactl_fd)
STUB_HANDLER_DECL(syscall_handler_landlock_create_ruleset)
STUB_HANDLER_DECL(syscall_handler_landlock_add_rule)
STUB_HANDLER_DECL(syscall_handler_landlock_restrict_self)
STUB_HANDLER_DECL(syscall_handler_process_mrelease)

#endif /* ROSETTA_REFACTORED_SYSCALL_HANDLERS_H */
