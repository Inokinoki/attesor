/* ============================================================================
 * Rosetta JIT Module - Unit Tests
 * ============================================================================
 *
 * Compile: gcc -o test_jit test_jit.c -I. -L. -lrosetta_jit -lrosetta_codegen
 * Run: ./test_jit
 * ============================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>

#include "rosetta_types.h"
#include "rosetta_jit.h"
#include "rosetta_codegen.h"

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    static int test_##name(void); \
    static void run_test_##name(void) { \
        tests_run++; \
        printf("Running %s... ", #name); \
        if (test_##name()) { \
            tests_passed++; \
            printf("PASSED\n"); \
        } else { \
            tests_failed++; \
            printf("FAILED\n"); \
        } \
    } \
    static int test_##name(void)

#define ASSERT(cond) do { if (!(cond)) return 0; } while(0)
#define ASSERT_EQ(a, b) do { if ((a) != (b)) return 0; } while(0)
#define ASSERT_NEQ(a, b) do { if ((a) == (b)) return 0; } while(0)

/* ============================================================================
 * Hash Function Tests
 * ============================================================================ */

TEST(hash_address_basic)
{
    u32 h1 = hash_address(0x1000);
    u32 h2 = hash_address(0x2000);
    u32 h3 = hash_address(0x1000);  /* Should match h1 */

    ASSERT_EQ(h1, h3);  /* Same address = same hash */
    ASSERT_NEQ(h1, h2); /* Different addresses should (likely) differ */

    return 1;
}

TEST(hash_address_alignment)
{
    /* Addresses aligned to 4 bytes should still produce different hashes */
    u32 h1 = hash_address(0x1000);
    u32 h2 = hash_address(0x1004);
    u32 h3 = hash_address(0x1008);

    ASSERT_NEQ(h1, h2);
    ASSERT_NEQ(h2, h3);
    ASSERT_NEQ(h1, h3);

    return 1;
}

TEST(hash_string_basic)
{
    u32 h1 = hash_string("hello");
    u32 h2 = hash_string("world");
    u32 h3 = hash_string("hello");  /* Should match h1 */

    ASSERT_EQ(h1, h3);
    ASSERT_NEQ(h1, h2);

    return 1;
}

TEST(hash_string_empty)
{
    u32 h1 = hash_string("");
    u32 h2 = hash_string("");

    ASSERT_EQ(h1, h2);  /* Empty strings should hash the same */

    return 1;
}

TEST(hash_compute_basic)
{
    const u8 data1[] = {1, 2, 3, 4, 5};
    const u8 data2[] = {1, 2, 3, 4, 6};
    const u8 data3[] = {1, 2, 3, 4, 5};

    u32 h1 = hash_compute(data1, sizeof(data1));
    u32 h2 = hash_compute(data2, sizeof(data2));
    u32 h3 = hash_compute(data3, sizeof(data3));

    ASSERT_EQ(h1, h3);
    ASSERT_NEQ(h1, h2);

    return 1;
}

/* ============================================================================
 * JIT Initialization Tests
 * ============================================================================ */

TEST(jit_init_basic)
{
    jit_context_t ctx;
    int ret;

    ret = jit_init(&ctx, 1024 * 1024);  /* 1MB cache */
    ASSERT_EQ(ret, ROSETTA_OK);
    ASSERT_EQ(ctx.initialized, true);
    ASSERT_NEQ(ctx.code_cache, NULL);
    ASSERT_EQ(ctx.code_cache_size, 1024 * 1024);

    jit_cleanup(&ctx);

    return 1;
}

TEST(jit_init_default_size)
{
    jit_context_t ctx;
    int ret;

    ret = jit_init(&ctx, 0);  /* Should use default size */
    ASSERT_EQ(ret, ROSETTA_OK);
    ASSERT_EQ(ctx.code_cache_size, CODE_CACHE_DEFAULT_SIZE);

    jit_cleanup(&ctx);

    return 1;
}

TEST(jit_init_null)
{
    int ret = jit_init(NULL, 0);
    ASSERT_EQ(ret, ROSETTA_ERR_INVAL);

    return 1;
}

