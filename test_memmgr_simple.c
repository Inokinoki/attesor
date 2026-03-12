/* Simple memory manager test */
#include "rosetta_memmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    printf("Testing memory manager...\n");

    /* Create memory manager */
    rosetta_memmgr_t *memmgr = rosetta_memmgr_create(0);
    if (!memmgr) {
        fprintf(stderr, "Failed to create memory manager\n");
        return 1;
    }

    printf("Memory manager created successfully\n");
    printf("Host base: %p\n", memmgr->host_base);
    printf("Total size: %zu\n", memmgr->total_size);

    /* Test mapping a simple segment */
    uint8_t test_data[] = {0x90, 0xC3, 0x00, 0x00};  // NOP, RET, zeros
    uint64_t addr = rosetta_memmgr_map_segment(
        memmgr,
        0x1000,
        test_data,
        sizeof(test_data),
        ROSETTA_PROT_READ | ROSETTA_PROT_EXEC,
        "test_segment"
    );

    printf("Segment mapped at: 0x%lx\n", addr);

    /* Test reading back */
    uint8_t read_data[4];
    ssize_t ret = rosetta_memmgr_read(memmgr, 0x1000, read_data, sizeof(read_data));
    printf("Read %zd bytes: ", ret);
    for (int i = 0; i < ret; i++) {
        printf("%02x ", read_data[i]);
    }
    printf("\n");

    /* Print memory map */
    rosetta_memmgr_print_map(memmgr);

    /* Cleanup */
    rosetta_memmgr_destroy(memmgr);

    printf("Memory manager test complete\n");
    return 0;
}
