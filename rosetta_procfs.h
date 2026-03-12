/* ============================================================================
 * Rosetta /proc Filesystem Emulation
 * ============================================================================
 *
 * This module provides /proc filesystem emulation for x86_64 binaries
 * running on ARM64 hosts, enabling Linux compatibility.
 *
 * Many Linux programs query /proc for system information, and this module
 * intercepts those requests and returns appropriate fake data.
 * ============================================================================ */

#ifndef ROSETTA_PROCFS_H
#define ROSETTA_PROCFS_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>  /* For ssize_t */

/* ============================================================================
 * /proc Path Support
 * ============================================================================ */

/**
 * Maximum path length for /proc paths
 */
#define ROSETTA_PROCFS_MAX_PATH 256

/**
 * Supported /proc paths
 */
typedef enum {
    ROSETTA_PROC_CPUINFO,      /* /proc/cpuinfo */
    ROSETTA_PROC_SELF_AUXV,    /* /proc/self/auxv */
    ROSETTA_PROC_SELF_CMDLINE, /* /proc/self/cmdline */
    ROSETTA_PROC_SELF_EXE,     /* /proc/self/exe */
    ROSETTA_PROC_MEMINFO,      /* /proc/meminfo */
    ROSETTA_PROC_SELF_STATUS,  /* /proc/self/status */
    ROSETTA_PROC_SELF_MAPS,    /* /proc/self/maps */
    ROSETTA_PROC_UNKNOWN       /* Unknown /proc path */
} rosetta_proc_path_t;

/**
 * /proc file handle
 */
typedef struct {
    rosetta_proc_path_t type;  /* Path type */
    char path[256];             /* Original path */
    uint64_t offset;            /* Current offset */
    int is_open;               /* Whether file is open */
} rosetta_proc_file_t;

/* ============================================================================
 * /proc Path API
 * ============================================================================ */

/**
 * Check if a path is a /proc path
 * @param path Path to check
 * @return 1 if /proc path, 0 otherwise
 */
int rosetta_proc_is_proc_path(const char *path);

/**
 * Get the type of /proc path
 * @param path Path to check
 * @return Path type, or ROSETTA_PROC_UNKNOWN if not supported
 */
rosetta_proc_path_t rosetta_proc_get_path_type(const char *path);

/**
 * Open a /proc file
 * @param path Path to open
 * @param flags Open flags
 * @return File descriptor, or -1 on error
 */
int rosetta_proc_open(const char *path, int flags);

/**
 * Read from a /proc file
 * @param fd File descriptor
 * @param buf Buffer to read into
 * @param count Number of bytes to read
 * @return Number of bytes read, or -1 on error
 */
ssize_t rosetta_proc_read(int fd, void *buf, size_t count);

/**
 * Close a /proc file
 * @param fd File descriptor
 * @return 0 on success, -1 on error
 */
int rosetta_proc_close(int fd);

/**
 * Handle readlink for /proc paths
 * @param path Path to readlink
 * @param buf Buffer to store result
 * @param size Buffer size
 * @return Number of bytes written, or -1 on error
 */
ssize_t rosetta_proc_readlink(const char *path, char *buf, size_t size);

/* ============================================================================
 * /proc Content Generation
 * ============================================================================ */

/**
 * Generate /proc/cpuinfo content
 * @param buf Buffer to write to
 * @param size Buffer size
 * @return Number of bytes written, or -1 on error
 */
int rosetta_proc_generate_cpuinfo(char *buf, size_t size);

/**
 * Generate /proc/self/auxv content
 * @param buf Buffer to write to
 * @param size Buffer size
 * @return Number of bytes written, or -1 on error
 */
int rosetta_proc_generate_auxv(char *buf, size_t size);

/**
 * Generate /proc/self/cmdline content
 * @param buf Buffer to write to
 * @param size Buffer size
 * @return Number of bytes written, or -1 on error
 */
int rosetta_proc_generate_cmdline(char *buf, size_t size);

/**
 * Get /proc/self/exe path
 * @param buf Buffer to store path
 * @param size Buffer size
 * @return Number of bytes written, or -1 on error
 */
int rosetta_proc_get_exe_path(char *buf, size_t size);

/**
 * Generate /proc/meminfo content
 * @param buf Buffer to write to
 * @param size Buffer size
 * @return Number of bytes written, or -1 on error
 */
int rosetta_proc_generate_meminfo(char *buf, size_t size);

/**
 * Generate /proc/self/status content
 * @param buf Buffer to write to
 * @param size Buffer size
 * @return Number of bytes written, or -1 on error
 */
int rosetta_proc_generate_status(char *buf, size_t size);

/**
 * Generate /proc/self/maps content
 * @param buf Buffer to write to
 * @param size Buffer size
 * @return Number of bytes written, or -1 on error
 */
int rosetta_proc_generate_maps(char *buf, size_t size);

/* ============================================================================
 * memfd_create Support
 * ============================================================================ */

/**
 * Create an anonymous file in memory (for /proc emulation)
 * @param name Name for the file
 * @param flags Creation flags
 * @return File descriptor, or -1 on error
 */
int rosetta_memfd_create(const char *name, unsigned int flags);

#endif /* ROSETTA_PROCFS_H */