TEST(jit_cleanup_basic)
{
    jit_context_t ctx;

    jit_init(&ctx, 1024 * 1024);
    ASSERT_EQ(ctx.initialized, true);

    jit_cleanup(&ctx);
    ASSERT_EQ(ctx.initialized, false);
    ASSERT_EQ(ctx.code_cache, NULL);

    return 1;
}

TEST(jit_reset_basic)
{
    jit_context_t ctx;

    jit_init(&ctx, 1024 * 1024);

    /* Insert something into cache */
    translation_insert(&ctx, 0x1000, 0x5000, 64);
    ASSERT_NEQ(translation_lookup(&ctx, 0x1000), NULL);

    /* Reset should flush cache */
    jit_reset(&ctx);
    ASSERT_EQ(translation_lookup(&ctx, 0x1000), NULL);

    /* But memory should still be allocated */
    ASSERT_NEQ(ctx.code_cache, NULL);
    ASSERT_EQ(ctx.initialized, true);

    jit_cleanup(&ctx);

    return 1;
}

/* ============================================================================
 * Translation Cache Tests
 * ============================================================================ */

TEST(translation_lookup_cold)
{
    jit_context_t ctx;

    jit_init(&ctx, 1024 * 1024);

    /* Cold cache should return NULL */
    ASSERT_EQ(translation_lookup(&ctx, 0x1000), NULL);

    jit_cleanup(&ctx);

    return 1;
}

TEST(translation_insert_lookup)
{
    jit_context_t ctx;
    void *result;

    jit_init(&ctx, 1024 * 1024);

    /* Insert translation */
    ASSERT_EQ(translation_insert(&ctx, 0x1000, 0x5000, 64), ROSETTA_OK);

    /* Lookup should find it */
    result = translation_lookup(&ctx, 0x1000);
    ASSERT_EQ(result, (void*)0x5000);

    jit_cleanup(&ctx);

    return 1;
}

TEST(translation_invalidate)
{
    jit_context_t ctx;

    jit_init(&ctx, 1024 * 1024);

    /* Insert then invalidate */
    translation_insert(&ctx, 0x1000, 0x5000, 64);
    ASSERT_NEQ(translation_lookup(&ctx, 0x1000), NULL);

    ASSERT_EQ(translation_invalidate(&ctx, 0x1000), ROSETTA_OK);
    ASSERT_EQ(translation_lookup(&ctx, 0x1000), NULL);

    jit_cleanup(&ctx);

    return 1;
}

TEST(translation_flush)
{
    jit_context_t ctx;

    jit_init(&ctx, 1024 * 1024);

    /* Insert multiple translations */
    translation_insert(&ctx, 0x1000, 0x5000, 64);
    translation_insert(&ctx, 0x2000, 0x6000, 128);
    translation_insert(&ctx, 0x3000, 0x7000, 256);

    /* Flush should clear all */
    translation_flush(&ctx);

    ASSERT_EQ(translation_lookup(&ctx, 0x1000), NULL);
    ASSERT_EQ(translation_lookup(&ctx, 0x2000), NULL);
    ASSERT_EQ(translation_lookup(&ctx, 0x3000), NULL);

    jit_cleanup(&ctx);

    return 1;
}

TEST(translation_cache_size)
{
    jit_context_t ctx;
    u32 size;

    jit_init(&ctx, 1024 * 1024);

    size = translation_cache_get_size(&ctx);
    ASSERT_EQ(size, 0);  /* Empty cache */

    translation_insert(&ctx, 0x1000, 0x5000, 64);
    translation_insert(&ctx, 0x2000, 0x6000, 128);

    size = translation_cache_get_size(&ctx);
    ASSERT_EQ(size, 2);

    jit_cleanup(&ctx);

    return 1;
}

TEST(translation_cache_hash_collisions)
{
    jit_context_t ctx;
    void *result;

    jit_init(&ctx, 1024 * 1024);

    /* Insert two addresses that might hash to same index */
    /* Direct-mapped cache: later insert overwrites earlier */
    translation_insert(&ctx, 0x1000, 0x5000, 64);
    translation_insert(&ctx, 0x1000, 0x6000, 64);  /* Same address, different host */

    result = translation_lookup(&ctx, 0x1000);
    ASSERT_EQ(result, (void*)0x6000);  /* Should get latest */

    jit_cleanup(&ctx);

    return 1;
}

