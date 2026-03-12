/* ============================================================================
 * Rosetta /proc Filesystem Emulation Test Suite
 * ============================================================================
 *
 * Tests for the /proc filesystem emulation functionality
 * ============================================================================ */

#include "rosetta_procfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

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
 * /proc Path Detection Tests
 * ============================================================================ */

/**
 * Test 1: /proc path detection
 */
void test_proc_path_detection(void)
{
    TEST_START("/proc path detection");

    TEST_ASSERT(rosetta_proc_is_proc_path("/proc/cpuinfo") == 1,
                "Should detect /proc/cpuinfo as /proc path");
    TEST_ASSERT(rosetta_proc_is_proc_path("/proc/self/auxv") == 1,
                "Should detect /proc/self/auxv as /proc path");
    TEST_ASSERT(rosetta_proc_is_proc_path("/etc/passwd") == 0,
                "Should not detect /etc/passwd as /proc path");

    TEST_PASS();
}

/**
 * Test 2: /proc path type detection
 */
void test_proc_path_type_detection(void)
{
    TEST_START("/proc path type detection");

    rosetta_proc_path_t type;

    type = rosetta_proc_get_path_type("/proc/cpuinfo");
    TEST_ASSERT(type == ROSETTA_PROC_CPUINFO,
                "Should identify cpuinfo path");

    type = rosetta_proc_get_path_type("/proc/self/auxv");
    TEST_ASSERT(type == ROSETTA_PROC_SELF_AUXV,
                "Should identify auxv path");

    type = rosetta_proc_get_path_type("/proc/self/cmdline");
    TEST_ASSERT(type == ROSETTA_PROC_SELF_CMDLINE,
                "Should identify cmdline path");

    type = rosetta_proc_get_path_type("/proc/self/exe");
    TEST_ASSERT(type == ROSETTA_PROC_SELF_EXE,
                "Should identify exe path");

    type = rosetta_proc_get_path_type("/proc/meminfo");
    TEST_ASSERT(type == ROSETTA_PROC_MEMINFO,
                "Should identify meminfo path");

    TEST_PASS();
}

/* ============================================================================
 * /proc Content Generation Tests
 * ============================================================================ */

/**
 * Test 3: /proc/cpuinfo generation
 */
void test_proc_cpuinfo_generation(void)
{
    TEST_START("/proc/cpuinfo generation");

    char buf[4096];
    int len = rosetta_proc_generate_cpuinfo(buf, sizeof(buf));

    TEST_ASSERT(len > 0, "Should generate cpuinfo content");
    TEST_ASSERT((size_t)len < sizeof(buf), "Content should fit in buffer");

    /* Verify content contains expected fields */
    TEST_ASSERT(strstr(buf, "processor") != NULL,
                "Should contain processor field");
    TEST_ASSERT(strstr(buf, "vendor_id") != NULL,
                "Should contain vendor_id field");
    TEST_ASSERT(strstr(buf, "model name") != NULL,
                "Should contain model name field");

    printf("(generated %d bytes)", len);

    TEST_PASS();
}

/**
 * Test 4: /proc/self/auxv generation
 */
void test_proc_auxv_generation(void)
{
    TEST_START("/proc/self/auxv generation");

    char buf[1024];
    int len = rosetta_proc_generate_auxv(buf, sizeof(buf));

    TEST_ASSERT(len > 0, "Should generate auxv content");
    TEST_ASSERT(len % 8 == 0, "Auxv should be multiple of 8 bytes");

    printf("(generated %d bytes)", len);

    TEST_PASS();
}

/**
 * Test 5: /proc/self/cmdline generation
 */
void test_proc_cmdline_generation(void)
{
    TEST_START("/proc/self/cmdline generation");

    char buf[1024];
    int len = rosetta_proc_generate_cmdline(buf, sizeof(buf));

    TEST_ASSERT(len >= 0, "Should generate cmdline content");
    TEST_ASSERT((size_t)len < sizeof(buf), "Content should fit in buffer");

    printf("(generated %d bytes)", len);

    TEST_PASS();
}

/**
 * Test 6: /proc/self/exe path
 */
