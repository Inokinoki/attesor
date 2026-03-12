/* ============================================================================
 * Rosetta Guest Memory Manager - Implementation
 * ============================================================================
 *
 * Manages guest virtual address space for x86_64 binaries
 * ============================================================================
 */

#include "rosetta_memmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

/* ============================================================================
 * Memory Manager Creation and Destruction
 * ============================================================================ */

/**
 * Create guest memory manager
 */
rosetta_memmgr_t *rosetta_memmgr_create(size_t size)
{
    /* Round up size to page boundary */
    if (size == 0) {
        size = ROSETTA_GUEST_SIZE;
    }
    size = (size + ROSETTA_PAGE_SIZE - 1) & ~(ROSETTA_PAGE_SIZE - 1);

    /* Allocate memory manager structure */
    rosetta_memmgr_t *mgr = calloc(1, sizeof(rosetta_memmgr_t));
    if (!mgr) {
        fprintf(stderr, "Failed to allocate memory manager\n");
        return NULL;
    }

    /* Allocate guest memory from host */
    void *host_mem = mmap(NULL, size,
                           PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS,
                           -1, 0);

    if (host_mem == MAP_FAILED) {
        fprintf(stderr, "Failed to allocate guest memory: %s\n", strerror(errno));
        free(mgr);
        return NULL;
    }

    /* Initialize memory manager */
    mgr->host_base = host_mem;
    mgr->guest_base = 0;  /* Guest addresses start at 0 */
    mgr->total_size = size;
    mgr->used_size = 0;
    mgr->regions = NULL;
    mgr->num_mappings = 0;
    mgr->num_faults = 0;

    printf("[MEMMGR] Created guest memory:\n");
    printf("[MEMMGR]   Host:  %p\n", host_mem);
    printf("[MEMMGR]   Guest: 0x0 (flat address space)\n");
    printf("[MEMMGR]   Size:  %zu MB\n", size / (1024 * 1024));

    return mgr;
}

/**
 * Destroy guest memory manager
 */
void rosetta_memmgr_destroy(rosetta_memmgr_t *mgr)
{
    if (!mgr) {
        return;
    }

    /* Free all memory regions */
    rosetta_mem_region_t *region = mgr->regions;
    while (region) {
        rosetta_mem_region_t *next = region->next;
        free(region);
        region = next;
    }

    /* Unmap guest memory */
    if (mgr->host_base) {
        munmap(mgr->host_base, mgr->total_size);
    }

    printf("[MEMMGR] Destroyed guest memory manager\n");
    free(mgr);
}

/* ============================================================================
 * Memory Region Management
 * ============================================================================ */

/**
 * Add region to memory map
 */
static rosetta_mem_region_t *add_region(rosetta_memmgr_t *mgr,
                                          uint64_t guest_addr,
                                          void *host_addr,
                                          size_t size,
                                          uint32_t prot,
                                          const char *name)
{
    /* Allocate region structure */
    rosetta_mem_region_t *region = calloc(1, sizeof(rosetta_mem_region_t));
    if (!region) {
        return NULL;
    }

    region->guest_addr = guest_addr;
    region->host_addr = host_addr;
    region->size = size;
    region->prot = prot;
    region->flags = 0;

    if (name) {
        strncpy(region->name, name, sizeof(region->name) - 1);
    } else {
        snprintf(region->name, sizeof(region->name), "region_%lu",
                 mgr->num_mappings);
    }

    /* Add to list */
    region->next = mgr->regions;
    mgr->regions = region;
    mgr->num_mappings++;
    mgr->used_size += size;

    return region;
}

/* ============================================================================
 * Memory Allocation and Mapping
 * ============================================================================ */

/**
 * Calculate host address from guest address
 */
static inline void *guest_to_host_offset(rosetta_memmgr_t *mgr,
                                          uint64_t guest_addr)
{
    /* Check if address is within guest memory range */
    if (guest_addr >= mgr->total_size) {
        return NULL;
    }

    /* Calculate offset (guest_base is 0, so guest_addr is the offset) */
    return (char *)mgr->host_base + guest_addr;
}

/**
 * Allocate guest memory region
 */
