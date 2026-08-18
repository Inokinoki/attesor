#ifndef OAH_ASSEMBLER_BUFFER_H
#define OAH_ASSEMBLER_BUFFER_H

#include "oah/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * AssemblerBuffer — decomp FUN_8000000278c4 / FUN_80000002a62c
 *
 * Layout (qword slots), recovered from field uses:
 *   [0] data pointer
 *   [1] current size in bytes
 *   [2] capacity in bytes
 *   [3] allocator tag (0 = malloc/free, nonzero = custom)
 *
 * Grow: capacity 0 → 0x4000, else double. Copies with memmove
 * (FUN_8000000268f0).
 */
typedef struct {
    u8 *data;
    u64 size;
    u64 capacity;
    s32 custom_alloc;
} oah_asm_buf;

void oah_asm_buf_init(oah_asm_buf *buf);
void oah_asm_buf_destroy(oah_asm_buf *buf);

/* orig: emit of a 32-bit ARM instruction. decomp: FUN_8000000278c4 */
void oah_asm_emit32(oah_asm_buf *buf, u32 insn);

#ifdef __cplusplus
}
#endif

#endif /* OAH_ASSEMBLER_BUFFER_H */
