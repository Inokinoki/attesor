#include "oah/linux_abi.h"

#include <string.h>
#include <stdio.h>

static int path_eq(const char *path, const char *want)
{
    return path && strcmp(path, want) == 0;
}

oah_proc_path_match oah_classify_proc_path(const char *path, oah_proc_path_kind kind,
                                           int self_pid)
{
    /* FUN_80000002e50c */
    oah_proc_path_match m = {false, false};
    char tmp[64];
    const char *self_s, *thread_s, *pid_fmt, *task_fmt;

    if (!path) {
        return m;
    }

    switch (kind) {
    case OAH_PROC_PATH_AUXV:
        self_s = "/proc/self/auxv";
        thread_s = "/proc/thread-self/auxv";
        pid_fmt = "/proc/%d/auxv";
        task_fmt = "/proc/%d/task/%d/auxv";
        break;
    case OAH_PROC_PATH_CMDLINE:
        self_s = "/proc/self/cmdline";
        thread_s = "/proc/thread-self/cmdline";
        pid_fmt = "/proc/%d/cmdline";
        task_fmt = "/proc/%d/task/%d/cmdline";
        break;
    case OAH_PROC_PATH_EXE:
    default:
        self_s = "/proc/self/exe";
        thread_s = "/proc/thread-self/exe";
        pid_fmt = "/proc/%d/exe";
        task_fmt = "/proc/%d/task/%d/exe";
        break;
    }

    if (path_eq(path, self_s)) {
        m.matched = true;
        m.is_thread_self = false;
        return m;
    }
    if (path_eq(path, thread_s)) {
        m.matched = true;
        m.is_thread_self = true;
        return m;
    }
    snprintf(tmp, sizeof(tmp), pid_fmt, self_pid);
    if (path_eq(path, tmp)) {
        m.matched = true;
        m.is_thread_self = false;
        return m;
    }
    snprintf(tmp, sizeof(tmp), task_fmt, self_pid, self_pid);
    if (path_eq(path, tmp)) {
        m.matched = true;
        m.is_thread_self = true;
        return m;
    }
    return m;
}

u32 oah_translate_open_flags(u32 x86_flags)
{
    /* FUN_80000002bf74 — bit shuffle between x86 and ARM Linux O_* */
    u32 in = x86_flags;
    u32 shifted = in >> 2;
    return (in & 0xfffc0000u) |
           (shifted & 0x4000u) |
           (in & 0x3fffu) |
           (shifted & 0x8000u) |
           ((in >> 14 & 1u) << 16) |
           ((in >> 15 & 1u) << 17);
}

int oah_write_fake_cpuinfo(FILE *out, unsigned nproc)
{
    /* FUN_80000002bf74 create_fake_cpuinfo_file format string. */
    unsigned i;
    if (!out) {
        return -1;
    }
    for (i = 0; i < nproc; i++) {
        if (fprintf(out,
                    "processor\t: %u\n"
                    "vendor_id\t: VirtualApple\n"
                    "cpu family\t: 6\n"
                    "model\t\t: 142\n"
                    "model name\t: VirtualApple @ 2.50GHz\n"
                    "stepping\t: 10\n"
                    "cpu MHz\t\t: 2502.057\n"
                    "cache size\t: 6144 KB\n"
                    "physical id\t: 0\n"
                    "siblings\t: %u\n"
                    "core id\t\t: %u\n"
                    "cpu cores\t: %u\n"
                    "apicid\t\t: %u\n"
                    "initial apicid\t: %u\n"
                    "fpu\t\t: yes\n"
                    "fpu_exception\t: no\n"
                    "cpuid level\t: 22\n"
                    "wp\t\t: yes\n"
                    "flags\t\t: fpu tsc de cx8 apic sep cmov pat pse36 clflush mmx fxsr sse sse2 syscall nx lm rep_good nopl pni cpuid pclmulqdq ssse3 cx16 sse4_1 popcnt sse4_2 aes lahf_lm movbe fma avx f16c rdrand bmi1 avx2 bmi2\n"
                    "bugs\t\t:\n"
                    "bogomips\t: 5184.11\n"
                    "clflush size\t: 64\n"
                    "cache_alignment\t: 64\n"
                    "address sizes\t: 39 bits physical, 48 bits virtual\n"
                    "power management:\n"
                    "\n",
                    i, nproc, i, nproc, i, i) < 0) {
            return -1;
        }
    }
    return 0;
}