void test_proc_exe_path(void)
{
    TEST_START("/proc/self/exe path");

    char buf[PATH_MAX];
    int len = rosetta_proc_get_exe_path(buf, sizeof(buf));

    TEST_ASSERT(len > 0, "Should get exe path");
    TEST_ASSERT(buf[0] == '/', "Path should be absolute");

    printf("(path: %s)", buf);

    TEST_PASS();
}

/**
 * Test 7: /proc/meminfo generation
 */
void test_proc_meminfo_generation(void)
{
    TEST_START("/proc/meminfo generation");

    char buf[2048];
    int len = rosetta_proc_generate_meminfo(buf, sizeof(buf));

    TEST_ASSERT(len > 0, "Should generate meminfo content");
    TEST_ASSERT((size_t)len < sizeof(buf), "Content should fit in buffer");

    /* Verify content contains expected fields */
    TEST_ASSERT(strstr(buf, "MemTotal") != NULL,
                "Should contain MemTotal field");
    TEST_ASSERT(strstr(buf, "MemFree") != NULL,
                "Should contain MemFree field");
    TEST_ASSERT(strstr(buf, "SwapTotal") != NULL,
                "Should contain SwapTotal field");

    printf("(generated %d bytes)", len);

    TEST_PASS();
}

/**
 * Test 8: /proc/self/status generation
 */
void test_proc_status_generation(void)
{
    TEST_START("/proc/self/status generation");

    char buf[4096];
    int len = rosetta_proc_generate_status(buf, sizeof(buf));

    TEST_ASSERT(len > 0, "Should generate status content");
    TEST_ASSERT((size_t)len < sizeof(buf), "Content should fit in buffer");

    /* Verify content contains expected fields */
    TEST_ASSERT(strstr(buf, "Pid:") != NULL,
                "Should contain Pid field");
    TEST_ASSERT(strstr(buf, "Uid:") != NULL,
                "Should contain Uid field");
    TEST_ASSERT(strstr(buf, "Threads:") != NULL,
                "Should contain Threads field");

    printf("(generated %d bytes)", len);

    TEST_PASS();
}

/**
 * Test 9: /proc/self/maps generation
 */
void test_proc_maps_generation(void)
{
    TEST_START("/proc/self/maps generation");

    char buf[1024];
    int len = rosetta_proc_generate_maps(buf, sizeof(buf));

    TEST_ASSERT(len > 0, "Should generate maps content");
    TEST_ASSERT((size_t)len < sizeof(buf), "Content should fit in buffer");

    /* Verify content looks like memory map */
    TEST_ASSERT(strstr(buf, "r-xp") != NULL || strstr(buf, "rw-p") != NULL,
                "Should contain memory permissions");

    printf("(generated %d bytes)", len);

    TEST_PASS();
}

/* ============================================================================
 * /proc File Operations Tests
 * ============================================================================ */

/**
 * Test 10: /proc file open and close
 */
void test_proc_file_operations(void)
{
    TEST_START("/proc file open/close");

    int fd = rosetta_proc_open("/proc/cpuinfo", O_RDONLY);
    TEST_ASSERT(fd >= 0, "Should open /proc/cpuinfo");

    int result = rosetta_proc_close(fd);
    TEST_ASSERT(result == 0, "Should close /proc file successfully");

    TEST_PASS();
}

/**
 * Test 11: /proc file read operations
 */
void test_proc_file_read(void)
{
    TEST_START("/proc file read operations");

    int fd = rosetta_proc_open("/proc/cpuinfo", O_RDONLY);
    TEST_ASSERT(fd >= 0, "Should open /proc/cpuinfo");

    char buf[1024];
    ssize_t bytes = rosetta_proc_read(fd, buf, sizeof(buf) - 1);
    TEST_ASSERT(bytes > 0, "Should read from /proc/cpuinfo");
    buf[bytes] = '\0';  /* Null terminate */

    TEST_ASSERT(strlen(buf) > 0, "Should have read some content");
    TEST_ASSERT(strstr(buf, "processor") != NULL,
                "Content should contain cpuinfo data");

    rosetta_proc_close(fd);

    printf("(read %zd bytes)", bytes);

    TEST_PASS();
}

