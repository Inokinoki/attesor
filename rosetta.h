/*
 * Rosetta Binary Translator - Master Header
 *
 * This master header includes all modular components of the Rosetta
 * binary translation layer. Rosetta translates x86_64 binaries to
 * ARM64 for execution on Apple Silicon.
 *
 * Usage: #include "rosetta.h" - This includes all necessary modules
 */

#ifndef ROSETTA_MASTER_H
#define ROSETTA_MASTER_H

/* ============================================================================
 * CORE TYPE DEFINITIONS
 * ============================================================================ */
#include "rosetta_types.h"

/* ============================================================================
 * INSTRUCTION DECODING
 * ============================================================================ */
#include "rosetta_x86_decode.h"      /* x86_64 instruction decoder */
#include "rosetta_arm64_decode.h"    /* ARM64 instruction decoder */

/* ============================================================================
 * CODE EMISSION
 * ============================================================================ */
#include "rosetta_arm64_emit.h"      /* ARM64 JIT code emitter */
#include "rosetta_codegen.h"         /* High-level code generation */

/* ============================================================================
 * TRANSLATION MODULES
 * ============================================================================ */
#include "rosetta_translate.h"       /* Main translation interface */
#include "rosetta_translate_alu.h"   /* ALU operations (ADD, SUB, AND, OR, etc.) */
#include "rosetta_translate_memory.h"/* Memory operations (LOAD, STORE, MOV) */
#include "rosetta_translate_branch.h"/* Control flow (JMP, JCC, CALL, RET) */
#include "rosetta_translate_bit.h"   /* Bit manipulation (BSF, BSR, POPCNT) */
#include "rosetta_translate_string.h"/* String operations (MOVS, STOS, LODS) */
#include "rosetta_translate_special.h"/* Special instructions (CPUID, RDTSC) */
#include "rosetta_translate_block.h" /* Basic block translation */
#include "rosetta_translate_dispatch.h"/* Translation dispatch table */

/* ============================================================================
 * EXECUTION ENGINE
 * ============================================================================ */
#include "rosetta_jit.h"             /* JIT compilation engine */
#include "rosetta_context.h"         /* CPU context management */
#include "rosetta_simd.h"            /* SIMD/FP instruction handling */
#include "rosetta_simd_mem.h"        /* SIMD memory operations */

/* ============================================================================
 * MEMORY MANAGEMENT
 * ============================================================================ */
#include "rosetta_memmgmt.h"         /* Memory management */
#include "rosetta_cache.h"           /* Translation cache */
#include "rosetta_transcache.h"      /* Translation block cache */
#include "rosetta_hash.h"            /* Hash table for lookups */
#include "rosetta_vector.h"          /* Vector operations */

/* ============================================================================
 * SYSTEM INTERFACE
 * ============================================================================ */
#include "rosetta_syscalls.h"        /* Syscall handling */

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */
#include "rosetta_utils.h"           /* Utility functions */
#include "rosetta_function_map.h"    /* Function mapping tables */

/* ============================================================================
 * CONVENIENCE MACROS
 * ============================================================================ */

/* Register mapping shortcuts */
#define ARM_X0  0
#define ARM_X1  1
#define ARM_X2  2
#define ARM_X3  3
#define ARM_X4  4
#define ARM_X5  5
#define ARM_X6  6
#define ARM_X7  7
#define ARM_X8  8
#define ARM_X9  9
#define ARM_X10 10
#define ARM_X11 11
#define ARM_X12 12
#define ARM_X13 13
#define ARM_X14 14
#define ARM_X15 15
#define ARM_X16 16
#define ARM_X17 17
#define ARM_X18 18
#define ARM_X19 19
#define ARM_X20 20
#define ARM_X21 21
#define ARM_X22 22
#define ARM_X23 23
#define ARM_X24 24
#define ARM_X25 25
#define ARM_X26 26
#define ARM_X27 27
#define ARM_X28 28
#define ARM_X29 29  /* Frame Pointer */
#define ARM_X30 30  /* Link Register */
#define ARM_SP  31  /* Stack Pointer / Zero Register */

/* x86_64 register shortcuts */
#define X86_RAX  0
#define X86_RCX  1
#define X86_RDX  2
#define X86_RBX  3
#define X86_RSI  4
#define X86_RDI  5
#define X86_RBP  6
#define X86_RSP  7
#define X86_R8   8
#define X86_R9   9
#define X86_R10  10
#define X86_R11  11
#define X86_R12  12
#define X86_R13  13
#define X86_R14  14
#define X86_R15  15

/* Translation flags */
#define ROSetta_BLOCK_VALID  0x01
#define ROSETTA_BLOCK_HOT    0x02
#define ROSETTA_BLOCK_LINKED 0x04

/* Error codes */
#define ROSETTA_OK           0
#define ROSETTA_ERR_NOMEM   -1
#define ROSETTA_ERR_INVAL   -2
#define ROSETTA_ERR_NOTIMPL -3
#define ROSETTA_ERR_FAULT   -4

/* Version information */
#define ROSETTA_VERSION_MAJOR 1
#define ROSETTA_VERSION_MINOR 0
#define ROSETTA_VERSION_PATCH 0

#endif /* ROSETTA_MASTER_H */
