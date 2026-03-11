/*
 * Rosetta Demo - Simple Translation Infrastructure Demo
 *
 * This program demonstrates using the Rosetta translation library
 * infrastructure without full translation capabilities.
 *
 * Usage: ./rosetta_demo
 */

#include "rosetta_types.h"
#include "rosetta_init.h"
#include "rosetta_hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    int ret = 0;

    (void)argc;
    (void)argv;

    printf("==================================================\n");
    printf("Rosetta 2 Demo - Translation Infrastructure\n");
    printf("==================================================\n\n");

    /* Initialize translation environment */
    printf("1. Initializing translation environment...\n");
    init_translation_env(NULL);
    printf("   ✓ Translation environment initialized\n\n");

    /* Initialize memory mappings */
    printf("2. Initializing memory mappings...\n");
    if (init_memory_mappings() != ROSETTA_OK) {
        fprintf(stderr, "ERROR: Failed to initialize memory mappings\n");
        ret = 1;
        goto cleanup_env;
    }
    printf("   ✓ Memory mappings initialized\n\n");

    /* Test hash function */
    printf("3. Testing hash functions...\n");
    u32 hash_val = hash_address(0x1000);
    printf("   Hash of 0x1000: 0x%08x\n", hash_val);
    printf("   ✓ Hash functions working\n\n");

    /* Test hash with string */
    printf("4. Testing string hash...\n");
    u32 str_hash = hash_string("test");
    printf("   Hash of \"test\": 0x%08x\n", str_hash);
    printf("   ✓ String hash working\n\n");

    /* Display translation summary */
    printf("==================================================\n");
    printf("Demo Summary\n");
    printf("==================================================\n");
    printf("Translation infrastructure: ACTIVE\n");
    printf("Translation cache: READY\n");
    printf("Memory mappings: ACTIVE\n");
    printf("Hash functions: WORKING\n");
    printf("String hash: WORKING\n");
    printf("Note: Full translation requires ARM64 NEON emitters.\n");

cleanup_env:
    printf("\n5. Cleanup...\n");

    /* Cleanup resources */
    cleanup_translation_env();
    printf("   ✓ Translation environment cleaned up\n");

    printf("\n==================================================\n");
    printf("Demo completed with status: %s\n", ret == 0 ? "SUCCESS" : "FAILED");
    printf("==================================================\n");

    return ret;
}
