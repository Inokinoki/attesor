#ifndef OAH_ELF_MAP_H
#define OAH_ELF_MAP_H

#include "oah/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OAH_MAX_LOAD_SEGS 16

typedef struct {
    u64 gva;     /* guest virtual address of mapping */
    u64 size;    /* bytes, page-rounded */
    u8 *host;    /* host pointer; identity when MAP_FIXED at gva */
    u32 prot;    /* PROT_* */
} oah_load_seg;

typedef struct {
    oah_load_seg segs[OAH_MAX_LOAD_SEGS];
    unsigned nseg;
    u64 entry;
    u64 phdr;        /* guest VA of program headers */
    u16 phnum;
    u16 phentsize;
    u64 load_end;    /* first byte after last PT_LOAD (for brk) */
    u64 brk;
    int fd;
} oah_image;

/*
 * orig: ElfMapper.cpp:399 / 401 map_elf
 * decomp: FUN_8000000900a0
 *
 *   first PT_LOAD file offset == 0
 *   first PT_LOAD file size covers ELF + program headers
 *
 * Only ET_EXEC/ET_DYN, EM_X86_64, ELF64 LSB. Maps PT_LOAD at p_vaddr
 * (ET_EXEC) or at a chosen slide (ET_DYN) so guest VA == host address.
 */
bool oah_map_elf(oah_image *img, const char *path);
void oah_unmap_elf(oah_image *img);

/* Translate a guest pointer; NULL if not in a PT_LOAD mapping. */
void *oah_guest_to_host(const oah_image *img, u64 gva, u64 len);

#ifdef __cplusplus
}
#endif

#endif /* OAH_ELF_MAP_H */
