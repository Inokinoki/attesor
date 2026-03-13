/* ============================================================================
 * Rosetta Cached Decode Integration Test
 * ============================================================================
 *
 * Tests the performance improvement from using cached instruction decoding
 * in realistic scenarios with repeated instruction patterns.
 * ============================================================================ */

#include "rosetta_x86_decode.h"
#include "rosetta_insn_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TEST_ITERATIONS 1000000
#define WARMUP_ITERATIONS 100000

/* Test instruction patterns that simulate real code */
static const struct {
    uint8_t bytes[15];
    uint8_t len;
    const char *description;
} test_patterns[] = {
    {{0x90}, 1, "NOP"},
    {{0x50}, 1, "PUSH RAX"},
    {{0x58}, 1, "POP RAX"},
    {{0x48, 0x89, 0xC8}, 3, "MOV RAX, RCX"},
    {{0x48, 0x01, 0xC8}, 3, "ADD RAX, RCX"},
    {{0x48, 0x29, 0xC8}, 3, "SUB RAX, RCX"},
    {{0x48, 0x31, 0xC8}, 3, "XOR RAX, RCX"},
    {{0x48, 0x85, 0xC0}, 3, "TEST RAX, RAX"},
    {{0x75, 0x02}, 2, "JNE +2"},
    {{0x74, 0x02}, 2, "JE +2"},
    {{0xEB, 0x02}, 2, "JMP +2"},
    {{0xC3}, 1, "RET"},
    {{0xB8, 0x01, 0x00, 0x00, 0x00}, 5, "MOV EAX, 1"},
    {{0x48, 0x83, 0xC0, 0x01}, 4, "ADD RAX, 1"},
    {{0x48, 0x83, 0xE8, 0x01}, 4, "SUB RAX, 1"},
    {{0x48, 0xFF, 0xC0}, 3, "INC RAX"},
    {{0x48, 0xFF, 0xC8}, 3, "DEC RAX"},
    {{0x48, 0x39, 0xC1}, 3, "CMP RAX, RCX"},
    {{0x48, 0x87, 0xC8}, 3, "XCHG RAX, RCX"},
    {{0x48, 0x85, 0xD2}, 3, "TEST RDX, RDX"},
};

#define NUM_PATTERNS (sizeof(test_patterns) / sizeof(test_patterns[0]))

