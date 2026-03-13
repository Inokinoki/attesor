/* ============================================================================
 * Rosetta Exception Handling Test Suite
 * ============================================================================
 *
 * Tests for the exception handling and signal translation functionality
 * ============================================================================ */

#include "rosetta_refactored_exception.h"
#include "rosetta_refactored_signal.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <string.h>

/* Test result tracking */
static int tests_passed = 0;
static int tests_failed = 0;
static sigjmp_buf jump_buffer;

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
 * Exception Handler Tests
 * ============================================================================ */

/* Custom exception handler for testing */
static int test_exception_handler(ros_exception_info_t *info, void *context)
{
    (void)context;

    if (info) {
        /* Handle various exception types for testing */
        if (info->type == ROS_EXCEPTION_TRAP) {
            /* Handle trap by continuing */
            return 0;
        }
        if (info->type == ROS_EXCEPTION_DAT_ABORT) {
            /* Handle data abort by continuing (for testing) */
            return 0;
        }
        if (info->type == ROS_EXCEPTION_BREAKPOINT) {
            /* Handle breakpoint by continuing */
            return 0;
        }
    }
    return -1;
}

/**
 * Test 1: Exception initialization
 */
void test_exception_init(void)
{
    TEST_START("Exception initialization");

    int result = rosetta_exception_init();
    TEST_ASSERT(result == 0, "Initialization should succeed");

    TEST_ASSERT(rosetta_exception_occurred() == false,
                "No exception should be pending initially");

    rosetta_exception_cleanup();
    TEST_PASS();
}

/**
 * Test 2: Exception registration
 */
void test_exception_register(void)
{
    TEST_START("Exception handler registration");

    rosetta_exception_init();

    int result = rosetta_exception_register_handler(
        ROS_EXCEPTION_TRAP, test_exception_handler);

    TEST_ASSERT(result == 0, "Handler registration should succeed");

    rosetta_exception_cleanup();
    TEST_PASS();
}

/**
 * Test 3: Trap exception handling
 */
void test_exception_trap(void)
{
    TEST_START("Trap exception handling");

    rosetta_exception_init();
    rosetta_exception_register_handler(ROS_EXCEPTION_TRAP, test_exception_handler);

    int result = rosetta_raise_trap(0x1000, 0xD4200000);

    TEST_ASSERT(result == 0, "Trap should be handled successfully");
    TEST_ASSERT(rosetta_exception_occurred() == true,
                "Exception should be marked as occurred");

    ros_exception_info_t *info = rosetta_exception_get_info();
    TEST_ASSERT(info != NULL, "Should be able to get exception info");
    TEST_ASSERT(info->type == ROS_EXCEPTION_TRAP,
                "Exception type should be TRAP");
    TEST_ASSERT(info->pc == 0x1000, "PC should match");

    rosetta_exception_clear();
    TEST_ASSERT(rosetta_exception_occurred() == false,
                "Exception should be cleared");

    rosetta_exception_cleanup();
    TEST_PASS();
}

/**
 * Test 4: Undefined instruction exception
 */
void test_exception_undefined(void)
{
    TEST_START("Undefined instruction exception");

    rosetta_exception_init();

    /* Raise undefined instruction exception */
    int result = rosetta_raise_undefined(0x2000, 0x00000000);

    /* Undefined instructions are fatal by default */
    TEST_ASSERT(result < 0, "Undefined instruction should be fatal");

    ros_exception_info_t *info = rosetta_exception_get_info();
    TEST_ASSERT(info != NULL, "Should have exception info");
    TEST_ASSERT(info->type == ROS_EXCEPTION_UNDEFINED,
                "Exception type should be UNDEFINED");
    TEST_ASSERT(info->sig == SIGILL, "Should map to SIGILL");

    rosetta_exception_cleanup();
    TEST_PASS();
}

/**
 * Test 5: Signal to exception mapping
 */
