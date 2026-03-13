/* Decoder performance benchmark */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "rosetta_types.h"
#include "rosetta_x86_decode.h"

#define TEST_ITERATIONS 1000000

typedef struct {
    const char *name;
    uint8_t bytes[16];
    int length;
} perf_test_t;

int main() {
    perf_test_t tests[] = {
        /* Simple instructions */
        { "NOP", {0x90}, 1 },
        { "RET", {0xC3}, 1 },
        { "PUSH RAX", {0x50}, 1 },
        { "POP RAX", {0x58}, 1 },
        
        /* Medium complexity */
        { "MOV RAX, RBX", {0x48, 0x89, 0xD8}, 3 },
        { "ADD RAX, RBX", {0x48, 0x01, 0xD8}, 3 },
        { "XOR RAX, RAX", {0x48, 0x31, 0xC0}, 3 },
        { "CMP RAX, RBX", {0x48, 0x39, 0xC3}, 3 },
        
        /* Complex instructions */
        { "MOV RAX, [RIP+disp32]", {0x48, 0x8B, 0x05, 0x00, 0x01, 0x00, 0x00}, 7 },
        { "MOV RAX, [RBX+RCX*8]", {0x48, 0x8B, 0x04, 0xCB}, 4 },
        { "MOV RAX, [RBX+RCX*4+0x10]", {0x48, 0x8B, 0x84, 0xCB, 0x10, 0x00, 0x00, 0x00}, 8 },
        
        /* SIMD instructions */
        { "MOVAPS XMM0, XMM1", {0x0F, 0x28, 0xC1}, 3 },
        { "ADDPS XMM0, XMM1", {0x0F, 0x58, 0xC1}, 3 },
        { "VADDPS XMM0, XMM1, XMM2", {0xC5, 0xF0, 0x58, 0xC2}, 4 },
        
        /* VEX instructions */
        { "VMULPS XMM0, XMM1, XMM2", {0xC5, 0xF0, 0x59, 0xC2}, 4 },
        
        /* AES/SHA instructions */
        { "AESENC XMM0, XMM1", {0x66, 0x0F, 0x38, 0xDC, 0xC1}, 5 },
        { "SHA256RNDS2 XMM0, XMM1", {0x0F, 0x38, 0xCB, 0xC1}, 4 },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);
    
    printf("Decoder Performance Benchmark\\n");
    printf("==============================\\n\\n");
    printf("Iterations per test: %d\\n\\n", TEST_ITERATIONS);

    clock_t start_total = clock();
    uint64_t total_bytes_decoded = 0;

    for (int i = 0; i < num_tests; i++) {
        perf_test_t *t = &tests[i];
        
        clock_t start = clock();
        
        for (int j = 0; j < TEST_ITERATIONS; j++) {
            x86_insn_t insn;
            memset(&insn, 0, sizeof(insn));
            decode_x86_insn(t->bytes, &insn);
        }
        
        clock_t end = clock();
        double elapsed_ms = ((double)(end - start) * 1000.0) / CLOCKS_PER_SEC;
        double ops_per_sec = (TEST_ITERATIONS / elapsed_ms) * 1000.0;
        double ns_per_op = (elapsed_ms * 1000000.0) / TEST_ITERATIONS;
        
        total_bytes_decoded += (uint64_t)t->length * TEST_ITERATIONS;
        
        printf("[%2d] %-25s Length: %d bytes\\n", i+1, t->name, t->length);
        printf("     Time: %.2f ms | %.0f ops/sec | %.2f ns/op\\n\\n", 
               elapsed_ms, ops_per_sec, ns_per_op);
    }

    clock_t end_total = clock();
    double total_elapsed_ms = ((double)(end_total - start_total) * 1000.0) / CLOCKS_PER_SEC;
    double total_ops_per_sec = ((num_tests * TEST_ITERATIONS) / total_elapsed_ms) * 1000.0;
    double total_bytes_per_sec = (total_bytes_decoded / (total_elapsed_ms / 1000.0)) / (1024.0 * 1024.0);
    
    printf("==============================\\n");
    printf("Performance Summary:\\n");
    printf("  Total time: %.2f ms\\n", total_elapsed_ms);
    printf("  Total ops: %d\\n", num_tests * TEST_ITERATIONS);
    printf("  Avg throughput: %.0f ops/sec\\n", total_ops_per_sec);
    printf("  Avg throughput: %.2f MB/sec\\n", total_bytes_per_sec);
    printf("==============================\\n");

    return 0;
}
