/* ============================================================================
 * Rosetta Translator - System Instruction Translation Header
 * ============================================================================
 *
 * This module provides translation functions for ARM64 system instructions
 * to x86_64 machine code.
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_SYSTEM_H
#define ROSETTA_TRANSLATE_SYSTEM_H

#include <stdint.h>
#include <stddef.h>
#include "rosetta_emit_x86.h"

/* System instruction encoding masks */
#define SVC_MASK    0xFF00001F
#define SVC_VAL     0xD4000001
#define BRK_MASK    0xFFE0001F
#define BRK_VAL     0xD4200000
#define HLT_MASK    0xFFFFFFFF
#define HLT_VAL     0xD4000000
#define MRS_MASK    0xFFC00000
#define MRS_VAL     0xD5300000
#define MSR_MASK    0xFFC00000
#define MSR_VAL     0xD5000000

/* System call numbers for common operations */
#define SYS_EXIT    1
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_MMAP    197
#define SYS_MUNMAP  159

/* System translation functions */
int translate_system_svc(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_system_brk(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_system_hlt(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_system_mrs(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
int translate_system_msr(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);

/* Dispatch function - returns 0 if instruction handled, -1 otherwise */
int translate_system_dispatch(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);

/* syscall emulation */
int translate_emulate_syscall(uint64_t syscall_num, uint64_t *args, uint64_t *result);

#endif /* ROSETTA_TRANSLATE_SYSTEM_H */
