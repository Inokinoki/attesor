#include "oah/condition.h"
#include "oah/host_syscall.h"
#include "test_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Optional: if reconstruct/bin/rosetta is present, memcmp reconstructed
 * tables against the ELF. CI without the binary still passes.
 *
 * VA 0x800000000000 is identity-mapped, so file offset = VA - base for
 * PT_LOAD contents.
 */
#define OAH_ELF_BASE 0x800000000000ULL
#define OAH_CC_TABLE_VA 0x800000012e1cULL
#define OAH_TEXT_VA 0x800000026000ULL
#define OAH_TEXT_SIZE 0x6c2a0u
#define OAH_SVC0 0xd4000001u
#define OAH_CMN_X0_FFF 0xb13ffc1fu

static u32 le32(const u8 *p)
{
    return (u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24);
}

static int decode_movz_x8(u32 w, u32 *nr)
{
    if ((w >> 23) == 0x1a5 && (w & 31) == 8) {
        unsigned hw = (w >> 21) & 3;
        *nr = ((w >> 5) & 0xffffu) << (hw * 16);
        return 1;
    }
    return 0;
}

static u8 *load_file(const char *path, size_t *len)
{
    FILE *f;
    u8 *buf;
    long n;

    f = fopen(path, "rb");
    if (!f) {
        return 0;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    n = ftell(f);
    if (n <= 0) {
        fclose(f);
        return 0;
    }
    rewind(f);
    buf = (u8 *)malloc((size_t)n);
    if (!buf || fread(buf, 1, (size_t)n, f) != (size_t)n) {
        free(buf);
        fclose(f);
        return 0;
    }
    fclose(f);
    *len = (size_t)n;
    return buf;
}

int main(void)
{
    size_t len = 0;
    u8 *elf = load_file("bin/rosetta", &len);
    const u8 *cc;
    size_t text_off;
    size_t off;
    u32 found[512];
    unsigned nfound = 0;
    unsigned i;

    if (!elf) {
        printf("binary: skipped (no bin/rosetta)\n");
        return 0;
    }

    CHECK(len > (size_t)(OAH_CC_TABLE_VA - OAH_ELF_BASE) + 16);
    cc = elf + (size_t)(OAH_CC_TABLE_VA - OAH_ELF_BASE);
    CHECK(memcmp(cc, oah_cc_arm_table, 16) == 0);

    text_off = (size_t)(OAH_TEXT_VA - OAH_ELF_BASE);
    CHECK(len >= text_off + OAH_TEXT_SIZE);
    for (off = text_off; off + 12 <= text_off + OAH_TEXT_SIZE; off += 4) {
        u32 nr;
        if (decode_movz_x8(le32(elf + off), &nr) &&
            le32(elf + off + 4) == OAH_SVC0 &&
            le32(elf + off + 8) == OAH_CMN_X0_FFF) {
            unsigned seen = 0;
            for (i = 0; i < nfound; i++) {
                if (found[i] == nr) {
                    seen = 1;
                    break;
                }
            }
            if (!seen) {
                CHECK(nfound < 512);
                found[nfound++] = nr;
            }
        }
    }

    CHECK_EQ_U64(nfound, oah_host_syscall_count());
    for (i = 0; i < nfound; i++) {
        CHECK(oah_host_syscall_has_wrapper(found[i]));
    }

    free(elf);
    return test_report("binary");
}
