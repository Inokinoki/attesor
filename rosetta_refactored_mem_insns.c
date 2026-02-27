/* ============================================================================
 * Rosetta Refactored - Memory Instruction Implementation
 * ============================================================================
 *
 * This module implements full ARM64 memory instruction translation
 * for load/store operations.
 *
 * Supported instructions:
 * - LDR, STR (Load/Store Register)
 * - LDRB, STRB (Load/Store Register Byte)
 * - LDRH, STRH (Load/Store Register Halfword)
 * - LDRSW (Load Register Signed Word)
 * - LDP, STP (Load/Store Pair)
 * - LDUR, STUR (Load/Store Register Unscaled)
 * ============================================================================ */

#include "rosetta_refactored_mem_insns.h"
#include "rosetta_arm64_decode.h"
#include <stdint.h>
#include <string.h>

/* ============================================================================
 * Address Decoding Helpers
 * ============================================================================ */

/**
 * get_mem_address - Calculate memory address from load/store instruction
 */
static inline uint64_t get_mem_address(uint32_t encoding, ThreadState *state)
{
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t option = (encoding >> 13) & 0x7;
    uint8_t size = (encoding >> 30) & 3;
    uint64_t base = state->guest.x[rn];
    uint64_t offset = 0;

    /* Check addressing mode based on instruction type */
    if ((encoding & 0x800000) != 0) {
        /* Immediate offset */
        uint64_t imm = (encoding >> 10) & 0x1FF;
        offset = imm << size;
    } else if ((encoding & 0x400000) == 0) {
        /* Register offset */
        uint8_t rm = (encoding >> 16) & 0x1F;
        offset = state->guest.x[rm];

        /* Apply extension if specified */
        if (option == 3) {
            offset = (int64_t)((int32_t)offset);  /* SXTW */
        } else if (option == 7) {
            offset = (uint64_t)((uint32_t)offset);  /* UXTW */
        }
    } else {
        /* Post/pre-index with 9-bit signed immediate */
        int64_t imm9 = (int64_t)((encoding >> 12) & 0x1FF);
        imm9 = (imm9 << 55) >> 55;  /* Sign extend */
        offset = imm9;
    }

    return base + offset;
}

/* ============================================================================
 * Load Instructions
 * ============================================================================ */

void translate_ldr(uint32_t encoding, ThreadState *state)
{
    uint8_t rt = encoding & 0x1F;
    uint8_t size = (encoding >> 30) & 3;
    uint64_t addr = get_mem_address(encoding, state);
    uint64_t value = 0;

    switch (size) {
        case 2:  /* Word (32-bit) */
            value = *(uint32_t *)addr;
            state->guest.x[rt] = value;
            break;
        case 3:  /* Doubleword (64-bit) */
            value = *(uint64_t *)addr;
            state->guest.x[rt] = value;
            break;
        default:
            break;
    }
}

void translate_ldrb(uint32_t encoding, ThreadState *state)
{
    uint8_t rt = encoding & 0x1F;
    uint64_t addr = get_mem_address(encoding, state);
    uint8_t value = *(uint8_t *)addr;
    state->guest.x[rt] = value;  /* Zero-extend */
}

void translate_ldrh(uint32_t encoding, ThreadState *state)
{
    uint8_t rt = encoding & 0x1F;
    uint64_t addr = get_mem_address(encoding, state);
    uint16_t value = *(uint16_t *)addr;
    state->guest.x[rt] = value;  /* Zero-extend */
}

void translate_ldrsw(uint32_t encoding, ThreadState *state)
{
    uint8_t rt = encoding & 0x1F;
    uint64_t addr = get_mem_address(encoding, state);
    int32_t value = *(int32_t *)addr;
    state->guest.x[rt] = (uint64_t)value;  /* Sign-extend */
}

/* ============================================================================
 * Store Instructions
 * ============================================================================ */

