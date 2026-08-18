#ifndef OAH_LINUX_ABI_H
#define OAH_LINUX_ABI_H

#include "oah/types.h"

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Path class for ThreadContextFcntl.cpp FUN_80000002e50c. */
typedef enum {
    OAH_PROC_PATH_EXE     = 0,
    OAH_PROC_PATH_CMDLINE = 1,
    OAH_PROC_PATH_AUXV    = 2
} oah_proc_path_kind;

typedef struct {
    bool matched;
    bool is_thread_self;
} oah_proc_path_match;

/*
 * orig: unnamed path helper used by open()
 * decomp: FUN_80000002e50c
 * Matches /proc/self/..., /proc/thread-self/..., /proc/<pid>/...,
 * and /proc/<pid>/task/<tid>/...
 */
oah_proc_path_match oah_classify_proc_path(const char *path, oah_proc_path_kind kind,
                                           int self_pid);

/*
 * x86_64 open(2) flags → AArch64 open(2) flags.
 * decomp: FUN_80000002bf74 flag shuffle at the top of the function.
 * Swaps bits 14↔16 and 15↔17 (O_DIRECT/O_DIRECTORY/O_LARGEFILE/O_NOFOLLOW).
 */
u32 oah_translate_open_flags(u32 x86_flags);

/* Fake /proc/cpuinfo body.
 * orig: ThreadContextFcntl.cpp create_fake_cpuinfo_file
 * decomp: FUN_80000002bf74 format string (vendor_id VirtualApple). */
#define OAH_FAKE_CPUINFO_VENDOR "VirtualApple"
#define OAH_FAKE_CPUINFO_MHZ    "2502.057"

int oah_write_fake_cpuinfo(FILE *out, unsigned nproc);

#ifdef __cplusplus
}
#endif

#endif /* OAH_LINUX_ABI_H */
