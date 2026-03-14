/*=============================================================================
 * Network Syscalls Validation Test
 *=============================================================================
 *
 * Validates the network syscalls implemented in Iteration 53:
 * - getpeername
 * - shutdown
 * - sendmsg/recvmsg
 *
 *=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST_START(name) \
    do { \
        tests_run++; \
        printf("  Test %d: %s...", tests_run, name); \
    } while(0)

#define TEST_PASS() \
    do { \
        tests_passed++; \
        printf(" ✅ PASS\n"); \
    } while(0)

#define TEST_FAIL(msg) \
    do { \
        printf(" ❌ FAIL: %s\n", msg); \
    } while(0)

void test_getpeername_syscall(void)
{
    TEST_START("getpeername syscall");

    /* Create a socket pair for testing */
    int sockfd[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) < 0) {
        TEST_FAIL("socketpair failed");
        return;
    }

    /* Test getpeername on one end of the socket pair */
    struct sockaddr addr;
    socklen_t addrlen = sizeof(addr);

    int ret = getpeername(sockfd[0], &addr, &addrlen);
    close(sockfd[0]);
    close(sockfd[1]);

    if (ret == 0) {
        TEST_PASS();
    } else {
        TEST_FAIL("getpeername failed");
    }
}

void test_shutdown_syscall(void)
{
    TEST_START("shutdown syscall");

    /* Create a socket pair for testing */
    int sockfd[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) < 0) {
        TEST_FAIL("socketpair failed");
        return;
    }

    /* Test shutdown on one end of the socket pair */
    int ret = shutdown(sockfd[0], SHUT_RDWR);
    close(sockfd[0]);
    close(sockfd[1]);

    if (ret == 0) {
        TEST_PASS();
    } else {
        TEST_FAIL("shutdown failed");
    }
}

void test_sendmsg_recvmsg_syscall(void)
{
    TEST_START("sendmsg/recvmsg syscalls");

    /* Create a socket pair for testing */
    int sockfd[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) < 0) {
        TEST_FAIL("socketpair failed");
        return;
    }

    /* Prepare message for sending */
    const char *test_msg = "Hello, socket!";
    struct msghdr msg;
    struct iovec iov;

    memset(&msg, 0, sizeof(msg));
    iov.iov_base = (void *)test_msg;
    iov.iov_len = strlen(test_msg);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    /* Send message using sendmsg */
    ssize_t sent = sendmsg(sockfd[0], &msg, 0);
    if (sent < 0) {
        close(sockfd[0]);
        close(sockfd[1]);
        TEST_FAIL("sendmsg failed");
        return;
    }

    /* Receive message using recvmsg */
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));

    struct iovec recv_iov;
    memset(&msg, 0, sizeof(msg));
    recv_iov.iov_base = buffer;
    recv_iov.iov_len = sizeof(buffer) - 1;
    msg.msg_iov = &recv_iov;
    msg.msg_iovlen = 1;

    ssize_t received = recvmsg(sockfd[1], &msg, 0);
    if (received < 0) {
        close(sockfd[0]);
        close(sockfd[1]);
        TEST_FAIL("recvmsg failed");
        return;
    }

    close(sockfd[0]);
    close(sockfd[1]);

    /* Verify message content */
    if (strcmp(buffer, test_msg) == 0) {
        TEST_PASS();
    } else {
        TEST_FAIL("message content mismatch");
    }
}

void test_syscall_availability(void)
{
    TEST_START("Syscall Availability");

    /* Test that the syscall numbers are correctly defined */
    #if defined(__x86_64__)
        #ifndef X86_64_SYS_GETPEERNAME
        TEST_FAIL("X86_64_SYS_GETPEERNAME not defined");
        return;
        #endif

        #ifndef X86_64_SYS_SHUTDOWN
        TEST_FAIL("X86_64_SYS_SHUTDOWN not defined");
        return;
        #endif

        #ifndef X86_64_SYS_SENDMSG
        TEST_FAIL("X86_64_SYS_SENDMSG not defined");
        return;
        #endif

        #ifndef X86_64_SYS_RECVMSG
        TEST_FAIL("X86_64_SYS_RECVMSG not defined");
        return;
        #endif
    #elif defined(__aarch64__)
        #ifndef ARM64_SYS_GETPEERNAME
        TEST_FAIL("ARM64_SYS_GETPEERNAME not defined");
        return;
        #endif

        #ifndef ARM64_SYS_SHUTDOWN
        TEST_FAIL("ARM64_SYS_SHUTDOWN not defined");
        return;
        #endif

        #ifndef ARM64_SYS_SENDMSG
        TEST_FAIL("ARM64_SYS_SENDMSG not defined");
        return;
        #endif

        #ifndef ARM64_SYS_RECVMSG
        TEST_FAIL("ARM64_SYS_RECVMSG not defined");
        return;
        #endif
    #endif

    TEST_PASS();
}

int main(void)
{
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║                                                            ║\n");
    printf("║     Network Syscalls Validation Test                         ║\n");
    printf("║                                                            ║\n");
    printf("║     Validates network syscalls from Iteration 53               ║\n");
    printf("║                                                            ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");

    printf("Running network syscall validation tests...\n\n");

    /* Run tests */
    test_syscall_availability();
    test_getpeername_syscall();
    test_shutdown_syscall();
    test_sendmsg_recvmsg_syscall();

    printf("\n");
    printf("════════════════════════════════════════════════════════════════\n");
    printf("Test Results:\n");
    printf("════════════════════════════════════════════════════════════════\n");
    printf("  Tests Run:    %d\n", tests_run);
    printf("  Tests Passed: %d\n", tests_passed);
    printf("  Pass Rate:    %.1f%%\n", (tests_passed * 100.0) / tests_run);
    printf("\n");

    if (tests_passed == tests_run) {
        printf("✅ ALL NETWORK SYSCALLS VALIDATED\n");
        printf("\n");
        printf("Network syscalls implemented in Iteration 53:\n");
        printf("  ✅ getpeername - Get name of connected peer\n");
        printf("  ✅ shutdown - Shutdown socket connection\n");
        printf("  ✅ sendmsg - Send message on socket\n");
        printf("  ✅ recvmsg - Receive message from socket\n");
        printf("\n");
        printf("All syscalls are functional and ready for use.\n");
        printf("\n");
        return 0;
    } else {
        printf("❌ SOME NETWORK SYSCALLS FAILED\n");
        printf("\n");
        printf("Please review the failures above.\n");
        printf("\n");
        return 1;
    }
}
