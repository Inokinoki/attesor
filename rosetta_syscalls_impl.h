/* ============================================================================
 * Rosetta Translator - Syscall Implementation Header
 * ============================================================================
 *
 * This module provides syscall handling functions for the Rosetta
 * binary translation layer. It translates Linux ARM64 syscalls to
 * host OS equivalents.
 * ============================================================================ */

#ifndef ROSETTA_SYSCALLS_IMPL_H
#define ROSETTA_SYSCALLS_IMPL_H

#include "rosetta_refactored.h"

/* ============================================================================
 * Basic I/O Syscalls
 * ============================================================================ */

/* Read from file descriptor */
int syscall_read(ThreadState *state);

/* Write to file descriptor */
int syscall_write(ThreadState *state);

/* Open a file */
int syscall_open(ThreadState *state);

/* Close a file descriptor */
int syscall_close(ThreadState *state);

/* Reposition read/write file offset */
int syscall_lseek(ThreadState *state);

/* Check file accessibility */
int syscall_access(ThreadState *state);

/* Create a pipe */
int syscall_pipe(ThreadState *state);

/* ============================================================================
 * Memory Management Syscalls
 * ============================================================================ */

/* Map files or devices into memory */
int syscall_mmap(ThreadState *state);

/* Unmap memory region */
int syscall_munmap(ThreadState *state);

/* Set protection on memory region */
int syscall_mprotect(ThreadState *state);

/* Change data segment size */
int syscall_brk(ThreadState *state);

/* ============================================================================
 * File Status Syscalls
 * ============================================================================ */

/* Get file status */
int syscall_stat(ThreadState *state);

/* Get file status by file descriptor */
int syscall_fstat(ThreadState *state);

/* Get file status (don't follow symlinks) */
int syscall_lstat(ThreadState *state);

/* ============================================================================
 * Process Management Syscalls
 * ============================================================================ */

/* Get process ID */
int syscall_getpid(ThreadState *state);

/* Get thread ID */
int syscall_gettid(ThreadState *state);

/* Get system information */
int syscall_uname(ThreadState *state);

/* Manipulate file descriptor */
int syscall_fcntl(ThreadState *state);

/* Terminate the calling process */
noreturn void syscall_exit(ThreadState *state);

/* Terminate all threads in process */
noreturn void syscall_exit_group(ThreadState *state);

/* Set pointer to thread ID */
int syscall_set_tid_address(ThreadState *state);

/* Get CPU number */
int syscall_getcpu(ThreadState *state);

/* ============================================================================
 * Time-related Syscalls
 * ============================================================================ */

/* Get time of day */
int syscall_gettimeofday(ThreadState *state);

/* Get clock time */
int syscall_clock_gettime(ThreadState *state);

/* High-resolution sleep */
int syscall_nanosleep(ThreadState *state);

/* Get clock resolution */
int syscall_clock_getres(ThreadState *state);

/* Set time of day */
int syscall_settimeofday(ThreadState *state);

/* ============================================================================
 * Signal Syscalls
 * ============================================================================ */

/* Examine and change a signal handler */
int syscall_rt_sigaction(ThreadState *state);

/* Examine and change blocked signals */
int syscall_rt_sigprocmask(ThreadState *state);

/* Yield the processor */
int syscall_sched_yield(ThreadState *state);

/* Send signal to process */
int syscall_kill(ThreadState *state);

/* ============================================================================
 * IPC and Synchronization Syscalls
 * ============================================================================ */

/* Fast userspace mutex */
int syscall_futex(ThreadState *state);

/* Set architecture-specific thread state */
int syscall_arch_prctl(ThreadState *state);

/* ============================================================================
 * Extended I/O Syscalls
 * ============================================================================ */

/* Manipulate file descriptor (ioctl) */
int syscall_ioctl(ThreadState *state);

/* Duplicate file descriptor */
int syscall_dup2(ThreadState *state);

/* Duplicate file descriptor with flags */
int syscall_dup3(ThreadState *state);

/* Wait for I/O events */
int syscall_poll(ThreadState *state);

/* Synchronous I/O multiplexing */
int syscall_select(ThreadState *state);

/* Read data from file descriptor into multiple buffers */
int syscall_readv(ThreadState *state);

/* Write data from multiple buffers to file descriptor */
int syscall_writev(ThreadState *state);

