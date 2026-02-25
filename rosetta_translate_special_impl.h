/* ============================================================================
 * Rosetta Translator - Special Translation Implementation Header
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_SPECIAL_IMPL_H
#define ROSETTA_TRANSLATE_SPECIAL_IMPL_H

#include "rosetta_codegen.h"

/* Special instructions */
void translate_special_cpuid(code_buffer_t *code_buf, const void *insn);
void translate_special_rdtsc(code_buffer_t *code_buf, const void *insn);
void translate_special_shld(code_buffer_t *code_buf, const void *insn,
                            uint8_t arm_rd, uint8_t arm_rm);
void translate_special_shrd(code_buffer_t *code_buf, const void *insn,
                            uint8_t arm_rd, uint8_t arm_rm);
void translate_special_cqo(code_buffer_t *code_buf, const void *insn);
void translate_special_cli(code_buffer_t *code_buf, const void *insn);
void translate_special_sti(code_buffer_t *code_buf, const void *insn);
void translate_special_nop(code_buffer_t *code_buf, const void *insn);

/* Bit manipulation */
void translate_bit_bsf(code_buffer_t *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_bit_bsr(code_buffer_t *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_bit_popcnt(code_buffer_t *code_buf, const void *insn,
                          uint8_t arm_rd, uint8_t arm_rm);
void translate_bit_bt(code_buffer_t *code_buf, const void *insn,
                      uint8_t arm_rd, uint8_t arm_rm);
void translate_bit_bts(code_buffer_t *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_bit_btr(code_buffer_t *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_bit_btc(code_buffer_t *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);

/* String instructions */
void translate_string_movs(code_buffer_t *code_buf, const void *insn);
void translate_string_stos(code_buffer_t *code_buf, const void *insn);
void translate_string_lods(code_buffer_t *code_buf, const void *insn);
void translate_string_cmps(code_buffer_t *code_buf, const void *insn);
void translate_string_scas(code_buffer_t *code_buf, const void *insn);

#endif /* ROSETTA_TRANSLATE_SPECIAL_IMPL_H */
