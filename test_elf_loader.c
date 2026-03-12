/* ============================================================================
 * Rosetta ELF Loader Test Suite
 * ============================================================================
 *
 * Tests for the ELF binary loader functionality
 * ============================================================================ */

#include "rosetta_elf_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
 * Test Cases
 * ============================================================================ */

/**
 * Test 1: Load existing test binaries
 */
void test_load_existing_binaries(void)
{
    TEST_START("Load existing x86_64 binaries");

    /* List of binaries to try loading */
    const char *test_binaries[] = {
        "simple_x86_pure.x86_64",
        "simple_x86_test.x86_64",
        "rosetta_demo",
        "rosetta_demo_full",
        NULL
    };

    int loaded_count = 0;

    for (int i = 0; test_binaries[i] != NULL; i++) {
        rosetta_elf_binary_t *binary = NULL;

        int result = rosetta_elf_load(test_binaries[i], &binary);
        if (result == 0 && binary != NULL) {
            printf("\n  Loaded: %s\n", test_binaries[i]);
            printf("    Entry: 0x%016lx\n", binary->entry_point);
            printf("    Base: 0x%016lx\n", binary->base_address);
            printf("    PIE: %s  Static: %s\n",
                   binary->is_pie ? "Yes" : "No",
                   binary->is_static ? "Yes" : "No");

            if (binary->interp) {
                printf("    Interp: %s\n", binary->interp);
            }

            printf("    Segments: %u  Sections: %u\n",
                   binary->num_segments, binary->num_sections);

            rosetta_elf_unload(binary);
            loaded_count++;
        }
    }

    if (loaded_count > 0) {
        TEST_PASS();
    } else {
        TEST_FAIL("No binaries could be loaded");
    }
}

/**
 * Test 2: Validate x86_64 detection
 */
void test_validate_x86_64(void)
{
    TEST_START("Validate x86_64 detection");

    rosetta_elf_binary_t *binary = NULL;

    /* Try to load a test binary */
    int result = rosetta_elf_load("simple_x86_pure.x86_64", &binary);
    if (result != 0 || binary == NULL) {
        /* Skip if binary not available */
        printf("SKIPPED (no test binary)\n");
        return;
    }

    TEST_ASSERT(rosetta_elf_validate_x86_64(binary) == 1,
                "Binary should be valid x86_64");

    rosetta_elf_unload(binary);
    TEST_PASS();
}

/**
 * Test 3: Get entry point
 */
void test_get_entry_point(void)
{
    TEST_START("Get entry point");

    rosetta_elf_binary_t *binary = NULL;

    int result = rosetta_elf_load("simple_x86_pure.x86_64", &binary);
    if (result != 0 || binary == NULL) {
        printf("SKIPPED (no test binary)\n");
        return;
    }

    uint64_t entry = rosetta_elf_get_entry_point(binary);
    TEST_ASSERT(entry != 0, "Entry point should not be zero");

    printf("(0x%016lx)", entry);

    rosetta_elf_unload(binary);
    TEST_PASS();
}

/**
 * Test 4: Get sections
 */
void test_get_sections(void)
{
    TEST_START("Get ELF sections");

    rosetta_elf_binary_t *binary = NULL;

    int result = rosetta_elf_load("simple_x86_pure.x86_64", &binary);
    if (result != 0 || binary == NULL) {
        printf("SKIPPED (no test binary)\n");
        return;
    }

    /* Try to get common sections */
    const char *section_names[] = {
        ".text",
        ".data",
        ".bss",
        ".rodata",
        NULL
    };

    int found_sections = 0;
    for (int i = 0; section_names[i] != NULL; i++) {
        rosetta_elf_section_t *sect = rosetta_elf_get_section(binary,
                                                             section_names[i]);
        if (sect != NULL) {
            found_sections++;
            printf("\n  Found section: %s at 0x%016lx (size=%lu)",
                   sect->name, sect->guest_addr, sect->size);
        }
    }

    TEST_ASSERT(found_sections > 0, "Should find at least one section");

    rosetta_elf_unload(binary);
    TEST_PASS();
}

/**
 * Test 5: Symbol lookup
 */
void test_symbol_lookup(void)
{
    TEST_START("Symbol lookup");

    rosetta_elf_binary_t *binary = NULL;

    int result = rosetta_elf_load("simple_x86_pure.x86_64", &binary);
    if (result != 0 || binary == NULL) {
        printf("SKIPPED (no test binary)\n");
        return;
    }

    /* Try to lookup common symbols */
    const char *test_symbols[] = {
        "main",
        "_start",
        "printf",
        "malloc",
        NULL
    };

    int found_symbols = 0;
    for (int i = 0; test_symbols[i] != NULL; i++) {
        uint64_t addr = rosetta_elf_lookup_symbol(binary, test_symbols[i]);
        if (addr != 0) {
            found_symbols++;
            printf("\n  Found symbol: %s at 0x%016lx", test_symbols[i], addr);
        }
    }

    /* Not all binaries have symbols, so don't fail if none found */
    printf("(found %d symbols)", found_symbols);

    rosetta_elf_unload(binary);
    TEST_PASS();
}

