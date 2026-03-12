/* ============================================================================
 * Rosetta Execution Engine - Test Program
 * ============================================================================
 *
 * This test program validates the execution engine's ability to:
 * 1. Load an x86_64 ELF binary
 * 2. Map ELF segments into guest memory
 * 3. Fetch x86_64 instructions from guest memory
 * 4. Execute a basic block of instructions
 * ============================================================================
 */

#include "rosetta_runner.h"
#include "rosetta_execute.h"
#include "rosetta_memmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    printf("=================================================================\n");
    printf("Rosetta Execution Engine Test\n");
    printf("=================================================================\n\n");

    /* Check for binary argument */
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <x86_64_binary>\n", argv[0]);
        return 1;
    }

    const char *binary_path = argv[1];
    printf("Binary: %s\n\n", binary_path);

    /* Create runner with default config */
    rosetta_runner_config_t config = rosetta_runner_default_config();
    config.verbose = 1;

    rosetta_runner_t *runner = rosetta_runner_create(&config);
    if (!runner) {
        fprintf(stderr, "Failed to create runner\n");
        return 1;
    }

    /* Load binary */
    printf("Step 1: Loading binary...\n");
    if (rosetta_runner_load_binary(runner, binary_path) != 0) {
        fprintf(stderr, "Failed to load binary\n");
        rosetta_runner_destroy(runner);
        return 1;
    }
    printf("  Binary loaded successfully\n\n");

    /* Prepare execution environment */
    printf("Step 2: Preparing execution environment...\n");
    if (rosetta_runner_prepare(runner) != 0) {
        fprintf(stderr, "Failed to prepare environment\n");
        rosetta_runner_destroy(runner);
        return 1;
    }
    printf("  Environment prepared\n\n");

    /* Execute binary */
    printf("Step 3: Executing binary...\n");
    printf("  Entry point: 0x%016lx\n\n", runner->entry_point);

    /* Count arguments */
    int test_argc = argc - 1;
    char **test_argv = argv + 1;

    int exit_code = rosetta_runner_execute(runner, test_argc, test_argv, NULL);

    printf("\n");
    printf("Step 4: Execution complete\n");
    printf("  Exit code: %d\n", exit_code);

    /* Print statistics */
    rosetta_runner_print_stats(runner);

    /* Cleanup */
    rosetta_runner_destroy(runner);

    printf("\n");
    printf("=================================================================\n");
    printf("Test Complete\n");
    printf("=================================================================\n");

    return exit_code;
}