/**
 * Test 12: Multiple sequential reads
 */
void test_proc_sequential_reads(void)
{
    TEST_START("Multiple sequential reads");

    int fd = rosetta_proc_open("/proc/cpuinfo", O_RDONLY);
    TEST_ASSERT(fd >= 0, "Should open /proc/cpuinfo");

    char buf1[512];
    char buf2[512];

    ssize_t bytes1 = rosetta_proc_read(fd, buf1, sizeof(buf1));
    TEST_ASSERT(bytes1 > 0, "First read should succeed");

    ssize_t bytes2 = rosetta_proc_read(fd, buf2, sizeof(buf2));
    /* Second read should return 0 (EOF) since we read everything */

    rosetta_proc_close(fd);

    TEST_PASS();
}

/**
 * Test 13: Invalid /proc path handling
 */
void test_proc_invalid_paths(void)
{
    TEST_START("Invalid /proc path handling");

    rosetta_proc_path_t type = rosetta_proc_get_path_type("/proc/invalid_path");
    TEST_ASSERT(type == ROSETTA_PROC_UNKNOWN,
                "Unknown path should return UNKNOWN type");

    /* Try to open unknown path */
    int fd = rosetta_proc_open("/proc/invalid_path", O_RDONLY);
    TEST_ASSERT(fd < 0, "Should fail to open unknown /proc path");

    TEST_PASS();
}

/**
 * Test 14: /proc readlink operations
 */
void test_proc_readlink(void)
{
    TEST_START("/proc readlink operations");

    char buf[PATH_MAX];
    ssize_t len = rosetta_proc_readlink("/proc/self/exe", buf, sizeof(buf));

    TEST_ASSERT(len > 0, "Should readlink /proc/self/exe");
    TEST_ASSERT(buf[0] == '/', "Path should be absolute");
    buf[len] = '\0';  /* Null terminate */

    printf("(resolved to: %s)", buf);

    TEST_PASS();
}

/**
 * Test 15: memfd_create functionality
 */
void test_memfd_create(void)
{
    TEST_START("memfd_create functionality");

    int fd = rosetta_memfd_create("test_memfd", 0);
    TEST_ASSERT(fd >= 0, "Should create memory file descriptor");

    /* Try to write to it */
    const char *test_data = "Hello, memfd!";
    ssize_t written = write(fd, test_data, strlen(test_data));
    TEST_ASSERT(written > 0, "Should write to memfd");

    /* Try to read from it */
    char read_buf[64];
    lseek(fd, 0, SEEK_SET);
    ssize_t bytes_read = read(fd, read_buf, sizeof(read_buf));
    TEST_ASSERT(bytes_read > 0, "Should read from memfd");

    read_buf[bytes_read] = '\0';
    TEST_ASSERT(strcmp(read_buf, test_data) == 0,
                "Read data should match written data");

    close(fd);

    TEST_PASS();
}

/* ============================================================================
 * Integration Tests
 * ============================================================================ */

/**
 * Test 16: Full /proc file cycle
 */
void test_proc_full_cycle(void)
{
    TEST_START("Full /proc file cycle");

    /* Open */
    int fd = rosetta_proc_open("/proc/meminfo", O_RDONLY);
    TEST_ASSERT(fd >= 0, "Should open /proc/meminfo");

    /* Read */
    char buf[1024];
    ssize_t bytes = rosetta_proc_read(fd, buf, sizeof(buf) - 1);
    TEST_ASSERT(bytes > 0, "Should read content");
    buf[bytes] = '\0';

    /* Verify content */
    TEST_ASSERT(strstr(buf, "MemTotal") != NULL,
                "Should contain MemTotal");

    /* Close */
    int result = rosetta_proc_close(fd);
    TEST_ASSERT(result == 0, "Should close successfully");

    TEST_PASS();
}

/**
 * Test 17: Multiple file handles
 */
