#include "oah/host_syscall.h"
#include "oah/result.h"
#include "test_common.h"

int main(void)
{
    CHECK_EQ_U64(oah_host_syscall_count(), 76);

    CHECK_STREQ(oah_host_syscall_name(56), "openat");
    CHECK_STREQ(oah_host_syscall_name(62), "lseek");
    CHECK_STREQ(oah_host_syscall_name(63), "read");
    CHECK_STREQ(oah_host_syscall_name(78), "readlinkat");
    CHECK_STREQ(oah_host_syscall_name(279), "memfd_create");
    CHECK_STREQ(oah_host_syscall_name(222), "mmap");
    CHECK_STREQ(oah_host_syscall_name(0), "io_setup");
    CHECK_STREQ(oah_host_syscall_name(288), "pkey_mprotect");
    CHECK_STREQ(oah_host_syscall_name(191), "mq_getsetattr");
    CHECK_STREQ(oah_host_syscall_name(195), "semget");

    CHECK(oah_host_syscall_has_wrapper(56));
    CHECK(!oah_host_syscall_has_wrapper(172)); /* getpid is not a Result wrapper */
    CHECK(!oah_host_syscall_has_wrapper(999));
    CHECK(oah_host_syscall_name(172) == 0);

    {
        oah_result ok = oah_result_from_linux_ret(7);
        CHECK(!oah_result_is_error(ok));
        CHECK_EQ_U64(ok.value, 7);
        CHECK_EQ_U64(oah_raw_from_linux_ret(7), 7);

        oah_result err = oah_result_from_linux_ret((u64)-2);
        CHECK(oah_result_is_error(err));
        CHECK_EQ_U64(err.value, 2);
        CHECK_EQ_U64(oah_raw_from_linux_ret((u64)-2), (u64)-1);

        /* -4095 is still errno; -4096 is a success-sized return */
        CHECK(oah_result_is_error(oah_result_from_linux_ret((u64)-4095)));
        CHECK_EQ_U64(oah_raw_from_linux_ret((u64)-4095), (u64)-1);
        CHECK(!oah_result_is_error(oah_result_from_linux_ret((u64)-4096)));
        CHECK_EQ_U64(oah_raw_from_linux_ret((u64)-4096), (u64)-4096);
    }

    return test_report("syscalls");
}
