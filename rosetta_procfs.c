/* ============================================================================
 * Rosetta /proc Filesystem Emulation - Implementation
 * ============================================================================
 *
 * This module implements /proc filesystem emulation for x86_64 binaries
 * running on ARM64 hosts.
 * ============================================================================ */

#include "rosetta_procfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/sysinfo.h>
#include <limits.h>
#include <sys/syscall.h>  /* For syscall numbers */

/* ============================================================================
 * /proc File Handle Management
 * ============================================================================ */

#define MAX_PROC_FILES 16

static rosetta_proc_file_t g_proc_files[MAX_PROC_FILES];
static int g_proc_files_initialized = 0;

/* ============================================================================
 * /proc Path Detection
 * ============================================================================ */

/**
 * Check if a path is a /proc path
 */
int rosetta_proc_is_proc_path(const char *path)
{
    if (!path) {
        return 0;
    }

    return (strncmp(path, "/proc/", 6) == 0) ||
           (strncmp(path, "/proc/self/", 11) == 0) ||
           (strcmp(path, "/proc") == 0);
}

/**
 * Get the type of /proc path
 */
rosetta_proc_path_t rosetta_proc_get_path_type(const char *path)
{
    if (!path) {
        return ROSETTA_PROC_UNKNOWN;
    }

    /* Check specific paths */
    if (strcmp(path, "/proc/cpuinfo") == 0) {
        return ROSETTA_PROC_CPUINFO;
    }

    if (strcmp(path, "/proc/self/auxv") == 0 ||
        strncmp(path, "/proc/self/task/", 15) == 0) {  /* Also handle /proc/self/task/<tid>/auxv */
        if (strstr(path, "/auxv")) {
            return ROSETTA_PROC_SELF_AUXV;
        }
    }

    if (strcmp(path, "/proc/self/cmdline") == 0) {
        return ROSETTA_PROC_SELF_CMDLINE;
    }

    if (strcmp(path, "/proc/self/exe") == 0) {
        return ROSETTA_PROC_SELF_EXE;
    }

    if (strcmp(path, "/proc/meminfo") == 0) {
        return ROSETTA_PROC_MEMINFO;
    }

    if (strcmp(path, "/proc/self/status") == 0) {
        return ROSETTA_PROC_SELF_STATUS;
    }

    if (strcmp(path, "/proc/self/maps") == 0) {
        return ROSETTA_PROC_SELF_MAPS;
    }

    return ROSETTA_PROC_UNKNOWN;
}

/* ============================================================================
 * /proc File Operations
 * ============================================================================ */

/**
 * Find a free /proc file handle slot
 */
static int find_free_proc_slot(void)
{
    if (!g_proc_files_initialized) {
        memset(g_proc_files, 0, sizeof(g_proc_files));
        g_proc_files_initialized = 1;
    }

    for (int i = 0; i < MAX_PROC_FILES; i++) {
        if (!g_proc_files[i].is_open) {
            return i;
        }
    }

    return -1;
}

/**
 * Open a /proc file
 */
int rosetta_proc_open(const char *path, int flags)
{
    /* Check if this is a supported /proc path */
    rosetta_proc_path_t type = rosetta_proc_get_path_type(path);
    if (type == ROSETTA_PROC_UNKNOWN) {
        errno = ENOENT;  /* No such file or directory */
        return -1;
    }

    int slot = find_free_proc_slot();
    if (slot < 0) {
        errno = EMFILE;  /* Too many open files */
        return -1;
    }

    rosetta_proc_file_t *file = &g_proc_files[slot];
    memset(file, 0, sizeof(*file));

    file->type = type;
    strncpy(file->path, path, sizeof(file->path) - 1);
    file->offset = 0;
    file->is_open = 1;

    /* Return a fake fd that maps to our slot */
    return 1000 + slot;  /* Use fd numbers > 1000 for /proc files */
}

/**
 * Read from a /proc file
 */
