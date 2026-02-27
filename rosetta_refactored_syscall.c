/* ============================================================================
 * Rosetta Refactored - Syscall Handling Implementation
 * ============================================================================
 *
 * This module implements syscall handling functions for the Rosetta
 * translation layer.
 * ============================================================================ */

#include "rosetta_refactored_syscall.h"
#include "rosetta_syscalls.h"
#include "rosetta_refactored_reg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

/* ============================================================================
 * Syscall Table
 * ============================================================================ */

/* Maximum syscall number we track */
#define MAX_SYSCALL_NUMBER 500

/* Syscall table */
static ros_syscall_desc_t g_syscall_table[MAX_SYSCALL_NUMBER];
static bool g_syscall_initialized = false;

/* ============================================================================
 * Forward Declarations - Wrapper functions around rosetta_syscalls.c
 * ============================================================================ */

extern int64_t translate_syscall(int number, uint64_t arg1, uint64_t arg2,
                                  uint64_t arg3, uint64_t arg4,
                                  uint64_t arg5, uint64_t arg6);
extern void syscall_entry(void);

/* ============================================================================
 * Syscall Initialization
 * ============================================================================ */

/**
 * rosetta_syscall_init - Initialize syscall handling
 */
int rosetta_syscall_init(void)
{
    if (g_syscall_initialized) {
        rosetta_syscall_cleanup();
    }

    /* Initialize syscall table */
    memset(g_syscall_table, 0, sizeof(g_syscall_table));

    g_syscall_initialized = true;
    return 0;
}

/**
 * rosetta_syscall_cleanup - Cleanup syscall handling
 */
void rosetta_syscall_cleanup(void)
{
    if (!g_syscall_initialized) {
        return;
    }

    memset(g_syscall_table, 0, sizeof(g_syscall_table));
    g_syscall_initialized = false;
}

/* ============================================================================
 * Syscall Execution
 * ============================================================================ */

/**
 * rosetta_syscall - Execute a syscall
 */
int64_t rosetta_syscall(int number, uint64_t arg1, uint64_t arg2,
                        uint64_t arg3, uint64_t arg4,
                        uint64_t arg5, uint64_t arg6)
{
    if (number < 0 || number >= MAX_SYSCALL_NUMBER) {
        return ROS_SYSCALL_UNIMPLEMENTED;
    }

    /* Check if we have a registered handler */
    if (g_syscall_table[number].handler != NULL) {
        return g_syscall_table[number].handler(arg1, arg2, arg3, arg4, arg5, arg6);
    }

    /* Use the underlying translation layer */
    return translate_syscall(number, arg1, arg2, arg3, arg4, arg5, arg6);
}

/**
 * rosetta_syscall_execute - Execute syscall from guest state
 */
int64_t rosetta_syscall_execute(int number, uint64_t *args)
{
    if (!args) {
        return ROS_SYSCALL_ERROR;
    }

    return rosetta_syscall(number, args[0], args[1], args[2],
                           args[3], args[4], args[5]);
}

/**
 * rosetta_handle_syscall - Handle syscall in translated code
 */
int rosetta_handle_syscall(int number, void *state)
{
    (void)state;  /* State is accessed via rosetta_get_reg() */

    if (number < 0 || number >= MAX_SYSCALL_NUMBER) {
        return ROS_SYSCALL_UNIMPLEMENTED;
    }

    /* Get arguments from guest registers (ARM64 convention: X0-X5) */
    uint64_t arg1 = rosetta_get_reg(0);
    uint64_t arg2 = rosetta_get_reg(1);
    uint64_t arg3 = rosetta_get_reg(2);
    uint64_t arg4 = rosetta_get_reg(3);
    uint64_t arg5 = rosetta_get_reg(4);
    uint64_t arg6 = rosetta_get_reg(5);

    /* Execute syscall */
    int64_t result = rosetta_syscall(number, arg1, arg2, arg3, arg4, arg5, arg6);

    /* Set result in X0 */
    rosetta_set_reg(0, (uint64_t)result);

    return ROS_SYSCALL_SUCCESS;
}

/* ============================================================================
 * Syscall Registration
 * ============================================================================ */

/**
 * rosetta_register_syscall - Register a syscall handler
 */
int rosetta_register_syscall(int number, const char *name,
                              ros_syscall_handler_t handler)
{
    if (number < 0 || number >= MAX_SYSCALL_NUMBER) {
        return -1;
    }

    if (!g_syscall_initialized) {
        rosetta_syscall_init();
    }

    g_syscall_table[number].number = number;
    g_syscall_table[number].name = name;
    g_syscall_table[number].handler = handler;

    return 0;
}

