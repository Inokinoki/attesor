#include "oah/guest.h"

#include <string.h>

void oah_guest_init(oah_guest *g, u64 entry, u64 rsp)
{
    memset(g, 0, sizeof(*g));
    g->rip = entry;
    g->gpr[OAH_X86_RSP] = rsp;
    g->rflags = OAH_RFLAGS_INIT;
}