ssize_t rosetta_proc_read(int fd, void *buf, size_t count)
{
    if (!buf || count == 0) {
        return 0;
    }

    /* Check if this is a /proc file fd */
    if (fd < 1000 || fd >= 1000 + MAX_PROC_FILES) {
        errno = EBADF;
        return -1;
    }

    int slot = fd - 1000;
    rosetta_proc_file_t *file = &g_proc_files[slot];

    if (!file->is_open) {
        errno = EBADF;
        return -1;
    }

    char temp_buf[4096];
    int content_len = 0;

    /* Generate content based on file type */
    switch (file->type) {
    case ROSETTA_PROC_CPUINFO:
        content_len = rosetta_proc_generate_cpuinfo(temp_buf, sizeof(temp_buf));
        break;

    case ROSETTA_PROC_SELF_AUXV:
        content_len = rosetta_proc_generate_auxv(temp_buf, sizeof(temp_buf));
        break;

    case ROSETTA_PROC_SELF_CMDLINE:
        content_len = rosetta_proc_generate_cmdline(temp_buf, sizeof(temp_buf));
        break;

    case ROSETTA_PROC_MEMINFO:
        content_len = rosetta_proc_generate_meminfo(temp_buf, sizeof(temp_buf));
        break;

    case ROSETTA_PROC_SELF_STATUS:
        content_len = rosetta_proc_generate_status(temp_buf, sizeof(temp_buf));
        break;

    case ROSETTA_PROC_SELF_MAPS:
        content_len = rosetta_proc_generate_maps(temp_buf, sizeof(temp_buf));
        break;

    default:
        errno = ENOENT;
        return -1;
    }

    if (content_len < 0) {
        return -1;
    }

    /* Calculate how much to read */
    int remaining = content_len - file->offset;
    if (remaining <= 0) {
        return 0;  /* EOF */
    }

    int to_read = (count < (size_t)remaining) ? count : remaining;
    memcpy(buf, temp_buf + file->offset, to_read);
    file->offset += to_read;

    return to_read;
}

/**
 * Close a /proc file
 */
int rosetta_proc_close(int fd)
{
    /* Check if this is a /proc file fd */
    if (fd < 1000 || fd >= 1000 + MAX_PROC_FILES) {
        errno = EBADF;
        return -1;
    }

    int slot = fd - 1000;
    rosetta_proc_file_t *file = &g_proc_files[slot];

    if (!file->is_open) {
        errno = EBADF;
        return -1;
    }

    file->is_open = 0;
    return 0;
}

/**
 * Handle readlink for /proc paths
 */
ssize_t rosetta_proc_readlink(const char *path, char *buf, size_t size)
{
    if (!path || !buf || size == 0) {
        errno = EINVAL;
        return -1;
    }

    /* Handle /proc/self/exe */
    if (strcmp(path, "/proc/self/exe") == 0 ||
        strncmp(path, "/proc/self/exe/", 14) == 0) {
        return rosetta_proc_get_exe_path(buf, size);
    }

    /* Other /proc paths not supported for readlink */
    errno = EINVAL;
    return -1;
}

/* ============================================================================
 * /proc Content Generation
 * ============================================================================ */

/**
 * Generate /proc/cpuinfo content
 * Returns fake ARM64 CPU information for compatibility
 */
int rosetta_proc_generate_cpuinfo(char *buf, size_t size)
{
    if (!buf || size == 0) {
        return -1;
    }

    /* Get system info */
    struct sysinfo info;
    sysinfo(&info);

    /* Get number of CPU cores */
    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    if (nprocs < 1) nprocs = 1;

    int len = snprintf(buf, size,
        "processor\t: 0\n"
        "vendor_id\t: AuthenticAMD\n"
        "cpu family\t: 23\n"
        "model\t\t: 1\n"
        "model name\t: ARM64 Processor (emulated)\n"
        "stepping\t: 1\n"
        "microcode\t: 0x0\n"
        "cpu MHz\t\t: %ld\n"
        "cache size\t: %ld KB\n"
        "physical id\t: 0\n"
        "siblings\t: %ld\n"
        "core id\t\t: 0\n"
        "cpu cores\t: %ld\n"
        "apicid\t\t: 0\n"
        "initial apicid\t: 0\n"
        "fpu\t\t\t: yes\n"
        "fpu_exception\t: yes\n"
        "cpuid level\t: 13\n"
        "wp\t\t\t: yes\n"
        "flags\t\t\t: fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush mmx fxsr sse sse2 ss ht syscall nx pdpe1gb rdtscp lm constant_tsc rep_good nopl xtopology tsc_reliable nonstop_tsc cpuid tsc_known_freq pni pclmulqdq ssse3 fma cx16 pcid sse4_1 sse4_2 x2apic movbe popcnt aes xsave avx f16c rdrand hypervisor lahf_lm abm 3dnowprefetch invpcid_single ssbd ibrs ibpb stibp tpr_shadow flexnor_fdrtsb fsrs rdcl_NO\n"
        "bugs\t\t\t: sysret_ss_attrs null_seg spectre_v1 spectre_v2 spec_store_bypass retbleed smt_rsb\n"
        "bogomips\t: %lu.00\n",
        sysconf(_SC_CLK_TCK) / 1000,  /* Approximate MHz */
        sysconf(_SC_PAGESIZE) / 1024,  /* Cache size in KB */
        nprocs,
        nprocs,
        info.mem_unit / 1024  /* Bogomips approximation */
    );

    if (len < 0 || (size_t)len >= size) {
        return -1;
    }

    return len;
}

