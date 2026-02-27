/* ============================================================================
 * Rosetta Translator - Bitfield Instruction Translation Header
 * ============================================================================
 *
 * This module provides translation functions for ARM64 bitfield
 * instructions to x86_64 machine code.
 *
 * Supported instructions:
 * - BFM  - Bitfield Move
 * - BFI  - Bitfield Insert
 * - UBFX - Unsigned Bitfield Extract
 * - SBFX - Signed Bitfield Extract
 * - UBFIZ - Unsigned Bitfield Insert with Zero-extend
 * - SBFIZ - Signed Bitfield Insert with Zero-extend
 * - UXTB - Unsigned Extend Byte
 * - UXTH - Unsigned Extend Halfword
 * - SXTB - Signed Extend Byte
 * - SXTH - Signed Extend Halfword
 * - ROR  - Rotate Right
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_BITFIELD_H
#define ROSETTA_TRANSLATE_BITFIELD_H

#include <stdint.h>
#include <stddef.h>
#include "rosetta_emit_x86.h"

/* Bitfield instruction encoding masks */
#define BITFIELD_BFM_MASK       0x7F800000
#define BITFIELD_BFM_VAL        0x33000000
#define BITFIELD_BFI_MASK       0x7F800000
#define BITFIELD_BFI_VAL        0x33000000
#define BITFIELD_UBFX_MASK      0x7F800000
#define BITFIELD_UBFX_VAL       0x53000000
#define BITFIELD_SBFX_MASK      0x7F800000
#define BITFIELD_SBFX_VAL       0x13000000
#define BITFIELD_UBFIZ_MASK     0x7F800000
#define BITFIELD_UBFIZ_VAL      0x73000000
#define BITFIELD_SBFIZ_MASK     0x7F800000
#define BITFIELD_SBFIZ_VAL      0x13000000
#define BITFIELD_ROR_MASK       0x7F800000
#define BITFIELD_ROR_VAL        0x13800000
#define BITFIELD_EXTR_MASK      0x7F800000
#define BITFIELD_EXTR_VAL       0x13800000

/* Extract instruction masks */
#define EXTRACT_UBFX_MASK       0x7F800000
#define EXTRACT_UBFX_VAL        0x53000000
#define EXTRACT_SBFX_MASK       0x7F800000
#define EXTRACT_SBFX_VAL        0x13000000
#define EXTRACT_UBFXW_MASK      0x7F800000
#define EXTRACT_UBFXW_VAL       0x53400000
#define EXTRACT_SBFXW_MASK      0x7F800000
#define EXTRACT_SBFXW_VAL       0x13400000

/* Extract field instruction masks */
#define EXTFIELD_BFM_MASK       0x7F800000
#define EXTFIELD_BFM_VAL        0x33000000
#define EXTFIELD_BFMW_MASK      0x7F800000
#define EXTFIELD_BFMW_VAL       0x33400000

/* Rotate instruction masks */
#define ROTATE_ROR_MASK         0x7F800000
#define ROTATE_ROR_VAL          0x13800000
#define ROTATE_RORW_MASK        0x7F800000
#define ROTATE_RORW_VAL         0x13800000

/* Extract register instruction masks */
#define EXTRACT_REG_EXTR_MASK   0x7F800000
#define EXTRACT_REG_EXTR_VAL    0x13800000

/* Bitfield move variants */
#define BITFIELD_MOVE_BFM       0x00
#define BITFIELD_MOVE_BFMW      0x01

/* ============================================================================
 * Bitfield Move Instructions
 * ============================================================================ */

/**
 * translate_bitfield_bfm - Translate BFM (Bitfield Move) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * Returns: 0 on success
 */
int translate_bitfield_bfm(uint32_t encoding, code_buf_t *code_buf,
                           uint64_t *guest_state);

/**
 * translate_bitfield_bfmw - Translate BFMW (32-bit Bitfield Move) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * Returns: 0 on success
 */
int translate_bitfield_bfmw(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state);

/* ============================================================================
 * Bitfield Insert Instructions
 * ============================================================================ */