/**
 * Test 6: Base address
 */
void test_base_address(void)
{
    TEST_START("Get base address");

    rosetta_elf_binary_t *binary = NULL;

    int result = rosetta_elf_load("simple_x86_pure.x86_64", &binary);
    if (result != 0 || binary == NULL) {
        printf("SKIPPED (no test binary)\n");
        return;
    }

    uint64_t base = rosetta_elf_get_base_address(binary);
    printf("(0x%016lx)", base);

    rosetta_elf_unload(binary);
    TEST_PASS();
}

/**
 * Test 7: Static vs dynamic detection
 */
void test_static_detection(void)
{
    TEST_START("Static vs dynamic detection");

    rosetta_elf_binary_t *binary = NULL;

    int result = rosetta_elf_load("simple_x86_pure.x86_64", &binary);
    if (result != 0 || binary == NULL) {
        printf("SKIPPED (no test binary)\n");
        return;
    }

    int is_static = rosetta_elf_is_static(binary);
    const char *interp = rosetta_elf_get_interpreter(binary);

    printf("\n  Static: %s  Interpreter: %s",
           is_static ? "Yes" : "No",
           interp ? interp : "(none)");

    rosetta_elf_unload(binary);
    TEST_PASS();
}

/**
 * Test 8: Print binary info
 */
void test_print_info(void)
{
    TEST_START("Print binary information");

    rosetta_elf_binary_t *binary = NULL;

    int result = rosetta_elf_load("simple_x86_pure.x86_64", &binary);
    if (result != 0 || binary == NULL) {
        printf("SKIPPED (no test binary)\n");
        return;
    }

    printf("\n");
    rosetta_elf_print_info(binary);

    rosetta_elf_unload(binary);
    TEST_PASS();
}

/**
 * Test 9: Invalid file handling
 */
void test_invalid_file(void)
{
    TEST_START("Invalid file handling");

    rosetta_elf_binary_t *binary = NULL;

    /* Try to load non-existent file */
    int result = rosetta_elf_load("nonexistent_file_12345.x86_64", &binary);

    TEST_ASSERT(result != 0, "Should fail to load non-existent file");
    TEST_ASSERT(binary == NULL, "Binary should be NULL on failure");

    TEST_PASS();
}

/**
 * Test 10: Guest to host address translation
 */
void test_guest_to_host(void)
{
    TEST_START("Guest to host address translation");

    rosetta_elf_binary_t *binary = NULL;

    int result = rosetta_elf_load("simple_x86_pure.x86_64", &binary);
    if (result != 0 || binary == NULL) {
        printf("SKIPPED (no test binary)\n");
        return;
    }

    /* Test with entry point (should be in .text section) */
    uint64_t entry = rosetta_elf_get_entry_point(binary);
    void *host_addr = rosetta_elf_guest_to_host(binary, entry);

    if (host_addr != NULL) {
        printf("\n  Entry 0x%016lx -> host %p", entry, host_addr);
    }

    rosetta_elf_unload(binary);
    TEST_PASS();
}

/* ============================================================================
 * Test Runner
 * ============================================================================ */

int main(int argc, char **argv)
{
    printf("=================================================================\n");
    printf("Rosetta ELF Loader Test Suite\n");
    printf("=================================================================\n\n");

    /* If a specific test binary is provided, use it */
    if (argc > 1) {
        printf("Testing binary: %s\n\n", argv[1]);

        rosetta_elf_binary_t *binary = NULL;
        int result = rosetta_elf_load(argv[1], &binary);

        if (result == 0 && binary != NULL) {
            printf("Successfully loaded!\n\n");
            rosetta_elf_print_info(binary);
            rosetta_elf_unload(binary);
            return 0;
        } else {
            printf("Failed to load binary: %s\n", argv[1]);
            return 1;
        }
    }

    /* Run all tests */
    test_load_existing_binaries();
    test_validate_x86_64();
    test_get_entry_point();
    test_get_sections();
    test_symbol_lookup();
    test_base_address();
    test_static_detection();
    test_print_info();
    test_invalid_file();
    test_guest_to_host();

    /* Print summary */
    printf("\n=================================================================\n");
    printf("Test Results:\n");
    printf("  Passed: %d\n", tests_passed);
    printf("  Failed: %d\n", tests_failed);
    printf("=================================================================\n");

    return (tests_failed > 0) ? 1 : 0;
}
