/* ============================================================================
 * Rosetta Performance Profiler - Identifies Hot Paths for Optimization
 * ============================================================================
 *
 * Profiles code execution to identify performance bottlenecks
 * ============================================================================ */

#include "rosetta_x86_decode.h"
#include "rosetta_insn_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PROFILE_ITERATIONS 10000000

/* Profile different instruction categories */
typedef struct {
    const char *name;
    const uint8_t bytes[15];
    uint8_t len;
} insn_profile_t;

static insn_profile_t instructions[] = {
    {"NOP", {0x90}, 1},
    {"RET", {0xC3}, 1},
    {"PUSH RAX", {0x50}, 1},
    {"POP RAX", {0x58}, 1},
    {"MOV RAX,RCX", {0x48, 0x89, 0xC8}, 3},
    {"ADD RAX,RCX", {0x48, 0x01, 0xC8}, 3},
    {"SUB RAX,RCX", {0x48, 0x29, 0xC8}, 3},
    {"XOR RAX,RCX", {0x48, 0x31, 0xC8}, 3},
    {"CMP RAX,RCX", {0x48, 0x39, 0xC1}, 3},
    {"TEST RAX,RAX", {0x48, 0x85, 0xC0}, 3},
    {"JE +2", {0x74, 0x02}, 2},
    {"JNE +2", {0x75, 0x02}, 2},
    {"JMP +2", {0xEB, 0x02}, 2},
    {"CALL", {0xE8, 0x00, 0x00, 0x00, 0x00}, 5},
    {"MOV EAX,1", {0xB8, 0x01, 0x00, 0x00, 0x00}, 5},
    {"ADD RAX,1", {0x48, 0x83, 0xC0, 0x01}, 4},
    {"INC RAX", {0x48, 0xFF, 0xC0}, 3},
    {"DEC RAX", {0x48, 0xFF, 0xC8}, 3},
};

#define NUM_INSTRUCTIONS (sizeof(instructions) / sizeof(instructions[0]))

static int profile_instruction_decode(int iterations)
{
    printf("\n=== Profiling Instruction Decode (%d iterations) ===\n", iterations);

    /* Warmup */
    for (int i = 0; i < 10000; i++) {
        int idx = i % NUM_INSTRUCTIONS;
        x86_insn_t insn;
        decode_x86_insn(instructions[idx].bytes, &insn);
    }

    /* Profile each instruction type */
    for (int i = 0; i < NUM_INSTRUCTIONS; i++) {
        clock_t start = clock();

        for (int j = 0; j < iterations / NUM_INSTRUCTIONS; j++) {
            x86_insn_t insn;
            decode_x86_insn(instructions[i].bytes, &insn);
        }

        clock_t end = clock();
        double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
        double ops_per_sec = ((iterations / NUM_INSTRUCTIONS) / time_ms) * 1000.0;

        printf("%-15s: %10.2f M ops/sec (%8.3f ns/op)\n",
               instructions[i].name,
               ops_per_sec / 1000000.0,
               time_ms * 1000000.0 / (iterations / NUM_INSTRUCTIONS));
    }

    return 0;
}

static int profile_categorization_speed(int iterations)
{
    printf("\n=== Profiling Categorization Speed (%d iterations) ===\n", iterations);

    clock_t start = clock();
    int categorized = 0;

    for (int i = 0; i < iterations; i++) {
        int idx = i % NUM_INSTRUCTIONS;
        const uint8_t *p = instructions[idx].bytes;
        uint8_t first = p[0];
        uint8_t second = p[1];

        /* Fast categorization */
        int category = 0;  /* simple */
        if (first < 0x40) category = 0;
        else if ((first >= 0x50 && first <= 0x5F)) category = 0;
        else if (first >= 0xB8 && first <= 0xBF) category = 0;
        else if ((first & 0xF0) == 0x40) {
            if (second < 0x40) category = 0;
            else if ((second >= 0x50 && second <= 0x5F)) category = 0;
        }
        else if (first == 0x0F) category = 3;
        else category = 1;

        categorized++;
    }

    clock_t end = clock();
    double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    double ops_per_sec = (iterations / time_ms) * 1000.0;

    printf("Categorization: %10.2f M ops/sec (%8.3f ns/op)\n",
           ops_per_sec / 1000000.0,
           time_ms * 1000000.0 / iterations);

    return 0;
}

