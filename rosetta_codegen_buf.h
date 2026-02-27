/* ============================================================================
 * Rosetta Binary Translator - Code Buffer Management Header
 * ============================================================================
 *
 * This header defines the interface for code buffer management.
 * ============================================================================ */

#ifndef ROSETTA_CODEGEN_BUF_H
#define ROSETTA_CODEGEN_BUF_H

#include "rosetta_types.h"

/* ============================================================================
 * Code Buffer Structure
 * ============================================================================ */

/* code_buffer_t is defined in rosetta_types.h */

/* Error codes are defined in rosetta_types.h */

/* ============================================================================
 * Code Buffer Management
 * ============================================================================ */

/**
 * code_buffer_init - Initialize a code buffer
 * @buf: Buffer structure to initialize
 * @buffer: Pre-allocated buffer (or NULL to allocate)
 * @size: Size of buffer
 * Returns: ROSETTA_OK on success, error code on failure
 */
int code_buffer_init(code_buffer_t *buf, u8 *buffer, u32 size);

/**
 * code_buffer_cleanup - Clean up a code buffer
 * @buf: Buffer structure to clean up
 */
void code_buffer_cleanup(code_buffer_t *buf);

/**
 * emit_byte - Emit a single byte
 * @buf: Code buffer
 * @byte: Byte to emit
 */
void emit_byte(code_buffer_t *buf, u8 byte);

/**
 * emit_word32 - Emit a 32-bit word (little-endian)
 * @buf: Code buffer
 * @word: Word to emit
 */
void emit_word32(code_buffer_t *buf, u32 word);

/**
 * emit_word64 - Emit a 64-bit word (little-endian)
 * @buf: Code buffer
 * @word: Word to emit
 */
void emit_word64(code_buffer_t *buf, u64 word);

/**
 * code_buffer_get_ptr - Get current write pointer
 * @buf: Code buffer
 * Returns: Pointer to current position
 */
u8 *code_buffer_get_ptr(code_buffer_t *buf);

/**
 * code_buffer_get_size - Get current buffer size
 * @buf: Code buffer
 * Returns: Number of bytes emitted
 */
u32 code_buffer_get_size(code_buffer_t *buf);

/* ============================================================================
 * Code Buffer Helper Functions
 * ============================================================================ */

/**
 * emit_bytes - Emit multiple bytes
 * @buf: Code buffer
 * @data: Data to emit
 * @len: Number of bytes
 */
void emit_bytes(code_buffer_t *buf, const u8 *data, u32 len);

/**
 * emit_align - Align buffer to specified boundary
 * @buf: Code buffer
 * @alignment: Alignment boundary (power of 2)
 */
void emit_align(code_buffer_t *buf, u32 alignment);

/**
 * emit_nop_n - Emit multiple NOPs
 * @buf: Code buffer
 * @count: Number of NOPs to emit
 */
void emit_nop_n(code_buffer_t *buf, u32 count);

/**
 * emit_ud2 - Emit UD2 (undefined instruction)
 * @buf: Code buffer
 */
void emit_ud2(code_buffer_t *buf);

#endif /* ROSETTA_CODEGEN_BUF_H */