/**
 * translate_bitfield_bfi - Translate BFI (Bitfield Insert) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * Returns: 0 on success
 */
int translate_bitfield_bfi(uint32_t encoding, code_buf_t *code_buf,
                           uint64_t *guest_state);

/* ============================================================================
 * Bitfield Extract Instructions
 * ============================================================================ */

/**
 * translate_bitfield_ubfx - Translate UBFX (Unsigned Bitfield Extract)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * Returns: 0 on success
 */
int translate_bitfield_ubfx(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state);

/**
 * translate_bitfield_sbfx - Translate SBFX (Signed Bitfield Extract)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * Returns: 0 on success
 */
int translate_bitfield_sbfx(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state);

/**
 * translate_bitfield_ubfxw - Translate UBFXW (32-bit Unsigned Bitfield Extract)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * Returns: 0 on success
 */
int translate_bitfield_ubfxw(uint32_t encoding, code_buf_t *code_buf,
                             uint64_t *guest_state);

/**
 * translate_bitfield_sbfxw - Translate SBFXW (32-bit Signed Bitfield Extract)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * Returns: 0 on success
 */
int translate_bitfield_sbfxw(uint32_t encoding, code_buf_t *code_buf,
                             uint64_t *guest_state);

/* ============================================================================
 * Bitfield Insert with Zero-extend Instructions
 * ============================================================================ */

/**
 * translate_bitfield_ubfiz - Translate UBFIZ (Unsigned Bitfield Insert Z-ext)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * Returns: 0 on success
 */
int translate_bitfield_ubfiz(uint32_t encoding, code_buf_t *code_buf,
                             uint64_t *guest_state);

/**
 * translate_bitfield_sbfiz - Translate SBFIZ (Signed Bitfield Insert Z-ext)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * Returns: 0 on success
 */
int translate_bitfield_sbfiz(uint32_t encoding, code_buf_t *code_buf,
                             uint64_t *guest_state);

/* ============================================================================
 * Rotate Instructions
 * ============================================================================ */

/**
 * translate_bitfield_ror - Translate ROR (Rotate Right) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * Returns: 0 on success
 */
int translate_bitfield_ror(uint32_t encoding, code_buf_t *code_buf,
                           uint64_t *guest_state);

/**
 * translate_bitfield_rorw - Translate RORW (32-bit Rotate Right)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * Returns: 0 on success
 */
int translate_bitfield_rorw(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state);

/* ============================================================================
 * Extract Register Instructions
 * ============================================================================ */

/**
 * translate_bitfield_extr - Translate EXTR (Extract Register) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * Returns: 0 on success
 */
int translate_bitfield_extr(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state);

/* ============================================================================
 * Extension Instructions
 * ============================================================================ */

/**
 * translate_bitfield_uxtb - Translate UXTB (Unsigned Extend Byte)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * Returns: 0 on success
 */
int translate_bitfield_uxtb(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state);

/**
 * translate_bitfield_uxth - Translate UXTH (Unsigned Extend Halfword)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * Returns: 0 on success
 */
int translate_bitfield_uxth(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state);

/**
 * translate_bitfield_sxtb - Translate SXTB (Signed Extend Byte)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * Returns: 0 on success
 */
int translate_bitfield_sxtb(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state);

/**
 * translate_bitfield_sxth - Translate SXTH (Signed Extend Halfword)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * Returns: 0 on success
 */
int translate_bitfield_sxth(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state);

/**
 * translate_bitfield_sxtw - Translate SXTW (Signed Extend Word)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * Returns: 0 on success
 */
int translate_bitfield_sxtw(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state);

/* ============================================================================
 * Dispatch Function
 * ============================================================================ */

/**
 * translate_bitfield_dispatch - Dispatch bitfield instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state
 * Returns: 0 if handled, -1 otherwise
 */
int translate_bitfield_dispatch(uint32_t encoding, code_buf_t *code_buf,
                                uint64_t *guest_state);

#endif /* ROSETTA_TRANSLATE_BITFIELD_H */
