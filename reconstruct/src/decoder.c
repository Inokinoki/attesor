#include "oah/decoder.h"

void oah_decoder_init(oah_decoder *d, const u8 *bytes, size_t len)
{
    d->insn_start = bytes;
    d->cursor = bytes;
    d->end = bytes + len;
    d->error = false;
}

static bool at_limit(const oah_decoder *d)
{
    return d->cursor == d->end || (d->cursor - d->insn_start) == OAH_X86_MAX_INSN;
}

static u8 read_byte(oah_decoder *d)
{
    if (at_limit(d)) {
        d->error = true;
        return 0;
    }
    return *d->cursor++;
}

u64 oah_decoder_read_int(oah_decoder *d, u32 size)
{
    /* FUN_80000008d500 Decoder.cpp read_int — little-endian, size 0..3 */
    u64 v = 0;
    u32 nbytes;
    u32 i;

    switch (size & 0xff) {
    case 0:
        nbytes = 1;
        break;
    case 1:
        nbytes = 2;
        break;
    case 2:
        nbytes = 4;
        break;
    case 3:
        nbytes = 8;
        break;
    default:
        /* Decoder.cpp:0x56 "invalid integer size" */
        d->error = true;
        return 0;
    }

    for (i = 0; i < nbytes; i++) {
        v |= (u64)read_byte(d) << (8 * i);
    }
    return v;
}
