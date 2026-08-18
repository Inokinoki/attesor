#include "oah/assembler_buffer.h"

#include <stdlib.h>
#include <string.h>

void oah_asm_buf_init(oah_asm_buf *buf)
{
    buf->data = NULL;
    buf->size = 0;
    buf->capacity = 0;
    buf->custom_alloc = 0;
}

void oah_asm_buf_destroy(oah_asm_buf *buf)
{
    free(buf->data);
    buf->data = NULL;
    buf->size = 0;
    buf->capacity = 0;
}

static void oah_asm_buf_grow(oah_asm_buf *buf)
{
    /* FUN_80000002a62c */
    u64 new_cap = 0x4000;
    if (buf->capacity != 0) {
        new_cap = buf->capacity << 1;
    }
    u8 *fresh = (u8 *)malloc((size_t)new_cap);
    if (!fresh) {
        return;
    }
    if (buf->data) {
        memcpy(fresh, buf->data, (size_t)buf->size);
        free(buf->data);
    }
    buf->data = fresh;
    buf->capacity = new_cap;
}

void oah_asm_emit32(oah_asm_buf *buf, u32 insn)
{
    /* FUN_8000000278c4 */
    if (buf->size + 4 > buf->capacity) {
        oah_asm_buf_grow(buf);
    }
    if (!buf->data || buf->size + 4 > buf->capacity) {
        return;
    }
    memcpy(buf->data + buf->size, &insn, 4);
    buf->size += 4;
}
