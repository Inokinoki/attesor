/* ============================================================================
 * Rosetta Refactored - Memory Instruction Header
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_MEM_INSNS_H
#define ROSETTA_REFACTORED_MEM_INSNS_H

#include "rosetta_refactored_types.h"
#include <stdint.h>

/* Load Instructions */
void translate_ldr(uint32_t encoding, ThreadState *state);
void translate_ldrb(uint32_t encoding, ThreadState *state);
void translate_ldrh(uint32_t encoding, ThreadState *state);
void translate_ldrsw(uint32_t encoding, ThreadState *state);

/* Store Instructions */
void translate_str(uint32_t encoding, ThreadState *state);
void translate_strb(uint32_t encoding, ThreadState *state);
void translate_strh(uint32_t encoding, ThreadState *state);

/* Load/Store Pair */
void translate_ldp(uint32_t encoding, ThreadState *state);
void translate_stp(uint32_t encoding, ThreadState *state);

/* Unscaled Immediate */
void translate_ldur(uint32_t encoding, ThreadState *state);
void translate_stur(uint32_t encoding, ThreadState *state);

#endif /* ROSETTA_REFACTORED_MEM_INSNS_H */
