/* ============================================================================
 * Rosetta Translator - MOV Instruction Translation Header
 * ============================================================================
 *
 * This header defines the interface for translating ARM64 MOV family
 * instructions (MOVZ, MOVK, MOVN) to x86_64.
 * ============================================================================ */

#ifndef ROSETTA_TRANS_MOV_H
#define ROSETTA_TRANS_MOV_H

#include "rosetta_types.h"

/**
 * translate_movz - Translate ARM64 MOVZ (Move Wide with Zero) instruction
 * @state: Thread state containing register values
 * @insn: Pointer to ARM64 instruction bytes
 *
 * MOVZ moves a 16-bit immediate to a register, zeroing other bits.
 * Optionally shifts the immediate by 0, 16, 32, or 48 bits.
 * Returns: 0 on success, -1 on error
 */
int translate_movz(ThreadState *state, const uint8_t *insn);

/**
 * translate_movk - Translate ARM64 MOVK (Move Wide with Keep) instruction
 * @state: Thread state
 * @insn: Pointer to instruction bytes
 *
 * MOVK moves a 16-bit immediate into a register, keeping other bits.
 */
int translate_movk(ThreadState *state, const uint8_t *insn);

/**
 * translate_movn - Translate ARM64 MOVN (Move Wide with Not) instruction
 * @state: Thread state
 * @insn: Pointer to instruction bytes
 *
 * MOVN moves the bitwise NOT of a 16-bit immediate to a register.
 */
int translate_movn(ThreadState *state, const uint8_t *insn);

/**
 * translate_mov - Translate ARM64 MOV (register alias) instruction
 * @state: Thread state
 * @insn: Pointer to instruction bytes
 *
 * MOV (register) is an alias for ORR with zero register.
 */
int translate_mov(ThreadState *state, const uint8_t *insn);

/**
 * translate_mvn - Translate ARM64 MVN (Move Not) instruction
 * @state: Thread state
 * @insn: Pointer to instruction bytes
 *
 * MVN moves the bitwise NOT of a register.
 */
int translate_mvn(ThreadState *state, const uint8_t *insn);

/**
 * translate_neg - Translate ARM64 NEG (Negate) instruction
 * @state: Thread state
 * @insn: Pointer to instruction bytes
 *
 * NEG negates a register (subtracts from zero).
 */
int translate_neg(ThreadState *state, const uint8_t *insn);

/**
 * translate_ngc - Translate ARM64 NGC (Negate with Carry) instruction
 * @state: Thread state
 * @insn: Pointer to instruction bytes
 *
 * NGC negates a register and subtracts the carry flag.
 */
int translate_ngc(ThreadState *state, const uint8_t *insn);

/**
 * translate_rev - Translate ARM64 REV (Reverse Bytes) instruction
 * @state: Thread state
 * @insn: Pointer to instruction bytes
 *
 * REV reverses the byte order in a register.
 */
int translate_rev(ThreadState *state, const uint8_t *insn);

/**
 * translate_rev16 - Translate ARM64 REV16 (Reverse Bytes in 16-bit halfwords)
 * @state: Thread state
 * @insn: Pointer to instruction bytes
 */
int translate_rev16(ThreadState *state, const uint8_t *insn);

/**
 * translate_rev32 - Translate ARM64 REV32 (Reverse Bytes in 32-bit words)
 * @state: Thread state
 * @insn: Pointer to instruction bytes
 */
int translate_rev32(ThreadState *state, const uint8_t *insn);

/**
 * build_imm64 - Build a 64-bit immediate from MOVZ/MOVK sequence
 * @state: Thread state
 * @insn: Pointer to current instruction
 * @max_insns: Maximum instructions to look ahead
 *
 * This helper builds a complete 64-bit immediate by decoding a sequence
 * of MOVZ followed by MOVK instructions.
 * Returns: The constructed 64-bit immediate value
 */
uint64_t build_imm64(ThreadState *state, const uint8_t *insn, int max_insns);

#endif /* ROSETTA_TRANS_MOV_H */