void test_signal_to_exception_mapping(void)
{
    TEST_START("Signal to exception mapping");

    /* Test various signal mappings */
    ros_exception_type_t type;

    type = rosetta_signal_to_exception(SIGSEGV, SEGV_MAPERR);
    TEST_ASSERT(type == ROS_EXCEPTION_DAT_ABORT,
                "SIGSEGV should map to DAT_ABORT");

    type = rosetta_signal_to_exception(SIGILL, ILL_ILLOPC);
    TEST_ASSERT(type == ROS_EXCEPTION_UNDEFINED,
                "SIGILL should map to UNDEFINED");

    type = rosetta_signal_to_exception(SIGFPE, FPE_INTDIV);
    TEST_ASSERT(type == ROS_EXCEPTION_FP,
                "SIGFPE should map to FP exception");

    type = rosetta_signal_to_exception(SIGTRAP, 1);  /* 1 = TRAP_BRKPT */
    TEST_ASSERT(type == ROS_EXCEPTION_TRAP,
                "SIGTRAP should map to TRAP");

    TEST_PASS();
}

/**
 * Test 6: Exception to signal mapping
 */
void test_exception_to_signal_mapping(void)
{
    TEST_START("Exception to signal mapping");

    int sig;

    sig = rosetta_exception_to_signal(ROS_EXCEPTION_TRAP);
    TEST_ASSERT(sig == SIGTRAP, "TRAP should map to SIGTRAP");

    sig = rosetta_exception_to_signal(ROS_EXCEPTION_UNDEFINED);
    TEST_ASSERT(sig == SIGILL, "UNDEFINED should map to SIGILL");

    sig = rosetta_exception_to_signal(ROS_EXCEPTION_DAT_ABORT);
    TEST_ASSERT(sig == SIGSEGV, "DAT_ABORT should map to SIGSEGV");

    sig = rosetta_exception_to_signal(ROS_EXCEPTION_FP);
    TEST_ASSERT(sig == SIGFPE, "FP exception should map to SIGFPE");

    TEST_PASS();
}

/* ============================================================================
 * Signal Handler Tests
 * ============================================================================ */

/**
 * Test 7: Signal handler initialization
 */
void test_signal_init(void)
{
    TEST_START("Signal handler initialization");

    int result = rosetta_init_signal_handlers();
    TEST_ASSERT(result == 0, "Signal handler init should succeed");

    rosetta_cleanup_signal_handlers();
    TEST_PASS();
}

/**
 * Test 8: Signal blocking/unblocking
 */
void test_signal_blocking(void)
{
    TEST_START("Signal masking");

    sigset_t mask, oldmask;

    /* Block SIGUSR1 for testing */
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);

    int result = rosetta_signal_block(SIGUSR1, &oldmask);
    TEST_ASSERT(result == 0, "Signal block should succeed");

    /* Restore original mask */
    rosetta_signal_set_mask(&oldmask);

    TEST_PASS();
}

/**
 * Test 9: BRK instruction handling
 */
void test_handle_brk(void)
{
    TEST_START("BRK instruction handling");

    rosetta_exception_init();

    /* BRK instructions are typically non-fatal debug traps */
    int result = rosetta_handle_brk(0x3000, 0);
    TEST_ASSERT(result == 0, "BRK should be handled successfully");

    rosetta_exception_cleanup();
    TEST_PASS();
}

/**
 * Test 10: HLT instruction handling
 */
void test_handle_hlt(void)
{
    TEST_START("HLT instruction handling");

    rosetta_exception_init();

    /* HLT with imm=0 is used for termination */
    int result = rosetta_handle_hlt(0x4000, 0);
    TEST_ASSERT(result < 0, "HLT(0) should signal termination");

    /* Other HLT values should be handled as traps */
    result = rosetta_handle_hlt(0x4000, 1);
    TEST_ASSERT(result == 0, "HLT(non-zero) should be handled as trap");

    rosetta_exception_cleanup();
    TEST_PASS();
}

/* ============================================================================
 * Integration Tests
 * ============================================================================ */

/**
 * Test 11: Full exception cycle
 */
