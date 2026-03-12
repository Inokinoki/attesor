/* ============================================================================
 * Rosetta Binary Runner Test Suite
 * ============================================================================
 *
 * Tests for the binary runner that demonstrates the complete pipeline:
 * 1. Load x86_64 ELF binary
 * 2. Validate binary format
 * 3. Map segments into memory
 * 4. Initialize CPU state
 * 5. Prepare for execution
 * ============================================================================ */

#include "rosetta_runner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Test result tracking */
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_START(name) \
    printf("Testing: %s... ", name); \
    fflush(stdout);

#define TEST_PASS() \
    do { \
        printf("PASSED\n"); \
        tests_passed++; \
    } while(0)

#define TEST_FAIL(msg) \
    do { \
        printf("FAILED: %s\n", msg); \
        tests_failed++; \
    } while(0)

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            TEST_FAIL(msg); \
            return; \
        } \
    } while(0)

/* ============================================================================
 * Runner Creation Tests
 * ============================================================================ */

/**
 * Test 1: Create runner with default config
 */
void test_runner_create_default(void)
{
    TEST_START("Create runner with default config");

    rosetta_runner_t *runner = rosetta_runner_create(NULL);

    TEST_ASSERT(runner != NULL, "Runner should be created");
    TEST_ASSERT(runner->config.verbose == 1, "Default verbose should be 1");
    TEST_ASSERT(runner->config.debug == 0, "Default debug should be 0");
    TEST_ASSERT(runner->binary == NULL, "No binary should be loaded");
    TEST_ASSERT(runner->is_running == 0, "Should not be running");

    rosetta_runner_destroy(runner);

    TEST_PASS();
}

/**
 * Test 2: Create runner with custom config
 */
void test_runner_create_custom(void)
{
    TEST_START("Create runner with custom config");

    rosetta_runner_config_t config = rosetta_runner_default_config();
    config.verbose = 0;
    config.debug = 1;
    config.max_instructions = 1000000;

    rosetta_runner_t *runner = rosetta_runner_create(&config);

    TEST_ASSERT(runner != NULL, "Runner should be created");
    TEST_ASSERT(runner->config.verbose == 0, "Verbose should be 0");
    TEST_ASSERT(runner->config.debug == 1, "Debug should be 1");
    TEST_ASSERT(runner->config.max_instructions == 1000000,
                "Max instructions should be set");

    rosetta_runner_destroy(runner);

    TEST_PASS();
}

/* ============================================================================
 * Binary Loading Tests
 * ============================================================================ */

/**
 * Test 3: Load static x86_64 binary
 */
void test_load_static_binary(void)
{
    TEST_START("Load static x86_64 binary");

    rosetta_runner_t *runner = rosetta_runner_create(NULL);

    int result = rosetta_runner_load_binary(runner, "simple_x86_pure.x86_64");

    TEST_ASSERT(result == 0, "Should load binary successfully");
    TEST_ASSERT(runner->binary != NULL, "Binary should be loaded");
    TEST_ASSERT(runner->entry_point != 0, "Entry point should be set");

    rosetta_runner_destroy(runner);

    TEST_PASS();
}

/**
 * Test 4: Validate x86_64 binary format
 */
void test_validate_x86_64_format(void)
{
    TEST_START("Validate x86_64 binary format");

    rosetta_runner_t *runner = rosetta_runner_create(NULL);

    rosetta_runner_load_binary(runner, "simple_x86_pure.x86_64");

    TEST_ASSERT(runner->binary != NULL, "Binary should be loaded");
    TEST_ASSERT(runner->binary->is_static == 1,
                "Should be identified as static binary");

    rosetta_runner_destroy(runner);

    TEST_PASS();
}

/**
 * Test 5: Entry point extraction
 */
void test_entry_point_extraction(void)
{
    TEST_START("Entry point extraction");

    rosetta_runner_t *runner = rosetta_runner_create(NULL);

    rosetta_runner_load_binary(runner, "simple_x86_pure.x86_64");

    TEST_ASSERT(runner->entry_point != 0, "Entry point should be non-zero");
    TEST_ASSERT(runner->entry_point < 0x100000000ULL,
                "Entry point should be in reasonable range");

    printf("(entry point: 0x%lx)", runner->entry_point);

    rosetta_runner_destroy(runner);

    TEST_PASS();
}

