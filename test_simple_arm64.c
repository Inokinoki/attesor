/* ============================================================================
 * Test Simple ARM64 Code Execution
 * ============================================================================ *
 * This test creates a minimal ARM64 function that just returns,
 * to verify that dynamic code execution works at all.
 * ============================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

/* Emit a single ARM64 instruction (little-endian) */
static void emit_arm64_insn(uint8_t **code_ptr, uint32_t insn) {
    uint8_t *code = *code_ptr;
    code[0] = (insn >> 0) & 0xFF;
    code[1] = (insn >> 8) & 0xFF;
    code[2] = (insn >> 16) & 0xFF;
    code[3] = (insn >> 24) & 0xFF;
    *code_ptr += 4;
}

/* Test: Create a simple ARM64 function that returns */
int test_simple_arm64_function(void) {
    const size_t code_size = 16;  // 4 instructions
    uint8_t *code;

    /* Allocate memory with RWX permissions */
    code = mmap(NULL, code_size,
                PROT_READ | PROT_WRITE | PROT_EXEC,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (code == MAP_FAILED) {
        perror("[TEST] mmap failed");
        return -1;
    }

    printf("[TEST] Allocated code at %p\n", code);

    /* Emit ARM64 instructions:
     * 1. MOV X0, #42   - Set return value to 42
     * 2. MOV X8, X0    - Copy to X8 (saved register)
     * 3. MOV X0, #0    - Set X0 to 0 (main return value register)
     * 4. RET           - Return to caller
     */
    uint8_t *code_ptr = code;
    emit_arm64_insn(&code_ptr, 0xD2800540);  // MOVZ X0, #42
    emit_arm64_insn(&code_ptr, 0xAA0803E0);  // MOV X8, X0
    emit_arm64_insn(&code_ptr, 0xD2800000);  // MOVZ X0, #0
    emit_arm64_insn(&code_ptr, 0xD65F03C0);  // RET

    printf("[TEST] Generated ARM64 code:\n");
    printf("[TEST]   ");
    for (size_t i = 0; i < code_size; i++) {
        printf("%02x ", code[i]);
    }
    printf("\n");

    /* Clear instruction cache */
    printf("[TEST] Clearing instruction cache...\n");
    __builtin___clear_cache((char *)code, (char *)code + code_size);

    /* Call the function */
    printf("[TEST] Calling ARM64 function...\n");
    typedef int (*arm64_func_t)(void);
    arm64_func_t func = (arm64_func_t)code;

    int result = func();
    printf("[TEST] Function returned: %d\n", result);

    /* Check if X8 was preserved (should be 42) */
    uint64_t x8_value;
    __asm__ volatile("mov %0, x8" : "=r"(x8_value));
    printf("[TEST] X8 (saved register) = %lu\n", x8_value);

    /* Clean up */
    munmap(code, code_size);

    return 0;
}

int main(void) {
    printf("=================================================================\n");
    printf("Simple ARM64 Code Execution Test\n");
    printf("=================================================================\n\n");

    if (test_simple_arm64_function() == 0) {
        printf("\n[TEST] ✅ SUCCESS: Simple ARM64 execution works!\n");
        return 0;
    } else {
        printf("\n[TEST] ❌ FAILED: Simple ARM64 execution failed!\n");
        return 1;
    }
}
