#include "oah/linux_abi.h"
#include "test_common.h"

#include <stdlib.h>
#include <string.h>

int main(void)
{
    int pid = 4242;
    oah_proc_path_match m;

    m = oah_classify_proc_path("/proc/self/exe", OAH_PROC_PATH_EXE, pid);
    CHECK(m.matched && !m.is_thread_self);

    m = oah_classify_proc_path("/proc/thread-self/auxv", OAH_PROC_PATH_AUXV, pid);
    CHECK(m.matched && m.is_thread_self);

    m = oah_classify_proc_path("/proc/4242/cmdline", OAH_PROC_PATH_CMDLINE, pid);
    CHECK(m.matched && !m.is_thread_self);

    m = oah_classify_proc_path("/proc/4242/task/4242/exe", OAH_PROC_PATH_EXE, pid);
    CHECK(m.matched && m.is_thread_self);

    m = oah_classify_proc_path("/proc/cpuinfo", OAH_PROC_PATH_EXE, pid);
    CHECK(!m.matched);

    m = oah_classify_proc_path(NULL, OAH_PROC_PATH_EXE, pid);
    CHECK(!m.matched);

    /* Low 14 bits unchanged. */
    CHECK_EQ_U64(oah_translate_open_flags(0x3fff), 0x3fff);
    /* x86 O_DIRECTORY 0x10000 → ARM 0x4000 */
    CHECK_EQ_U64(oah_translate_open_flags(0x10000), 0x4000);
    /* x86 O_NOFOLLOW 0x20000 → ARM 0x8000 */
    CHECK_EQ_U64(oah_translate_open_flags(0x20000), 0x8000);
    /* x86 O_DIRECT 0x4000 → ARM 0x10000 */
    CHECK_EQ_U64(oah_translate_open_flags(0x4000), 0x10000);
    /* x86 O_LARGEFILE 0x8000 → ARM 0x20000 */
    CHECK_EQ_U64(oah_translate_open_flags(0x8000), 0x20000);
    /* O_CLOEXEC (bit 19 on both) stays */
    CHECK_EQ_U64(oah_translate_open_flags(0x80000), 0x80000);

    {
        char *buf = NULL;
        size_t sz = 0;
        FILE *f = open_memstream(&buf, &sz);
        CHECK(f != NULL);
        CHECK(oah_write_fake_cpuinfo(f, 2) == 0);
        fclose(f);
        CHECK(buf != NULL);
        CHECK(strstr(buf, "vendor_id\t: VirtualApple") != NULL);
        CHECK(strstr(buf, "cpu MHz\t\t: 2502.057") != NULL);
        CHECK(strstr(buf, "processor\t: 0") != NULL);
        CHECK(strstr(buf, "processor\t: 1") != NULL);
        CHECK(strstr(buf, "siblings\t: 2") != NULL);
        free(buf);
    }

    return test_report("linux_abi");
}