void test_proc_multiple_files(void)
{
    TEST_START("Multiple /proc file handles");

    int fd1 = rosetta_proc_open("/proc/cpuinfo", O_RDONLY);
    int fd2 = rosetta_proc_open("/proc/meminfo", O_RDONLY);
    int fd3 = rosetta_proc_open("/proc/self/status", O_RDONLY);

    TEST_ASSERT(fd1 >= 0 && fd2 >= 0 && fd3 >= 0,
                "Should open multiple /proc files");

    /* Read from all files */
    char buf[256];
    rosetta_proc_read(fd1, buf, sizeof(buf));
    rosetta_proc_read(fd2, buf, sizeof(buf));
    rosetta_proc_read(fd3, buf, sizeof(buf));

    /* Close all files */
    rosetta_proc_close(fd1);
    rosetta_proc_close(fd2);
    rosetta_proc_close(fd3);

    TEST_PASS();
}

/**
 * Test 18: /proc content accuracy
 */
void test_proc_content_accuracy(void)
{
    TEST_START("/proc content accuracy");

    char buf[1024];
    rosetta_proc_generate_cpuinfo(buf, sizeof(buf));

    /* Verify CPU info contains realistic data */
    TEST_ASSERT(strstr(buf, "processor\t: 0") != NULL,
                "Should have processor 0");
    TEST_ASSERT(strstr(buf, "fpu\t\t\t: yes") != NULL,
                "Should indicate FPU support");

    rosetta_proc_generate_meminfo(buf, sizeof(buf));

    /* Verify meminfo contains realistic data */
    TEST_ASSERT(strstr(buf, "MemTotal:\t") != NULL,
                "Should have MemTotal");
    TEST_ASSERT(strstr(buf, "MemFree:\t") != NULL,
                "Should have MemFree");

    TEST_PASS();
}

/**
 * Test 19: Large read handling
 */
void test_proc_large_read(void)
{
    TEST_START("Large read handling");

    int fd = rosetta_proc_open("/proc/cpuinfo", O_RDONLY);
    TEST_ASSERT(fd >= 0, "Should open /proc/cpuinfo");

    /* Try to read more than available */
    char buf[10000];
    ssize_t bytes = rosetta_proc_read(fd, buf, sizeof(buf));

    TEST_ASSERT(bytes > 0 && bytes < (ssize_t)sizeof(buf),
                "Should return partial data");
    TEST_ASSERT(bytes < 4096, "Should not return excessive data");

    rosetta_proc_close(fd);

    TEST_PASS();
}

/**
 * Test 20: /proc path edge cases
 */
void test_proc_edge_cases(void)
{
    TEST_START("/proc path edge cases");

    /* Test NULL path */
    TEST_ASSERT(rosetta_proc_is_proc_path(NULL) == 0,
                "NULL path should not be /proc path");

    /* Test empty string */
    TEST_ASSERT(rosetta_proc_is_proc_path("") == 0,
                "Empty string should not be /proc path");

    /* Test root /proc */
    TEST_ASSERT(rosetta_proc_is_proc_path("/proc") == 1,
                "/proc should be recognized as /proc path");

    TEST_PASS();
}

/* ============================================================================
 * Test Runner
 * ============================================================================ */

int main(int argc, char **argv)
{
    printf("=================================================================\n");
    printf("Rosetta /proc Filesystem Emulation Test Suite\n");
    printf("=================================================================\n\n");

    /* Run all tests */
    test_proc_path_detection();
    test_proc_path_type_detection();
    test_proc_cpuinfo_generation();
    test_proc_auxv_generation();
    test_proc_cmdline_generation();
    test_proc_exe_path();
    test_proc_meminfo_generation();
    test_proc_status_generation();
    test_proc_maps_generation();
    test_proc_file_operations();
    test_proc_file_read();
    test_proc_sequential_reads();
    test_proc_invalid_paths();
    test_proc_readlink();
    test_memfd_create();
    test_proc_full_cycle();
    test_proc_multiple_files();
    test_proc_content_accuracy();
    test_proc_large_read();
    test_proc_edge_cases();

    /* Print summary */
    printf("\n=================================================================\n");
    printf("Test Results:\n");
    printf("  Passed: %d\n", tests_passed);
    printf("  Failed: %d\n", tests_failed);
    printf("=================================================================\n");

    return (tests_failed > 0) ? 1 : 0;
}