/**
 * rosetta_unregister_syscall - Unregister a syscall handler
 */
int rosetta_unregister_syscall(int number)
{
    if (number < 0 || number >= MAX_SYSCALL_NUMBER) {
        return -1;
    }

    memset(&g_syscall_table[number], 0, sizeof(ros_syscall_desc_t));
    return 0;
}

/* ============================================================================
 * Syscall Information
 * ============================================================================ */

/**
 * rosetta_syscall_get_name - Get syscall name
 */
const char *rosetta_syscall_get_name(int number)
{
    if (number < 0 || number >= MAX_SYSCALL_NUMBER) {
        return "unknown";
    }

    if (g_syscall_table[number].name != NULL) {
        return g_syscall_table[number].name;
    }

    return "unknown";
}

/**
 * rosetta_syscall_is_implemented - Check if syscall is implemented
 */
bool rosetta_syscall_is_implemented(int number)
{
    if (number < 0 || number >= MAX_SYSCALL_NUMBER) {
        return false;
    }

    return (g_syscall_table[number].handler != NULL) ||
           (translate_syscall(number, 0, 0, 0, 0, 0, 0) != ROS_SYSCALL_UNIMPLEMENTED);
}

/**
 * rosetta_syscall_count - Get number of registered syscalls
 */
int rosetta_syscall_count(void)
{
    int count = 0;
    int i;

    for (i = 0; i < MAX_SYSCALL_NUMBER; i++) {
        if (g_syscall_table[i].handler != NULL) {
            count++;
        }
    }

    return count;
}

/* ============================================================================
 * Common Syscalls (convenience wrappers)
 * ============================================================================ */

/**
 * rosetta_sys_exit - Exit syscall
 */
void rosetta_sys_exit(int status)
{
    rosetta_syscall(93, (uint64_t)status, 0, 0, 0, 0, 0);  /* ARM64 SYS_EXIT = 93 */
    _exit(status);  /* Should not return */
}

/**
 * rosetta_sys_read - Read syscall
 */
ssize_t rosetta_sys_read(int fd, void *buf, size_t count)
{
    int64_t result = rosetta_syscall(63, (uint64_t)fd, (uint64_t)buf,
                                      (uint64_t)count, 0, 0, 0);
    return (ssize_t)result;
}

/**
 * rosetta_sys_write - Write syscall
 */
ssize_t rosetta_sys_write(int fd, const void *buf, size_t count)
{
    int64_t result = rosetta_syscall(64, (uint64_t)fd, (uint64_t)buf,
                                      (uint64_t)count, 0, 0, 0);
    return (ssize_t)result;
}

/**
 * rosetta_sys_open - Open syscall
 */
int rosetta_sys_open(const char *pathname, int flags, uint32_t mode)
{
    int64_t result = rosetta_syscall(56, (uint64_t)pathname, (uint64_t)flags,
                                      (uint64_t)mode, 0, 0, 0);
    return (int)result;
}

/**
 * rosetta_sys_close - Close syscall
 */
int rosetta_sys_close(int fd)
{
    int64_t result = rosetta_syscall(57, (uint64_t)fd, 0, 0, 0, 0, 0);
    return (int)result;
}

/**
 * rosetta_sys_mmap - Mmap syscall
 */
void *rosetta_sys_mmap(void *addr, size_t length, int prot,
                       int flags, int fd, int64_t offset)
{
    int64_t result = rosetta_syscall(222, (uint64_t)addr, (uint64_t)length,
                                      (uint64_t)prot, (uint64_t)flags,
                                      (uint64_t)fd, (uint64_t)offset);

    if (result == -1) {
        return MAP_FAILED;
    }

    return (void *)(uintptr_t)result;
}

/**
 * rosetta_sys_munmap - Munmap syscall
 */
int rosetta_sys_munmap(void *addr, size_t length)
{
    int64_t result = rosetta_syscall(215, (uint64_t)addr, (uint64_t)length,
                                      0, 0, 0, 0);
    return (int)result;
}

/**
 * rosetta_sys_brk - Brk syscall
 */
void *rosetta_sys_brk(void *addr)
{
    int64_t result = rosetta_syscall(214, (uint64_t)addr, 0, 0, 0, 0, 0);
    return (void *)(uintptr_t)result;
}
