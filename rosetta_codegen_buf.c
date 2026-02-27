/* ============================================================================
 * Rosetta Binary Translator - Code Buffer Management
 * ============================================================================
 *
 * This module contains code buffer management functions.
 * Each function manages the code buffer for x86_64 machine code emission.
 * ============================================================================ */

#include "rosetta_codegen_buf.h"
#include <string.h>
#include <stdlib.h>

/* ============================================================================
 * Code Buffer Management
 * ============================================================================ */

int code_buffer_init(code_buffer_t *buf, u8 *buffer, u32 size) {
    if (!buf) return ROSETTA_ERR_INVAL;

    if (buffer) {
        buf->buffer = buffer;
        buf->owns_buffer = false;
    } else {
        buf->buffer = (u8 *)malloc(size);
        if (!buf->buffer) return ROSETTA_ERR_NOMEM;
        buf->owns_buffer = true;
    }

    buf->size = size;
    buf->offset = 0;
    buf->error = false;
    return ROSETTA_OK;
}

void code_buffer_cleanup(code_buffer_t *buf) {
    if (!buf) return;

    if (buf->owns_buffer && buf->buffer) {
        free(buf->buffer);
    }
    buf->buffer = NULL;
    buf->size = 0;
    buf->offset = 0;
}

void emit_byte(code_buffer_t *buf, u8 byte) {
    if (!buf || !buf->buffer) {
        return;
    }

    if (buf->offset >= buf->size) {
        buf->error = true;
        return;
    }

    buf->buffer[buf->offset++] = byte;
}

void emit_word32(code_buffer_t *buf, u32 word) {
    emit_byte(buf, (word >> 0) & 0xFF);
    emit_byte(buf, (word >> 8) & 0xFF);
    emit_byte(buf, (word >> 16) & 0xFF);
    emit_byte(buf, (word >> 24) & 0xFF);
}

void emit_word64(code_buffer_t *buf, u64 word) {
    emit_word32(buf, (u32)(word & 0xFFFFFFFF));
    emit_word32(buf, (u32)(word >> 32));
}

u8 *code_buffer_get_ptr(code_buffer_t *buf) {
    if (!buf) return NULL;
    return &buf->buffer[buf->offset];
}

u32 code_buffer_get_size(code_buffer_t *buf) {
    if (!buf) return 0;
    return buf->offset;
}

/* ============================================================================
 * Code Buffer Helper Functions
 * ============================================================================ */

void emit_bytes(code_buffer_t *buf, const u8 *data, u32 len) {
    u32 i;
    for (i = 0; i < len; i++) {
        emit_byte(buf, data[i]);
    }
}

void emit_align(code_buffer_t *buf, u32 alignment) {
    while (buf->offset & (alignment - 1)) {
        emit_byte(buf, 0x90);  /* NOP */
    }
}

void emit_nop_n(code_buffer_t *buf, u32 count) {
    u32 i;
    for (i = 0; i < count; i++) {
        emit_byte(buf, 0x90);  /* NOP */
    }
}

void emit_ud2(code_buffer_t *buf) {
    /* UD2: 0F 0B - Undefined instruction (for debugging) */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x0B);
}
