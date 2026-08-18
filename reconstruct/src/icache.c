#include "oah/icache.h"

#ifdef __aarch64__
#include <stddef.h>
#endif

u64 oah_icache_line_count(u64 addr, u64 size)
{
    /* FUN_8000000268b0: (size + (addr & 63) - 1) >> 6 , then loop while >= 0
     * so the iteration count is that value + 1. */
    if (size == 0) {
        return 0;
    }
    return ((size + (addr & (OAH_ICACHE_LINE - 1)) - 1) >> 6) + 1;
}

void oah_icache_invalidate(void *addr, u64 size)
{
    (void)addr;
    (void)size;
#ifdef __aarch64__
    u64 p = (u64)addr & ~(u64)(OAH_ICACHE_LINE - 1);
    u64 n = oah_icache_line_count((u64)addr, size);
    u64 i;
    __asm__ __volatile__("dsb ish" ::: "memory");
    for (i = 0; i < n; i++) {
        __asm__ __volatile__("ic ivau, %0" :: "r"(p) : "memory");
        p += OAH_ICACHE_LINE;
    }
    __asm__ __volatile__("dsb ish\n\tisb" ::: "memory");
#endif
}