uint64_t rosetta_memmgr_alloc(rosetta_memmgr_t *mgr,
                               uint64_t guest_addr,
                               size_t size,
                               uint32_t prot,
                               const char *name)
{
    if (!mgr || size == 0) {
        return 0;
    }

    /* Round size to page boundary */
    size = (size + ROSETTA_PAGE_SIZE - 1) & ~(ROSETTA_PAGE_SIZE - 1);

    /* If guest_addr is 0, allocate at next available address */
    if (guest_addr == 0) {
        guest_addr = mgr->guest_base + mgr->used_size;
    }

    /* Check if address is within guest memory range */
    void *host_addr = guest_to_host_offset(mgr, guest_addr);
    if (!host_addr) {
        fprintf(stderr, "[MEMMGR] Invalid guest address: 0x%lx\n", guest_addr);
        return 0;
    }

    /* Check for enough space */
    if (guest_addr + size > mgr->guest_base + mgr->total_size) {
        fprintf(stderr, "[MEMMGR] Not enough memory\n");
        return 0;
    }

    /* Add region to map */
    rosetta_mem_region_t *region = add_region(mgr, guest_addr, host_addr,
                                                size, prot, name);
    if (!region) {
        return 0;
    }

    printf("[MEMMGR] Allocated: 0x%lx-0x%lx %c%c%c [%s]\n",
           guest_addr,
           guest_addr + size - 1,
           (prot & ROSETTA_PROT_READ) ? 'R' : '-',
           (prot & ROSETTA_PROT_WRITE) ? 'W' : '-',
           (prot & ROSETTA_PROT_EXEC) ? 'X' : '-',
           region->name);

    return guest_addr;
}

/**
 * Map ELF segment into guest memory
 */
uint64_t rosetta_memmgr_map_segment(rosetta_memmgr_t *mgr,
                                     uint64_t guest_addr,
                                     const void *host_addr,
                                     size_t size,
                                     uint32_t prot,
                                     const char *name)
{
    if (!mgr || !host_addr || size == 0) {
        return 0;
    }

    /* Round size to page boundary */
    size_t map_size = (size + ROSETTA_PAGE_SIZE - 1) & ~(ROSETTA_PAGE_SIZE - 1);

    /* Check if address is within guest memory range */
    void *target_host = guest_to_host_offset(mgr, guest_addr);
    if (!target_host) {
        fprintf(stderr, "[MEMMGR] Invalid guest address: 0x%lx\n", guest_addr);
        return 0;
    }

    /* Check for enough space */
    if (guest_addr + map_size > mgr->guest_base + mgr->total_size) {
        fprintf(stderr, "[MEMMGR] Not enough memory for segment\n");
        return 0;
    }

    /* Copy segment data to guest memory */
    memcpy(target_host, host_addr, size);

    /* Zero any remaining space in the page */
    if (map_size > size) {
        memset((char *)target_host + size, 0, map_size - size);
    }

    /* Add region to map */
    rosetta_mem_region_t *region = add_region(mgr, guest_addr, target_host,
                                                map_size, prot, name);
    if (!region) {
        fprintf(stderr, "[MEMMGR] Failed to add region\n");
        return 0;
    }

    /* Debug: Check region name before printing */
    if (!region->name) {
        fprintf(stderr, "[MEMMGR] ERROR: region->name is NULL!\n");
        return 0;
    }

    /* Simplified printf to debug */
    fprintf(stderr, "[MEMMGR DEBUG] About to print segment info\n");
    fprintf(stderr, "[MEMMGR DEBUG] guest_addr=%lx map_size=%zu size=%zu\n",
            guest_addr, map_size, size);
    fprintf(stderr, "[MEMMGR DEBUG] region->name=%s\n", region->name);

    printf("[MEMMGR] Mapped: 0x%lx-0x%lx %c%c%c [%s] (%zu bytes)\n",
           guest_addr,
           guest_addr + map_size - 1,
           (prot & ROSETTA_PROT_READ) ? 'R' : '-',
           (prot & ROSETTA_PROT_WRITE) ? 'W' : '-',
           (prot & ROSETTA_PROT_EXEC) ? 'X' : '-',
           region->name ? region->name : "(null)",
           size);

    fprintf(stderr, "[MEMMGR DEBUG] Printed successfully\n");

    return guest_addr;
}

/* ============================================================================
 * Address Translation
 * ============================================================================ */

/**
 * Translate guest virtual address to host physical
 */
void *rosetta_memmgr_guest_to_host(rosetta_memmgr_t *mgr,
                                    uint64_t guest_addr)
{
    if (!mgr) {
        return NULL;
    }

    /* Quick bounds check */
    if (guest_addr >= mgr->total_size) {
        return NULL;
    }

    /* Calculate offset (guest_base is 0, so guest_addr is the offset) */
    return (char *)mgr->host_base + guest_addr;
}

