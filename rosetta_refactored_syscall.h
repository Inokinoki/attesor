/* ============================================================================
 * Rosetta Refactored - Syscall Handling Header
 * ============================================================================
 *
 * This module provides syscall handling functions for the Rosetta
 * translation layer, wrapping the underlying syscall implementation.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_SYSCALL_H
#define ROSETTA_REFACTORED_SYSCALL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

/* ============================================================================
 * Syscall Configuration
 * ============================================================================ */

/* Maximum number of syscall arguments */
#define ROS_SYSCALL_MAX_ARGS    6

/* Syscall return values */
#define ROS_SYSCALL_SUCCESS     0
#define ROS_SYSCALL_ERROR       (-1)
#define ROS_SYSCALL_UNIMPLEMENTED (-2)

/* ============================================================================
 * Syscall Handler Types
 * ============================================================================ */

/* Syscall handler function type */
typedef int64_t (*ros_syscall_handler_t)(uint64_t arg1, uint64_t arg2,
                                          uint64_t arg3, uint64_t arg4,
                                          uint64_t arg5, uint64_t arg6);

/* Syscall descriptor */
typedef struct {
    int number;                     /* Syscall number */
    const char *name;               /* Syscall name */
    ros_syscall_handler_t handler;  /* Handler function */
} ros_syscall_desc_t;

/* ============================================================================
 * Syscall Initialization
 * ============================================================================ */

/**
 * rosetta_syscall_init - Initialize syscall handling
 * Returns: 0 on success, -1 on error
 */
int rosetta_syscall_init(void);

/**
 * rosetta_syscall_cleanup - Cleanup syscall handling
 */
void rosetta_syscall_cleanup(void);

/* ============================================================================
 * Syscall Execution
 * ============================================================================ */

/**
 * rosetta_syscall - Execute a syscall
 * @number: Syscall number (ARM64 convention)
 * @arg1-arg6: Syscall arguments
 * Returns: Syscall result
 */
int64_t rosetta_syscall(int number, uint64_t arg1, uint64_t arg2,
                        uint64_t arg3, uint64_t arg4,
                        uint64_t arg5, uint64_t arg6);

/**
 * rosetta_syscall_execute - Execute syscall from guest state
 * @number: Syscall number
 * @args: Array of syscall arguments
 * Returns: Syscall result
 */
int64_t rosetta_syscall_execute(int number, uint64_t *args);

/**
 * rosetta_handle_syscall - Handle syscall in translated code
 * @number: Syscall number
 * @state: Guest thread state
 * Returns: 0 on success, -1 on error
 */
int rosetta_handle_syscall(int number, void *state);

/* ============================================================================
 * Syscall Registration
 * ============================================================================ */

/**
 * rosetta_register_syscall - Register a syscall handler
 * @number: Syscall number
 * @name: Syscall name
 * @handler: Handler function
 * Returns: 0 on success, -1 on error
 */
int rosetta_register_syscall(int number, const char *name,
                              ros_syscall_handler_t handler);

/**
 * rosetta_unregister_syscall - Unregister a syscall handler
 * @number: Syscall number
 * Returns: 0 on success, -1 on error
 */
int rosetta_unregister_syscall(int number);

/* ============================================================================
 * Syscall Information
 * ============================================================================ */

/**
 * rosetta_syscall_get_name - Get syscall name
 * @number: Syscall number
 * Returns: Syscall name or "unknown"
 */
const char *rosetta_syscall_get_name(int number);

/**
 * rosetta_syscall_is_implemented - Check if syscall is implemented
 * @number: Syscall number
 * Returns: true if implemented, false otherwise
 */
bool rosetta_syscall_is_implemented(int number);

/**
 * rosetta_syscall_count - Get number of registered syscalls
 * Returns: Number of syscalls
 */
int rosetta_syscall_count(void);

/* ============================================================================
 * Common Syscalls (convenience wrappers)
 * ============================================================================ */

/**
 * rosetta_sys_exit - Exit syscall
 * @status: Exit status
 */
void rosetta_sys_exit(int status);

/**
 * rosetta_sys_read - Read syscall
 * @fd: File descriptor
 * @buf: Buffer to read into
 * @count: Number of bytes to read
 * Returns: Number of bytes read, or -1 on error
 */
ssize_t rosetta_sys_read(int fd, void *buf, size_t count);

/**
 * rosetta_sys_write - Write syscall
 * @fd: File descriptor
 * @buf: Buffer to write
 * @count: Number of bytes to write
 * Returns: Number of bytes written, or -1 on error
 */
ssize_t rosetta_sys_write(int fd, const void *buf, size_t count);

/**
 * rosetta_sys_open - Open syscall
 * @pathname: File path
 * @flags: Open flags
 * @mode: File mode
 * Returns: File descriptor, or -1 on error
 */
int rosetta_sys_open(const char *pathname, int flags, uint32_t mode);

/**
 * rosetta_sys_close - Close syscall
 * @fd: File descriptor
 * Returns: 0 on success, -1 on error
 */
int rosetta_sys_close(int fd);

/**
 * rosetta_sys_mmap - Mmap syscall
 * @addr: Hint address
 * @length: Mapping length
 * @prot: Protection flags
 * @flags: Mapping flags
 * @fd: File descriptor
 * @offset: Offset in file
 * Returns: Mapped address, or MAP_FAILED on error
 */
void *rosetta_sys_mmap(void *addr, size_t length, int prot,
                       int flags, int fd, int64_t offset);

/**
 * rosetta_sys_munmap - Munmap syscall
 * @addr: Address to unmap
 * @length: Mapping length
 * Returns: 0 on success, -1 on error
 */
int rosetta_sys_munmap(void *addr, size_t length);

/**
 * rosetta_sys_brk - Brk syscall
 * @addr: New program break address
 * Returns: New program break address
 */
void *rosetta_sys_brk(void *addr);

#endif /* ROSETTA_REFACTORED_SYSCALL_H */
