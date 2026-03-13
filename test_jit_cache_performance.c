/* ============================================================================
 * Rosetta 2 JIT Cache Performance Test
 * ============================================================================
 *
 * Tests the performance improvement from JIT code caching
 * ============================================================================ */

#include "rosetta_jit.h"
#include "rosetta_x86_decode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TEST_ITERATIONS 1000000
#define NUM_TEST_BLOCKS 100

/* Test x86_64 instructions */
static const uint8_t test_instructions[][15] = {
    {0x90},                         /* NOP */
    {0x50},                         /* PUSH RAX */
    {0x58},                         /* POP RAX */
    {0x48, 0x89, 0xC8},             /* MOV RAX, RCX */
    {0x48, 0x01, 0xC8},             /* ADD RAX, RCX */
    {0x48, 0x29, 0xC8},             /* SUB RAX, RCX */
    {0x48, 0x31, 0xC8},             /* XOR RAX, RCX */
    {0x48, 0x85, 0xC0},             /* TEST RAX, RAX */
    {0x75, 0x02},                   /* JNE +2 */
    {0xC3},                         /* RET */
};

static int benchmark_cache_lookup(int iterations)
{
    printf("\n1. Testing JIT cache lookup performance (%d iterations):\n", iterations);

    jit_context_t *ctx;
    jit_context_t local_ctx;

    /* Use local context */
    ctx = &local_ctx;
    if (jit_init(ctx, CODE_CACHE_DEFAULT_SIZE) != 0) {
        printf("   ERROR: Failed to initialize JIT\n");
        return -1;
    }

    /* Insert some test blocks */
    for (int i = 0; i < NUM_TEST_BLOCKS; i++) {
        u64 guest_pc = 0x1000 + (i * 0x10);
        TranslationCacheEntry *entry = &ctx->cache[i];
        entry->guest_addr = guest_pc;
        entry->host_addr = 0x10000 + (i * 0x100);
        entry->block_size = 0x100;
        entry->flags = BLOCK_FLAG_VALID;
        entry->hash = hash_address(guest_pc);
    }

    /* Benchmark cache lookups */
    clock_t start = clock();
    u64 total_hits = 0;

    for (int i = 0; i < iterations; i++) {
        u64 guest_pc = 0x1000 + ((i % NUM_TEST_BLOCKS) * 0x10);
        void *host_addr = translation_lookup(ctx, guest_pc);
        if (host_addr != NULL) {
            total_hits++;
        }
    }

    clock_t end = clock();
    double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    double ops_per_sec = (iterations / time_ms) * 1000.0;

    printf("   Total lookups: %d\n", iterations);
    printf("   Cache hits: %lu\n", total_hits);
    printf("   Time: %.2f ms\n", time_ms);
    printf("   Performance: %.2f M lookups/sec\n", ops_per_sec / 1000000.0);

    jit_cleanup(ctx);

    return 0;
}

static int benchmark_cache_insert(int iterations)
{
    printf("\n2. Testing JIT cache insert performance (%d iterations):\n", iterations);

    jit_context_t ctx;
    if (jit_init(&ctx, CODE_CACHE_DEFAULT_SIZE) != 0) {
        printf("   ERROR: Failed to initialize JIT\n");
        return -1;
    }

    /* Benchmark cache inserts */
    clock_t start = clock();

    for (int i = 0; i < iterations; i++) {
        u64 guest_pc = 0x1000 + ((i % NUM_TEST_BLOCKS) * 0x10);
        TranslationCacheEntry *entry = &ctx.cache[i % NUM_TEST_BLOCKS];
        entry->guest_addr = guest_pc;
        entry->host_addr = 0x10000 + (i * 0x100);
        entry->block_size = 0x100;
        entry->flags = BLOCK_FLAG_VALID;
        entry->hash = hash_address(guest_pc);
    }

    clock_t end = clock();
    double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    double ops_per_sec = (iterations / time_ms) * 1000.0;

    printf("   Total inserts: %d\n", iterations);
    printf("   Time: %.2f ms\n", time_ms);
    printf("   Performance: %.2f M inserts/sec\n", ops_per_sec / 1000000.0);

    jit_cleanup(&ctx);

    return 0;
}

static int benchmark_hash_function(int iterations)
{
    printf("\n3. Testing hash function performance (%d iterations):\n", iterations);

    clock_t start = clock();

    for (int i = 0; i < iterations; i++) {
        u64 addr = 0x1000 + (i * 0x10);
        u32 hash = hash_address(addr);
        (void)hash;  /* Prevent optimization */
    }

    clock_t end = clock();
    double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    double ops_per_sec = (iterations / time_ms) * 1000.0;

    printf("   Total hashes: %d\n", iterations);
    printf("   Time: %.2f ms\n", time_ms);
    printf("   Performance: %.2f M hashes/sec\n", ops_per_sec / 1000000.0);

    return 0;
}

static int benchmark_combined_operations(int iterations)
{
    printf("\n4. Testing combined cache operations (%d iterations):\n", iterations);

    jit_context_t ctx;
    if (jit_init(&ctx, CODE_CACHE_DEFAULT_SIZE) != 0) {
        printf("   ERROR: Failed to initialize JIT\n");
        return -1;
    }

    /* Mix of lookups, inserts, and hash operations */
    clock_t start = clock();
    u64 total_hits = 0;

    for (int i = 0; i < iterations; i++) {
        u64 guest_pc = 0x1000 + ((i % NUM_TEST_BLOCKS) * 0x10);
        u32 hash = hash_address(guest_pc);

        /* Lookup */
        void *host_addr = translation_lookup(&ctx, guest_pc);
        if (!host_addr) {
            /* Insert */
            TranslationCacheEntry *entry = &ctx.cache[i % NUM_TEST_BLOCKS];
            entry->guest_addr = guest_pc;
            entry->host_addr = 0x10000 + (i * 0x100);
            entry->block_size = 0x100;
            entry->flags = BLOCK_FLAG_VALID;
            entry->hash = hash;
        } else {
            total_hits++;
        }
    }

    clock_t end = clock();
    double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    double ops_per_sec = (iterations / time_ms) * 1000.0;

    printf("   Total operations: %d\n", iterations);
    printf("   Cache hits: %lu\n", total_hits);
    printf("   Time: %.2f ms\n", time_ms);
    printf("   Performance: %.2f M ops/sec\n", ops_per_sec / 1000000.0);

    jit_cleanup(&ctx);

    return 0;
}

int main(int argc, char **argv)
{
    printf("=================================================================\n");
    printf("Rosetta 2 JIT Cache Performance Test\n");
    printf("=================================================================\n");

    printf("\nJIT Cache Configuration:\n");
    printf("   Cache size: %u entries\n", TRANSLATION_CACHE_SIZE);
    printf("   Code cache size: %u bytes\n", CODE_CACHE_DEFAULT_SIZE);

    /* Run benchmarks */
    benchmark_hash_function(TEST_ITERATIONS);
    benchmark_cache_lookup(TEST_ITERATIONS);
    benchmark_cache_insert(TEST_ITERATIONS);
    benchmark_combined_operations(TEST_ITERATIONS);

    printf("\n=================================================================\n");
    return 0;
}