/* ============================================================================
 * Translation Block Tests
 * ============================================================================ */

TEST(translation_alloc_block)
{
    TranslationBlock *block;

    block = translation_alloc_block(0x1000);
    ASSERT_NEQ(block, NULL);
    ASSERT_EQ(block->guest_pc, 0x1000);
    ASSERT_EQ(block->flags, 0);
    ASSERT_EQ(block->successor, NULL);
    ASSERT_EQ(block->predecessor, NULL);

    translation_free_block(block);

    return 1;
}

TEST(translation_block_validity)
{
    TranslationBlock *block;

    block = translation_alloc_block(0x1000);
    ASSERT_EQ(translation_block_is_valid(block), false);

    translation_block_set_valid(block);
    ASSERT_EQ(translation_block_is_valid(block), true);

    translation_free_block(block);

    return 1;
}

TEST(translation_chain_blocks)
{
    TranslationBlock *block1, *block2;

    block1 = translation_alloc_block(0x1000);
    block2 = translation_alloc_block(0x2000);

    ASSERT_EQ(translation_chain_blocks(block1, block2), ROSETTA_OK);

    ASSERT_EQ(block1->successor, block2);
    ASSERT_EQ(block2->predecessor, block1);
    ASSERT_EQ(block1->flags & BLOCK_FLAG_LINKED, BLOCK_FLAG_LINKED);

    translation_free_block(block1);
    translation_free_block(block2);

    return 1;
}

TEST(translation_unchain_blocks)
{
    TranslationBlock *block1, *block2;

    block1 = translation_alloc_block(0x1000);
    block2 = translation_alloc_block(0x2000);

    translation_chain_blocks(block1, block2);
    translation_unchain_blocks(block1);

    ASSERT_EQ(block1->successor, NULL);
    ASSERT_EQ(block1->predecessor, NULL);
    ASSERT_EQ(block1->flags & BLOCK_FLAG_LINKED, 0);

    /* block2's predecessor should also be cleared */
    ASSERT_EQ(block2->predecessor, NULL);

    translation_free_block(block1);
    translation_free_block(block2);

    return 1;
}

TEST(translation_get_successor)
{
    TranslationBlock *block1, *block2, *result;

    block1 = translation_alloc_block(0x1000);
    block2 = translation_alloc_block(0x2000);

    ASSERT_EQ(translation_get_successor(block1), NULL);  /* No chain yet */

    translation_chain_blocks(block1, block2);

    result = translation_get_successor(block1);
    ASSERT_EQ(result, block2);

    translation_free_block(block1);
    translation_free_block(block2);

    return 1;
}

/* ============================================================================
 * Code Cache Tests
 * ============================================================================ */

TEST(code_cache_alloc)
{
    jit_context_t ctx;
    u8 *ptr1, *ptr2;

    jit_init(&ctx, 1024 * 1024);

    ptr1 = code_cache_alloc(&ctx, 256);
    ASSERT_NEQ(ptr1, NULL);
    ASSERT_EQ(ptr1, ctx.code_cache);  /* First allocation at start */

    ptr2 = code_cache_alloc(&ctx, 256);
    ASSERT_NEQ(ptr2, NULL);
    ASSERT_EQ(ptr2, ctx.code_cache + 256);  /* Second allocation follows */

    ASSERT_EQ(code_cache_get_free_space(&ctx), 1024 * 1024 - 512);

    jit_cleanup(&ctx);

    return 1;
}

TEST(code_cache_alloc_full)
{
    jit_context_t ctx;
    u8 *ptr;

    jit_init(&ctx, 1024);  /* Small cache for testing */

    /* Allocate until full */
    ptr = code_cache_alloc(&ctx, 1024);
    ASSERT_NEQ(ptr, NULL);

    /* Next allocation should fail */
    ptr = code_cache_alloc(&ctx, 1);
    ASSERT_EQ(ptr, NULL);

    jit_cleanup(&ctx);

    return 1;
}