/* ============================================================================
 * Environment Preparation Tests
 * ============================================================================ */

/**
 * Test 6: Prepare execution environment
 */
void test_prepare_environment(void)
{
    TEST_START("Prepare execution environment");

    rosetta_runner_t *runner = rosetta_runner_create(NULL);

    rosetta_runner_load_binary(runner, "simple_x86_pure.x86_64");

    int result = rosetta_runner_prepare(runner);

    TEST_ASSERT(result == 0, "Should prepare successfully");
    TEST_ASSERT(runner->thread_state != NULL, "Thread state should be created");

    rosetta_runner_destroy(runner);

    TEST_PASS();
}

/* ============================================================================
 * Execution Tests
 * ============================================================================ */

/**
 * Test 7: Execute binary (validation only)
 */
void test_execute_binary(void)
{
    TEST_START("Execute binary (validation)");

    rosetta_runner_t *runner = rosetta_runner_create(NULL);

    rosetta_runner_load_binary(runner, "simple_x86_pure.x86_64");
    rosetta_runner_prepare(runner);

    /* Prepare argv for execution */
    char *argv[] = {"simple_x86_pure.x86_64", NULL};

    int result = rosetta_runner_execute(runner, 1, argv, NULL);

    TEST_ASSERT(result == 0, "Execution setup should succeed");
    TEST_ASSERT(runner->exit_code == 0, "Exit code should be 0");

    rosetta_runner_destroy(runner);

    TEST_PASS();
}

/**
 * Test 8: Simplified execution interface
 */
void test_simplified_execution(void)
{
    TEST_START("Simplified execution interface (rosetta_run)");

    char *argv[] = {"simple_x86_pure.x86_64", NULL};

    /* This will load, prepare, and validate execution setup */
    int result = rosetta_run("simple_x86_pure.x86_64", argv);

    TEST_ASSERT(result == 0, "Should complete successfully");

    TEST_PASS();
}

/* ============================================================================
 * Runner Statistics Tests
 * ============================================================================ */

/**
 * Test 9: Get runner statistics
 */
void test_runner_statistics(void)
{
    TEST_START("Get runner statistics");

    rosetta_runner_t *runner = rosetta_runner_create(NULL);

    rosetta_runner_load_binary(runner, "simple_x86_pure.x86_64");
    rosetta_runner_prepare(runner);

    uint64_t instructions = 0;
    int result = rosetta_runner_get_stats(runner, &instructions);

    TEST_ASSERT(result == 0, "Should get statistics successfully");

    rosetta_runner_destroy(runner);

    TEST_PASS();
}

/**
 * Test 10: Print runner statistics
 */
void test_print_statistics(void)
{
    TEST_START("Print runner statistics");

    rosetta_runner_t *runner = rosetta_runner_create(NULL);

    rosetta_runner_load_binary(runner, "simple_x86_pure.x86_64");
    rosetta_runner_prepare(runner);

    /* This should print statistics without crashing */
    rosetta_runner_print_stats(runner);

    rosetta_runner_destroy(runner);

    TEST_PASS();
}

/* ============================================================================
 * Error Handling Tests
 * ============================================================================ */

/**
 * Test 11: Load non-existent binary
 */
void test_load_nonexistent_binary(void)
{
    TEST_START("Load non-existent binary");

    rosetta_runner_t *runner = rosetta_runner_create(NULL);

    int result = rosetta_runner_load_binary(runner, "nonexistent.x86_64");

    TEST_ASSERT(result != 0, "Should fail to load");
    TEST_ASSERT(runner->binary == NULL, "No binary should be loaded");

    rosetta_runner_destroy(runner);

    TEST_PASS();
}

/**
 * Test 12: Execute without binary
 */
void test_execute_without_binary(void)
{
    TEST_START("Execute without binary");

    rosetta_runner_t *runner = rosetta_runner_create(NULL);

    /* Don't load any binary */
    rosetta_runner_prepare(runner);

    char *argv[] = {"test", NULL};

    int result = rosetta_runner_execute(runner, 1, argv, NULL);

    TEST_ASSERT(result != 0, "Should fail without binary");

    rosetta_runner_destroy(runner);

    TEST_PASS();
}

