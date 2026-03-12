/* ============================================================================
 * Minimal Memory Access Test
 * ============================================================================
 *
 * This test verifies that guest memory access works correctly
 * ============================================================================ */

#include "rosetta_memmgr.h"
#include "rosetta_exec_context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    printf("=================================================================\n");
    printf("Minimal Memory Access Test\n");
    printf("=================================================================\n\n");

    /* Create memory manager */
    rosetta_memmgr_t *memmgr = rosetta_memmgr_create(1024 * 1024);  /* 1MB */
    if (!memmgr) {
        fprintf(stderr, "Failed to create memory manager\n");
        return 1;
    }

    printf("Memory manager created:\n");
    printf("  host_base: %p\n", memmgr->host_base);
    printf("  total_size: 0x%lx\n\n", memmgr->total_size);

    /* Create execution context */
    rosetta_exec_context_t exec_ctx;
    exec_ctx.guest_mem_base = memmgr->host_base;
    exec_ctx.guest_mem_size = memmgr->total_size;
    exec_ctx.state = NULL;
    memset(exec_ctx.reserved, 0, sizeof(exec_ctx.reserved));

    printf("Execution context:\n");
    printf("  guest_mem_base: %p\n", exec_ctx.guest_mem_base);
    printf("  guest_mem_size: 0x%lx\n\n", exec_ctx.guest_mem_size);

    /* Test: Write to guest memory */
    uint64_t test_addr = 0x1000;
    uint64_t test_value = 0xDEADBEEFCAFEBABE;

    printf("Test 1: Writing 0x%lx to guest address 0x%lx\n", test_value, test_addr);
    rosetta_mem_write64(&exec_ctx, test_addr, test_value);

    /* Test: Read from guest memory */
    printf("Test 2: Reading from guest address 0x%lx\n", test_addr);
    uint64_t read_value = rosetta_mem_read64(&exec_ctx, test_addr);
    printf("  Read value: 0x%lx\n", read_value);

    /* Verify */
    if (read_value == test_value) {
        printf("✅ SUCCESS: Memory read/write works correctly!\n");
    } else {
        printf("❌ FAILED: Expected 0x%lx, got 0x%lx\n", test_value, read_value);
    }

    /* Cleanup */
    rosetta_memmgr_destroy(memmgr);

    printf("\n=================================================================\n");
    printf("Test Complete\n");
    printf("=================================================================\n");

    return (read_value == test_value) ? 0 : 1;
}