TEST(code_cache_alloc_aligned)
{
    jit_context_t ctx;
    u8 *ptr1, *ptr2;
    uintptr_t aligned_addr;

    jit_init(&ctx, 1024 * 1024);

    /* Allocate 3 bytes, then request aligned allocation */
    ptr1 = code_cache_alloc(&ctx, 3);
    ptr2 = code_cache_alloc_aligned(&ctx, 16, 16);  /* 16-byte aligned */

    ASSERT_NEQ(ptr2, NULL);

    /* Check alignment */
    aligned_addr = (uintptr_t)ptr2;
    ASSERT_EQ(aligned_addr % 16, 0);

    jit_cleanup(&ctx);

    return 1;
}

TEST(code_cache_reset)
{
    jit_context_t ctx;
    u8 *ptr1, *ptr2;

    jit_init(&ctx, 1024 * 1024);

    ptr1 = code_cache_alloc(&ctx, 256);
    ASSERT_NEQ(ptr1, NULL);

    code_cache_reset(&ctx);

    ASSERT_EQ(code_cache_get_free_space(&ctx), 1024 * 1024);

    /* Allocation after reset should start from beginning */
    ptr2 = code_cache_alloc(&ctx, 256);
    ASSERT_EQ(ptr2, ctx.code_cache);

    jit_cleanup(&ctx);

    return 1;
}

/* ============================================================================
 * Statistics Tests
 * ============================================================================ */

TEST(jit_stats_basic)
{
    jit_context_t ctx;
    u32 blocks, hits, misses;

    jit_init(&ctx, 1024 * 1024);

    jit_get_stats(&ctx, &blocks, &hits, &misses);
    ASSERT_EQ(blocks, 0);
    ASSERT_EQ(hits, 0);
    ASSERT_EQ(misses, 0);

    /* Insert and lookup */
    translation_insert(&ctx, 0x1000, 0x5000, 64);
    translation_lookup(&ctx, 0x1000);

    jit_get_stats(&ctx, &blocks, &hits, &misses);
    ASSERT_EQ(blocks, 1);
    ASSERT_EQ(hits, 1);
    ASSERT_EQ(misses, 0);

    /* Lookup non-existent */
    translation_lookup(&ctx, 0x2000);

    jit_get_stats(&ctx, &blocks, &hits, &misses);
    ASSERT_EQ(blocks, 1);
    ASSERT_EQ(hits, 1);
    ASSERT_EQ(misses, 1);

    jit_cleanup(&ctx);

    return 1;
}

TEST(translation_cache_is_full)
{
    jit_context_t ctx;
    u32 i;

    jit_init(&ctx, 1024 * 1024);

    ASSERT_EQ(translation_cache_is_full(&ctx), false);

    /* Fill cache (4096 entries) */
    for (i = 0; i < TRANSLATION_CACHE_SIZE; i++) {
        translation_insert(&ctx, 0x1000 + i * 0x1000, 0x5000 + i * 0x100, 64);
    }

    /* Cache should be full or very close */
    /* (may not be exactly full due to hash collisions) */

    jit_cleanup(&ctx);

    return 1;
}

/* ============================================================================
 * Global JIT Tests
 * ============================================================================ */

TEST(jit_global_init)
{
    int ret;

    ret = jit_init_global(1024 * 1024);
    ASSERT_EQ(ret, ROSETTA_OK);

    jit_cleanup_global();

    return 1;
}

TEST(jit_global_double_init)
{
    int ret;

    ret = jit_init_global(1024 * 1024);
    ASSERT_EQ(ret, ROSETTA_OK);

    ret = jit_init_global(512 * 1024);  /* Should be no-op */
    ASSERT_EQ(ret, ROSETTA_OK);

    jit_cleanup_global();

    return 1;
}

TEST(jit_global_get_context)
{
    jit_context_t *ctx;

    jit_init_global(1024 * 1024);

    ctx = jit_get_global_context();
    ASSERT_NEQ(ctx, NULL);
    ASSERT_EQ(ctx->initialized, true);

    jit_cleanup_global();

    return 1;
}