/**
 * Test 13: Runner status checks
 */
void test_runner_status(void)
{
    TEST_START("Runner status checks");

    rosetta_runner_t *runner = rosetta_runner_create(NULL);

    int status = rosetta_runner_status(runner);

    TEST_ASSERT(status == 0, "Initial status should be stopped");

    rosetta_runner_destroy(runner);

    TEST_PASS();
}

/* ============================================================================
 * Integration Tests
 * ============================================================================ */

/**
 * Test 14: Complete pipeline test
 */
void test_complete_pipeline(void)
{
    TEST_START("Complete pipeline test");

    /* Create runner */
    rosetta_runner_t *runner = rosetta_runner_create(NULL);
    TEST_ASSERT(runner != NULL, "Runner should be created");

    /* Load binary */
    int result = rosetta_runner_load_binary(runner, "simple_x86_pure.x86_64");
    TEST_ASSERT(result == 0, "Binary should load");

    /* Validate binary */
    TEST_ASSERT(runner->binary != NULL, "Binary should be loaded");
    TEST_ASSERT(runner->entry_point != 0, "Entry point should be set");

    /* Prepare environment */
    result = rosetta_runner_prepare(runner);
    TEST_ASSERT(result == 0, "Environment should be prepared");
    TEST_ASSERT(runner->thread_state != NULL, "Thread state should exist");

    /* Execute */
    char *argv[] = {"simple_x86_pure.x86_64", NULL};
    result = rosetta_runner_execute(runner, 1, argv, NULL);
    TEST_ASSERT(result == 0, "Execution should setup successfully");

    /* Get statistics */
    uint64_t instructions = 0;
    result = rosetta_runner_get_stats(runner, &instructions);
    TEST_ASSERT(result == 0, "Statistics should be available");

    /* Cleanup */
    rosetta_runner_destroy(runner);

    TEST_PASS();
}

/**
 * Test 15: Multiple binary loads
 */
void test_multiple_binary_loads(void)
{
    TEST_START("Multiple binary loads");

    /* Load first binary */
    rosetta_runner_t *runner1 = rosetta_runner_create(NULL);
    int result = rosetta_runner_load_binary(runner1, "simple_x86_pure.x86_64");
    TEST_ASSERT(result == 0, "First binary should load");

    /* Load second binary */
    rosetta_runner_t *runner2 = rosetta_runner_create(NULL);
    result = rosetta_runner_load_binary(runner2, "simple_x86_test.x86_64");
    TEST_ASSERT(result == 0, "Second binary should load");

    /* Both should have different entry points (likely) */
    printf("(EP1: 0x%lx, EP2: 0x%lx)",
           runner1->entry_point, runner2->entry_point);

    rosetta_runner_destroy(runner1);
    rosetta_runner_destroy(runner2);

    TEST_PASS();
}

/* ============================================================================
 * Test Runner
 * ============================================================================ */

int main(int argc, char **argv)
{
    printf("=================================================================\n");
    printf("Rosetta Binary Runner Test Suite\n");
    printf("=================================================================\n\n");

    /* Check if test binaries exist */
    if (access("simple_x86_pure.x86_64", F_OK) != 0) {
        printf("WARNING: Test binary 'simple_x86_pure.x86_64' not found\n");
        printf("Some tests will fail. Run 'make test_bins' first.\n\n");
    }

    /* Run all tests */
    test_runner_create_default();
    test_runner_create_custom();
    test_load_static_binary();
    test_validate_x86_64_format();
    test_entry_point_extraction();
    test_prepare_environment();
    test_execute_binary();
    test_simplified_execution();
    test_runner_statistics();
    test_print_statistics();
    test_load_nonexistent_binary();
    test_execute_without_binary();
    test_runner_status();
    test_complete_pipeline();
    test_multiple_binary_loads();

    /* Print summary */
    printf("\n=================================================================\n");
    printf("Test Results:\n");
    printf("  Passed: %d\n", tests_passed);
    printf("  Failed: %d\n", tests_failed);
    printf("=================================================================\n");

    return (tests_failed > 0) ? 1 : 0;
}
