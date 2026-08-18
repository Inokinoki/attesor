#include "oah/decode_insn.h"
#include "oah/elf_map.h"
#include "oah/guest.h"
#include "oah/interp.h"
#include "oah/syscall_bridge.h"
#include "test_common.h"

#include <string.h>
#include <unistd.h>

static void image_identity(oah_image *img, u8 *buf, size_t n)
{
    memset(img, 0, sizeof(*img));
    img->fd = -1;
    img->nseg = 1;
    img->segs[0].gva = (u64)(uintptr_t)buf;
    img->segs[0].size = n;
    img->segs[0].host = buf;
    img->segs[0].prot = 3;
    img->entry = (u64)(uintptr_t)buf;
    img->load_end = img->entry + n;
    img->brk = img->load_end;
}

int main(void)
{
    CHECK_EQ_U64(oah_x86_syscall_to_host(OAH_X86_NR_WRITE),
#if defined(__aarch64__)
                 64
#else
                 OAH_X86_NR_WRITE
#endif
    );
    CHECK_EQ_U64(oah_x86_syscall_to_host(OAH_X86_NR_EXIT),
#if defined(__aarch64__)
                 93
#else
                 OAH_X86_NR_EXIT
#endif
    );

    {
        /* write(pipe, "hi", 2); exit(7) */
        int p[2];
        u8 buf[64];
        u8 *msg;
        oah_image img;
        oah_guest g;
        oah_interp_result r;
        u8 got[8];
        ssize_t n;
        u8 *c;

        CHECK(pipe(p) == 0);
        memset(buf, 0xcc, sizeof(buf));
        c = buf;
        /* mov eax, 1 */
        memcpy(c, "\xb8\x01\x00\x00\x00", 5);
        c += 5;
        /* mov edi, fd */
        *c++ = 0xbf;
        memcpy(c, &p[1], 4);
        c += 4;
        /* lea rsi, [rip+disp] — disp patched after we know msg offset */
        *c++ = 0x48;
        *c++ = 0x8d;
        *c++ = 0x35;
        u8 *disp_at = c;
        c += 4;
        /* mov edx, 2 */
        memcpy(c, "\xba\x02\x00\x00\x00", 5);
        c += 5;
        /* syscall */
        memcpy(c, "\x0f\x05", 2);
        c += 2;
        /* mov eax, 60 */
        memcpy(c, "\xb8\x3c\x00\x00\x00", 5);
        c += 5;
        /* mov edi, 7 */
        memcpy(c, "\xbf\x07\x00\x00\x00", 5);
        c += 5;
        /* syscall */
        memcpy(c, "\x0f\x05", 2);
        c += 2;
        msg = c;
        memcpy(msg, "hi", 2);
        {
            s32 rel = (s32)(msg - (disp_at + 4));
            memcpy(disp_at, &rel, 4);
        }

        image_identity(&img, buf, sizeof(buf));
        oah_guest_init(&g, (u64)(uintptr_t)buf, 0);
        r = oah_interp_run(&g, &img, 100);
        CHECK_EQ_U64(r.status, OAH_INTERP_EXIT);
        CHECK_EQ_U64((u64)r.exit_code, 7);
        close(p[1]);
        n = read(p[0], got, sizeof(got));
        CHECK_EQ_U64((u64)n, 2);
        CHECK(got[0] == 'h' && got[1] == 'i');
        close(p[0]);
    }

    {
        u8 buf[8] = {0x06, 0, 0, 0, 0, 0, 0, 0}; /* invalid */
        oah_image img;
        oah_guest g;
        oah_interp_result r;
        image_identity(&img, buf, sizeof(buf));
        oah_guest_init(&g, (u64)(uintptr_t)buf, 0);
        r = oah_interp_run(&g, &img, 10);
        CHECK_EQ_U64(r.status, OAH_INTERP_DECODE);
    }

    return test_report("interp");
}