static int profile_memory_operations(int iterations)
{
    printf("\n=== Profiling Memory Operations (%d iterations) ===\n", iterations);

    /* Profile different memory operation patterns */
    struct {
        const char *name;
        const uint8_t bytes[15];
        uint8_t len;
    } mem_ops[] = {
        {"MOV [RAX],RBX", {0x48, 0x89, 0x18}, 3},
        {"MOV RBX,[RAX]", {0x48, 0x8B, 0x18}, 3},
        {"LEA RAX,[RBX+RCX*8]", {0x48, 0x8D, 0x04, 0xCB}, 4},
        {"MOV RAX,[RIP+disp32]", {0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00}, 7},
    };

    for (size_t i = 0; i < sizeof(mem_ops) / sizeof(mem_ops[0]); i++) {
        clock_t start = clock();

        for (int j = 0; j < iterations / 4; j++) {
            x86_insn_t insn;
            decode_x86_insn(mem_ops[i].bytes, &insn);
        }

        clock_t end = clock();
        double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
        double ops_per_sec = ((iterations / 4) / time_ms) * 1000.0;

        printf("%-20s: %10.2f M ops/sec (%8.3f ns/op)\n",
               mem_ops[i].name,
               ops_per_sec / 1000000.0,
               time_ms * 4000000.0 / (iterations / 4));
    }

    return 0;
}

static int profile_branch_instructions(int iterations)
{
    printf("\n=== Profiling Branch Instructions (%d iterations) ===\n", iterations);

    struct {
        const char *name;
        const uint8_t bytes[15];
        uint8_t len;
    } branches[] = {
        {"JE +2", {0x74, 0x02}, 2},
        {"JNE +2", {0x75, 0x02}, 2},
        {"JL +2", {0x7C, 0x02}, 2},
        {"JG +2", {0x7F, 0x02}, 2},
        {"JMP +2", {0xEB, 0x02}, 2},
        {"CALL", {0xE8, 0x00, 0x00, 0x00, 0x00}, 5},
        {"RET", {0xC3}, 1},
    };

    for (size_t i = 0; i < sizeof(branches) / sizeof(branches[0]); i++) {
        clock_t start = clock();

        for (int j = 0; j < iterations / 7; j++) {
            x86_insn_t insn;
            decode_x86_insn(branches[i].bytes, &insn);
        }

        clock_t end = clock();
        double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
        double ops_per_sec = ((iterations / 7) / time_ms) * 1000.0;

        printf("%-10s: %10.2f M ops/sec (%8.3f ns/op)\n",
               branches[i].name,
               ops_per_sec / 1000000.0,
               time_ms * 7000000.0 / (iterations / 7));
    }

    return 0;
}

int main(int argc, char **argv)
{
    printf("=================================================================\n");
    printf("Rosetta Performance Profiler - Hot Path Analysis\n");
    printf("=================================================================\n");

    printf("\nProfiling Configuration:");
    printf("\n   Iterations: %d", PROFILE_ITERATIONS);
    printf("\n   Instructions: %lu", NUM_INSTRUCTIONS);

    /* Run profiling */
    profile_instruction_decode(PROFILE_ITERATIONS);
    profile_categorization_speed(PROFILE_ITERATIONS * 10);
    profile_memory_operations(PROFILE_ITERATIONS / 4);
    profile_branch_instructions(PROFILE_ITERATIONS / 7);

    printf("\n=================================================================\n");
    printf("Profiling Complete\n");
    printf("=================================================================\n");

    return 0;
}