static int benchmark_uncached_decode(int iterations)
{
    printf("\n1. Benchmarking UNCACHED decode (%d iterations):\n", iterations);

    /* Warmup */
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        int idx = i % NUM_PATTERNS;
        x86_insn_t insn;
        decode_x86_insn(test_patterns[idx].bytes, &insn);
    }

    /* Benchmark */
    clock_t start = clock();
    int total_decoded = 0;

    for (int i = 0; i < iterations; i++) {
        int idx = i % NUM_PATTERNS;
        x86_insn_t insn;
        int len = decode_x86_insn(test_patterns[idx].bytes, &insn);
        if (len > 0) {
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

static int benchmark_cached_decode(int iterations)
{
    printf("\n2. Benchmarking CACHED decode (%d iterations):\n", iterations);

    /* Initialize and enable cache */
    insn_cache_init();
    insn_cache_set_enabled(1);

    /* Warmup - populate cache */
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        int idx = i % NUM_PATTERNS;
        x86_insn_t insn;
        decode_x86_insn_cached(test_patterns[idx].bytes, &insn);
    }

    /* Reset statistics */
    insn_cache_flush();

    /* Benchmark */
    clock_t start = clock();
    int total_decoded = 0;

    for (int i = 0; i < iterations; i++) {
        int idx = i % NUM_PATTERNS;
        x86_insn_t insn;
        int len = decode_x86_insn_cached(test_patterns[idx].bytes, &insn);
        if (len > 0) {
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

static int benchmark_mixed_workload(int iterations)
{
    printf("\n3. Benchmarking MIXED workload (%d iterations):\n", iterations);

    /* Initialize cache */
    insn_cache_init();
    insn_cache_set_enabled(1);

    /* Mixed workload: 80% cached patterns, 20% new instructions */
    clock_t start = clock();
    int total_decoded = 0;
    int cache_hits = 0;

    for (int i = 0; i < iterations; i++) {
        x86_insn_t insn;
        const uint8_t *bytes;

        if (i % 5 < 4) {
            /* 80% - Use cached patterns */
            int idx = i % NUM_PATTERNS;
            bytes = test_patterns[idx].bytes;

            /* Check if it's in cache */
            if (insn_cache_lookup((u64)bytes, &insn)) {
                cache_hits++;
                total_decoded++;
                continue;
            }

            int len = decode_x86_insn(bytes, &insn);
            if (len > 0) {
                insn_cache_insert((u64)bytes, &insn);
                total_decoded++;
            }
        } else {
            /* 20% - Simulate new instructions (different addresses) */
            uint8_t temp_insn[15] = {0x90, 0x90, 0x90};  /* Triple NOP */
            bytes = temp_insn + (i % 3);  /* Varying addresses */

            int len = decode_x86_insn(bytes, &insn);
            if (len > 0) {
                insn_cache_insert((u64)bytes, &insn);
                total_decoded++;
            }
        }
    }

    clock_t end = clock();
    double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    double ops_per_sec = (iterations / time_ms) * 1000.0;

    printf("   Total operations: %d\n", iterations);
    printf("   Cache hits: %d\n", cache_hits);
    printf("   Successful decodes: %d\n", total_decoded);
    printf("   Time: %.2f ms\n", time_ms);
    printf("   Performance: %.2f M ops/sec\n", ops_per_sec / 1000000.0);
    printf("   Cache hit rate: %.1f%%\n", (100.0 * cache_hits) / iterations);

    return 0;
}

static int benchmark_loop_simulation(int iterations)
{
    printf("\n4. Benchmarking LOOP simulation (%d iterations):\n", iterations);

    /* Initialize cache */
    insn_cache_init();
    insn_cache_set_enabled(1);

    /* Simulate a tight loop with repeated instructions */
    clock_t start = clock();
    int total_decoded = 0;

    /* Loop body instructions */
    const uint8_t loop_body[][15] = {
        {0x48, 0x83, 0xC0, 0x01},  /* ADD RAX, 1 */
        {0x48, 0x39, 0xC1},         /* CMP RAX, RCX */
        {0x72, 0xF8},               /* JB -8 */
        {0xC3},                     /* RET */
    };

    int loop_iterations = 100;
    for (int i = 0; i < iterations; i++) {
        for (int j = 0; j < loop_iterations; j++) {
            int idx = j % 4;
            const uint8_t *bytes = loop_body[idx];

            x86_insn_t insn;
            int len = decode_x86_insn_cached(bytes, &insn);
            if (len > 0) {
                total_decoded++;
            }
        }
    }

    clock_t end = clock();
    double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    double ops_per_sec = ((iterations * loop_iterations) / time_ms) * 1000.0;

    printf("   Total operations: %d\n", iterations * loop_iterations);
    printf("   Successful decodes: %d\n", total_decoded);
    printf("   Time: %.2f ms\n", time_ms);
    printf("   Performance: %.2f M ops/sec\n", ops_per_sec / 1000000.0);

    /* Print cache statistics */
    insn_cache_stats_t stats;
    insn_cache_get_stats(&stats);

    printf("\n   Cache Statistics:\n");
    printf("   Lookups: %lu\n", stats.lookups);
    printf("   Hits: %lu\n", stats.hits);
    printf("   Misses: %lu\n", stats.misses);
    if (stats.lookups > 0) {
        printf("   Hit rate: %.1f%%\n", (100.0 * stats.hits) / stats.lookups);
    }

    return 0;
}

int main(int argc, char **argv)
{
    printf("=================================================================\n");
    printf("Rosetta Cached Decode Integration Test\n");
    printf("=================================================================\n");

    printf("\nTest Configuration:");
    printf("\n   Test iterations: %d", TEST_ITERATIONS);
    printf("\n   Warmup iterations: %d", WARMUP_ITERATIONS);
    printf("\n   Test patterns: %lu", NUM_PATTERNS);
    printf("\n   Cache size: %u entries", INSN_CACHE_SIZE);

    /* Run benchmarks */
    benchmark_uncached_decode(TEST_ITERATIONS);
    benchmark_cached_decode(TEST_ITERATIONS);
    benchmark_mixed_workload(TEST_ITERATIONS);
    benchmark_loop_simulation(TEST_ITERATIONS / 100);  /* Fewer iterations */

    /* Cleanup */
    insn_cache_cleanup();

    printf("\n=================================================================\n");
    printf("Integration Test Complete\n");
    printf("=================================================================\n");

    return 0;
}
