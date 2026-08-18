#include "oah/bitmask_imm.h"

/* ARM ARM bitmask-immediate decode. Used both to implement encode (search)
 * and to test. The original FUN_80000002a6bc is a bit-scan of the same space. */

static u64 ror_n(u64 x, unsigned rot, unsigned width)
{
    rot %= width;
    u64 mask = (width == 64) ? ~0ULL : ((1ULL << width) - 1ULL);
    x &= mask;
    if (rot == 0) {
        return x;
    }
    return ((x >> rot) | (x << (width - rot))) & mask;
}

static u64 replicate(u64 val, unsigned esize, unsigned width)
{
    u64 r = 0;
    unsigned i;
    u64 mask = (width == 64) ? ~0ULL : 0xffffffffULL;
    for (i = 0; i < width; i += esize) {
        r |= val << i;
    }
    return r & mask;
}

static int highest_set_bit(u32 x)
{
    if (x == 0) {
        return -1;
    }
    return 31 - __builtin_clz(x);
}

static bool decode_bitmask(u8 N, u8 immr, u8 imms, bool is_64, u64 *out)
{
    unsigned width = is_64 ? 64 : 32;
    int len = highest_set_bit((u32)((N ? 0x40 : 0) | (~imms & 0x3f)));
    unsigned levels, esize, S, R;
    u64 welem;

    if (len < 1) {
        return false;
    }
    if (!is_64 && (N || len > 5)) {
        return false;
    }
    levels = (1u << (unsigned)len) - 1u;
    esize = 1u << ((unsigned)len + 0); /* 2^len ? ARM: esize = 2^(len+1) wait */

    /* ARM pseudocode:
     *   len = HighestSetBit(N:NOT(imms))
     *   levels = ZeroExtend(Ones(len), 6)   // len bits of 1
     *   if len < 1 then UNDEFINED
     *   S = imms AND levels
     *   R = immr AND levels
     *   esize = 1 << (len+1) is WRONG
     *
     * Actual: esize = 1 << len  where len is HighestSetBit of the 7-bit
     * pattern, and the element size is 2^len? Let's use:
     *   esize = 1u << (len)
     * No: HighestSetBit of 0b1xxxxxx gives len=6 → esize=64 = 2^6.
     * HighestSetBit of 0b01xxxxx (N=0, imms with top 0) len=5 → esize=32.
     * So esize = 1 << len. And we need len>=1 so min esize=2.
     */
    esize = 1u << (unsigned)len;
    if (esize > width) {
        return false;
    }
    S = (unsigned)imms & levels;
    R = (unsigned)immr & levels;
    if (S == levels) {
        /* all-ones element is reserved */
        return false;
    }
    welem = (1ULL << (S + 1)) - 1ULL;
    *out = replicate(ror_n(welem, R, esize), esize, width);
    return true;
}

bool oah_is_bitmask_immediate(bool is_64, u64 value, oah_bitmask_imm *out)
{
    /* FUN_80000002a6bc: 64-bit when param_1==1; reject values whose
     * truncated form is 0 or ~0 of the element size. */
    unsigned n_max = is_64 ? 2 : 1;
    unsigned n, immr, imms;

    if (!is_64) {
        value &= 0xffffffffULL;
    }
    if (value == 0 || value == (is_64 ? ~0ULL : 0xffffffffULL)) {
        return false;
    }

    for (n = 0; n < n_max; n++) {
        for (imms = 0; imms < 64; imms++) {
            for (immr = 0; immr < 64; immr++) {
                u64 decoded;
                if (!decode_bitmask((u8)n, (u8)immr, (u8)imms, is_64, &decoded)) {
                    continue;
                }
                if (decoded == value) {
                    if (out) {
                        out->N = (u8)n;
                        out->immr = (u8)immr;
                        out->imms = (u8)imms;
                    }
                    return true;
                }
            }
        }
    }
    return false;
}