void test_exception_cycle(void)
{
    TEST_START("Full exception handling cycle");

    rosetta_exception_init();
    rosetta_exception_register_handler(ROS_EXCEPTION_TRAP,
                                       test_exception_handler);

    /* Raise exception */
    rosetta_raise_trap(0x5000, 0xD4200000);

    /* Check exception info */
    ros_exception_info_t *info = rosetta_exception_get_info();
    TEST_ASSERT(info != NULL, "Should have exception info");
    TEST_ASSERT(info->pc == 0x5000, "PC should be correct");
    TEST_ASSERT(info->type == ROS_EXCEPTION_TRAP, "Type should be TRAP");

    /* Clear and verify */
    rosetta_exception_clear();
    TEST_ASSERT(rosetta_exception_occurred() == false,
                "Exception should be cleared");

    /* Cleanup */
    rosetta_exception_cleanup();
    TEST_PASS();
}

/**
 * Test 12: Fault handler registration
 */
void test_fault_handler(void)
{
    TEST_START("Fault handler registration");

    rosetta_exception_init();

    int result = rosetta_set_fault_handler(test_exception_handler);
    TEST_ASSERT(result == 0, "Fault handler registration should succeed");

    /* Test data abort with fault handler */
    result = rosetta_raise_data_abort(0x6000, 0xdeadbeef, 0);
    TEST_ASSERT(result == 0, "Data abort should be handled by fault handler");

    rosetta_exception_cleanup();
    TEST_PASS();
}

/**
 * Test 13: Exception flags
 */
void test_exception_flags(void)
{
    TEST_START("Exception flags");

    rosetta_exception_init();

    /* Raise undefined instruction (should be fatal) */
    rosetta_raise_undefined(0x7000, 0);

    ros_exception_info_t *info = rosetta_exception_get_info();
    TEST_ASSERT(info != NULL, "Should have exception info");
    TEST_ASSERT((info->flags & ROS_EXCEPTION_FLAG_FATAL) != 0,
                "Undefined instruction should be marked as fatal");

    rosetta_exception_cleanup();
    TEST_PASS();
}

/**
 * Test 14: Multiple exception types
 */
void test_multiple_exceptions(void)
{
    TEST_START("Multiple exception type handling");

    rosetta_exception_init();
    rosetta_exception_register_handler(ROS_EXCEPTION_TRAP,
                                       test_exception_handler);
    rosetta_exception_register_handler(ROS_EXCEPTION_BREAKPOINT,
                                       test_exception_handler);

    /* Test different exception types */
    rosetta_raise_trap(0x1000, 0);
    TEST_ASSERT(rosetta_exception_occurred() == true,
                "TRAP should occur");

    rosetta_exception_clear();

    rosetta_exception_cleanup();
    TEST_PASS();
}

/**
 * Test 15: Signal handler cleanup
 */
void test_signal_cleanup(void)
{
    TEST_START("Signal handler cleanup");

    /* Initialize signal handlers */
    rosetta_init_signal_handlers();

    /* Cleanup should reset handlers to default */
    rosetta_cleanup_signal_handlers();

    /* Verify by checking we can still raise signals */
    /* (We don't actually test this as it would crash the test) */

    TEST_PASS();
}

/* ============================================================================
 * Test Runner
 * ============================================================================ */

int main(int argc, char **argv)
{
    printf("=================================================================\n");
    printf("Rosetta Exception Handling Test Suite\n");
    printf("=================================================================\n\n");

    /* Run all tests */
    test_exception_init();
    test_exception_register();
    test_exception_trap();
    test_exception_undefined();
    test_signal_to_exception_mapping();
    test_exception_to_signal_mapping();
    test_signal_init();
    test_signal_blocking();
    test_handle_brk();
    test_handle_hlt();
    test_exception_cycle();
    test_fault_handler();
    test_exception_flags();
    test_multiple_exceptions();
    test_signal_cleanup();

    /* Print summary */
    printf("\n=================================================================\n");
    printf("Test Results:\n");
    printf("  Passed: %d\n", tests_passed);
    printf("  Failed: %d\n", tests_failed);
    printf("=================================================================\n");

    return (tests_failed > 0) ? 1 : 0;
}
