#include "oah/assembler.h"
#include "oah/elf_map.h"
#include "oah/guest.h"
#include "oah/translate.h"
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
    img->entry = (u64)(uintptr_t)buf;
    img->load_end = img->entry + n;
    img->brk = img->load_end;
}

int main(void)
{
    {
        u8 bytes[] = {0xb8, 0x01, 0x00, 0x00, 0x00, 0x90}; /* mov eax,1; nop */
        oah_image img;
        oah_asm_buf buf;
        u32 first;
        u64 next = 0;
        image_identity(&img, bytes, sizeof(bytes));
        oah_asm_buf_init(&buf);
        CHECK(oah_translate_block(&buf, &img, (u64)(uintptr_t)bytes, &next));
        CHECK(buf.size >= 8);
        memcpy(&first, buf.data, 4);
        CHECK_EQ_U64(first, 0x52800028u); /* movz w8, #1 */
        oah_asm_buf_destroy(&buf);
    }

    {
        int p[2];
        u8 buf[64];
        u8 *c, *msg, *disp_at;
        oah_image img;
        oah_guest g;
        oah_interp_result r;
        u8 got[8];
        ssize_t n;
        s32 rel;

        CHECK(pipe(p) == 0);
        memset(buf, 0xcc, sizeof(buf));
        c = buf;
        memcpy(c, "\xb8\x01\x00\x00\x00", 5);
        c += 5;
        *c++ = 0xbf;
        memcpy(c, &p[1], 4);
        c += 4;
        *c++ = 0x48;
        *c++ = 0x8d;
        *c++ = 0x35;
        disp_at = c;
        c += 4;
        memcpy(c, "\xba\x02\x00\x00\x00", 5);
        c += 5;
        memcpy(c, "\x0f\x05", 2);
        c += 2;
        memcpy(c, "\xb8\x3c\x00\x00\x00", 5);
        c += 5;
        memcpy(c, "\xbf\x07\x00\x00\x00", 5);
        c += 5;
        memcpy(c, "\x0f\x05", 2);
        c += 2;
        msg = c;
        memcpy(msg, "hi", 2);
        rel = (s32)(msg - (disp_at + 4));
        memcpy(disp_at, &rel, 4);

        image_identity(&img, buf, sizeof(buf));
        oah_guest_init(&g, (u64)(uintptr_t)buf, 0);
        r = oah_jit_run(&g, &img, 10000);
        CHECK_EQ_U64(r.status, OAH_INTERP_EXIT);
        CHECK_EQ_U64((u64)r.exit_code, 7);
        close(p[1]);
        n = read(p[0], got, sizeof(got));
        CHECK_EQ_U64((u64)n, 2);
        CHECK(got[0] == 'h' && got[1] == 'i');
        close(p[0]);
    }

    {
        /* Host remap: cmp x8,#1 / movz x8,#64 before svc. */
        oah_asm_buf buf;
        u32 w;
        unsigned i;
        int saw_cmp1 = 0;
        int saw_mov64 = 0;
        int saw_svc = 0;

        oah_asm_buf_init(&buf);
        CHECK(oah_emit_runtime_syscall_host(&buf));
        CHECK((buf.size & 3u) == 0);
        for (i = 0; i + 4 <= buf.size; i += 4) {
            memcpy(&w, buf.data + i, 4);
            if (w == 0xf100051fu) {
                saw_cmp1 = 1;
            }
            if (w == 0xd2800808u) {
                saw_mov64 = 1;
            }
            if (w == 0xd4000001u) {
                saw_svc = 1;
            }
        }
        CHECK(saw_cmp1);
        CHECK(saw_mov64);
        CHECK(saw_svc);
        oah_asm_buf_destroy(&buf);
    }

    return test_report("translate");
}
