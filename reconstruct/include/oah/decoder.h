#ifndef OAH_DECODER_H
#define OAH_DECODER_H

#include "oah/types.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* x86 instruction max length. decomp FUN_80000008d500:
 *   (cursor - insn_start) == 0xf  → error */
#define OAH_X86_MAX_INSN 15

/*
 * orig: Decoder.cpp:0x56 read_int  "invalid integer size"
 * decomp: FUN_80000008d500
 *
 * Decoder cursor:
 *   end         +0x10
 *   insn_start  +0x18
 *   cursor      +0x20
 *   error       +0x28
 *
 * size: 0=8-bit, 1=16-bit LE, 2=32-bit LE, 3=64-bit LE.
 */
typedef struct {
    const u8 *end;
    const u8 *insn_start;
    const u8 *cursor;
    bool error;
} oah_decoder;

void oah_decoder_init(oah_decoder *d, const u8 *bytes, size_t len);

/* Returns 0 and sets error on overrun / size>15 / invalid size. */
u64 oah_decoder_read_int(oah_decoder *d, u32 size);

#ifdef __cplusplus
}
#endif

#endif /* OAH_DECODER_H */
