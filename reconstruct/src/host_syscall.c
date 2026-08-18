#include "oah/host_syscall.h"

#include "host_syscall_table.inc"

u32 oah_host_syscall_count(void)
{
    return OAH_HOST_SYSCALL_COUNT;
}

const char *oah_host_syscall_name(u32 nr)
{
    u32 i;
    for (i = 0; i < OAH_HOST_SYSCALL_COUNT; i++) {
        if (oah_host_syscall_table[i].nr == nr) {
            return oah_host_syscall_table[i].name;
        }
    }
    return 0;
}

bool oah_host_syscall_has_wrapper(u32 nr)
{
    return oah_host_syscall_name(nr) != 0;
}