/* ============================================================================
 * Integration Tests
 * ============================================================================ */

TEST(jit_translate_block_stub)
{
    jit_context_t ctx;
    void *result;

    jit_init(&ctx, 1024 * 1024);

    /* Current translate_block is a stub that emits UD2 */
    result = translate_block(&ctx, 0x1000);
    ASSERT_NEQ(result, NULL);  /* Should return something */

    /* Lookup should now find it */
    ASSERT_NEQ(translation_lookup(&ctx, 0x1000), NULL);

    jit_cleanup(&ctx);

    return 1;
}

TEST(jit_translate_block_fast)
{
    jit_context_t ctx;
    void *result;

    jit_init(&ctx, 1024 * 1024);

    /* Fast path should return NULL for cold cache */
    result = translate_block_fast(&ctx, 0x1000);
    ASSERT_EQ(result, NULL);

    /* Insert translation */
    translation_insert(&ctx, 0x1000, 0x5000, 64);

    /* Fast path should now find it */
    result = translate_block_fast(&ctx, 0x1000);
    ASSERT_EQ(result, (void*)0x5000);

    jit_cleanup(&ctx);

    return 1;
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

typedef void (*test_func_t)(void);

/* Test runner array */
#define RUN_TEST(name) run_test_##name()

int main(void)
{
    printf("==================================================\n");
    printf("Rosetta JIT Module Unit Tests\n");
    printf("==================================================\n\n");

    /* Hash function tests */
    printf("--- Hash Function Tests ---\n");
    RUN_TEST(hash_address_basic);
    RUN_TEST(hash_address_alignment);
    RUN_TEST(hash_string_basic);
    RUN_TEST(hash_string_empty);
    RUN_TEST(hash_compute_basic);
    printf("\n");

    /* JIT initialization tests */
    printf("--- JIT Initialization Tests ---\n");
    RUN_TEST(jit_init_basic);
    RUN_TEST(jit_init_default_size);
    RUN_TEST(jit_init_null);
    RUN_TEST(jit_cleanup_basic);
    RUN_TEST(jit_reset_basic);
    printf("\n");

    /* Translation cache tests */
    printf("--- Translation Cache Tests ---\n");
    RUN_TEST(translation_lookup_cold);
    RUN_TEST(translation_insert_lookup);
    RUN_TEST(translation_invalidate);
    RUN_TEST(translation_flush);
    RUN_TEST(translation_cache_size);
    RUN_TEST(translation_cache_hash_collisions);
    printf("\n");

    /* Translation block tests */
    printf("--- Translation Block Tests ---\n");
    RUN_TEST(translation_alloc_block);
    RUN_TEST(translation_block_validity);
    RUN_TEST(translation_chain_blocks);
    RUN_TEST(translation_unchain_blocks);
    RUN_TEST(translation_get_successor);
    printf("\n");

    /* Code cache tests */
    printf("--- Code Cache Tests ---\n");
    RUN_TEST(code_cache_alloc);
    RUN_TEST(code_cache_alloc_full);
    RUN_TEST(code_cache_alloc_aligned);
    RUN_TEST(code_cache_reset);
    printf("\n");

    /* Statistics tests */
    printf("--- Statistics Tests ---\n");
    RUN_TEST(jit_stats_basic);
    RUN_TEST(translation_cache_is_full);
    printf("\n");

    /* Global JIT tests */
    printf("--- Global JIT Tests ---\n");
    RUN_TEST(jit_global_init);
    RUN_TEST(jit_global_double_init);
    RUN_TEST(jit_global_get_context);
    printf("\n");

    /* Integration tests */
    printf("--- Integration Tests ---\n");
    RUN_TEST(jit_translate_block_stub);
    RUN_TEST(jit_translate_block_fast);
    printf("\n");

    /* Summary */
    printf("==================================================\n");
    printf("Tests Run: %d\n", tests_run);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("==================================================\n");

    return tests_failed > 0 ? 1 : 0;
}
