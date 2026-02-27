/* ============================================================================
 * Rosetta Translator - Branch Instruction Translation Header
 * ============================================================================
 *
 * This module provides translation functions for ARM64 branch instructions
 * to x86_64 machine code.
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_BRANCH_MAIN_H
#define ROSETTA_TRANSLATE_BRANCH_MAIN_H

#include <stdint.h>
#include <stddef.h>
#include "rosetta_emit_x86.h"

/* Branch instruction encoding masks */
#define BRANCH_B_MASK       0xFC000000
#define BRANCH_B_VAL        0x14000000
#define BRANCH_BL_MASK      0xFC000000
#define BRANCH_BL_VAL       0x94000000
#define BRANCH_BR_MASK      0xFFFFFC00
#define BRANCH_BR_VAL       0xD61F0000
#define BRANCH_BLR_MASK     0xFFFFFC00
#define BRANCH_BLR_VAL      0xD63F0000
#define BRANCH_RET_MASK     0xFFFFFBFF
#define BRANCH_RET_VAL      0xD65F0000
#define BRANCH_BCOND_MASK   0xFF000010
#define BRANCH_BCOND_VAL    0x54000000
#define BRANCH_CBZ_MASK     0x7F800000
#define BRANCH_CBZ_VAL      0x34000000
#define BRANCH_CBNZ_MASK    0x7F800000
#define BRANCH_CBNZ_VAL     0x35000000
#define BRANCH_TBZ_MASK     0x7F000000
#define BRANCH_TBZ_VAL      0x36000000
#define BRANCH_TBNZ_MASK    0x7F000000
#define BRANCH_TBNZ_VAL     0x37000000

/* Branch translation functions */
int translate_branch_b(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t pc);
int translate_branch_bl(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t pc);
int translate_branch_br(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_branch_blr(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_branch_ret(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_branch_bcond(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t pc);
int translate_branch_cbz(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t pc);
int translate_branch_cbnz(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t pc);
int translate_branch_tbz(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t pc);
int translate_branch_tbnz(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t pc);

/* Dispatch function - returns 0 if instruction handled, -1 otherwise */
int translate_branch_dispatch(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t pc, int *terminated);

#endif /* ROSETTA_TRANSLATE_BRANCH_MAIN_H */