/* ============================================================================
 * Filesystem Syscalls
 * ============================================================================ */

/* Get current working directory */
int syscall_getcwd(ThreadState *state);

/* Change current working directory */
int syscall_chdir(ThreadState *state);

/* Rename a file or directory */
int syscall_rename(ThreadState *state);

/* Create a directory */
int syscall_mkdir(ThreadState *state);

/* Remove a directory */
int syscall_rmdir(ThreadState *state);

/* Delete a file */
int syscall_unlink(ThreadState *state);

/* Create a symbolic link */
int syscall_symlink(ThreadState *state);

/* Read value of a symbolic link */
int syscall_readlink(ThreadState *state);

/* Change file permissions */
int syscall_chmod(ThreadState *state);

/* Change owner and group of a file */
int syscall_lchown(ThreadState *state);

/* Get directory entries */
int syscall_getdents(ThreadState *state);

/* ============================================================================
 * Process and Thread Syscalls
 * ============================================================================ */

/* Wait for process to change state */
int syscall_wait4(ThreadState *state);

/* Get process capabilities */
int syscall_capget(ThreadState *state);

/* Set process capabilities */
int syscall_capset(ThreadState *state);

/* Get memory resident set information */
int syscall_mincore(ThreadState *state);

/* Get/set resource limits */
int syscall_prlimit(ThreadState *state);

/* Create a child process */
int syscall_clone(ThreadState *state);

/* Execute program */
int syscall_execve(ThreadState *state);

/* Set/clear list of robust mutexes */
int syscall_set_robust_list(ThreadState *state);

/* Get list of robust mutexes */
int syscall_get_robust_list(ThreadState *state);

/* ============================================================================
 * Network Syscalls
 * ============================================================================ */

/* Create a socket */
int syscall_socket(ThreadState *state);

/* Initiate a connection on a socket */
int syscall_connect(ThreadState *state);

/* Send a message from a socket */
int syscall_sendto(ThreadState *state);

/* Receive a message from a socket */
int syscall_recvfrom(ThreadState *state);

/* Create an epoll instance */
int syscall_epoll_create(ThreadState *state);

/* Control interface for epoll */
int syscall_epoll_ctl(ThreadState *state);

/* Wait for events on an epoll instance */
int syscall_epoll_wait(ThreadState *state);

/* ============================================================================
 * Memory Management Helpers
 * ============================================================================ */

/* Map guest memory region */
void *memory_map_guest(uint64_t guest, uint64_t size);

/* Unmap guest memory region */
int memory_unmap_guest(uint64_t guest, uint64_t size);

/* Change protection on guest memory region */
int memory_protect_guest(uint64_t guest, uint64_t size, int32_t prot);

/* Initialize memory management subsystem */
int memory_init(void);

/* Clean up memory management subsystem */
void memory_cleanup(void);

/* Map guest memory with specific protection */
void *memory_map_guest_with_prot(uint64_t guest, uint64_t size, int32_t prot);

/* Translate guest address to host address */
void *memory_translate_addr(uint64_t guest);

/* ============================================================================
 * Helper Utilities
 * ============================================================================ */

/* Debug trace helper */
void helper_debug_trace(const char *msg, uint64_t value);

/* Performance counter */
uint64_t helper_perf_counter(void);

/* Block translation helper */
void *helper_block_translate(uint64_t guest_pc);

/* Block insert helper */
int helper_block_insert(uint64_t guest, uint64_t host, size_t size);

/* Block remove helper */
int helper_block_remove(uint64_t guest_pc);

/* Block invalidate helper */
void helper_block_invalidate(void);

/* Context switch helper */
void helper_context_switch(CPUContext *old_ctx, CPUContext *new_ctx);

/* Syscall enter hook */
void helper_syscall_enter(ThreadState *state, int nr);

/* Syscall exit hook */
void helper_syscall_exit(ThreadState *state, int64_t result);

/* Interrupt handler */
void helper_interrupt(ThreadState *state, int vector);

/* ============================================================================
 * Syscall Dispatch Table
 * ============================================================================ */

/* Initialize syscall table */
void init_syscall_table(void);

/* Handle syscall dispatch */
int64_t handle_syscall(ThreadState *state, int nr);

#endif /* ROSETTA_SYSCALLS_IMPL_H */
