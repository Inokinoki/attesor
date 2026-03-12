/* ============================================================================
 * Minimal ARM64 Code Execution Test
 * ============================================================================
 *
 * This test verifies that we can execute simple ARM64 code
 * ============================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct {
    void *guest_mem_base;
    uint64_t guest_mem_size;
    void *state;
    uint64_t reserved[8];
} exec_context_t;

int main()
{
    printf("=================================================================\n");
    printf("Minimal ARM64 Code Execution Test\n");
    printf("=================================================================\n\n");

    /* Create simple ARM64 code: NOP, NOP, RET */
    uint8_t code[64];
    memset(code, 0, sizeof(code));

    /* ARM64 NOP: 0x1F2003D5 */
    code[0] = 0xD5;
    code[1] = 0x03;
    code[2] = 0x20;
    code[3] = 0x1F;

    /* ARM64 RET: 0xD65F03C0 */
    code[4] = 0xC0;
    code[5] = 0x03;
    code[6] = 0x5F;
    code[7] = 0xD6;

    printf("Generated ARM64 code:\n");
    printf("  Bytes: ");
    for (int i = 0; i < 8; i++) {
        printf("%02x ", code[i]);
    }
    printf("\n\n");

    /* Allocate executable memory */
    long page_size = sysconf(_SC_PAGESIZE);
    void *exec_code = mmap(NULL, page_size,
                           PROT_READ | PROT_WRITE | PROT_EXEC,
                           MAP_PRIVATE | MAP_ANONYMOUS,
                           -1, 0);

    if (exec_code == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    printf("Allocated executable memory at %p\n", exec_code);

    /* Copy code to executable memory */
    memcpy(exec_code, code, 8);

    /* Create execution context */
    exec_context_t exec_ctx;
    memset(&exec_ctx, 0, sizeof(exec_ctx));

    /* Set X18 and call the code */
    printf("Setting X18 to %p and calling code...\n", &exec_ctx);

    __asm__ volatile(
        "mov x18, %0\n"
        "blr %1\n"
        :
        : "r"(&exec_ctx), "r"(exec_code)
        : "x18"
    );

    printf("✅ SUCCESS: ARM64 code executed without segfault!\n");

    /* Cleanup */
    munmap(exec_code, page_size);

    printf("\n=================================================================\n");
    printf("Test Complete\n");
    printf("=================================================================\n");

    return 0;
}