void translate_str(uint32_t encoding, ThreadState *state)
{
    uint8_t rt = encoding & 0x1F;
    uint8_t size = (encoding >> 30) & 3;
    uint64_t addr = get_mem_address(encoding, state);
    uint64_t value = state->guest.x[rt];

    switch (size) {
        case 2:  /* Word (32-bit) */
            *(uint32_t *)addr = (uint32_t)value;
            break;
        case 3:  /* Doubleword (64-bit) */
            *(uint64_t *)addr = value;
            break;
        default:
            break;
    }
}

void translate_strb(uint32_t encoding, ThreadState *state)
{
    uint8_t rt = encoding & 0x1F;
    uint64_t addr = get_mem_address(encoding, state);
    uint8_t value = (uint8_t)state->guest.x[rt];
    *(uint8_t *)addr = value;
}

void translate_strh(uint32_t encoding, ThreadState *state)
{
    uint8_t rt = encoding & 0x1F;
    uint64_t addr = get_mem_address(encoding, state);
    uint16_t value = (uint16_t)state->guest.x[rt];
    *(uint16_t *)addr = value;
}

/* ============================================================================
 * Load/Store Pair Instructions
 * ============================================================================ */

void translate_ldp(uint32_t encoding, ThreadState *state)
{
    uint8_t rt1 = encoding & 0x1F;
    uint8_t rt2 = (encoding >> 10) & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;
    int64_t imm7 = (int64_t)((encoding >> 15) & 0x7F);

    uint64_t offset = sf ? (imm7 * 8) : (imm7 * 4);
    uint64_t addr = state->guest.x[rn] + offset;

    if (sf) {
        state->guest.x[rt1] = *(uint64_t *)addr;
        state->guest.x[rt2] = *(uint64_t *)(addr + 8);
    } else {
        state->guest.x[rt1] = *(uint32_t *)addr;
        state->guest.x[rt2] = *(uint32_t *)(addr + 4);
    }
}

void translate_stp(uint32_t encoding, ThreadState *state)
{
    uint8_t rt1 = encoding & 0x1F;
    uint8_t rt2 = (encoding >> 10) & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;
    int64_t imm7 = (int64_t)((encoding >> 15) & 0x7F);

    uint64_t offset = sf ? (imm7 * 8) : (imm7 * 4);
    uint64_t addr = state->guest.x[rn] + offset;

    if (sf) {
        *(uint64_t *)addr = state->guest.x[rt1];
        *(uint64_t *)(addr + 8) = state->guest.x[rt2];
    } else {
        *(uint32_t *)addr = state->guest.x[rt1];
        *(uint32_t *)(addr + 4) = state->guest.x[rt2];
    }
}

/* ============================================================================
 * Unscaled Immediate Instructions
 * ============================================================================ */

void translate_ldur(uint32_t encoding, ThreadState *state)
{
    uint8_t rt = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t size = (encoding >> 30) & 3;
    int64_t imm9 = (int64_t)((encoding >> 12) & 0x1FF);
    imm9 = (imm9 << 55) >> 55;  /* Sign extend */

    uint64_t addr = state->guest.x[rn] + imm9;

    switch (size) {
        case 2:
            state->guest.x[rt] = *(uint32_t *)addr;
            break;
        case 3:
            state->guest.x[rt] = *(uint64_t *)addr;
            break;
    }
}

void translate_stur(uint32_t encoding, ThreadState *state)
{
    uint8_t rt = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t size = (encoding >> 30) & 3;
    int64_t imm9 = (int64_t)((encoding >> 12) & 0x1FF);
    imm9 = (imm9 << 55) >> 55;

    uint64_t addr = state->guest.x[rn] + imm9;
    uint64_t value = state->guest.x[rt];

    switch (size) {
        case 2:
            *(uint32_t *)addr = (uint32_t)value;
            break;
        case 3:
            *(uint64_t *)addr = value;
            break;
    }
}