/**
 * Generate /proc/self/auxv content
 * Returns fake auxiliary vector for compatibility
 */
int rosetta_proc_generate_auxv(char *buf, size_t size)
{
    if (!buf || size == 0) {
        return -1;
    }

    /* Fake auxiliary vector - minimal set for compatibility */
    unsigned long auxv[] = {
        6,   /* AT_PAGESZ */
        16,  /* AT_CLKTCK */
        17,  /* AT_PHDR */
        3,   /* AT_PHENT */
        4,   /* AT_PHNUM */
        5,   /* AT_BASE */
        9,   /* AT_ENTRY */
        0    /* AT_NULL */
    };

    int offset = 0;
    for (int i = 0; i < (int)(sizeof(auxv)/sizeof(auxv[0])); i++) {
        if (offset + 16 > (int)size) {
            break;
        }
        memcpy(buf + offset, &auxv[i], 8);
        offset += 8;
    }

    return offset;
}

/**
 * Generate /proc/self/cmdline content
 * Returns command line arguments
 */
int rosetta_proc_generate_cmdline(char *buf, size_t size)
{
    if (!buf || size == 0) {
        return -1;
    }

    /* For now, return empty or a fake command line */
    /* This would need to be populated from actual process info */
    if (size > 0) {
        buf[0] = '\0';  /* Empty command line */
        return 1;
    }

    return 0;
}

/**
 * Get /proc/self/exe path
 * Returns path to the current executable
 */
int rosetta_proc_get_exe_path(char *buf, size_t size)
{
    if (!buf || size == 0) {
        return -1;
    }

    /* Read /proc/self/exe on the host system */
    ssize_t len = readlink("/proc/self/exe", buf, size);

    if (len < 0) {
        /* Fallback: try to get executable path from /proc/self/comm */
        FILE *f = fopen("/proc/self/comm", "r");
        if (f) {
            if (fgets(buf, size, f)) {
                /* Remove newline */
                char *nl = strchr(buf, '\n');
                if (nl) *nl = '\0';
                len = strlen(buf);
            } else {
                len = -1;
            }
            fclose(f);
        } else {
            len = -1;
        }
    }

    return len;
}

/**
 * Generate /proc/meminfo content
 * Returns system memory information
 */
int rosetta_proc_generate_meminfo(char *buf, size_t size)
{
    if (!buf || size == 0) {
        return -1;
    }

    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        return -1;
    }

    unsigned long total_ram = info.totalram * info.mem_unit;
    unsigned long free_ram = info.freeram * info.mem_unit;
    unsigned long total_swap = info.totalswap * info.mem_unit;
    unsigned long free_swap = info.freeswap * info.mem_unit;

    int len = snprintf(buf, size,
        "MemTotal:\t%lu kB\n"
        "MemFree:\t%lu kB\n"
        "MemAvailable:\t%lu kB\n"
        "Buffers:\t0 kB\n"
        "Cached:\t0 kB\n"
        "SwapCached:\t0 kB\n"
        "Active:\t0 kB\n"
        "Inactive:\t0 kB\n"
        "SwapTotal:\t%lu kB\n"
        "SwapFree:\t%lu kB\n"
        "Dirty:\t0 kB\n"
        "Writeback:\t0 kB\n"
        "AnonPages:\t0 kB\n"
        "Mapped:\t0 kB\n"
        "Shmem:\t0 kB\n"
        "Slab:\t0 kB\n"
        "SReclaimable:\t0 kB\n"
        "PageTables:\t0 kB\n"
        "NFS_Unstable:\t0 kB\n"
        "Bounce:\t0 kB\n"
        "WritebackTmp:\t0 kB\n"
        "CommitLimit:\t%lu kB\n"
        "Committed_AS:\t%lu kB\n"
        "VmallocTotal:\t0 kB\n"
        "VmallocUsed:\t0 kB\n"
        "VmallocChunk:\t0 kB\n",
        total_ram / 1024,
        free_ram / 1024,
        free_ram / 1024,
        total_swap / 1024,
        free_swap / 1024,
        (total_ram + total_swap) / 1024,
        (total_ram + total_swap) / 1024
    );

    if (len < 0 || (size_t)len >= size) {
        return -1;
    }

    return len;
}

