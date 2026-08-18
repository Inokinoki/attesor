#ifndef OAH_ICACHE_H
#define OAH_ICACHE_H

#include "oah/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OAH_ICACHE_LINE 64u

/*
 * decomp: FUN_8000000268b0  (was misnamed translation_lookup in the old map)
 *
 *   start = addr & ~63
 *   lines_minus_1 = (size + (addr & 63) - 1) >> 6
 *   DSB ISH; IC IVAU each line; DSB ISH; ISB
 *
 * On non-Apple-Silicon hosts this only computes the line count.
 */
u64 oah_icache_line_count(u64 addr, u64 size);
void oah_icache_invalidate(void *addr, u64 size);

#ifdef __cplusplus
}
#endif

#endif /* OAH_ICACHE_H */
