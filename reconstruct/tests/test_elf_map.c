#include "oah/elf_map.h"
#include "test_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    {
        oah_image img;
        CHECK(!oah_map_elf(&img, "/no/such/file"));
    }

    if (access("bin/rosetta", R_OK) == 0) {
        oah_image img;
        CHECK(!oah_map_elf(&img, "bin/rosetta"));
    }

    {
        char path[] = "/tmp/oah-helloXXXXXX";
        char cmd[256];
        int fd = mkstemp(path);
        oah_image img;
        CHECK(fd >= 0);
        close(fd);
        snprintf(cmd, sizeof(cmd), "python3 tools/mk_hello_elf.py %s", path);
        CHECK_EQ_U64((u64)system(cmd), 0);
        CHECK(oah_map_elf(&img, path));
        CHECK(img.nseg >= 1);
        CHECK_EQ_U64(img.segs[0].gva, 0x400000);
        CHECK(oah_guest_to_host(&img, img.entry, 2) != 0);
        oah_unmap_elf(&img);
        unlink(path);
    }

    return test_report("elf_map");
}
