/* ============================================================================
 * Rosetta Translator - Memory Instruction Translation Header
 * ============================================================================
 *
 * This module provides translation functions for ARM64 memory instructions
 * to x86_64 machine code.
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_MEM_MAIN_H
#define ROSETTA_TRANSLATE_MEM_MAIN_H

#include <stdint.h>
#include <stddef.h>
#include "rosetta_emit_x86.h"

/* Memory instruction encoding masks */
#define MEM_LDR_IMM_MASK    0xFFC00000
#define MEM_LDR_IMM_VAL     0xF9400000
#define MEM_STR_IMM_MASK    0xFFC00000
#define MEM_STR_IMM_VAL     0xF9000000
#define MEM_LDRB_MASK       0xFFC00000
#define MEM_LDRB_VAL        0x39400000
#define MEM_STRB_MASK       0xFFC00000
#define MEM_STRB_VAL        0x39000000
#define MEM_LDRH_MASK       0xFFC00000
#define MEM_LDRH_VAL        0x79400000
#define MEM_STRH_MASK       0xFFC00000
#define MEM_STRH_VAL        0x79000000
#define MEM_LDRSB_MASK      0xFFC00000
#define MEM_LDRSB_VAL       0x39800000
#define MEM_LDRSH_MASK      0xFFC00000
#define MEM_LDRSH_VAL       0x79800000
#define MEM_LDRSW_MASK      0xFFC00000
#define MEM_LDRSW_VAL       0xB9800000
#define MEM_LDUR_MASK       0xFFC00000
#define MEM_LDUR_VAL        0xF8400000
#define MEM_STUR_MASK       0xFFC00000
#define MEM_STUR_VAL        0xF8000000
#define MEM_LDP_MASK      0xFF800000
#define MEM_LDP_VAL       0xA9400000
#define MEM_STP_MASK      0xFF800000
#define MEM_STP_VAL       0xA9000000

/* Memory translation functions */
int translate_mem_ldr_imm(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_mem_str_imm(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_mem_ldrb(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_mem_strb(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_mem_ldrh(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_mem_strh(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_mem_ldrsb(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_mem_ldrsh(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_mem_ldrsw(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_mem_ldur(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_mem_stur(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_mem_ldp(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_mem_stp(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);

/* Dispatch function - returns 0 if instruction handled, -1 otherwise */
int translate_mem_dispatch(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);

#endif /* ROSETTA_TRANSLATE_MEM_MAIN_H */
