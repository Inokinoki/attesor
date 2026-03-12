/* Minimal test for runner execution */

#include "rosetta_runner.h"
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    printf("Test 1: Create runner\n");
    rosetta_runner_t *runner = rosetta_runner_create(NULL);
    if (!runner) {
        printf("FAILED: Could not create runner\n");
        return 1;
    }
    printf("PASSED: Runner created\n");

    printf("\nTest 2: Load binary\n");
    if (rosetta_runner_load_binary(runner, "simple_x86_pure.x86_64") != 0) {
        printf("FAILED: Could not load binary\n");
        rosetta_runner_destroy(runner);
        return 1;
    }
    printf("PASSED: Binary loaded\n");

    printf("\nTest 3: Prepare environment\n");
    if (rosetta_runner_prepare(runner) != 0) {
        printf("FAILED: Could not prepare environment\n");
        rosetta_runner_destroy(runner);
        return 1;
    }
    printf("PASSED: Environment prepared\n");

    printf("\nTest 4: Execute (setup only)\n");
    char *argv[] = {"simple_x86_pure.x86_64", NULL};
    if (rosetta_runner_execute(runner, 1, argv, NULL) != 0) {
        printf("FAILED: Could not execute\n");
        rosetta_runner_destroy(runner);
        return 1;
    }
    printf("PASSED: Execution setup complete\n");

    printf("\nTest 5: Cleanup\n");
    rosetta_runner_destroy(runner);
    printf("PASSED: Runner destroyed\n");

    printf("\nAll tests passed!\n");
    return 0;
}
