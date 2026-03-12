/* ============================================================================
 * Rosetta Guest Memory Manager
 * ============================================================================
 *
 * Manages guest virtual address space for x86_64 binaries running on ARM64
 * ============================================================================
 */

#ifndef ROSETTA_MEMMGR_H
#define ROSETTA_MEMMGR_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>  /* For ssize_t */

/* ============================================================================
 * Memory Configuration
 * ============================================================================ */

#define ROSETTA_GUEST_SIZE     (1ULL << 30)  /* 1GB guest memory */
#define ROSETTA_GUEST_BASE     0x40000000ULL   /* Guest base address */
#define ROSETTA_PAGE_SIZE      4096           /* 4KB pages */
#define ROSETTA_STACK_SIZE     (8 * 1024 * 1024)  /* 8MB stack */

/* Memory protection flags */
#define ROSETTA_PROT_NONE      0x0
#define ROSETTA_PROT_READ      0x1
#define ROSETTA_PROT_WRITE     0x2
#define ROSETTA_PROT_EXEC      0x4

/* ============================================================================
 * Memory Mapping Structure
 * ============================================================================ */

/**
 * Memory region descriptor
 */
typedef struct rosetta_mem_region {
    uint64_t guest_addr;        /* Guest virtual address */
    void     *host_addr;        /* Host physical address */
    uint64_t size;              /* Region size */
    uint32_t prot;              /* Protection flags */
    uint32_t flags;             /* Additional flags */
    char     name[64];          /* Region name (for debugging) */
    struct rosetta_mem_region *next;  /* Next region in list */
} rosetta_mem_region_t;

/**
 * Guest memory manager
 */
typedef struct {
    void     *host_base;        /* Base of host memory */
    uint64_t  guest_base;       /* Base of guest virtual address space */
    size_t    total_size;       /* Total size of guest memory */
    size_t    used_size;        /* Currently used memory */

    /* Memory regions */
    rosetta_mem_region_t *regions;  /* List of mapped regions */

    /* Statistics */
    uint64_t num_mappings;      /* Number of memory mappings */
    uint64_t num_faults;        /* Number of page faults */

} rosetta_memmgr_t;

/* ============================================================================
 * Memory Manager API
 * ============================================================================ */

/**
 * Create guest memory manager
 * @param size Size of guest address space (default: 1GB)
 * @return Memory manager, or NULL on error
 */
rosetta_memmgr_t *rosetta_memmgr_create(size_t size);

/**
 * Destroy guest memory manager
 * @param mgr Memory manager to destroy
 */
void rosetta_memmgr_destroy(rosetta_memmgr_t *mgr);

/**
 * Allocate guest memory region
 * @param mgr Memory manager
 * @param guest_addr Desired guest address (or 0 for any)
 * @param size Size of region
 * @param prot Protection flags
 * @param name Region name
 * @return Guest virtual address, or 0 on error
 */
uint64_t rosetta_memmgr_alloc(rosetta_memmgr_t *mgr,
                               uint64_t guest_addr,
                               size_t size,
                               uint32_t prot,
                               const char *name);

/**
 * Map ELF segment into guest memory
 * @param mgr Memory manager
 * @param guest_addr Target guest address
 * @param host_addr Source data in host memory
 * @param size Size of segment
 * @param prot Protection flags
 * @param name Segment name
 * @return Guest virtual address, or 0 on error
 */
uint64_t rosetta_memmgr_map_segment(rosetta_memmgr_t *mgr,
                                     uint64_t guest_addr,
                                     const void *host_addr,
                                     size_t size,
                                     uint32_t prot,
                                     const char *name);

/**
 * Translate guest virtual address to host physical
 * @param mgr Memory manager
 * @param guest_addr Guest virtual address
 * @return Host physical address, or NULL if not mapped
 */
void *rosetta_memmgr_guest_to_host(rosetta_memmgr_t *mgr,
                                    uint64_t guest_addr);

/**
 * Read from guest memory
 * @param mgr Memory manager
 * @param guest_addr Guest virtual address
 * @param buf Buffer to read into
 * @param size Number of bytes to read
 * @return Number of bytes read, or -1 on error
 */
ssize_t rosetta_memmgr_read(rosetta_memmgr_t *mgr,
                             uint64_t guest_addr,
                             void *buf,
                             size_t size);

/**
 * Write to guest memory
 * @param mgr Memory manager
 * @param guest_addr Guest virtual address
 * @param buf Buffer to write from
 * @param size Number of bytes to write
 * @return Number of bytes written, or -1 on error
 */
ssize_t rosetta_memmgr_write(rosetta_memmgr_t *mgr,
                              uint64_t guest_addr,
                              const void *buf,
                              size_t size);

/**
 * Allocate guest stack
 * @param mgr Memory manager
 * @param size Stack size (default: 8MB)
 * @return Guest virtual address of stack top, or 0 on error
 */
uint64_t rosetta_memmgr_alloc_stack(rosetta_memmgr_t *mgr, size_t size);

/**
 * Print memory map (for debugging)
 * @param mgr Memory manager
 */
void rosetta_memmgr_print_map(rosetta_memmgr_t *mgr);

/**
 * Get memory manager statistics
 * @param mgr Memory manager
 * @param total_size Output: total size
 * @param used_size Output: used size
 * @param num_regions Output: number of regions
 */
void rosetta_memmgr_get_stats(rosetta_memmgr_t *mgr,
                               size_t *total_size,
                               size_t *used_size,
                               uint32_t *num_regions);

#endif /* ROSETTA_MEMMGR_H */
