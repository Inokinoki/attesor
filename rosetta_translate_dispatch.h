/* ============================================================================
 * Rosetta Translation Dispatcher
 * ============================================================================
 *
 * This module handles dispatching of decoded x86_64 instructions to the
 * appropriate translation handlers based on instruction type.
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_DISPATCH_H
#define ROSETTA_TRANSLATE_DISPATCH_H

#include "rosetta_types.h"
#include "rosetta_x86_decode.h"
#include "rosetta_codegen.h"
#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * Translation Result Structure
 * ============================================================================ */

/**
 * Result of translating a single instruction
 */
typedef struct {
    bool success;           /* Translation succeeded */
    bool is_block_end;      /* Instruction ends the basic block */
    int insn_length;        /* Length of translated instruction */
} TranslateResult;

/* ============================================================================
 * Instruction Classification
 * ============================================================================ */

/**
 * Instruction type categories for dispatch
 */
typedef enum {
    INSN_UNKNOWN = 0,       /* Unknown/unsupported instruction */
    INSN_ALU,               /* Arithmetic/logic operations */
    INSN_MEMORY,            /* Memory operations (MOV, LEA, etc.) */
    INSN_BRANCH,            /* Control flow (JMP, CALL, RET, etc.) */
    INSN_BIT,               /* Bit manipulation */
    INSN_STRING,            /* String operations */
    INSN_SPECIAL,           /* Special instructions (CPUID, etc.) */
    INSN_FP,                /* Floating point operations */
    INSN_SIMD               /* SIMD/NEON operations */
} InsnCategory;

/* ============================================================================
 * Dispatcher Functions
 * ============================================================================ */

/**
 * Classify an x86_64 instruction into a category
 * @param insn Decoded x86 instruction
 * @return Instruction category
 */
InsnCategory dispatch_classify_insn(const x86_insn_t *insn);

/**
 * Translate a single x86_64 instruction to ARM64
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register (mapped)
 * @param arm_rm Source ARM register (mapped)
 * @param block_pc Current block PC
 * @return Translation result
 */
TranslateResult dispatch_translate_insn(code_buffer_t *code_buf,
                                        const x86_insn_t *insn,
                                        uint8_t arm_rd,
                                        uint8_t arm_rm,
                                        uint64_t block_pc);

/**
 * Get instruction category name for debugging
 * @param category Instruction category
 * @return Category name string
 */
const char *dispatch_category_name(InsnCategory category);

#endif /* ROSETTA_TRANSLATE_DISPATCH_H */
