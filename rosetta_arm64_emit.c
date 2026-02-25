/* ============================================================================
 * Rosetta ARM64 Code Emitter
 * ============================================================================
 *
 * This module handles emission of ARM64 instructions for the JIT compiler.
 * Each function emits a specific ARM64 instruction encoding.
 * ============================================================================ */

#include "rosetta_arm64_emit.h"
#include <stdint.h>
#include <stddef.h>

/* Global code cache buffer */
static uint8_t g_code_cache[1024 * 1024];  /* 1MB code cache */
static size_t g_code_cache_offset = 0;

/* ============================================================================
 * Code Buffer Management
 * ============================================================================ */

/**
 * Initialize code buffer
 * @param buf Code buffer to initialize
 * @param buffer_ptr Pre-allocated buffer or NULL for internal cache
 * @param size Buffer size
 */
void code_buffer_init_arm64(code_buffer_t *buf, u8 *buffer_ptr, u32 size)
{
    buf->buffer = buffer_ptr ? buffer_ptr : g_code_cache + g_code_cache_offset;
    buf->size = size;
    buf->offset = 0;
    buf->error = false;
}

/**
 * Get code size
 * @param buf Code buffer
 * @return Current code size
 */
u32 code_buffer_get_size_arm64(code_buffer_t *buf)
{
    return buf->offset;
}

/* ============================================================================
 * Basic Emit Functions
 * ============================================================================ */

void emit_byte_arm64(code_buffer_t *buf, u8 byte)
{
    if (buf->offset >= buf->size) {
        buf->error = true;
        return;
    }
    buf->buffer[buf->offset++] = byte;
}

void emit_word32_arm64(code_buffer_t *buf, u32 word)
{
    emit_byte_arm64(buf, (word >> 0) & 0xFF);
    emit_byte_arm64(buf, (word >> 8) & 0xFF);
    emit_byte_arm64(buf, (word >> 16) & 0xFF);
    emit_byte_arm64(buf, (word >> 24) & 0xFF);
}

void emit_arm64_insn(code_buffer_t *buf, u32 insn)
{
    emit_byte_arm64(buf, (insn >> 0) & 0xFF);
    emit_byte_arm64(buf, (insn >> 8) & 0xFF);
    emit_byte_arm64(buf, (insn >> 16) & 0xFF);
    emit_byte_arm64(buf, (insn >> 24) & 0xFF);
}

/* ============================================================================
 * Register Mapping
 * ============================================================================ */

uint8_t map_x86_to_arm(uint8_t x86_reg)
{
    return x86_reg & 0x0F;  /* RAX-R15 -> X0-X15 */
}

uint8_t x86_map_gpr(uint8_t arm64_reg)
{
    return arm64_reg & 0x0F;  /* X0-X15 -> RAX-R15 */
}

/* ============================================================================
 * Data Processing (Register) - ALU Operations
 * ============================================================================ */

void emit_add_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    u32 insn = 0x0B000000;  /* ADD (shifted) */
    insn |= (u32)(dst & 31) << 0;
    insn |= (u32)(src1 & 31) << 5;
    insn |= (u32)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_sub_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    u32 insn = 0x4B000000;  /* SUB (shifted) */
    insn |= (u32)(dst & 31) << 0;
    insn |= (u32)(src1 & 31) << 5;
    insn |= (u32)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_and_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    u32 insn = 0x0A000000;  /* AND (shifted) */
    insn |= (u32)(dst & 31) << 0;
    insn |= (u32)(src1 & 31) << 5;
    insn |= (u32)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_and_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint16_t imm)
{
    u32 insn = 0x12000000;  /* AND immediate */
    insn |= (u32)(dst & 31) << 0;
    insn |= (u32)(src & 31) << 5;
    insn |= (u32)(imm & 0xFFFF) << 10;
    emit_arm64_insn(buf, insn);
}

void emit_orr_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    u32 insn = 0xAA000000;  /* ORR (shifted) */
    insn |= (u32)(dst & 31) << 0;
    insn |= (u32)(src1 & 31) << 5;
    insn |= (u32)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_eor_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    u32 insn = 0xCA000000;  /* EOR (shifted) */
    insn |= (u32)(dst & 31) << 0;
    insn |= (u32)(src1 & 31) << 5;
    insn |= (u32)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

void emit_mov_reg(code_buffer_t *buf, uint8_t dst, uint8_t src)
{
    /* MOV is alias for ORR with ZR */
    u32 insn = 0xAA000000;  /* ORR (shifted) */
    insn |= (u32)(dst & 31) << 0;
    insn |= (u32)(31 & 31) << 5;   /* XZR */
    insn |= (u32)(src & 31) << 16;
    emit_arm64_insn(buf, insn);
}

/* End of rosetta_arm64_emit.c */