/* ============================================================================
 * Memory Access Functions
 * ============================================================================ */

/**
 * Read from guest memory
 */
ssize_t rosetta_memmgr_read(rosetta_memmgr_t *mgr,
                             uint64_t guest_addr,
                             void *buf,
                             size_t size)
{
    if (!mgr || !buf || size == 0) {
        return -1;
    }

    /* Translate address */
    void *host_addr = rosetta_memmgr_guest_to_host(mgr, guest_addr);
    if (!host_addr) {
        fprintf(stderr, "[MEMMGR] Invalid read address: 0x%lx\n", guest_addr);
        return -1;
    }

    /* Check bounds */
    if (guest_addr + size > mgr->total_size) {
        fprintf(stderr, "[MEMMGR] Read out of bounds\n");
        return -1;
    }

    /* Copy data */
    memcpy(buf, host_addr, size);
    return size;
}

/**
 * Write to guest memory
 */
ssize_t rosetta_memmgr_write(rosetta_memmgr_t *mgr,
                              uint64_t guest_addr,
                              const void *buf,
                              size_t size)
{
    if (!mgr || !buf || size == 0) {
        return -1;
    }

    /* Translate address */
    void *host_addr = rosetta_memmgr_guest_to_host(mgr, guest_addr);
    if (!host_addr) {
        fprintf(stderr, "[MEMMGR] Invalid write address: 0x%lx\n", guest_addr);
        return -1;
    }

    /* Check bounds */
    if (guest_addr + size > mgr->total_size) {
        fprintf(stderr, "[MEMMGR] Write out of bounds\n");
        return -1;
    }

    /* Copy data */
    memcpy(host_addr, buf, size);
    return size;
}

/* ============================================================================
 * Stack Allocation
 * ============================================================================ */

/**
 * Allocate guest stack
 */
uint64_t rosetta_memmgr_alloc_stack(rosetta_memmgr_t *mgr, size_t size)
{
    if (!mgr) {
        return 0;
    }

    if (size == 0) {
        size = ROSETTA_STACK_SIZE;
    }

    /* Round to page boundary */
    size = (size + ROSETTA_PAGE_SIZE - 1) & ~(ROSETTA_PAGE_SIZE - 1);

    /* Allocate stack at high address */
    uint64_t stack_addr = mgr->total_size - size;

    /* Align stack */
    stack_addr &= ~15ULL;  /* 16-byte alignment */

    uint64_t result = rosetta_memmgr_alloc(mgr, stack_addr, size,
                                            ROSETTA_PROT_READ | ROSETTA_PROT_WRITE,
                                            "stack");
    if (result == 0) {
        return 0;
    }

    /* Return stack top (highest address) */
    return result + size;
}

/* ============================================================================
 * Debugging and Statistics
 * ============================================================================ */

/**
 * Print memory map
 */
void rosetta_memmgr_print_map(rosetta_memmgr_t *mgr)
{
    if (!mgr) {
        return;
    }

    printf("\n");
    printf("=================================================================\n");
    printf("Guest Memory Map\n");
    printf("=================================================================\n");
    printf("Host Base:  %p\n", mgr->host_base);
    printf("Guest Base: 0x%lx\n", mgr->guest_base);
    printf("Total Size: %zu MB\n", mgr->total_size / (1024 * 1024));
    printf("Used Size:  %zu MB\n", mgr->used_size / (1024 * 1024));
    printf("Mappings:   %lu\n", mgr->num_mappings);
    printf("\n");

    printf("Regions:\n");
    rosetta_mem_region_t *region = mgr->regions;
    while (region) {
        printf("  0x%016lx-0x%016lx %c%c%c %s\n",
               region->guest_addr,
               region->guest_addr + region->size - 1,
               (region->prot & ROSETTA_PROT_READ) ? 'R' : '-',
               (region->prot & ROSETTA_PROT_WRITE) ? 'W' : '-',
               (region->prot & ROSETTA_PROT_EXEC) ? 'X' : '-',
               region->name);
        region = region->next;
    }

    printf("=================================================================\n");
}

/**
 * Get memory manager statistics
 */
void rosetta_memmgr_get_stats(rosetta_memmgr_t *mgr,
                               size_t *total_size,
                               size_t *used_size,
                               uint32_t *num_regions)
{
    if (!mgr) {
        return;
    }

    if (total_size) {
        *total_size = mgr->total_size;
    }
    if (used_size) {
        *used_size = mgr->used_size;
    }
    if (num_regions) {
        *num_regions = mgr->num_mappings;
    }
}
