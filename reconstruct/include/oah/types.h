#ifndef OAH_TYPES_H
#define OAH_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   s8;
typedef int32_t  s32;
typedef int64_t  s64;

/* MemSize — AssemblerBase.hpp ldp_post/stp_pre:
 *   "size == MemSize::S64 || size == MemSize::S32"
 * S32=2, S64=3 from (param & 0xfe) == 2 and (param == 3) << 31. */
typedef enum {
    OAH_MEM_S8   = 0,
    OAH_MEM_S16  = 1,
    OAH_MEM_S32  = 2,
    OAH_MEM_S64  = 3,
    OAH_MEM_S128 = 4
} oah_mem_size;

/* ir::OperandSize — Translator.h:232
 *   "size <= ir::OperandSize::S128" with check `4 < bVar3`. */
typedef enum {
    OAH_OPSIZE_S8   = 0,
    OAH_OPSIZE_S16  = 1,
    OAH_OPSIZE_S32  = 2,
    OAH_OPSIZE_S64  = 3,
    OAH_OPSIZE_S128 = 4
} oah_operand_size;

/* x86 condition codes. Translator.h:264 requires value < 16.
 * P and NP (10, 11) are rejected by translate_condition_code. */
typedef enum {
    OAH_CC_O  = 0,
    OAH_CC_NO = 1,
    OAH_CC_B  = 2,
    OAH_CC_AE = 3,
    OAH_CC_E  = 4,
    OAH_CC_NE = 5,
    OAH_CC_BE = 6,
    OAH_CC_A  = 7,
    OAH_CC_S  = 8,
    OAH_CC_NS = 9,
    OAH_CC_P  = 10,
    OAH_CC_NP = 11,
    OAH_CC_L  = 12,
    OAH_CC_GE = 13,
    OAH_CC_LE = 14,
    OAH_CC_G  = 15
} oah_x86_cc;

/* ARM condition field (bits 31-28 of B.cond). */
typedef enum {
    OAH_ARM_EQ = 0,
    OAH_ARM_NE = 1,
    OAH_ARM_CS = 2,
    OAH_ARM_CC = 3,
    OAH_ARM_MI = 4,
    OAH_ARM_PL = 5,
    OAH_ARM_VS = 6,
    OAH_ARM_VC = 7,
    OAH_ARM_HI = 8,
    OAH_ARM_LS = 9,
    OAH_ARM_GE = 10,
    OAH_ARM_LT = 11,
    OAH_ARM_GT = 12,
    OAH_ARM_LE = 13,
    OAH_ARM_AL = 14
} oah_arm_cc;

/* AbiKind — ThreadContextRegisterState.cpp switch + asserts.
 * Numeric values are the switch cases in FUN_8000000407cc. */
typedef enum {
    OAH_ABI_NONE            = 0,
    OAH_ABI_RUNTIME         = 1,
    OAH_ABI_NEW_THREAD      = 2,  /* "arm pc should not be NewThread AbiKind" */
    OAH_ABI_TRANSLATED_CODE = 3,  /* AbiKind::TranslatedCode */
    OAH_ABI_KIND_4          = 4,
    OAH_ABI_KIND_5          = 5,
    OAH_ABI_KIND_7          = 7,
    OAH_ABI_RUNTIME_SYSCALL = 8,  /* "in runtime_syscall but instruction is not marked as a syscall" */
    OAH_ABI_SIGTRAMP        = 9,  /* "arm pc should not be Sigtramp AbiKind" */
    OAH_ABI_RUNTIME_REQUEST = 10
} oah_abi_kind;

#ifdef __cplusplus
}
#endif

#endif /* OAH_TYPES_H */
