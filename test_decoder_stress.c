/* Decoder stress test with large instruction sequences */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "rosetta_types.h"
#include "rosetta_x86_decode.h"

#define INSTRUCTION_COUNT 10000

typedef struct {
    const char *name;
    uint8_t bytes[16];
    int length;
} stress_insn_t;

/* Common instruction patterns for stress testing */
stress_insn_t instructions[] = {
    /* ALU operations */
    { "ADD RAX, RBX", {0x48, 0x01, 0xD8}, 3 },
    { "SUB RAX, RBX", {0x48, 0x29, 0xD8}, 3 },
    { "AND RAX, RBX", {0x48, 0x21, 0xD8}, 3 },
    { "OR RAX, RBX", {0x48, 0x09, 0xD8}, 3 },
    { "XOR RAX, RAX", {0x48, 0x31, 0xC0}, 3 },
    { "CMP RAX, RBX", {0x48, 0x39, 0xC3}, 3 },
    { "INC RAX", {0x48, 0xFF, 0xC0}, 3 },
    { "DEC RAX", {0x48, 0xFF, 0xC8}, 3 },
    { "SHL RAX, 1", {0x48, 0xD1, 0xE0}, 3 },
    { "SHR RAX, 1", {0x48, 0xD1, 0xE8}, 3 },
    
    /* Memory operations */
    { "MOV RAX, RBX", {0x48, 0x8B, 0xC3}, 3 },
    { "MOV RBX, RAX", {0x48, 0x8B, 0xD8}, 3 },
    { "LEA RAX, [RAX+RBX]", {0x48, 0x8D, 0x04, 0x18}, 4 },
    { "MOV RAX, [RIP+0x100]", {0x48, 0x8B, 0x05, 0x00, 0x01, 0x00, 0x00}, 7 },
    { "MOV RAX, [RBX+RCX*8]", {0x48, 0x8B, 0x04, 0xCB}, 4 },
    { "XCHG RAX, RBX", {0x48, 0x93}, 2 },
    
    /* Stack operations */
    { "PUSH RAX", {0x50}, 1 },
    { "PUSH RBX", {0x53}, 1 },
    { "PUSH RCX", {0x51}, 1 },
    { "PUSH RDX", {0x52}, 1 },
    { "POP RDX", {0x5A}, 1 },
    { "POP RCX", {0x59}, 1 },
    { "POP RBX", {0x5B}, 1 },
    { "POP RAX", {0x58}, 1 },
    
    /* Branch operations */
    { "CALL label", {0xE8, 0x00, 0x00, 0x00, 0x00}, 5 },
    { "RET", {0xC3}, 1 },
    { "JMP label", {0xE9, 0x00, 0x00, 0x00, 0x00}, 5 },
    { "JE label", {0x74, 0x00}, 2 },
    { "JNE label", {0x75, 0x00}, 2 },
    
    /* SIMD operations */
    { "MOVAPS XMM0, XMM1", {0x0F, 0x28, 0xC1}, 3 },
    { "ADDPS XMM0, XMM1", {0x0F, 0x58, 0xC1}, 3 },
    { "XORPS XMM0, XMM0", {0x0F, 0x57, 0xC0}, 3 },
    { "PXOR XMM0, XMM0", {0x66, 0x0F, 0xEF, 0xC0}, 4 },
    
    /* AVX operations */
    { "VADDPS XMM0, XMM1, XMM2", {0xC5, 0xF0, 0x58, 0xC2}, 4 },
    { "VMULPS XMM0, XMM1, XMM2", {0xC5, 0xF0, 0x59, 0xC2}, 4 },
    
    /* AES/SHA operations */
    { "AESENC XMM0, XMM1", {0x66, 0x0F, 0x38, 0xDC, 0xC1}, 5 },
    { "SHA256RNDS2 XMM0, XMM1", {0x0F, 0x38, 0xCB, 0xC1}, 4 },
    
    /* Special instructions */
    { "NOP", {0x90}, 1 },
    { "CPUID", {0x0F, 0xA2}, 2 },
    { "RDTSC", {0x0F, 0x31}, 2 },
    { "MFENCE", {0x0F, 0xAE, 0xF0}, 3 },
    { "LFENCE", {0x0F, 0xAE, 0xE8}, 3 },
    { "SFENCE", {0x0F, 0xAE, 0xF8}, 3 },
};

int num_instructions = sizeof(instructions) / sizeof(instructions[0]);

int main() {
    printf("Decoder Stress Test\\n");
    printf("==================\\n\\n");
    printf("Testing decoder with %d random instruction sequence...\\n\\n", INSTRUCTION_COUNT);
    
    srand(time(NULL));
    
    clock_t start = clock();
    
    int total_decoded = 0;
    int total_errors = 0;
    int total_bytes = 0;
    
    for (int i = 0; i < INSTRUCTION_COUNT; i++) {
        /* Pick a random instruction */
        int idx = rand() % num_instructions;
        stress_insn_t *insn_def = &instructions[idx];
        
        /* Decode the instruction */
        x86_insn_t insn;
        memset(&insn, 0, sizeof(insn));
        
        int decoded = decode_x86_insn(insn_def->bytes, &insn);
        
        /* Validate the decode */
        if (decoded != insn_def->length) {
            total_errors++;
            if (total_errors <= 10) {  /* Only print first 10 errors */
                printf("[ERROR %d] Instruction %d (%s) decoded as %d bytes, expected %d\\n",
                       total_errors, i, insn_def->name, decoded, insn_def->length);
            }
        } else {
            total_decoded++;
            total_bytes += decoded;
        }
    }
    
    clock_t end = clock();
    double elapsed_ms = ((double)(end - start) * 1000.0) / CLOCKS_PER_SEC;
    double ops_per_sec = (INSTRUCTION_COUNT / elapsed_ms) * 1000.0;
    
    printf("\\n==================\\n");
    printf("Stress Test Results:\\n");
    printf("  Total Instructions: %d\\n", INSTRUCTION_COUNT);
    printf("  Successfully Decoded: %d\\n", total_decoded);
    printf("  Errors: %d\\n", total_errors);
    printf("  Total Bytes: %d\\n", total_bytes);
    printf("  Time: %.2f ms\\n", elapsed_ms);
    printf("  Throughput: %.0f ops/sec\\n", ops_per_sec);
    printf("\\n");
    
    if (total_errors == 0) {
        printf("✅ STRESS TEST PASSED: All %d instructions decoded correctly!\\n", INSTRUCTION_COUNT);
        return 0;
    } else {
        printf("❌ STRESS TEST FAILED: %d errors detected\\n", total_errors);
        return 1;
    }
}