/**
 * Generate /proc/self/status content
 * Returns process status information
 */
int rosetta_proc_generate_status(char *buf, size_t size)
{
    if (!buf || size == 0) {
        return -1;
    }

    int len = snprintf(buf, size,
        "Name:\trosetta\n"
        "State:\tR (running)\n"
        "Pid:\t%d\n"
        "PPid:\t%d\n"
        "Uid:\t%d\t%d\t%d\t%d\n"
        "Gid:\t%d\t%d\t%d\t%d\n"
        "Threads:\t1\n"
        "VmPeak:\t0 kB\n"
        "VmSize:\t0 kB\n"
        "VmRSS:\t0 kB\n"
        "VmData:\t0 kB\n"
        "VmStk:\t0 kB\n"
        "VmExe:\t0 kB\n"
        "VmLib:\t0 kB\n"
        "SigQ:\t0/0\n"
        "SigPnd:\t0000000000000000\n"
        "ShdPnd:\t0000000000000000\n"
        "CapInh:\t0000000000000000\n"
        "CapPrm:\t0000000000000000\n"
        "CapEff:\t0000000000000000\n"
        "CapBnd:\t0000000000000000\n"
        "Seccomp:\t0\n"
        "Cpus_allowed:\t%f\n"
        "Cpus_allowed_list:\t0\n"
        "Mems_allowed:\t00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000001\n"
        "Mems_allowed_list:\t0\n"
        "voluntary_ctxt_switches:\t0\n"
        "nonvoluntary_ctxt_switches:\t0\n",
        getpid(),
        getppid(),
        getuid(), getuid(), getuid(), getuid(),
        getgid(), getgid(), getgid(), getgid(),
        (unsigned long long)-1  /* All CPUs allowed */
    );

    if (len < 0 || (size_t)len >= size) {
        return -1;
    }

    return len;
}

/**
 * Generate /proc/self/maps content
 * Returns memory map information
 */
int rosetta_proc_generate_maps(char *buf, size_t size)
{
    if (!buf || size == 0) {
        return -1;
    }

    /* Return minimal maps - would need actual memory map info */
    int len = snprintf(buf, size,
        "400000-401000 r-xp 00000000 00:00 0\n"
        "401000-402000 rw-p 00000000 00:00 0\n"
    );

    if (len < 0 || (size_t)len >= size) {
        return -1;
    }

    return len;
}

/* ============================================================================
 * memfd_create Implementation
 * ============================================================================ */

/**
 * Create an anonymous file in memory
 * On Linux, uses memfd_create system call
 * On other platforms, creates a temporary file as fallback
 */
int rosetta_memfd_create(const char *name, unsigned int flags)
{
#ifdef __linux__
    /* Use Linux memfd_create system call */
    return syscall(__NR_memfd_create, name, flags);
#else
    /* Fallback for non-Linux systems (e.g., macOS) */
    char template[64];

    if (name) {
        snprintf(template, sizeof(template), "/tmp/rosetta_%s_XXXXXX", name);
    } else {
        snprintf(template, sizeof(template), "/tmp/rosetta_memfd_XXXXXX");
    }

    /* Create temporary file */
    int fd = mkstemp(template);

    if (fd >= 0) {
        /* Unlink the file so it will be deleted when closed */
        unlink(template);

        /* Set close-on-exec flag */
        fcntl(fd, F_SETFD, FD_CLOEXEC);
    }

    return fd;
#endif
}
