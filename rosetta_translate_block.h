/* ============================================================================
 * Rosetta Translation Block Module
 * ============================================================================
 *
 * This module handles translation of x86_64 basic blocks to ARM64 code.
 * It coordinates instruction decoding, dispatching, and code generation.
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_BLOCK_H
#define ROSETTA_TRANSLATE_BLOCK_H

#include "rosetta_types.h"
#include "rosetta_x86_decode.h"
#include "rosetta_arm64_emit.h"
#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * Translation Block Constants
 * ============================================================================ */

/**
 * Maximum instructions per basic block
 * Limits translation time and code cache fragmentation
 */
#define MAX_BLOCK_INSTRUCTIONS 64

/**
 * Maximum code size per block (bytes)
 * Each ARM64 instruction is 4 bytes, so this allows ~16K instructions
 */
#define MAX_BLOCK_CODE_SIZE 65536

/* ============================================================================
 * Translation Block Structure
 * ============================================================================ */

/**
 * Translation result for a basic block
 */
typedef struct {
    void *code;               /* Pointer to translated code */
    size_t size;              /* Size of translated code in bytes */
    uint64_t guest_pc;        /* Guest PC of block start */
    uint64_t host_pc;         /* Host PC of translated code */
    int insn_count;           /* Number of instructions translated */
    bool ends_with_branch;    /* Block ends with branch/return */
    bool is_cached;           /* Block is in translation cache */
} TranslationResult;

/* ============================================================================
 * Block Translation Functions
 * ============================================================================ */

/**
 * Translate a basic block of x86_64 instructions to ARM64
 *
 * @param guest_pc Guest x86_64 PC to translate
 * @return Translation result structure
 */
TranslationResult translate_block(uint64_t guest_pc);

/**
 * Fast path translation for hot blocks
 *
 * Uses optimized lookup and potentially cached translations.
 *
 * @param guest_pc Guest x86_64 PC to translate
 * @return Translated code pointer, or NULL on failure
 */
void *translate_block_fast(uint64_t guest_pc);

/**
 * Execute a translated block
 *
 * @param state Thread state
 * @param block Translated code block
 */
void execute_translated_block(void *state, void *block);

/* ============================================================================
 * Block Optimization Functions
 * ============================================================================ */

/**
 * Optimize a translated block
 *
 * Performs peephole optimization and code size reduction.
 *
 * @param block Translated code block
 * @param size Size of translated code
 * @return 0 on success, -1 on failure
 */
int translate_block_optimize(void *block, size_t size);

/**
 * Link two translated blocks
 *
 * Creates a direct jump between frequently executed block pairs.
 *
 * @param from_block Source block
 * @param to_block Target block
 * @param size Size of from block
 * @return 0 on success, -1 on failure
 */
int translate_block_link(void *from_block, void *to_block, size_t size);

/**
 * Unlink a translated block
 *
 * Removes direct jumps to this block.
 *
 * @param block Block to unlink
 * @return 0 on success, -1 on failure
 */
int translate_block_unlink(void *block);

#endif /* ROSETTA_TRANSLATE_BLOCK_H */
