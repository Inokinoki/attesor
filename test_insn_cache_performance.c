/* ============================================================================
 * Rosetta Instruction Cache Performance Test
 * ============================================================================
 *
 * Tests the performance improvement from instruction decode caching
 * ============================================================================ */

#include "rosetta_insn_cache.h"
#include "rosetta_x86_decode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TEST_ITERATIONS 1000000
#define NUM_TEST_INSN 100

/* Test x86_64 instructions */
static const struct {
    const uint8_t bytes[15];
    size_t len;
} test_instructions[NUM_TEST_INSN] = {
    {{0x90}, 1},                         /* NOP */
    {{0x50}, 1},                         /* PUSH RAX */
    {{0x58}, 1},                         /* POP RAX */
    {{0x48, 0x89, 0xC8}, 3},            /* MOV RAX, RCX */
    {{0x48, 0x01, 0xC8}, 3},            /* ADD RAX, RCX */
    {{0x48, 0x29, 0xC8}, 3},            /* SUB RAX, RCX */
    {{0x48, 0x31, 0xC8}, 3},            /* XOR RAX, RCX */
    {{0x48, 0x85, 0xC0}, 3},            /* TEST RAX, RAX */
    {{0x75, 0x02}, 2},                   /* JNE +2 */
    {{0xC3}, 1},                         /* RET */
    {{0xB8, 0x01, 0x00, 0x00, 0x00}, 5}, /* MOV EAX, 1 */
    {{0x48, 0x83, 0xC0, 0x01}, 4},      /* ADD RAX, 1 */
    {{0x48, 0x83, 0xE8, 0x01}, 4},      /* SUB RAX, 1 */
    {{0x48, 0xFF, 0xC0}, 3},            /* INC RAX */
    {{0x48, 0xFF, 0xC8}, 3},            /* DEC RAX */
    {{0x48, 0x39, 0xC1}, 3},            /* CMP RAX, RCX */
    {{0x74, 0x02}, 2},                   /* JE +2 */
    {{0xEB, 0x02}, 2},                   /* JMP +2 */
    {{0x48, 0x87, 0xC8}, 3},            /* XCHG RAX, RCX */
    {{0x48, 0x85, 0xD2}, 3},            /* TEST RDX, RDX */
};

static int benchmark_decode_without_cache(int iterations)
{
    printf("\n1. Benchmarking decode WITHOUT cache (%d iterations):\n", iterations);

    clock_t start = clock();
    int total_decoded = 0;

    for (int i = 0; i < iterations; i++) {
        int insn_idx = i % NUM_TEST_INSN;
        const uint8_t *bytes = test_instructions[insn_idx].bytes;
        size_t len = test_instructions[insn_idx].len;

        x86_insn_t insn;
        int decoded = decode_x86_insn(bytes, &insn);
        if (decoded > 0) {
            total_decoded++;
        }
    }

    clock_t end = clock();
    double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    double ops_per_sec = (iterations / time_ms) * 1000.0;

    printf("   Total decodes: %d\n", iterations);
    printf("   Successful: %d\n", total_decoded);
    printf("   Time: %.2f ms\n", time_ms);
    printf("   Performance: %.2f M ops/sec\n", ops_per_sec / 1000000.0);

    return 0;
}

static int benchmark_decode_with_cache(int iterations)
{
    printf("\n2. Benchmarking decode WITH cache (%d iterations):\n", iterations);

    /* Initialize and enable cache */
    insn_cache_init();
    insn_cache_set_enabled(1);

    /* Populate cache */
    for (int i = 0; i < NUM_TEST_INSN; i++) {
        const uint8_t *bytes = test_instructions[i].bytes;
        x86_insn_t insn;
        decode_x86_insn(bytes, &insn);
        insn_cache_insert((u64)bytes, &insn);
    }

    clock_t start = clock();
    int total_decoded = 0;

    for (int i = 0; i < iterations; i++) {
        int insn_idx = i % NUM_TEST_INSN;
        const uint8_t *bytes = test_instructions[insn_idx].bytes;

        x86_insn_t insn;

        /* Try cache first */
        if (insn_cache_lookup((u64)bytes, &insn)) {
            total_decoded++;
        } else {
            /* Cache miss - decode and insert */
            int decoded = decode_x86_insn(bytes, &insn);
            if (decoded > 0) {
                insn_cache_insert((u64)bytes, &insn);
                total_decoded++;
            }
        }
    }

    clock_t end = clock();
    double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    double ops_per_sec = (iterations / time_ms) * 1000.0;

    printf("   Total lookups: %d\n", iterations);
    printf("   Successful: %d\n", total_decoded);
    printf("   Time: %.2f ms\n", time_ms);
    printf("   Performance: %.2f M ops/sec\n", ops_per_sec / 1000000.0);

    /* Print cache statistics */
    insn_cache_stats_t stats;
    insn_cache_get_stats(&stats);

    printf("\n   Cache Statistics:\n");
    printf("   Lookups: %lu\n", stats.lookups);
    printf("   Hits: %lu\n", stats.hits);
    printf("   Misses: %lu\n", stats.misses);
    printf("   Hit rate: %.1f%%\n", insn_cache_hit_rate());

    return 0;
}

static int benchmark_cache_operations(int iterations)
{
    printf("\n3. Benchmarking cache operations (%d iterations):\n", iterations);

    insn_cache_init();

    x86_insn_t test_insn;
    memset(&test_insn, 0, sizeof(test_insn));

    /* Benchmark inserts */
    clock_t start = clock();
    for (int i = 0; i < iterations; i++) {
        u64 guest_pc = 0x1000 + (i * 0x10);
        insn_cache_insert(guest_pc, &test_insn);
    }
    clock_t end = clock();
    double insert_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    double insert_ops = (iterations / insert_time) * 1000.0;

    printf("   Inserts: %d in %.2f ms (%.2f M ops/sec)\n",
           iterations, insert_time, insert_ops / 1000000.0);

    /* Benchmark lookups */
    start = clock();
    int hits = 0;
    for (int i = 0; i < iterations; i++) {
        u64 guest_pc = 0x1000 + (i * 0x10);
        x86_insn_t insn;
        if (insn_cache_lookup(guest_pc, &insn)) {
            hits++;
        }
    }
    end = clock();
    double lookup_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    double lookup_ops = (iterations / lookup_time) * 1000.0;

    printf("   Lookups: %d in %.2f ms (%.2f M ops/sec, %d hits)\n",
           iterations, lookup_time, lookup_ops / 1000000.0, hits);

    return 0;
}

int main(int argc, char **argv)
{
    printf("=================================================================\n");
    printf("Rosetta Instruction Cache Performance Test\n");
    printf("=================================================================\n");

    printf("\nCache Configuration:\n");
    printf("   Cache size: %u entries\n", INSN_CACHE_SIZE);
    printf("   Test iterations: %d\n", TEST_ITERATIONS);
    printf("   Test instructions: %d\n", NUM_TEST_INSN);

    /* Run benchmarks */
    benchmark_decode_without_cache(TEST_ITERATIONS);
    benchmark_decode_with_cache(TEST_ITERATIONS);
    benchmark_cache_operations(TEST_ITERATIONS);

    /* Cleanup */
    insn_cache_cleanup();

    printf("\n=================================================================\n");
    return 0;
}
