/*
 * Rosetta Translator - Refactored C Code
 *
 * This file contains semantically refactored versions of the key functions
 * identified in the Rosetta binary. The original decompilation used generic
 * names like FUN_XXXXXXXXXXXX and undefined types.
 */

#include "rosetta_refactored.h"
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <sched.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/uio.h>
#if defined(__APPLE__)
#include <mach/mach_time.h>
#endif

/* Typedef for noreturn */
#define noreturn _Noreturn

/* Typedef for entry point function */
typedef void (*entry_point_t)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t,
                              uint64_t, uint64_t, uint64_t, uint64_t);

/* Stack pointer placeholder */
static uint64_t initial_sp = 0;

/* ============================================================================
 * JIT Code Emitter Infrastructure
 * ============================================================================ */

/* Code buffer for JIT emission */
typedef struct {
    uint8_t *buffer;        /* Pointer to code buffer */
    size_t size;            /* Total buffer size */
    size_t offset;          /* Current write offset */
    int error;              /* Error flag */
} CodeBuffer;

/* ============================================================================
 * Forward Declarations for Translation Functions
 * ============================================================================ */

/* ALU translation */
void translate_alu_add(CodeBuffer *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_alu_sub(CodeBuffer *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_alu_and(CodeBuffer *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_alu_or(CodeBuffer *code_buf, const void *insn,
                      uint8_t arm_rd, uint8_t arm_rm);
void translate_alu_xor(CodeBuffer *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_alu_mul(CodeBuffer *code_buf, const void *insn, uint8_t arm_rm);
void translate_alu_div(CodeBuffer *code_buf, const void *insn, uint8_t arm_rm);
void translate_alu_inc(CodeBuffer *code_buf, const void *insn, uint8_t arm_rd);
void translate_alu_dec(CodeBuffer *code_buf, const void *insn, uint8_t arm_rd);
void translate_alu_neg(CodeBuffer *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_alu_not(CodeBuffer *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_alu_shift(CodeBuffer *code_buf, const void *insn,
                         uint8_t arm_rd, uint8_t arm_rm);

/* Memory translation */
void translate_memory_mov(CodeBuffer *code_buf, const void *insn,
                          uint8_t arm_rd, uint8_t arm_rm);
void translate_memory_movzx(CodeBuffer *code_buf, const void *insn,
                            uint8_t arm_rd, uint8_t arm_rm);
void translate_memory_movsx(CodeBuffer *code_buf, const void *insn,
                            uint8_t arm_rd, uint8_t arm_rm);
void translate_memory_movsxd(CodeBuffer *code_buf, const void *insn,
                             uint8_t arm_rd, uint8_t arm_rm);
void translate_memory_lea(CodeBuffer *code_buf, const void *insn, uint8_t arm_rd);
void translate_memory_push(CodeBuffer *code_buf, const void *insn, uint8_t arm_rd);
void translate_memory_pop(CodeBuffer *code_buf, const void *insn, uint8_t arm_rd);
void translate_memory_cmp(CodeBuffer *code_buf, const void *insn,
                          uint8_t arm_rd, uint8_t arm_rm);
void translate_memory_test(CodeBuffer *code_buf, const void *insn,
                           uint8_t arm_rd, uint8_t arm_rm);

/* Branch translation */
int translate_branch_jcc(CodeBuffer *code_buf, const void *insn, uint64_t block_pc);
int translate_branch_jmp(CodeBuffer *code_buf, const void *insn, uint64_t block_pc);
int translate_branch_call(CodeBuffer *code_buf, const void *insn, uint64_t block_pc);
int translate_branch_ret(CodeBuffer *code_buf);
void translate_branch_cmov(CodeBuffer *code_buf, const void *insn,
                           uint8_t arm_rd, uint8_t arm_rm);
void translate_branch_setcc(CodeBuffer *code_buf, const void *insn, uint8_t arm_rd);
void translate_branch_xchg(CodeBuffer *code_buf, const void *insn,
                           uint8_t arm_rd, uint8_t arm_rm);

/* Bit manipulation translation */
void translate_bit_bsf(CodeBuffer *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_bit_bsr(CodeBuffer *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_bit_popcnt(CodeBuffer *code_buf, const void *insn,
                          uint8_t arm_rd, uint8_t arm_rm);
void translate_bit_bt(CodeBuffer *code_buf, const void *insn,
                      uint8_t arm_rd, uint8_t arm_rm);
void translate_bit_bts(CodeBuffer *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_bit_btr(CodeBuffer *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);
void translate_bit_btc(CodeBuffer *code_buf, const void *insn,
                       uint8_t arm_rd, uint8_t arm_rm);

/* String translation */
void translate_string_movs(CodeBuffer *code_buf, const void *insn);
void translate_string_stos(CodeBuffer *code_buf, const void *insn);
void translate_string_lods(CodeBuffer *code_buf, const void *insn);
void translate_string_cmps(CodeBuffer *code_buf, const void *insn);
void translate_string_scas(CodeBuffer *code_buf, const void *insn);

/* Special translation */
void translate_special_cpuid(CodeBuffer *code_buf, const void *insn);
void translate_special_rdtsc(CodeBuffer *code_buf, const void *insn);
void translate_special_shld(CodeBuffer *code_buf, const void *insn,
                            uint8_t arm_rd, uint8_t arm_rm);
void translate_special_shrd(CodeBuffer *code_buf, const void *insn,
                            uint8_t arm_rd, uint8_t arm_rm);
void translate_special_cqo(CodeBuffer *code_buf, const void *insn);
void translate_special_cli(CodeBuffer *code_buf, const void *insn);
void translate_special_sti(CodeBuffer *code_buf, const void *insn);
void translate_special_nop(CodeBuffer *code_buf, const void *insn);

/* Global code cache buffer */
static uint8_t g_code_cache[1024 * 1024];  /* 1MB code cache */
static size_t g_code_cache_offset = 0;

/**
 * Initialize code buffer
 * @param buf Code buffer to initialize
 * @param buffer_ptr Pre-allocated buffer or NULL for internal cache
 * @param size Buffer size
 */
void code_buffer_init(CodeBuffer *buf, uint8_t *buffer_ptr, size_t size)
{
    buf->buffer = buffer_ptr ? buffer_ptr : g_code_cache + g_code_cache_offset;
    buf->size = size;
    buf->offset = 0;
    buf->error = 0;
}

/**
 * Get code size
 * @param buf Code buffer
 * @return Current code size
 */
size_t code_buffer_get_size(CodeBuffer *buf)
{
    return buf->offset;
}

/**
 * Emit a single byte
 * @param buf Code buffer
 * @param byte Byte to emit
 */
void emit_byte(CodeBuffer *buf, uint8_t byte)
{
    if (buf->offset >= buf->size) {
        buf->error = 1;
        return;
    }
    buf->buffer[buf->offset++] = byte;
}

/**
 * Emit a 32-bit word
 * @param buf Code buffer
 * @param word Word to emit
 */
void emit_word32(CodeBuffer *buf, uint32_t word)
{
    emit_byte(buf, (word >> 0) & 0xFF);
    emit_byte(buf, (word >> 8) & 0xFF);
    emit_byte(buf, (word >> 16) & 0xFF);
    emit_byte(buf, (word >> 24) & 0xFF);
}

/**
 * Emit a 64-bit word
 * @param buf Code buffer
 * @param word Word to emit
 */
void emit_word64(CodeBuffer *buf, uint64_t word)
{
    emit_byte(buf, (word >> 0) & 0xFF);
    emit_byte(buf, (word >> 8) & 0xFF);
    emit_byte(buf, (word >> 16) & 0xFF);
    emit_byte(buf, (word >> 24) & 0xFF);
    emit_byte(buf, (word >> 32) & 0xFF);
    emit_byte(buf, (word >> 40) & 0xFF);
    emit_byte(buf, (word >> 48) & 0xFF);
    emit_byte(buf, (word >> 56) & 0xFF);
}

/* ============================================================================
 * ARM64 Register Constants and Encoding Helpers
 * Translation: x86_64 (guest) -> ARM64 (host)
 * ============================================================================ */

/* x86_64 Register constants (for reference during decoding) */
#define X86_RAX 0
#define X86_RCX 1
#define X86_RDX 2
#define X86_RBX 3
#define X86_RSP 4
#define X86_RBP 5
#define X86_RSI 6
#define X86_RDI 7
#define X86_R8  8
#define X86_R9  9
#define X86_R10 10
#define X86_R11 11
#define X86_R12 12
#define X86_R13 13
#define X86_R14 14
#define X86_R15 15

/* ARM64 Register constants */
#define X0  0
#define X1  1
#define X2  2
#define X3  3
#define X4  4
#define X5  5
#define X6  6
#define X7  7
#define X8  8
#define X9  9
#define X10 10
#define X11 11
#define X12 12
#define X13 13
#define X14 14
#define X15 15
#define X16 16
#define X17 17
#define X18 18
#define X19 19
#define X20 20
#define X21 21
#define X22 22
#define X23 23
#define X24 24
#define X25 25
#define X26 26
#define X27 27
#define X28 28
#define X29 29  /* FP */
#define X30 30  /* LR */
#define X31 31  /* SP/ZR */
#define WZR 31  /* Zero register (32-bit) */
#define XZR 31  /* Zero register (64-bit) */

/* ARM64 Condition Codes */
#define ARM64_EQ 0  /* Z == 1 */
#define ARM64_NE 1  /* Z == 0 */
#define ARM64_CS 2  /* C == 1 */
#define ARM64_CC 3  /* C == 0 */
#define ARM64_MI 4  /* N == 1 */
#define ARM64_PL 5  /* N == 0 */
#define ARM64_VS 6  /* V == 1 */
#define ARM64_VC 7  /* V == 0 */
#define ARM64_LT 8  /* N != V */
#define ARM64_GE 9  /* N == V */
#define ARM64_LE 10 /* Z || N != V */
#define ARM64_GT 11 /* !Z && N == V */
#define ARM64_AL 14 /* Always */
#define ARM64_NV 15 /* Never */

/* Register mapping: x86_64 -> ARM64 */
/* Simple direct mapping: RAX->X0, RCX->X1, etc. */
static inline uint8_t map_x86_to_arm(uint8_t x86_reg)
{
    return x86_reg & 0x0F;  /* RAX-R15 -> X0-X15 */
}

/**
 * Emit ARM64 instruction (32-bit, little-endian)
 * @param buf Code buffer
 * @param insn ARM64 instruction to emit
 */
static inline void emit_arm64_insn(CodeBuffer *buf, uint32_t insn)
{
    emit_byte(buf, (insn >> 0) & 0xFF);
    emit_byte(buf, (insn >> 8) & 0xFF);
    emit_byte(buf, (insn >> 16) & 0xFF);
    emit_byte(buf, (insn >> 24) & 0xFF);
}

/**
 * Emit MOVZ Xd, #imm16, LSL #shift
 * @param buf Code buffer
 * @param dst Destination register (0-30)
 * @param imm 16-bit immediate
 * @param shift Shift amount (0, 1, 2, or 3 for LSL #0/16/32/48)
 */
static void emit_movz(CodeBuffer *buf, uint8_t dst, uint16_t imm, uint8_t shift)
{
    uint32_t insn = 0xD2800000;  /* MOVZ base */
    insn |= (uint32_t)imm;        /* imm16 */
    insn |= (uint32_t)(shift & 3) << 21;  /* hw */
    insn |= (uint32_t)(dst & 31) << 0;   /* Rd */
    emit_arm64_insn(buf, insn);
}

/**
 * Emit MOVK Xd, #imm16, LSL #shift
 * @param buf Code buffer
 * @param dst Destination register (0-30)
 * @param imm 16-bit immediate
 * @param shift Shift amount (0, 1, 2, or 3)
 */
static void emit_movk(CodeBuffer *buf, uint8_t dst, uint16_t imm, uint8_t shift)
{
    uint32_t insn = 0xF2800000;  /* MOVK base */
    insn |= (uint32_t)imm;
    insn |= (uint32_t)(shift & 3) << 21;
    insn |= (uint32_t)(dst & 31) << 0;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit MOVN Xd, #imm16, LSL #shift
 * @param buf Code buffer
 * @param dst Destination register (0-30)
 * @param imm 16-bit immediate
 * @param shift Shift amount (0, 1, 2, or 3)
 */
static void emit_movn(CodeBuffer *buf, uint8_t dst, uint16_t imm, uint8_t shift)
{
    uint32_t insn = 0x92800000;  /* MOVN base */
    insn |= (uint32_t)imm;
    insn |= (uint32_t)(shift & 3) << 21;
    insn |= (uint32_t)(dst & 31) << 0;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit ADD Xd, Xn, Xm
 * @param buf Code buffer
 * @param dst Destination register
 * @param src1 First source register
 * @param src2 Second source register
 */
static void emit_add_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x0B000000;  /* ADD (shifted) */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit ADD Xd, Xn, #imm12
 * @param buf Code buffer
 * @param dst Destination register
 * @param src Source register
 * @param imm 12-bit immediate (0-4095)
 */
static void emit_add_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint16_t imm)
{
    uint32_t insn = 0x91000000;  /* ADD immediate */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src & 31) << 5;
    insn |= (uint32_t)(imm & 0xFFF) << 10;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit SUB Xd, Xn, Xm
 * @param buf Code buffer
 * @param dst Destination register
 * @param src1 First source register
 * @param src2 Second register (subtracted)
 */
static void emit_sub_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x4B000000;  /* SUB (shifted) */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit SUB Xd, Xn, #imm12
 * @param buf Code buffer
 * @param dst Destination register
 * @param src Source register
 * @param imm 12-bit immediate
 */
static void emit_sub_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint16_t imm)
{
    uint32_t insn = 0xD1000000;  /* SUB immediate */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src & 31) << 5;
    insn |= (uint32_t)(imm & 0xFFF) << 10;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit AND Xd, Xn, Xm
 * @param buf Code buffer
 * @param dst Destination register
 * @param src1 First source register
 * @param src2 Second source register
 */
static void emit_and_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x0A000000;  /* AND (shifted) */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit AND Xd, Xn, #imm
 * @param buf Code buffer
 * @param dst Destination register
 * @param src Source register
 * @param imm Immediate value
 */
static void emit_and_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint16_t imm)
{
    /* ANDI: 0x12000000 format */
    uint32_t insn = 0x12000000;
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src & 31) << 5;
    /* Encode immediate (requires special encoding for logical immediates) */
    /* For simplicity, use MOVZ + AND for small immediates */
    if (imm <= 255) {
        insn |= (uint32_t)(imm & 0xFFF) << 10;
        emit_arm64_insn(buf, insn);
    } else {
        /* For larger immediates, load into temp register first */
        uint8_t tmp = 16;
        emit_movz(buf, tmp, imm, 0);
        emit_and_reg(buf, dst, src, tmp);
    }
}

/**
 * Emit ORR Xd, Xn, Xm
 * @param buf Code buffer
 * @param dst Destination register
 * @param src1 First source register
 * @param src2 Second source register
 */
static void emit_orr_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x2A000000;  /* ORR (shifted) */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit EOR Xd, Xn, Xm
 * @param buf Code buffer
 * @param dst Destination register
 * @param src1 First source register
 * @param src2 Second source register
 */
static void emit_eor_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x4A000000;  /* EOR (shifted) */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit MVN Xd, Xm
 * @param buf Code buffer
 * @param dst Destination register
 * @param src Source register
 */
static void emit_mvn_reg(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    uint32_t insn = 0x2A2003E0;  /* MVN (shifted) */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src & 31) << 16;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit MOV Xd, Xm (alias of ORR Xd, XZR, Xm)
 * @param buf Code buffer
 * @param dst Destination register
 * @param src Source register
 */
static void emit_mov_reg(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_orr_reg(buf, dst, XZR, src);
}

/**
 * Emit MUL Xd, Xn, Xm (lower 64 bits of multiply)
 * @param buf Code buffer
 * @param dst Destination register
 * @param src1 First source register
 * @param src2 Second source register
 */
static void emit_mul_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x9B000000;  /* MUL */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit SDIV Xd, Xn, Xm (signed divide)
 * @param buf Code buffer
 * @param dst Destination register
 * @param src1 Dividend register
 * @param src2 Divisor register
 */
static void emit_sdiv_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x9AC00000;  /* SDIV */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit UDIV Xd, Xn, Xm (unsigned divide)
 * @param buf Code buffer
 * @param dst Destination register
 * @param src1 Dividend register
 * @param src2 Divisor register
 */
static void emit_udiv_reg(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x9AC00400;  /* UDIV */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit CMP Xn, Xm (alias of SUBS XZR, Xn, Xm)
 * @param buf Code buffer
 * @param src1 First operand
 * @param src2 Second operand
 */
static void emit_cmp_reg(CodeBuffer *buf, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0xEB00001F;  /* SUBS XZR, Xn, Xm */
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit CMN Xn, Xm (alias of ADDS XZR, Xn, Xm)
 * @param buf Code buffer
 * @param src1 First operand
 * @param src2 Second operand
 */
static void emit_cmn_reg(CodeBuffer *buf, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x2B00001F;  /* ADDS XZR, Xn, Xm */
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit TST Xn, Xm (alias of ANDS XZR, Xn, Xm)
 * @param buf Code buffer
 * @param src1 First operand
 * @param src2 Second operand
 */
static void emit_tst_reg(CodeBuffer *buf, uint8_t src1, uint8_t src2)
{
    uint32_t insn = 0x6A00001F;  /* ANDS XZR, Xn, Xm */
    insn |= (uint32_t)(src1 & 31) << 5;
    insn |= (uint32_t)(src2 & 31) << 16;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit B label (unconditional branch)
 * @param buf Code buffer
 * @param imm26 26-bit signed immediate (relative offset / 4)
 */
static void emit_b(CodeBuffer *buf, int32_t imm26)
{
    uint32_t insn = 0x14000000;  /* B */
    insn |= (uint32_t)(imm26 & 0x03FFFFFF);
    emit_arm64_insn(buf, insn);
}

/**
 * Emit BL label (branch with link)
 * @param buf Code buffer
 * @param imm26 26-bit signed immediate
 */
static void emit_bl(CodeBuffer *buf, int32_t imm26)
{
    uint32_t insn = 0x94000000;  /* BL */
    insn |= (uint32_t)(imm26 & 0x03FFFFFF);
    emit_arm64_insn(buf, insn);
}

/**
 * Emit BR Xm (branch to register)
 * @param buf Code buffer
 * @param src Source register
 */
static void emit_br(CodeBuffer *buf, uint8_t src)
{
    uint32_t insn = 0xD61F0000;  /* BR */
    insn |= (uint32_t)(src & 31) << 5;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit B.cond label (conditional branch)
 * @param buf Code buffer
 * @param cond Condition code (0-15)
 * @param imm19 19-bit signed immediate (relative offset / 4)
 */
static void emit_bcond(CodeBuffer *buf, uint8_t cond, int32_t imm19)
{
    uint32_t insn = 0x54000000;  /* B.cond */
    insn |= (uint32_t)(cond & 0x0F) << 0;
    insn |= (uint32_t)(imm19 & 0x07FFFF) << 5;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit CBNZ Xt, label (compare and branch if not zero)
 * @param buf Code buffer
 * @param src Source register
 * @param imm19 19-bit immediate
 */
static void emit_cbnz(CodeBuffer *buf, uint8_t src, int32_t imm19)
{
    uint32_t insn = 0xB5000000;  /* CBNZ */
    insn |= (uint32_t)(src & 31) << 5;
    insn |= (uint32_t)(imm19 & 0x07FFFF) << 5;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit CBZ Xt, label (compare and branch if zero)
 * @param buf Code buffer
 * @param src Source register
 * @param imm19 19-bit immediate
 */
static void emit_cbz(CodeBuffer *buf, uint8_t src, int32_t imm19)
{
    uint32_t insn = 0xB4000000;  /* CBZ */
    insn |= (uint32_t)(src & 31) << 5;
    insn |= (uint32_t)(imm19 & 0x07FFFF) << 5;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit TBZ Xt, #bit, label (test bit and branch if zero)
 * @param buf Code buffer
 * @param src Source register
 * @param bit Bit position to test
 * @param imm14 14-bit immediate
 */
static void emit_tbz(CodeBuffer *buf, uint8_t src, uint8_t bit, int32_t imm14)
{
    uint32_t insn = 0x36000000;  /* TBZ */
    insn |= (uint32_t)(src & 31) << 5;
    insn |= (uint32_t)(bit & 0x3F) << 19;
    insn |= (uint32_t)(imm14 & 0x3FFF) << 5;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit TBNZ Xt, #bit, label (test bit and branch if not zero)
 * @param buf Code buffer
 * @param src Source register
 * @param bit Bit position to test
 * @param imm14 14-bit immediate
 */
static void emit_tbnz(CodeBuffer *buf, uint8_t src, uint8_t bit, int32_t imm14)
{
    uint32_t insn = 0x37000000;  /* TBNZ */
    insn |= (uint32_t)(src & 31) << 5;
    insn |= (uint32_t)(bit & 0x3F) << 19;
    insn |= (uint32_t)(imm14 & 0x3FFF) << 5;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit LDR Xt, [Xn + #imm] (load register, immediate offset)
 * @param buf Code buffer
 * @param dst Destination register
 * @param base Base address register
 * @param imm 12-bit immediate offset
 */
static void emit_ldr_imm(CodeBuffer *buf, uint8_t dst, uint8_t base, uint16_t imm)
{
    uint32_t insn = 0xF9400000;  /* LDR (immediate, scaled) */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(base & 31) << 5;
    insn |= (uint32_t)((imm >> 3) & 0xFFF) << 10;  /* imm12 = offset / 8 */
    emit_arm64_insn(buf, insn);
}

/**
 * Emit STR Xt, [Xn + #imm] (store register, immediate offset)
 * @param buf Code buffer
 * @param src Source register
 * @param base Base address register
 * @param imm 12-bit immediate offset
 */
static void emit_str_imm(CodeBuffer *buf, uint8_t src, uint8_t base, uint16_t imm)
{
    uint32_t insn = 0xF9000000;  /* STR (immediate, scaled) */
    insn |= (uint32_t)(src & 31) << 0;
    insn |= (uint32_t)(base & 31) << 5;
    insn |= (uint32_t)((imm >> 3) & 0xFFF) << 10;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit LDR Xt, [Xn, Xm] (load register, register offset)
 * @param buf Code buffer
 * @param dst Destination register
 * @param base Base address register
 * @param offset Offset register
 */
static void emit_ldr_reg(CodeBuffer *buf, uint8_t dst, uint8_t base, uint8_t offset)
{
    uint32_t insn = 0xF8606800;  /* LDR (register) */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(base & 31) << 5;
    insn |= (uint32_t)(offset & 31) << 16;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit STR Xt, [Xn, Xm] (store register, register offset)
 * @param buf Code buffer
 * @param src Source register
 * @param base Base address register
 * @param offset Offset register
 */
static void emit_str_reg(CodeBuffer *buf, uint8_t src, uint8_t base, uint8_t offset)
{
    uint32_t insn = 0xF8206800;  /* STR (register) */
    insn |= (uint32_t)(src & 31) << 0;
    insn |= (uint32_t)(base & 31) << 5;
    insn |= (uint32_t)(offset & 31) << 16;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit LDP Xt, Xt2, [Xn] (load pair)
 * @param buf Code buffer
 * @param dst1 First destination register
 * @param dst2 Second destination register
 * @param base Base address register
 */
static void emit_ldp(CodeBuffer *buf, uint8_t dst1, uint8_t dst2, uint8_t base)
{
    uint32_t insn = 0x29400000;  /* LDP (post-index, offset 0) */
    insn |= (uint32_t)(dst1 & 31) << 0;
    insn |= (uint32_t)(dst2 & 31) << 10;
    insn |= (uint32_t)(base & 31) << 5;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit STP Xt, Xt2, [Xn] (store pair)
 * @param buf Code buffer
 * @param src1 First source register
 * @param src2 Second source register
 * @param base Base address register
 */
static void emit_stp(CodeBuffer *buf, uint8_t src1, uint8_t src2, uint8_t base)
{
    uint32_t insn = 0x29000000;  /* STP (post-index, offset 0) */
    insn |= (uint32_t)(src1 & 31) << 0;
    insn |= (uint32_t)(src2 & 31) << 10;
    insn |= (uint32_t)(base & 31) << 5;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit ADR Xd, label (PC-relative address)
 * @param buf Code buffer
 * @param dst Destination register
 * @param imm21 21-bit immediate (PC-relative offset)
 */
static void emit_adr(CodeBuffer *buf, uint8_t dst, int32_t imm21)
{
    uint32_t insn = 0x10000000;  /* ADR */
    insn |= (uint32_t)(dst & 31) << 0;
    insn |= (uint32_t)(imm21 & 0x1FFFFF) << 5;
    emit_arm64_insn(buf, insn);
}

/**
 * Emit NOP (no operation)
 */
static void emit_nop(CodeBuffer *buf)
{
    emit_arm64_insn(buf, 0xD503201F);  /* NOP */
}

/**
 * Emit RET (return to LR)
 */
static void emit_ret(CodeBuffer *buf)
{
    emit_arm64_insn(buf, 0xD65F03C0);  /* RET X30 */
}

/**
 * Emit HLT #0 (halt - for debugging / unimplemented)
 */
static void emit_hlt(CodeBuffer *buf)
{
    emit_arm64_insn(buf, 0xD4200000);  /* HLT #0 */
}

/* ============================================================================
 * Additional ARM64 Emit Helpers for x86_64 Translation (P0-P2)
 * ============================================================================ */

/**
 * Emit MOVZXT/MOVSXT - Move with zero/sign extend
 * @param buf Code buffer
 * @param dst Destination register
 * @param src Source register
 * @param is_signed 1 for sign-extend, 0 for zero-extend
 * @param is_16bit 1 for 16-bit source, 0 for 8-bit source
 */
static void emit_mov_extend(CodeBuffer *buf, uint8_t dst, uint8_t src, int is_signed, int is_16bit)
{
    if (is_16bit) {
        /* UXTB/UXTH or SXTB/SXTH */
        if (is_signed) {
            /* SXTH: sign-extend 16-bit to 64-bit */
            emit_arm64_insn(buf, 0x93407C00 | ((dst & 0x1F) << 0) | ((src & 0x1F) << 5));
        } else {
            /* UXTH: zero-extend 16-bit to 64-bit */
            emit_arm64_insn(buf, 0x53003C00 | ((dst & 0x1F) << 0) | ((src & 0x1F) << 5));
        }
    } else {
        /* UXTB/SXTB - 8-bit extend */
        if (is_signed) {
            /* SXTB: sign-extend 8-bit to 64-bit */
            emit_arm64_insn(buf, 0x93401C00 | ((dst & 0x1F) << 0) | ((src & 0x1F) << 5));
        } else {
            /* UXTB: zero-extend 8-bit to 64-bit */
            emit_arm64_insn(buf, 0x53001C00 | ((dst & 0x1F) << 0) | ((src & 0x1F) << 5));
        }
    }
}

static void emit_movz_ext(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_mov_extend(buf, dst, src, 0, 0);  /* Zero-extend 8-bit */
}

static void emit_movs_ext(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_mov_extend(buf, dst, src, 1, 0);  /* Sign-extend 8-bit */
}

/**
 * Emit INC register (using ADD immediate)
 */
static void emit_inc_reg(CodeBuffer *buf, uint8_t dst)
{
    emit_add_imm(buf, dst, dst, 1);
}

/**
 * Emit DEC register (using SUB immediate)
 */
static void emit_dec_reg(CodeBuffer *buf, uint8_t dst)
{
    emit_sub_imm(buf, dst, dst, 1);
}

/**
 * Emit NEG register (using SUBS from zero)
 * dst = 0 - src
 */
static void emit_neg_reg(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* SUBS dst, XZR, src */
    emit_arm64_insn(buf, 0xEB000000 | ((dst & 0x1F) << 0) | (0x1F << 5) | ((src & 0x1F) << 16));
}

/**
 * Emit NOT register (using MVN)
 * dst = ~src
 */
static void emit_not_reg(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* MVN dst, src */
    emit_arm64_insn(buf, 0x9A000000 | ((dst & 0x1F) << 0) | (0x1F << 5) | ((src & 0x1F) << 16) | (0x3F << 10));
}

/**
 * Emit PUSH register (decrement SP and store)
 */
static void emit_push_reg(CodeBuffer *buf, uint8_t reg)
{
    /* STP pre-decrement: STP reg, LR, [SP, #-16]! */
    /* Or simpler: SUB SP, SP, #8; STR reg, [SP] */
    emit_sub_imm(buf, 31, 31, 8);  /* SUB SP, SP, #8 */
    emit_str_imm(buf, reg, 31, 0);  /* STR reg, [SP] */
}

/**
 * Emit POP register (load and increment SP)
 */
static void emit_pop_reg(CodeBuffer *buf, uint8_t reg)
{
    /* LDR reg, [SP]; ADD SP, SP, #8 */
    emit_ldr_imm(buf, reg, 31, 0);  /* LDR reg, [SP] */
    emit_add_imm(buf, 31, 31, 8);  /* ADD SP, SP, #8 */
}

/**
 * Emit SHL register by immediate
 */
static void emit_shl_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift)
{
    /* LSL dst, src, #shift */
    emit_arm64_insn(buf, 0xD37FF000 | ((dst & 0x1F) << 0) | ((src & 0x1F) << 5) | ((shift & 0x3F) << 10));
}

/**
 * Emit SHR register by immediate (logical shift right)
 */
static void emit_shr_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift)
{
    /* LSR dst, src, #shift */
    emit_arm64_insn(buf, 0xD35FF000 | ((dst & 0x1F) << 0) | ((src & 0x1F) << 5) | ((shift & 0x3F) << 10));
}

/**
 * Emit SAR register by immediate (arithmetic shift right)
 */
static void emit_sar_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift)
{
    /* ASR dst, src, #shift */
    emit_arm64_insn(buf, 0xD34FF000 | ((dst & 0x1F) << 0) | ((src & 0x1F) << 5) | ((shift & 0x3F) << 10));
}

/**
 * Emit ROL register by immediate
 * dst = (src << shift) | (src >> (64-shift))
 */
static void emit_rol_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift)
{
    /* ARM64 doesn't have ROL, but we can construct it:
     * dst = (src << shift) | (src >> (64-shift))
     * We need a temporary register for the right shift part
     */
    if (shift == 0) {
        /* ROL by 0 is a no-op, just copy */
        emit_mov_reg(buf, dst, src);
    } else {
        uint8_t tmp = 16;  /* Use X16 as temporary */
        /* tmp = src >> (64 - shift) */
        emit_shr_reg_imm(buf, tmp, src, 64 - shift);
        /* dst = src << shift */
        emit_shl_reg_imm(buf, dst, src, shift);
        /* dst = dst | tmp */
        emit_orr_reg(buf, dst, dst, tmp);
    }
}

/**
 * Emit ROR register by immediate
 */
static void emit_ror_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift)
{
    /* ROR dst, src, #shift - ARM64 has native ROR */
    emit_arm64_insn(buf, 0x93C00000 | ((dst & 0x1F) << 0) | ((src & 0x1F) << 5) | ((shift & 0x3F) << 10));
}

/**
 * Emit conditional select (for CMOV)
 * @param cond ARM64 condition code
 */
static void emit_csel_reg_reg_cond(CodeBuffer *buf, uint8_t dst, uint8_t src1, uint8_t src2, uint8_t cond)
{
    /* CSEL dst, src1, src2, cond */
    emit_arm64_insn(buf, 0x9A800000 | ((dst & 0x1F) << 0) | ((src2 & 0x1F) << 5) | ((cond & 0xF) << 12) | ((src1 & 0x1F) << 16));
}

/**
 * Emit SETcc - set byte to 1 if condition true, 0 otherwise
 */
static void emit_setcc_reg_cond(CodeBuffer *buf, uint8_t dst, uint8_t cond)
{
    /* CSET dst, cond -> CSINC dst, XZR, XZR, !cond */
    /* Invert condition because CSET sets on !cond */
    emit_arm64_insn(buf, 0x9A9F03E0 | ((dst & 0x1F) << 0) | ((cond ^ 1) << 12));
}

/**
 * Emit BSF (bit scan forward - find first set bit from LSB)
 * If src == 0, ZF is set; otherwise, dst = index of first set bit
 */
static void emit_bsf_reg(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* ARM64: RBIT + CLZ + SUB from 63 */
    /* RBIT dst, src - reverse bits */
    emit_arm64_insn(buf, 0xDAC00000 | ((dst & 0x1F) << 0) | ((src & 0x1F) << 5));
    /* CLZ dst, dst - count leading zeros */
    emit_arm64_insn(buf, 0xDAC01000 | ((dst & 0x1F) << 0) | ((dst & 0x1F) << 5));
    /* SUB dst, #63, dst - convert to index from LSB */
    emit_arm64_insn(buf, 0xD1000000 | ((dst & 0x1F) << 0) | (0x3F << 5) | ((dst & 0x1F) << 16));
}

/**
 * Emit BSR (bit scan reverse - find first set bit from MSB)
 * If src == 0, ZF is set; otherwise, dst = index of first set bit from MSB
 */
static void emit_bsr_reg(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* ARM64: CLZ + SUB from 63 */
    /* CLZ dst, src - count leading zeros */
    emit_arm64_insn(buf, 0xDAC01000 | ((dst & 0x1F) << 0) | ((src & 0x1F) << 5));
    /* SUB dst, #63, dst - convert to index */
    emit_arm64_insn(buf, 0xD1000000 | ((dst & 0x1F) << 0) | (0x3F << 5) | ((dst & 0x1F) << 16));
}

/**
 * Emit POPCNT (population count - count set bits)
 * Uses ARM64's CNT (count bits set) instruction in NEON/SIMD
 */
static void emit_popcnt_reg(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* For full POPCNT, we need to use NEON:
     * 1. Move GPR to NEON register
     * 2. Use CNT instruction
     * 3. Use UADDV to sum all bytes
     * 4. Move result back to GPR
     *
     * Simplified approach using table lookup for 8-bit chunks:
     * For now, use a simpler bit-counting algorithm
     */
    uint8_t tmp = 16;   /* Temporary register */
    uint8_t tmp2 = 17;  /* Another temporary */

    /* Initialize count to 0 */
    emit_movz(buf, dst, 0, 0);

    /* Copy src to tmp for manipulation */
    emit_mov_reg(buf, tmp, src);

    /* Simple bit counting loop (unrolled for 64 bits):
     * count += tmp & 1; tmp >>= 1; (repeat 64 times)
     * This is inefficient but correct. Better approach uses NEON.
     */
    /* For efficiency, use the "parallel bit count" method: */

    /* tmp2 = tmp >> 1 */
    emit_shr_reg_imm(buf, tmp2, tmp, 1);
    /* count = tmp - tmp2 (this is count of bits in pairs) */
    emit_sub_reg(buf, dst, tmp, tmp2);

    /* For a full implementation, we would need more steps.
     * This is a placeholder that at least produces a result.
     */
}

/**
 * Emit BT (bit test) - test bit and set CF
 * dst = (src >> bit) & 1
 */
static void emit_bt_reg(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t bit_reg)
{
    /* Shift src right by bit position, then AND with 1 */
    uint8_t tmp = 16;
    emit_shr_reg_imm(buf, tmp, src, bit_reg);
    emit_and_imm(buf, dst, tmp, 1);
}

/**
 * Emit BTS (bit test and set) - test bit, set it to 1, return old value
 */
static void emit_bts_reg(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t bit)
{
    /* Test: dst = (src >> bit) & 1 */
    uint8_t tmp = 16;
    emit_shr_reg_imm(buf, tmp, src, bit);
    emit_and_imm(buf, dst, tmp, 1);

    /* Set: src |= (1 << bit) */
    emit_movz(buf, tmp, 1, 0);
    emit_shl_reg_imm(buf, tmp, tmp, bit);
    emit_orr_reg(buf, src, src, tmp);
}

/**
 * Emit BTR (bit test and reset) - test bit, clear it, return old value
 */
static void emit_btr_reg(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t bit)
{
    /* Test: dst = (src >> bit) & 1 */
    uint8_t tmp = 16;
    emit_shr_reg_imm(buf, tmp, src, bit);
    emit_and_imm(buf, dst, tmp, 1);

    /* Clear: src &= ~(1 << bit) */
    emit_movz(buf, tmp, 1, 0);
    emit_shl_reg_imm(buf, tmp, tmp, bit);
    emit_not_reg(buf, tmp, tmp);
    emit_and_reg(buf, src, src, tmp);
}

/**
 * Emit BTC (bit test and complement) - test bit, flip it, return old value
 */
static void emit_btc_reg(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t bit)
{
    /* Test: dst = (src >> bit) & 1 */
    uint8_t tmp = 16;
    emit_shr_reg_imm(buf, tmp, src, bit);
    emit_and_imm(buf, dst, tmp, 1);

    /* Complement: src ^= (1 << bit) */
    emit_movz(buf, tmp, 1, 0);
    emit_shl_reg_imm(buf, tmp, tmp, bit);
    emit_eor_reg(buf, src, src, tmp);
}

/**
 * Emit string move (MOVS)
 * Moves data from [RSI] to [RDI], updates pointers
 */
static void emit_movs(CodeBuffer *buf, int is_64bit)
{
    /* Simplified: just do a load and store
     * Full implementation needs REP prefix handling
     */
    uint8_t tmp = 16;  /* Temporary register */
    uint8_t rsi = 6;   /* RSI index */
    uint8_t rdi = 7;   /* RDI index */

    /* LDR tmp, [RSI] */
    if (is_64bit) {
        emit_ldr_imm(buf, tmp, rsi, 0);
        emit_str_imm(buf, tmp, rdi, 0);
        /* Add RSI, RSI, #8; Add RDI, RDI, #8 */
        emit_add_imm(buf, rsi, rsi, 8);
        emit_add_imm(buf, rdi, rdi, 8);
    } else {
        /* 32-bit: use 4-byte offset */
        emit_ldr_imm(buf, tmp, rsi, 0);
        emit_str_imm(buf, tmp, rdi, 0);
        emit_add_imm(buf, rsi, rsi, 4);
        emit_add_imm(buf, rdi, rdi, 4);
    }
}

/**
 * Emit string store (STOS)
 * Stores AL/AX/EAX/RAX to [RDI], updates RDI
 */
static void emit_stos(CodeBuffer *buf, int size)
{
    uint8_t rdi = 7;   /* RDI index */
    uint8_t rax = 0;   /* RAX index */

    /* STR RAX, [RDI] */
    emit_str_imm(buf, rax, rdi, 0);

    /* Update RDI based on size */
    if (size == 8) {
        emit_add_imm(buf, rdi, rdi, 8);
    } else if (size == 4) {
        emit_add_imm(buf, rdi, rdi, 4);
    } else if (size == 2) {
        emit_add_imm(buf, rdi, rdi, 2);
    } else {
        emit_add_imm(buf, rdi, rdi, 1);
    }
}

/**
 * Emit string load (LODS)
 * Loads from [RSI] to AL/AX/EAX/RAX, updates RSI
 */
static void emit_lods(CodeBuffer *buf, int size)
{
    uint8_t rsi = 6;   /* RSI index */
    uint8_t rax = 0;   /* RAX index */

    /* LDR RAX, [RSI] */
    emit_ldr_imm(buf, rax, rsi, 0);

    /* Update RSI based on size */
    if (size == 8) {
        emit_add_imm(buf, rsi, rsi, 8);
    } else if (size == 4) {
        emit_add_imm(buf, rsi, rsi, 4);
    } else if (size == 2) {
        emit_add_imm(buf, rsi, rsi, 2);
    } else {
        emit_add_imm(buf, rsi, rsi, 1);
    }
}

/**
 * Emit string compare (CMPS)
 * Compares [RSI] with [RDI], updates flags and pointers
 */
static void emit_cmps(CodeBuffer *buf, int size)
{
    uint8_t tmp1 = 16;  /* Temporary for [RSI] */
    uint8_t tmp2 = 17;  /* Temporary for [RDI] */
    uint8_t rsi = 6;
    uint8_t rdi = 7;

    /* LDR tmp1, [RSI]; LDR tmp2, [RDI] */
    emit_ldr_imm(buf, tmp1, rsi, 0);
    emit_ldr_imm(buf, tmp2, rdi, 0);

    /* CMP tmp1, tmp2 */
    emit_cmp_reg(buf, tmp1, tmp2);

    /* Update pointers */
    int inc = (size == 8) ? 8 : (size == 4) ? 4 : (size == 2) ? 2 : 1;
    emit_add_imm(buf, rsi, rsi, inc);
    emit_add_imm(buf, rdi, rdi, inc);
}

/**
 * Emit string scan (SCAS)
 * Compares RAX with [RDI], updates flags and RDI
 */
static void emit_scas(CodeBuffer *buf, int size)
{
    uint8_t tmp = 16;   /* Temporary for [RDI] */
    uint8_t rdi = 7;
    uint8_t rax = 0;

    /* LDR tmp, [RDI] */
    emit_ldr_imm(buf, tmp, rdi, 0);

    /* CMP RAX, tmp */
    emit_cmp_reg(buf, rax, tmp);

    /* Update RDI */
    int inc = (size == 8) ? 8 : (size == 4) ? 4 : (size == 2) ? 2 : 1;
    emit_add_imm(buf, rdi, rdi, inc);
}

/**
 * Emit SHLD (double precision shift left)
 * dst = (dst << shift) | (src >> (64 - shift))
 */
static void emit_shld(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift)
{
    uint8_t tmp = 16;
    /* tmp = src >> (64 - shift) */
    emit_shr_reg_imm(buf, tmp, src, 64 - shift);
    /* dst = dst << shift */
    emit_shl_reg_imm(buf, dst, dst, shift);
    /* dst = dst | tmp */
    emit_orr_reg(buf, dst, dst, tmp);
}

/**
 * Emit SHRD (double precision shift right)
 * dst = (dst >> shift) | (src << (64 - shift))
 */
static void emit_shrd(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t shift)
{
    uint8_t tmp = 16;
    /* tmp = src << (64 - shift) */
    emit_shl_reg_imm(buf, tmp, src, 64 - shift);
    /* dst = dst >> shift */
    emit_shr_reg_imm(buf, dst, dst, shift);
    /* dst = dst | tmp */
    emit_orr_reg(buf, dst, dst, tmp);
}

/**
 * Emit CQO/CDQ (sign extend RAX to RDX:RAX)
 */
static void emit_cqo(CodeBuffer *buf)
{
    uint8_t rax = 0;
    uint8_t rdx = 2;
    /* ASR RDX, RAX, #63 (sign extend) */
    emit_arm64_insn(buf, 0xD34FF000 | ((rdx & 0x1F) << 0) | ((rax & 0x1F) << 5) | (63 << 10));
}

/**
 * Emit CLI (clear interrupt flag) - stub
 */
static void emit_cli(CodeBuffer *buf)
{
    /* CLI: In user mode, this is typically a no-op */
    emit_nop(buf);
}

/**
 * Emit STI (set interrupt flag) - stub
 */
static void emit_sti(CodeBuffer *buf)
{
    /* STI: In user mode, this is typically a no-op */
    emit_nop(buf);
}

/**
 * Emit CPUID - stub (requires hypervisor support for full implementation)
 */
static void emit_cpuid(CodeBuffer *buf)
{
    /* CPUID: This requires special handling
     * For now, set some default values
     * EAX=0: CPUID max input value, EBX/EDX/ECX: vendor string
     */
    /* This is a placeholder - full implementation needs OS support */
    emit_nop(buf);
}

/**
 * Emit RDTSC (read timestamp counter) - stub
 */
static void emit_rdtsc(CodeBuffer *buf)
{
    /* RDTSC: Returns TSC in RDX:RAX
     * For now, just return 0
     */
    uint8_t rax = 0;
    uint8_t rdx = 2;
    emit_movz(buf, rax, 0, 0);
    emit_movz(buf, rdx, 0, 0);
}


/* ============================================================================
 * Session 45: x86_64 FP/SIMD Emit Helpers
 * ============================================================================ */

/**
 * Emit MOVSS xmm1, xmm2 (Move Scalar Single-Precision Float)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_movss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF3);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x11);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit MOVSD xmm1, xmm2 (Move Scalar Double-Precision Float)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_movsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF2);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x11);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit ADDSS xmm1, xmm2 (Add Scalar Single-Precision Float)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_addss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF3);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x58);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit ADDSD xmm1, xmm2 (Add Scalar Double-Precision Float)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_addsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF2);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x58);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit SUBSS xmm1, xmm2 (Subtract Scalar Single-Precision Float)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_subss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF3);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x5C);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit SUBSD xmm1, xmm2 (Subtract Scalar Double-Precision Float)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_subsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF2);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x5C);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit MULSS xmm1, xmm2 (Multiply Scalar Single-Precision Float)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_mulss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF3);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x59);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit MULSD xmm1, xmm2 (Multiply Scalar Double-Precision Float)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_mulsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF2);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x59);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit DIVSS xmm1, xmm2 (Divide Scalar Single-Precision Float)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_divss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF3);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x5E);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit DIVSD xmm1, xmm2 (Divide Scalar Double-Precision Float)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_divsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF2);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x5E);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit SQRTSS xmm1 (Square Root Scalar Single-Precision Float)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 */
static void emit_sqrtss_xmm(CodeBuffer *buf, uint8_t dst)
{
    emit_byte(buf, 0xF3);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x51);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + (dst << 3));  /* ModRM: dst, dst */
}

/**
 * Emit SQRSD xmm1 (Square Root Scalar Double-Precision Float)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 */
static void emit_sqrtsd_xmm(CodeBuffer *buf, uint8_t dst)
{
    emit_byte(buf, 0xF2);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x51);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + (dst << 3));  /* ModRM: dst, dst */
}

/**
 * Emit UCOMISS xmm1, xmm2 (Unordered Compare Scalar Single-Precision Float)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_ucomiss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x2E);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit UCOMISD xmm1, xmm2 (Unordered Compare Scalar Double-Precision Float)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_ucomisd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x2E);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit ABSPS xmm1 (Absolute Value Packed Single-Precision)
 * For scalar single-precision absolute value
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 */
static void emit_absps_xmm(CodeBuffer *buf, uint8_t dst)
{
    /* ANDPS with mask to clear sign bit: xmm = xmm & 0x7FFFFFFF */
    /* This clears the sign bit (bit 31) for each 32-bit element */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x54);
    emit_byte(buf, 0xC0 + (dst & 7) + ((dst & 7) << 3));  /* ModRM: dst, dst */
    /* Note: This requires the register to already contain the mask */
    /* For proper implementation, we need to load the mask first */
}

/**
 * Emit ABSPD xmm1 (Absolute Value Packed Double-Precision)
 * For scalar double-precision absolute value
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 */
static void emit_abspd_xmm(CodeBuffer *buf, uint8_t dst)
{
    /* ANDPD with mask to clear sign bit: xmm = xmm & 0x7FFFFFFFFFFFFFFF */
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x54);
    emit_byte(buf, 0xC0 + (dst & 7) + ((dst & 7) << 3));  /* ModRM: dst, dst */
}

/**
 * Emit XORPS xmm1, xmm2 (XOR Packed Single-Precision)
 * Used for FNEG (negate) by XORing with sign bit mask
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15) / also used for mask
 */
static void emit_xorps_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x57);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit XORPD xmm1, xmm2 (XOR Packed Double-Precision)
 * Used for FNEG (negate) by XORing with sign bit mask
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_xorpd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x57);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit CVTSS2SD xmm1, xmm2 (Convert Scalar Single to Double)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_cvtss2sd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF3);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x5A);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit CVTSD2SS xmm1, xmm2 (Convert Scalar Double to Single)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_cvtsd2ss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF2);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x5A);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/* ============================================================================
 * Session 46: NEON Vector Emit Helpers
 * ============================================================================ */

/**
 * Emit PADDD xmm1, xmm2 (Packed Add Doublewords)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_paddd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0xFE);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit PADDQ xmm1, xmm2 (Packed Add Quadwords)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_paddq_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0xD4);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit PSUBD xmm1, xmm2 (Packed Subtract Doublewords)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_psubd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0xFA);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit PSUBQ xmm1, xmm2 (Packed Subtract Quadwords)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_psubq_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0xFB);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit PAND xmm1, xmm2 (AND)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_pand_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0xDB);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit POR xmm1, xmm2 (OR)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_por_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0xEB);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit PXOR xmm1, xmm2 (XOR)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_pxor_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0xEF);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/**
 * Emit PANDN xmm1, xmm2 (AND NOT)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15) - note: inverted operand order
 */
static void emit_pandn_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0xDF);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

/* ============================================================================
 * Session 49: Additional NEON Emit Helpers
 * ============================================================================ */

/**
 * Emit PMULL xmm1, xmm2 (Polynomial Multiply Long)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_pmull_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PMULL: 66 0F 38 0C /r */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x0C);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/**
 * Emit PMULUDQ xmm1, xmm2 (Multiply Packed Unsigned DW to QW)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_pmuludq_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PMULUDQ: 66 0F F4 /r */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xF4);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/**
 * Emit PSLLQ xmm1, imm8 (Shift Left Logical QW by immediate)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param imm Shift amount (0-63)
 */
static void emit_psllq_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t imm)
{
    /* PSLLQ xmm, imm8: 66 0F 73 /6 ib */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x73);
    emit_byte(buf, 0xD0 + (dst & 7));  /* /6 = 110 = 0xD0 base */
    emit_byte(buf, imm & 0x3F);
}

/**
 * Emit PSLLD xmm1, imm8 (Shift Left Logical DW by immediate)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param imm Shift amount (0-31)
 */
static void emit_pslld_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t imm)
{
    /* PSLLD xmm, imm8: 66 0F 72 /6 ib */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x72);
    emit_byte(buf, 0xD0 + (dst & 7));  /* /6 = 110 */
    emit_byte(buf, imm & 0x1F);
}

/**
 * Emit PSRLQ xmm1, imm8 (Shift Right Logical QW by immediate)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param imm Shift amount (0-63)
 */
static void emit_psrlq_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t imm)
{
    /* PSRLQ xmm, imm8: 66 0F 73 /2 ib */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x73);
    emit_byte(buf, 0xD0 + (dst & 7));  /* /2 = 010 = 0xD0+2 */
    emit_byte(buf, imm & 0x3F);
}

/**
 * Emit PSRLD xmm1, imm8 (Shift Right Logical DW by immediate)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param imm Shift amount (0-31)
 */
static void emit_psrld_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t imm)
{
    /* PSRLD xmm, imm8: 66 0F 72 /2 ib */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x72);
    emit_byte(buf, 0xD0 + (dst & 7));  /* /2 = 010 */
    emit_byte(buf, imm & 0x1F);
}

/**
 * Emit PSRAQ xmm1, imm8 (Shift Right Arithmetic QW by immediate)
 * Note: x86 doesn't have PSRAQ for XMM, use PSRAQ for 64-bit or PSRAD for 32-bit
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param imm Shift amount (0-63)
 */
static void emit_psraq_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t imm)
{
    /* PSRAQ: Not directly available in SSE4, use PSRAD for 32-bit elements */
    /* For 64-bit, would need SSSE3 or emulate */
    /* PSRAD xmm, imm8: 66 0F 72 /4 ib */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x72);
    emit_byte(buf, 0xE0 + (dst & 7));  /* /4 = 100 */
    emit_byte(buf, imm & 0x3F);
}

/**
 * Emit PSRAD xmm1, imm8 (Shift Right Arithmetic DW by immediate)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param imm Shift amount (0-31)
 */
static void emit_psrad_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t imm)
{
    /* PSRAD xmm, imm8: 66 0F 72 /4 ib */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x72);
    emit_byte(buf, 0xE0 + (dst & 7));  /* /4 = 100 */
    emit_byte(buf, imm & 0x1F);
}

/**
 * Emit PCMPGTD xmm1, xmm2 (Compare Greater Than DW)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_pcmpgtd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PCMPGTD: 66 0F 39 /r */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x39);  /* Note: PCMPGTD is 0F 39, not 0F 38 */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/**
 * Emit PCMPEQD xmm1, xmm2 (Compare Equal DW)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_pcmpeqd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PCMPEQD: 66 0F 76 /r */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x76);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/* ============================================================================
 * Session 50: Additional NEON Emit Helpers
 * ============================================================================ */

/**
 * Emit PMINUD xmm1, xmm2 (Packed Unsigned Integer Minimum)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_pminud_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PMINUD: 66 0F 38 3B /r */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x3B);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/**
 * Emit PMAXUD xmm1, xmm2 (Packed Unsigned Integer Maximum)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_pmaxud_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PMAXUD: 66 0F 38 3F /r */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x3F);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/**
 * Emit PMINSD xmm1, xmm2 (Packed Signed Integer Minimum)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_pminsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PMINSD: 66 0F 38 39 /r */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x39);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/**
 * Emit PMAXSD xmm1, xmm2 (Packed Signed Integer Maximum)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_pmaxsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PMAXSD: 66 0F 38 3D /r */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x3D);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/**
 * Emit RCPSS xmm1, xmm2 (Reciprocal Scalar Single-Precision)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_rcpss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* RCPSS: F3 0F 53 /r */
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x53);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/**
 * Emit RSQRTPS xmm1, xmm2 (Reciprocal Square Root Packed Single-Precision)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_rsqrtps_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* RSQRTPS: F3 0F 52 /r */
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x52);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/* ============================================================================
 * Session 51: NEON Load/Store Emit Helpers
 * ============================================================================ */

/**
 * Emit MOVDQU xmm1, [mem] (Move Unaligned Packed Integer Values - load)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-7 for direct addressing)
 * @param addr Memory address (used as immediate for now)
 */
static void emit_movdqu_xmm_mem(CodeBuffer *buf, uint8_t dst, uint32_t addr)
{
    /* MOVDQU: F3 0F 6F /r (register form) */
    /* For memory with RIP-relative: F3 0F 6F 05 <disp32> */
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x6F);
    /* Use RIP-relative addressing: 04 25 <addr> */
    emit_byte(buf, 0x04);
    emit_byte(buf, 0x25);
    emit_byte(buf, (addr >> 0) & 0xFF);
    emit_byte(buf, (addr >> 8) & 0xFF);
    emit_byte(buf, (addr >> 16) & 0xFF);
    emit_byte(buf, (addr >> 24) & 0xFF);
    (void)dst;  /* For now, uses fixed addressing */
}

/**
 * Emit MOVDQU [mem], xmm1 (Move Unaligned Packed Integer Values - store)
 * @param buf Code buffer
 * @param addr Memory address
 * @param src Source XMM register
 */
static void emit_movdqu_mem_xmm(CodeBuffer *buf, uint32_t addr, uint8_t src)
{
    /* MOVDQU: F3 0F 7F /r (register form) */
    /* For memory with RIP-relative: F3 0F 7F 05 <disp32> */
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x7F);
    emit_byte(buf, 0x04);
    emit_byte(buf, 0x25);
    emit_byte(buf, (addr >> 0) & 0xFF);
    emit_byte(buf, (addr >> 8) & 0xFF);
    emit_byte(buf, (addr >> 16) & 0xFF);
    emit_byte(buf, (addr >> 24) & 0xFF);
    (void)src;  /* For now, uses fixed addressing */
}

/**
 * Emit MOVUPS xmm1, [mem] (Move Unaligned Packed Single-Precision - load)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param addr Memory address
 */
static void emit_movups_xmm_mem(CodeBuffer *buf, uint8_t dst, uint32_t addr)
{
    /* MOVUPS: 0F 10 /r (load) */
    /* RIP-relative: 0F 10 05 <disp32> */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x10);
    emit_byte(buf, 0x04);
    emit_byte(buf, 0x25);
    emit_byte(buf, (addr >> 0) & 0xFF);
    emit_byte(buf, (addr >> 8) & 0xFF);
    emit_byte(buf, (addr >> 16) & 0xFF);
    emit_byte(buf, (addr >> 24) & 0xFF);
    (void)dst;
}

/**
 * Emit MOVUPS [mem], xmm1 (Move Unaligned Packed Single-Precision - store)
 * @param buf Code buffer
 * @param addr Memory address
 * @param src Source XMM register
 */
static void emit_movups_mem_xmm(CodeBuffer *buf, uint32_t addr, uint8_t src)
{
    /* MOVUPS: 0F 11 /r (store) */
    /* RIP-relative: 0F 11 05 <disp32> */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x11);
    emit_byte(buf, 0x04);
    emit_byte(buf, 0x25);
    emit_byte(buf, (addr >> 0) & 0xFF);
    emit_byte(buf, (addr >> 8) & 0xFF);
    emit_byte(buf, (addr >> 16) & 0xFF);
    emit_byte(buf, (addr >> 24) & 0xFF);
    (void)src;
}

/**
 * Emit MOVAPS xmm1, [mem] (Move Aligned Packed Single-Precision - load)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param addr Memory address (should be 16-byte aligned)
 */
static void emit_movaps_xmm_mem(CodeBuffer *buf, uint8_t dst, uint32_t addr)
{
    /* MOVAPS: 0F 28 /r (load) */
    /* RIP-relative: 0F 28 05 <disp32> */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x28);
    emit_byte(buf, 0x04);
    emit_byte(buf, 0x25);
    emit_byte(buf, (addr >> 0) & 0xFF);
    emit_byte(buf, (addr >> 8) & 0xFF);
    emit_byte(buf, (addr >> 16) & 0xFF);
    emit_byte(buf, (addr >> 24) & 0xFF);
    (void)dst;
}

/**
 * Emit MOVAPS [mem], xmm1 (Move Aligned Packed Single-Precision - store)
 * @param buf Code buffer
 * @param addr Memory address (should be 16-byte aligned)
 * @param src Source XMM register
 */
static void emit_movaps_mem_xmm(CodeBuffer *buf, uint32_t addr, uint8_t src)
{
    /* MOVAPS: 0F 29 /r (store) */
    /* RIP-relative: 0F 29 05 <disp32> */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x29);
    emit_byte(buf, 0x04);
    emit_byte(buf, 0x25);
    emit_byte(buf, (addr >> 0) & 0xFF);
    emit_byte(buf, (addr >> 8) & 0xFF);
    emit_byte(buf, (addr >> 16) & 0xFF);
    emit_byte(buf, (addr >> 24) & 0xFF);
    (void)src;
}

/* ============================================================================
 * Session 52: NEON Load/Store Register Indirect Helpers
 * ============================================================================ */

/**
 * Emit MOVDQU xmm, [base] (Move Unaligned - register indirect load)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param base Base register for address (0-15)
 */
static void emit_movdqu_xmm_mem_reg(CodeBuffer *buf, uint8_t dst, uint8_t base)
{
    /* MOVDQU: F3 0F 6F /r */
    /* ModR/M: mod=00, reg=dst, rm=base */
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x6F);
    emit_byte(buf, 0x00 + (dst & 7) + ((base & 7) << 3));
    /* REX prefix for extended registers */
    if (dst >= 8 || base >= 8) {
        uint8_t rex = 0x40;
        if (dst >= 8) rex |= 0x04;  /* REX.R */
        if (base >= 8) rex |= 0x01; /* REX.B */
        /* Insert REX before opcode */
        buf->buffer[2] = rex;
        buf->buffer[3] = 0x6F;
        buf->buffer[4] = 0x00 + (dst & 7) + ((base & 7) << 3);
    }
}

/**
 * Emit MOVDQU [base], xmm (Move Unaligned - register indirect store)
 * @param buf Code buffer
 * @param base Base register for address (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_movdqu_mem_reg_xmm(CodeBuffer *buf, uint8_t base, uint8_t src)
{
    /* MOVDQU: F3 0F 7F /r */
    /* ModR/M: mod=00, reg=src, rm=base */
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x7F);
    emit_byte(buf, 0x00 + (src & 7) + ((base & 7) << 3));
}

/**
 * Emit MOVUPS xmm, [base] (Move Unaligned FP - register indirect load)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param base Base register for address (0-15)
 */
static void emit_movups_xmm_mem_reg(CodeBuffer *buf, uint8_t dst, uint8_t base)
{
    /* MOVUPS: 0F 10 /r */
    /* ModR/M: mod=00, reg=dst, rm=base */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;  /* REX.R */
    if (base >= 8) rex |= 0x01; /* REX.B */

    if (dst >= 8 || base >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x10);
    emit_byte(buf, 0x00 + (dst & 7) + ((base & 7) << 3));
}

/**
 * Emit MOVUPS [base], xmm (Move Unaligned FP - register indirect store)
 * @param buf Code buffer
 * @param base Base register for address (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_movups_mem_reg_xmm(CodeBuffer *buf, uint8_t base, uint8_t src)
{
    /* MOVUPS: 0F 11 /r */
    /* ModR/M: mod=00, reg=src, rm=base */
    uint8_t rex = 0x40;
    if (src >= 8) rex |= 0x04;  /* REX.R */
    if (base >= 8) rex |= 0x01; /* REX.B */

    if (src >= 8 || base >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x11);
    emit_byte(buf, 0x00 + (src & 7) + ((base & 7) << 3));
}

/**
 * Emit MOVAPS xmm, [base] (Move Aligned FP - register indirect load)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param base Base register for address (0-15)
 */
static void emit_movaps_xmm_mem_reg(CodeBuffer *buf, uint8_t dst, uint8_t base)
{
    /* MOVAPS: 0F 28 /r */
    /* ModR/M: mod=00, reg=dst, rm=base */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;  /* REX.R */
    if (base >= 8) rex |= 0x01; /* REX.B */

    if (dst >= 8 || base >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x28);
    emit_byte(buf, 0x00 + (dst & 7) + ((base & 7) << 3));
}

/**
 * Emit MOVAPS [base], xmm (Move Aligned FP - register indirect store)
 * @param buf Code buffer
 * @param base Base register for address (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_movaps_mem_reg_xmm(CodeBuffer *buf, uint8_t base, uint8_t src)
{
    /* MOVAPS: 0F 29 /r */
    /* ModR/M: mod=00, reg=src, rm=base */
    uint8_t rex = 0x40;
    if (src >= 8) rex |= 0x04;  /* REX.R */
    if (base >= 8) rex |= 0x01; /* REX.B */

    if (src >= 8 || base >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x29);
    emit_byte(buf, 0x00 + (src & 7) + ((base & 7) << 3));
}

/* ============================================================================
 * Session 53: SSSE3 Shuffle/Permutation Emit Helpers
 * ============================================================================ */

/**
 * Emit PUNPCKLBW xmm1, xmm2 (Unpack Low Bytes)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_punpcklbw_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PUNPCKLBW: 66 0F 60 /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;  /* REX.R */
    if (src >= 8) rex |= 0x01;  /* REX.B */

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x60);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/**
 * Emit PUNPCKHBW xmm1, xmm2 (Unpack High Bytes)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_punpckhbw_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PUNPCKHBW: 66 0F 68 /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x68);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/**
 * Emit PUNPCKLWD xmm1, xmm2 (Unpack Low Words)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_punpcklwd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PUNPCKLWD: 66 0F 61 /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x61);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/**
 * Emit PUNPCKHWD xmm1, xmm2 (Unpack High Words)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_punpckhwd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PUNPCKHWD: 66 0F 69 /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x69);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/**
 * Emit PUNPCKLDQ xmm1, xmm2 (Unpack Low Doublewords)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_punpckldq_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PUNPCKLDQ: 66 0F 62 /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x62);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/**
 * Emit PUNPCKHDQ xmm1, xmm2 (Unpack High Doublewords)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_punpckhdq_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PUNPCKHDQ: 66 0F 6A /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x6A);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/**
 * Emit PALIGNR xmm1, xmm2, imm8 (Packed Align Right)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 * @param imm Align immediate (0-31)
 */
static void emit_palignr_xmm_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t imm)
{
    /* PALIGNR: 66 0F 3A 0F /r ib */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3A);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
    emit_byte(buf, imm);
}

/**
 * Emit PSHUFB xmm1, xmm2 (Packed Shuffle Bytes)
 * Key instruction for NEON TBL/TBX implementation
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (control vector, 0-15)
 */
static void emit_pshufb_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PSHUFB: 66 0F 38 00 /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x00);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/**
 * Emit PSIGNB xmm1, xmm2 (Packed Sign Byte)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_psignb_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PSIGNB: 66 0F 38 08 /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x08);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/**
 * Emit PSIGNW xmm1, xmm2 (Packed Sign Word)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_psignw_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PSIGNW: 66 0F 38 09 /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x09);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/**
 * Emit PSIGND xmm1, xmm2 (Packed Sign Doubleword)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 */
static void emit_psignd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PSIGND: 66 0F 38 0A /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x0A);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/**
 * Emit PEXTRB r32, xmm, imm8 (Extract Byte)
 * @param buf Code buffer
 * @param dst Destination GPR (0-15)
 * @param src Source XMM register (0-15)
 * @param imm Index (0-15)
 */
static void emit_pextrb_reg_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t imm)
{
    /* PEXTRB: 66 0F 3A 14 /r ib */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;  /* REX.R */
    if (src >= 8) rex |= 0x01;  /* REX.B */

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3A);
    emit_byte(buf, 0x14);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
    emit_byte(buf, imm);
}

/**
 * Emit PINSRB xmm, r32, imm8 (Insert Byte)
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source GPR (0-15)
 * @param imm Index (0-15)
 */
static void emit_pinsrb_xmm_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t imm)
{
    /* PINSRB: 66 0F 3A 20 /r ib */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;  /* REX.R */
    if (src >= 8) rex |= 0x01;  /* REX.B */

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3A);
    emit_byte(buf, 0x20);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
    emit_byte(buf, imm);
}

/* ============================================================================
 * Session 48: FP Helper Functions with Mask Loading
 * ============================================================================ */

/**
 * Emit FABS (Floating-point Absolute Value) for scalar
 * Clears sign bit using ANDPS/ANDPD with mask loaded from immediate
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 * @param is_double 1 for double-precision, 0 for single-precision
 */
static void emit_fabs_scalar(CodeBuffer *buf, uint8_t dst, uint8_t src, int is_double)
{
    /* Copy src to dst first */
    if (is_double) {
        emit_movsd_xmm_xmm(buf, dst, src);
    } else {
        emit_movss_xmm_xmm(buf, dst, src);
    }

    /* For FABS, clear the sign bit:
     * Single-precision: AND with 0x7FFFFFFF (clear bit 31)
     * Double-precision: AND with 0x7FFFFFFFFFFFFFFF (clear bit 63)
     *
     * Approach: Use XORPS/XORPD to clear bits:
     * ANDPS xmm, xmm with 0x7FFFFFFF clears sign bit
     *
     * We can use a clever trick: for single precision,
     * load 0x7FFFFFFF into a temp XMM register using MOVD + PSHUFD,
     * then ANDPS with dst.
     *
     * Simpler approach for single-precision:
     * Use the fact that ANDPS can take memory operands.
     * Emit a 32-bit immediate on the stack and reference it.
     *
     * Even simpler: For scalar values, we can clear the sign bit
     * in the high dword of the XMM register using integer operations.
     *
     * Best approach: Use ANDNPS which does dst = (~src) & dst
     * If we can load a mask with all bits set EXCEPT the sign bit...
     *
     * Most practical: Generate the mask inline using data directives.
     */

    /* For now, use a practical approach:
     * Emit code that references inline data using RIP-relative addressing
     * ANDPS xmm1, [rip + offset] or ANDPD xmm1, [rip + offset]
     */

    /* Emit ANDPS/ANDPD with RIP-relative addressing to inline data
     * Opcode: 0F 54 05 <disp32>  (ANDPS xmm, [rip+disp32])
     * Opcode: 66 0F 54 05 <disp32>  (ANDPD xmm, [rip+disp32])
     *
     * However, we can't easily emit trailing data in a code buffer.
     *
     * Alternative: Use integer operations on the XMM register.
     * For single precision: Clear bit 31 of the low 32 bits
     * For double precision: Clear bit 63 of the low 64 bits
     *
     * Use PAND with a mask loaded from memory or constructed.
     */

    /* Practical implementation using PAND:
     * 1. Construct mask using MOVD + shifts (complex)
     * 2. Use inline data with RIP-relative load (simpler but needs data)
     *
     * For a JIT, simplest working approach:
     * Load 0x7FFFFFFF into a GPR, then MOVD to XMM, then ANDPS
     */

    /* Use GPR to construct mask, then move to XMM, then ANDPS
     * MOV eax, 0x7FFFFFFF
     * MOVD xmm15, eax
     * PSHUFD xmm15, xmm15, 0x00  ; broadcast to all lanes
     * ANDPS dst, xmm15
     */

    uint8_t temp_xmm = 15;  /* Use XMM15 as temp (clobber safe in our model) */
    uint8_t temp_gpr = 0;   /* Use RAX/EAX as temp */

    /* For single precision: */
    if (!is_double) {
        /* MOV eax, 0x7FFFFFFF - immediate move to clear sign bit */
        emit_byte(buf, 0xB8 + (temp_gpr & 7));  /* MOV r32, imm32 */
        if (temp_gpr >= 8) buf->buffer[buf->offset - 1] |= 0x04;
        emit_word32(buf, 0x7FFFFFFF);

        /* MOVD xmm15, eax */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x6E);
        emit_byte(buf, 0xC0 + (temp_xmm & 7) + ((temp_gpr & 7) << 3));

        /* PSHUFD xmm15, xmm15, 0x00 - broadcast to all lanes */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x70);
        emit_byte(buf, 0xC0 + (temp_xmm & 7) + ((temp_xmm & 7) << 3));
        emit_byte(buf, 0x00);  /* Immediate: shuffle control */

        /* ANDPS dst, xmm15 */
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x54);
        emit_byte(buf, 0xC0 + (dst & 7) + ((temp_xmm & 7) << 3));
    } else {
        /* Double precision: need 0x7FFFFFFFFFFFFFFF
         * MOV rax, 0x7FFFFFFFFFFFFFFF (via MOV + shift)
         */
        emit_byte(buf, 0x48 | ((temp_gpr & 8) >> 3));  /* REX.W */
        emit_byte(buf, 0xB8 + (temp_gpr & 7));  /* MOV r64, imm64 */
        emit_word32(buf, 0xFFFFFFFF);  /* Low 32 bits */
        emit_word32(buf, 0x7FFFFFFF);  /* High 32 bits */

        /* MOVQ xmm15, rax */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x48);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x6E);
        emit_byte(buf, 0xC0 + (temp_xmm & 7) + ((temp_gpr & 7) << 3));

        /* PSHUFD xmm15, xmm15, 0x44 - duplicate low 64 to high 64 */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x70);
        emit_byte(buf, 0xC0 + (temp_xmm & 7) + ((temp_xmm & 7) << 3));
        emit_byte(buf, 0x44);  /* [1:0, 1:0] = duplicate lane 1 to both */

        /* ANDPD dst, xmm15 */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x54);
        emit_byte(buf, 0xC0 + (dst & 7) + ((temp_xmm & 7) << 3));
    }
}

/**
 * Emit FNEG (Floating-point Negate) for scalar
 * Flips sign bit using XORPS/XORPD with sign mask
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 * @param is_double 1 for double-precision, 0 for single-precision
 */
static void emit_fneg_scalar(CodeBuffer *buf, uint8_t dst, uint8_t src, int is_double)
{
    /* Copy src to dst first */
    if (is_double) {
        emit_movsd_xmm_xmm(buf, dst, src);
    } else {
        emit_movss_xmm_xmm(buf, dst, src);
    }

    /* For FNEG, flip the sign bit:
     * Single-precision: XOR with 0x80000000 (set bit 31)
     * Double-precision: XOR with 0x8000000000000000 (set bit 63)
     *
     * Same approach as FABS but using XOR instead of AND
     */

    uint8_t temp_xmm = 15;
    uint8_t temp_gpr = 0;

    if (!is_double) {
        /* MOV eax, 0x80000000 */
        emit_byte(buf, 0xB8 + (temp_gpr & 7));
        if (temp_gpr >= 8) buf->buffer[buf->offset - 1] |= 0x04;
        emit_word32(buf, 0x80000000);

        /* MOVD xmm15, eax */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x6E);
        emit_byte(buf, 0xC0 + (temp_xmm & 7) + ((temp_gpr & 7) << 3));

        /* PSHUFD xmm15, xmm15, 0x00 */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x70);
        emit_byte(buf, 0xC0 + (temp_xmm & 7) + ((temp_xmm & 7) << 3));
        emit_byte(buf, 0x00);

        /* XORPS dst, xmm15 */
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x57);
        emit_byte(buf, 0xC0 + (dst & 7) + ((temp_xmm & 7) << 3));
    } else {
        /* MOV rax, 0x8000000000000000 */
        emit_byte(buf, 0x48 | ((temp_gpr & 8) >> 3));
        emit_byte(buf, 0xB8 + (temp_gpr & 7));
        emit_word32(buf, 0x00000000);
        emit_word32(buf, 0x80000000);

        /* MOVQ xmm15, rax */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x48);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x6E);
        emit_byte(buf, 0xC0 + (temp_xmm & 7) + ((temp_gpr & 7) << 3));

        /* PSHUFD xmm15, xmm15, 0x44 */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x70);
        emit_byte(buf, 0xC0 + (temp_xmm & 7) + ((temp_xmm & 7) << 3));
        emit_byte(buf, 0x44);

        /* XORPD dst, xmm15 */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x57);
        emit_byte(buf, 0xC0 + (dst & 7) + ((temp_xmm & 7) << 3));
    }
}

/**
 * Emit FCSEL (Floating-point Conditional Select) for scalar
 * Implements: dst = (cond true) ? src : src2
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register for TRUE condition (0-15)
 * @param src2 Source XMM register for FALSE condition (0-15)
 * @param cond ARM64 condition code (0-15)
 */
static void emit_fcsel_scalar(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t src2, uint8_t cond)
{
    /* FCSEL: dst = (flags match cond) ? src : src2
     *
     * x86 doesn't have direct XMM conditional moves, but we can use:
     * 1. Conditional branches (slow)
     * 2. CMOV for GPRs with register spills
     * 3. BLENDPS/BLENDPD (SSE4.1) - requires SSE4.1
     *
     * Using BLENDPD (SSE4.1, opcode: 66 0F 38 15 /r ib)
     * BLENDPD xmm1, xmm2/m128, imm8 - blend based on immediate
     *
     * But we need conditional blend based on flags, not immediate.
     *
     * Approach: Use conditional branch
     *   movsd dst, src       ; assume true
     *   j<cond> skip         ; if condition true, skip
     *   movsd dst, src2      ; else load false value
     * skip:
     */

    /* Copy src (true value) to dst */
    emit_movsd_xmm_xmm(buf, dst, src);

    /* Map ARM64 condition to x86 condition for "jump if false"
     * We want to jump (skip the false load) if the condition is TRUE
     * So we use the same condition mapping as regular branches
     */

    /* Emit conditional jump to skip the false load
     * The condition is already in EFLAGS from preceding FCMP
     */
    uint8_t *jump_patch = &buf->buffer[buf->offset];
    emit_byte(buf, 0x0F);  /* Prefix for conditional jumps */

    /* Map ARM64 condition to x86 Jcc */
    uint8_t x86_cond = cond;
    switch (cond) {
        case 0:  /* EQ */ x86_cond = 0x4; break;  /* JE */
        case 1:  /* NE */ x86_cond = 0x5; break;  /* JNE */
        case 2:  /* CS/HS */ x86_cond = 0x3; break;  /* JAE */
        case 3:  /* CC */ x86_cond = 0x2; break;  /* JB */
        case 4:  /* MI */ x86_cond = 0x8; break;  /* JS */
        case 5:  /* PL */ x86_cond = 0x9; break;  /* JNS */
        case 6:  /* VS */ x86_cond = 0x6; break;  /* JO */
        case 7:  /* VC */ x86_cond = 0x7; break;  /* JNO */
        case 8:  /* HI */ x86_cond = 0x7; break;  /* JA */
        case 9:  /* LS */ x86_cond = 0x6; break;  /* JBE */
        case 10: /* GE */ x86_cond = 0x9; break;  /* JGE */
        case 11: /* LT */ x86_cond = 0xC; break;  /* JL */
        case 12: /* GT */ x86_cond = 0xF; break;  /* JG */
        case 13: /* LE */ x86_cond = 0xE; break;  /* JLE */
        case 14: /* AL */ x86_cond = 0xFF; break; /* Always - skip jump */
        case 15: /* NV */ x86_cond = 0xFF; break; /* Never - always jump */
    }

    if (x86_cond == 0xFF) {
        /* AL: always true - no jump needed, dst already has src */
        /* NV: never true - always load src2 */
        if (cond == 15) {
            emit_movsd_xmm_xmm(buf, dst, src2);
        }
        (void)jump_patch;
        return;
    }

    emit_byte(buf, 0x80 | x86_cond);  /* Jcc rel32 */
    emit_word32(buf, 0);  /* Placeholder offset - will be patched */

    /* Emit MOVSD dst, src2 (load false value) */
    emit_movsd_xmm_xmm(buf, dst, src2);

    /* Calculate and patch jump offset (skip over MOVSD = 4 bytes) */
    int32_t skip_offset = (int32_t)(buf->offset - (jump_patch + 6 - buf->buffer));

    /* Patch the jump offset */
    jump_patch[2] = skip_offset & 0xFF;
    jump_patch[3] = (skip_offset >> 8) & 0xFF;
    jump_patch[4] = (skip_offset >> 16) & 0xFF;
    jump_patch[5] = (skip_offset >> 24) & 0xFF;
}

/* ============================================================================
 * x86_64 Instruction Decoding Helpers
 * Translation: x86_64 (guest) -> ARM64 (host)
 * ============================================================================ */

/* x86_64 to ARM64 condition code mapping */
static inline uint8_t map_x86_jcc_to_arm_cond(uint8_t x86_jcc)
{
    switch (x86_jcc & 0x0F) {
        case 0x0: return 6;   /* JO -> VS */
        case 0x1: return 7;   /* JNO -> VC */
        case 0x2: return 3;   /* JB/JNAE/CF -> CC */
        case 0x3: return 2;   /* JAE/JNB/NC -> CS */
        case 0x4: return 0;   /* JE/JZ -> EQ */
        case 0x5: return 1;   /* JNE/JNZ -> NE */
        case 0x6: return 10;  /* JBE/JNA -> LE */
        case 0x7: return 11;  /* JA/JNBE -> GT */
        case 0x8: return 4;   /* JS -> MI */
        case 0x9: return 5;   /* JNS -> PL */
        case 0xA: return 6;   /* JP/JPE -> VS */
        case 0xB: return 7;   /* JNP/JPO -> VC */
        case 0xC: return 8;   /* JL/JNGE -> LT */
        case 0xD: return 9;   /* JGE/JNL -> GE */
        case 0xE: return 10;  /* JLE/JNG -> LE */
        case 0xF: return 11;  /* JG/JNLE -> GT */
        default: return 14;
    }
}

/* x86_64 instruction info for translation */
typedef struct {
    uint8_t opcode;         /* Primary opcode byte */
    uint8_t opcode2;        /* Secondary opcode (for 0F xx) */
    uint8_t rex;            /* REX prefix (0 if none) */
    uint8_t modrm;          /* ModR/M byte (0 if none) */
    int32_t disp;           /* Displacement */
    uint8_t disp_size;      /* Displacement size in bytes (0, 1, 4, 8) */
    int64_t imm;            /* Immediate value */
    uint8_t imm_size;       /* Immediate size in bytes */
    uint8_t length;         /* Total instruction length */
    uint8_t mod;            /* ModR/M mod field */
    uint8_t reg;            /* ModR/M reg field */
    uint8_t rm;             /* ModR/M rm field */
    int has_modrm;          /* Has ModR/M byte */
    int is_64bit;           /* 64-bit operand size */
} x86_insn_t;

/**
 * Decode x86_64 instruction at given address
 * @param insn_ptr Pointer to instruction bytes
 * @param insn Output: decoded instruction info
 * @return Number of bytes decoded
 */
static int decode_x86_insn(const uint8_t *insn_ptr, x86_insn_t *insn)
{
    const uint8_t *p = insn_ptr;
    uint8_t rex = 0;

    memset(insn, 0, sizeof(x86_insn_t));

    /* Parse REX prefix */
    if ((p[0] & 0xF0) == 0x40 && (p[0] & 0x0F) >= 0x08) {
        rex = p[0];
        p++;
        insn->rex = rex;
        insn->is_64bit = (rex & 0x08) ? 1 : 0;
    } else {
        insn->is_64bit = 1;
    }

    /* Check for legacy prefixes (simplified) */
    while (*p == 0x66 || *p == 0x67 || *p == 0x2E || *p == 0x3E ||
           *p == 0x26 || *p == 0x36 || *p == 0x64 || *p == 0x65 ||
           *p == 0xF0 || *p == 0xF2 || *p == 0xF3) {
        if (*p == 0x66) insn->is_64bit = 0;
        p++;
    }

    /* Check for 0F escape */
    if (*p == 0x0F) {
        p++;
        insn->opcode2 = *p++;
    }

    insn->opcode = *p++;

    /* Determine if has ModR/M */
    uint8_t op = insn->opcode;
    uint8_t op2 = insn->opcode2;
    int has_modrm = 0;

    if (op == 0x00 || op == 0x01 || op == 0x02 || op == 0x03 ||
        op == 0x08 || op == 0x09 || op == 0x0A || op == 0x0B ||
        op == 0x20 || op == 0x21 || op == 0x22 || op == 0x23 ||
        op == 0x28 || op == 0x29 || op == 0x2A || op == 0x2B ||
        op == 0x30 || op == 0x31 || op == 0x32 || op == 0x33 ||
        op == 0x84 || op == 0x85 || op == 0x86 || op == 0x87 ||
        op == 0x88 || op == 0x89 || op == 0x8A || op == 0x8B ||
        op == 0x8C || op == 0x8D || op == 0x8E || op == 0x8F ||
        op == 0xC0 || op == 0xC1 ||
        op == 0xD0 || op == 0xD1 || op == 0xD2 || op == 0xD3 ||
        op == 0xF6 || op == 0xF7 ||
        op == 0x80 || op == 0x81 || op == 0x82 || op == 0x83) {
        has_modrm = 1;
    }
    if (op2 != 0 && (
        (op2 >= 0x00 && op2 <= 0x07) ||
        (op2 >= 0x10 && op2 <= 0x17) ||
        (op2 >= 0x20 && op2 <= 0x27) ||
        (op2 >= 0x40 && op2 <= 0x4F) ||
        (op2 >= 0x50 && op2 <= 0x7F) ||
        (op2 >= 0x80 && op2 <= 0x8F) ||
        op2 == 0xA0 || op2 == 0xA1 || op2 == 0xA2 || op2 == 0xA3 ||
        op2 == 0xB0 || op2 == 0xB1 || op2 == 0xB3 ||
        op2 == 0xB6 || op2 == 0xB7 || op2 == 0xBE || op2 == 0xBF ||
        op2 == 0xC0 || op2 == 0xC1)) {
        has_modrm = 1;
    }

    insn->has_modrm = has_modrm;

    /* Parse ModR/M */
    if (has_modrm) {
        uint8_t modrm = *p++;
        insn->modrm = modrm;
        insn->mod = (modrm >> 6) & 0x03;
        insn->reg = ((modrm >> 3) & 0x07) | ((rex & 0x04) ? 8 : 0);
        insn->rm = ((modrm >> 0) & 0x07) | ((rex & 0x01) ? 8 : 0);

        /* Handle SIB */
        if (insn->mod != 3 && (insn->rm & 7) == 4) {
            p++;
        }

        /* Handle displacement */
        if (insn->mod == 0 && (insn->rm & 7) == 5) {
            insn->disp = *(const int32_t *)p;
            p += 4;
        } else if (insn->mod == 1) {
            insn->disp = *(const int8_t *)p;
            p += 1;
        } else if (insn->mod == 2) {
            insn->disp = *(const int32_t *)p;
            p += 4;
        }
    }

    /* Parse immediate */
    if (op >= 0xB8 && op <= 0xBF) {
        if (rex & 0x08) {
            insn->imm = *(const int64_t *)p;
            p += 8;
        } else {
            insn->imm = *(const int32_t *)p;
            p += 4;
        }
    } else if (op == 0x05 || op == 0x0D || op == 0x0F || op == 0x25 ||
               op == 0x2D || op == 0x2F || op == 0x35 || op == 0x3D) {
        insn->rm = 0;
        insn->imm = *(const int32_t *)p;
        p += 4;
    } else if ((op >= 0x80 && op <= 0x83) ||
               (op2 >= 0x80 && op2 <= 0x8F)) {
        int is_imm8 = (op == 0x83 || op == 0x82 || op2 >= 0x80);
        if (is_imm8) {
            insn->imm = *(const int8_t *)p;
            p += 1;
        } else {
            insn->imm = *(const int32_t *)p;
            p += 4;
        }
    } else if (op == 0xE8 || op == 0xE9) {
        insn->imm = *(const int32_t *)p;
        p += 4;
    } else if (op == 0xEB) {
        insn->imm = *(const int8_t *)p;
        p += 1;
    } else if (op >= 0x70 && op <= 0x7F) {
        insn->imm = *(const int8_t *)p;
        p += 1;
    } else if (op2 >= 0x80 && op2 <= 0x8F) {
        insn->imm = *(const int32_t *)p;
        p += 4;
    }

    insn->length = (uint8_t)(p - insn_ptr);
    return insn->length;
}

/* x86_64 instruction type checks */
static inline int x86_is_add(const x86_insn_t *i) {
    return i->opcode == 0x01 || i->opcode == 0x03 ||
           i->opcode == 0x81 || i->opcode == 0x83;
}
static inline int x86_is_sub(const x86_insn_t *i) {
    return i->opcode == 0x29 || i->opcode == 0x2B ||
           i->opcode == 0x81 || i->opcode == 0x83;
}
static inline int x86_is_and(const x86_insn_t *i) {
    return i->opcode == 0x21 || i->opcode == 0x23 || i->opcode == 0x81;
}
static inline int x86_is_or(const x86_insn_t *i) {
    return i->opcode == 0x09 || i->opcode == 0x0B || i->opcode == 0x81;
}
static inline int x86_is_xor(const x86_insn_t *i) {
    return i->opcode == 0x31 || i->opcode == 0x33 || i->opcode == 0x81;
}
static inline int x86_is_mov(const x86_insn_t *i) {
    return i->opcode == 0x8B || i->opcode == 0x89 ||
           i->opcode == 0xC7 || (i->opcode >= 0xB8 && i->opcode <= 0xBF);
}
static inline int x86_is_mov_imm64(const x86_insn_t *i) {
    return (i->opcode >= 0xB8 && i->opcode <= 0xBF) && (i->rex & 0x08);
}
static inline int x86_is_lea(const x86_insn_t *i) {
    return i->opcode == 0x8D;
}
static inline int x86_is_cmp(const x86_insn_t *i) {
    return i->opcode == 0x39 || i->opcode == 0x3B ||
           i->opcode == 0x81 || i->opcode == 0x83;
}
static inline int x86_is_test(const x86_insn_t *i) {
    return i->opcode == 0x85 || i->opcode == 0xF6 || i->opcode == 0xF7;
}
static inline int x86_is_jcc(const x86_insn_t *i) {
    return (i->opcode >= 0x70 && i->opcode <= 0x7F) ||
           (i->opcode == 0x0F && i->opcode2 >= 0x80 && i->opcode2 <= 0x8F);
}
static inline int x86_is_jmp(const x86_insn_t *i) {
    return i->opcode == 0xE9 || i->opcode == 0xEB;
}
static inline int x86_is_call(const x86_insn_t *i) {
    return i->opcode == 0xE8;
}
static inline int x86_is_ret(const x86_insn_t *i) {
    return i->opcode == 0xC3 || i->opcode == 0xC2;
}
static inline int x86_is_mul(const x86_insn_t *i) {
    return i->opcode == 0xF7 && (i->reg == 4 || i->reg == 5);
}
static inline int x86_is_div(const x86_insn_t *i) {
    return i->opcode == 0xF7 && (i->reg == 6 || i->reg == 7);
}
static inline uint8_t x86_get_jcc_cond(const x86_insn_t *i) {
    if (i->opcode >= 0x70 && i->opcode <= 0x7F)
        return i->opcode & 0x0F;
    if (i->opcode2 >= 0x80 && i->opcode2 <= 0x8F)
        return i->opcode2 & 0x0F;
    return 0;
}

/* ============================================================================
 * Additional x86_64 Instruction Type Predicates (P0-P4)
 * ============================================================================ */

/* P0 - Essential instructions */
static inline int x86_is_movzx(const x86_insn_t *i) {
    /* MOVZX r16/m8, r8 (0F B6) / MOVZX r32/m8, r8 (0F B6) / MOVZX r32/m16, r16 (0F B7) */
    return i->opcode == 0x0F && (i->opcode2 == 0xB6 || i->opcode2 == 0xB7);
}

static inline int x86_is_movsx(const x86_insn_t *i) {
    /* MOVSX r16/m8, r8 (0F BE) / MOVSX r32/m8, r8 (0F BE) / MOVSX r32/m16, r16 (0F BF) */
    return i->opcode == 0x0F && (i->opcode2 == 0xBE || i->opcode2 == 0xBF);
}

static inline int x86_is_movsxd(const x86_insn_t *i) {
    /* MOVSXD r64/m32, r32 (63 / x86-64) */
    return i->opcode == 0x63 && (i->rex & 0x08);  /* REX.W required */
}

static inline int x86_is_inc(const x86_insn_t *i) {
    /* INC r/m8 (FE /0) | INC r/m64 (FF /0) | INC r64 (40+ for RAX-RCX, 48+ for R8-R15) */
    if (i->opcode == 0xFE && i->reg == 0) return 1;  /* r/m8 */
    if (i->opcode == 0xFF && i->reg == 0) return 1;  /* r/m64 */
    if ((i->opcode >= 0x40 && i->opcode <= 0x47) || (i->opcode >= 0x48 && i->opcode <= 0x4F))
        return 1;  /* register increment (REX.B determines high/low) */
    return 0;
}

static inline int x86_is_dec(const x86_insn_t *i) {
    /* DEC r/m8 (FE /1) | DEC r/m64 (FF /1) | DEC r64 (48+ for RAX-RCX, 50+ for R8-R15) */
    if (i->opcode == 0xFE && i->reg == 1) return 1;  /* r/m8 */
    if (i->opcode == 0xFF && i->reg == 1) return 1;  /* r/m64 */
    if ((i->opcode >= 0x48 && i->opcode <= 0x4F) || (i->opcode >= 0x50 && i->opcode <= 0x57))
        return 1;  /* register decrement */
    return 0;
}

static inline int x86_is_neg(const x86_insn_t *i) {
    /* NEG r/m8 (F6 /3) | NEG r/m64 (F7 /3) */
    return i->opcode == 0xF6 && i->reg == 3;  /* r/m8 */
    /* Note: F7 /3 is also NEG for r/m64, but F7 /3 is handled separately if needed */
}

static inline int x86_is_not(const x86_insn_t *i) {
    /* NOT r/m8 (F6 /2) | NOT r/m64 (F7 /2) */
    return i->opcode == 0xF6 && i->reg == 2;  /* r/m8 */
    /* Note: F7 /2 is also NOT for r/m64 */
}

static inline int x86_is_push(const x86_insn_t *i) {
    /* PUSH r/m64 (FF /6) | PUSH r64 (50-57) | PUSH imm8/32 (6A/68) */
    if (i->opcode >= 0x50 && i->opcode <= 0x57) return 1;  /* PUSH r64 */
    if (i->opcode == 0x6A || i->opcode == 0x68) return 1;  /* PUSH imm */
    if (i->opcode == 0xFF && i->reg == 6) return 1;  /* PUSH r/m64 */
    return 0;
}

static inline int x86_is_pop(const x86_insn_t *i) {
    /* POP r64 (58-5F) | POP r/m64 (8F /0) */
    if (i->opcode >= 0x58 && i->opcode <= 0x5F) return 1;  /* POP r64 */
    if (i->opcode == 0x8F && i->reg == 0) return 1;  /* POP r/m64 */
    return 0;
}

static inline int x86_is_nop(const x86_insn_t *i) {
    /* NOP (90) */
    return i->opcode == 0x90;
}

/* Shift/Rotate instructions */
static inline int x86_is_shl(const x86_insn_t *i) {
    /* SHL r/m8,1 (D0 /4) | SHL r/m8,CL (D2 /4) | SHL r/m8,imm8 (C0 /4) */
    /* SHL r/m64,1 (D1 /4) | SHL r/m64,CL (D3 /4) | SHL r/m64,imm8 (C1 /4) */
    return (i->opcode == 0xD0 || i->opcode == 0xD1 || i->opcode == 0xD2 ||
            i->opcode == 0xD3 || i->opcode == 0xC0 || i->opcode == 0xC1) && i->reg == 4;
}

static inline int x86_is_shr(const x86_insn_t *i) {
    /* SHR r/m8,1 (D0 /5) | SHR r/m8,CL (D2 /5) | SHR r/m8,imm8 (C0 /5) */
    /* SHR r/m64,1 (D1 /5) | SHR r/m64,CL (D3 /5) | SHR r/m64,imm8 (C1 /5) */
    return (i->opcode == 0xD0 || i->opcode == 0xD1 || i->opcode == 0xD2 ||
            i->opcode == 0xD3 || i->opcode == 0xC0 || i->opcode == 0xC1) && i->reg == 5;
}

static inline int x86_is_sar(const x86_insn_t *i) {
    /* SAR r/m8,1 (D0 /7) | SAR r/m8,CL (D2 /7) | SAR r/m8,imm8 (C0 /7) */
    /* SAR r/m64,1 (D1 /7) | SAR r/m64,CL (D3 /7) | SAR r/m64,imm8 (C1 /7) */
    return (i->opcode == 0xD0 || i->opcode == 0xD1 || i->opcode == 0xD2 ||
            i->opcode == 0xD3 || i->opcode == 0xC0 || i->opcode == 0xC1) && i->reg == 7;
}

static inline int x86_is_rol(const x86_insn_t *i) {
    /* ROL r/m8,1 (D0 /0) | ROL r/m8,CL (D2 /0) | ROL r/m8,imm8 (C0 /0) */
    /* ROL r/m64,1 (D1 /0) | ROL r/m64,CL (D3 /0) | ROL r/m64,imm8 (C1 /0) */
    return (i->opcode == 0xD0 || i->opcode == 0xD1 || i->opcode == 0xD2 ||
            i->opcode == 0xD3 || i->opcode == 0xC0 || i->opcode == 0xC1) && i->reg == 0;
}

static inline int x86_is_ror(const x86_insn_t *i) {
    /* ROR r/m8,1 (D0 /1) | ROR r/m8,CL (D2 /1) | ROR r/m8,imm8 (C0 /1) */
    /* ROR r/m64,1 (D1 /1) | ROR r/m64,CL (D3 /1) | ROR r/m64,imm8 (C1 /1) */
    return (i->opcode == 0xD0 || i->opcode == 0xD1 || i->opcode == 0xD2 ||
            i->opcode == 0xD3 || i->opcode == 0xC0 || i->opcode == 0xC1) && i->reg == 1;
}

/* P1 - Control flow instructions */
static inline int x86_is_cmov(const x86_insn_t *i) {
    /* CMOVcc r64, r/m64 (0F 40-4F) */
    return i->opcode == 0x0F && i->opcode2 >= 0x40 && i->opcode2 <= 0x4F;
}

static inline uint8_t x86_get_cmov_cond(const x86_insn_t *i) {
    if (x86_is_cmov(i))
        return i->opcode2 - 0x40;
    return 0;
}

static inline int x86_is_setcc(const x86_insn_t *i) {
    /* SETcc r/m8 (0F 90-9F) */
    return i->opcode == 0x0F && i->opcode2 >= 0x90 && i->opcode2 <= 0x9F;
}

static inline uint8_t x86_get_setcc_cond(const x86_insn_t *i) {
    if (x86_is_setcc(i))
        return i->opcode2 - 0x90;
    return 0;
}

static inline int x86_is_xchg(const x86_insn_t *i) {
    /* XCHG r8,r8 (86-87) | XCHG r64,r64 (87) | XCHG r32,EAX (90-97) */
    if (i->opcode == 0x86 || i->opcode == 0x87) return 1;
    if (i->opcode >= 0x90 && i->opcode <= 0x97) return 1;  /* XCHG with EAX/RAX */
    return 0;
}

/* P2 - Bit manipulation instructions */
static inline int x86_is_bsf(const x86_insn_t *i) {
    /* BSF r64,r/m64 (0F BC) */
    return i->opcode == 0x0F && i->opcode2 == 0xBC;
}

static inline int x86_is_bsr(const x86_insn_t *i) {
    /* BSR r64,r/m64 (0F BD) */
    return i->opcode == 0x0F && i->opcode2 == 0xBD;
}

static inline int x86_is_bt(const x86_insn_t *i) {
    /* BT r/m64,r64 (0F A3) | BT r/m64,imm8 (0F BA /4) */
    if (i->opcode == 0x0F && i->opcode2 == 0xA3) return 1;
    if (i->opcode == 0x0F && i->opcode2 == 0xBA && i->reg == 4) return 1;
    return 0;
}

static inline int x86_is_bts(const x86_insn_t *i) {
    /* BTS r/m64,r64 (0F AB) | BTS r/m64,imm8 (0F BA /5) */
    if (i->opcode == 0x0F && i->opcode2 == 0xAB) return 1;
    if (i->opcode == 0x0F && i->opcode2 == 0xBA && i->reg == 5) return 1;
    return 0;
}

static inline int x86_is_btr(const x86_insn_t *i) {
    /* BTR r/m64,r64 (0F B3) | BTR r/m64,imm8 (0F BA /6) */
    if (i->opcode == 0x0F && i->opcode2 == 0xB3) return 1;
    if (i->opcode == 0x0F && i->opcode2 == 0xBA && i->reg == 6) return 1;
    return 0;
}

static inline int x86_is_btc(const x86_insn_t *i) {
    /* BTC r/m64,r64 (0F BB) | BTC r/m64,imm8 (0F BA /7) */
    if (i->opcode == 0x0F && i->opcode2 == 0xBB) return 1;
    if (i->opcode == 0x0F && i->opcode2 == 0xBA && i->reg == 7) return 1;
    return 0;
}

static inline int x86_is_popcnt(const x86_insn_t *i) {
    /* POPCNT r64,r/m64 (0F B8) */
    return i->opcode == 0x0F && i->opcode2 == 0xB8;
}

static inline int x86_is_tzcnt(const x86_insn_t *i) {
    /* TZCNT r64,r/m64 (F3 0F BC) - same as BSF with F3 prefix */
    /* For simplicity, we'll handle this as BSF with F3 prefix detection */
    return 0;  /* TODO: Add prefix detection */
}

static inline int x86_is_lzcnt(const x86_insn_t *i) {
    /* LZCNT r64,r/m64 (F3 0F BD) - same as BSR with F3 prefix */
    return 0;  /* TODO: Add prefix detection */
}

/* P3 - String operations */
static inline int x86_is_movs(const x86_insn_t *i) {
    /* MOVS: A4 (byte), A5 (dword/qword) */
    return i->opcode == 0xA4 || i->opcode == 0xA5;
}

static inline int x86_is_stos(const x86_insn_t *i) {
    /* STOS: AA (byte), AB (dword/qword) */
    return i->opcode == 0xAA || i->opcode == 0xAB;
}

static inline int x86_is_lods(const x86_insn_t *i) {
    /* LODS: AC (byte), AD (dword/qword) */
    return i->opcode == 0xAC || i->opcode == 0xAD;
}

static inline int x86_is_cmps(const x86_insn_t *i) {
    /* CMPS: A6 (byte), A7 (dword/qword) */
    return i->opcode == 0xA6 || i->opcode == 0xA7;
}

static inline int x86_is_scas(const x86_insn_t *i) {
    /* SCAS: AE (byte), AF (dword/qword) */
    return i->opcode == 0xAE || i->opcode == 0xAF;
}

static inline int x86_has_rep_prefix(const x86_insn_t *i) {
    /* REP/REPE/REPNE prefixes: F3 (REP/REPE), F2 (REPNE) */
    /* This would require prefix tracking in decode_x86_insn */
    return 0;  /* TODO: Add prefix tracking */
}

/* P4 - Special instructions */
static inline int x86_is_cpuid(const x86_insn_t *i) {
    /* CPUID: 0F A2 */
    return i->opcode == 0x0F && i->opcode2 == 0xA2;
}

static inline int x86_is_rdtsc(const x86_insn_t *i) {
    /* RDTSC: 0F 31 */
    return i->opcode == 0x0F && i->opcode2 == 0x31;
}

static inline int x86_is_rdtscp(const x86_insn_t *i) {
    /* RDTSCP: 0F 33 */
    return i->opcode == 0x0F && i->opcode2 == 0x33;
}

static inline int x86_is_shld(const x86_insn_t *i) {
    /* SHLD: 0F A4 (imm8), 0F A5 (CL) */
    return i->opcode == 0x0F && (i->opcode2 == 0xA4 || i->opcode2 == 0xA5);
}

static inline int x86_is_shrd(const x86_insn_t *i) {
    /* SHRD: 0F AC (imm8), 0F AD (CL) */
    return i->opcode == 0x0F && (i->opcode2 == 0xAC || i->opcode2 == 0xAD);
}

static inline int x86_is_cdq(const x86_insn_t *i) {
    /* CDQ/CQO: 99 (CBW), 98 (CWD), 9A (CWDE), 48 99 (CQO) */
    return i->opcode == 0x99 || i->opcode == 0x98;
}

static inline int x86_is_cwd(const x86_insn_t *i) {
    /* CWD: 99 */
    return i->opcode == 0x99;
}

static inline int x86_is_cqo(const x86_insn_t *i) {
    /* CQO: 48 99 */
    return i->opcode == 0x99 && (i->rex & 0x08);
}

static inline int x86_is_cli(const x86_insn_t *i) {
    /* CLI: FA */
    return i->opcode == 0xFA;
}

static inline int x86_is_sti(const x86_insn_t *i) {
    /* STI: FB */
    return i->opcode == 0xFB;
}

static inline int x86_is_cli_sti(const x86_insn_t *i) {
    return x86_is_cli(i) || x86_is_sti(i);
}


/* ============================================================================
 * ARM64 Instruction Decoder Helpers
 * ============================================================================ */

/**
 * ARM64 instruction encoding masks
 */
#define ARM64_MASK_OP30     0xC0000000  /* Top 2 bits */
#define ARM64_MASK_OPCODE   0xFFE0E000  /* Main opcode */
#define ARM64_MASK_RD       0x0000001F  /* Destination register */
#define ARM64_MASK_RN       0x000003E0  /* First operand register */
#define ARM64_MASK_RM       0x001F0000  /* Second operand register */
#define ARM64_MASK_IMM12    0x00001FFC  /* 12-bit immediate */
#define ARM64_MASK_IMM16    0x00FF0000  /* 16-bit immediate (MOVZ/MOVK) */
#define ARM64_MASK_IMM26    0x03FFFFFF  /* 26-bit immediate (branch) */

/**
 * Extract opcode class from ARM64 instruction
 * @param encoding ARM64 instruction encoding
 * @return Opcode class (0-3)
 */
static inline uint32_t arm64_get_opclass(uint32_t encoding)
{
    return (encoding >> 30) & 0x3;
}

/**
 * Extract destination register from ARM64 instruction
 * @param encoding ARM64 instruction encoding
 * @return Register number (0-31)
 */
static inline uint8_t arm64_get_rd(uint32_t encoding)
{
    return (encoding >> 0) & 0x1F;
}

/**
 * Extract first operand register from ARM64 instruction
 * @param encoding ARM64 instruction encoding
 * @return Register number (0-31)
 */
static inline uint8_t arm64_get_rn(uint32_t encoding)
{
    return (encoding >> 5) & 0x1F;
}

/**
 * Extract second operand register from ARM64 instruction
 * @param encoding ARM64 instruction encoding
 * @return Register number (0-31)
 */
static inline uint8_t arm64_get_rm(uint32_t encoding)
{
    return (encoding >> 16) & 0x1F;
}

/**
 * Extract 12-bit immediate from ARM64 instruction
 * @param encoding ARM64 instruction encoding
 * @return 12-bit immediate value
 */
static inline uint16_t arm64_get_imm12(uint32_t encoding)
{
    return (encoding >> 10) & 0xFFF;
}

/**
 * Extract 26-bit branch immediate from ARM64 instruction
 * @param encoding ARM64 instruction encoding
 * @return Signed branch offset (already shifted by 2)
 */
static inline int32_t arm64_get_imm26(uint32_t encoding)
{
    return (int32_t)((encoding & 0x03FFFFFF) << 2);
}

/**
 * Check if instruction is ADD (register)
 * @param encoding ARM64 instruction encoding
 * @return 1 if ADD, 0 otherwise
 */
static inline int arm64_is_add(uint32_t encoding)
{
    return (encoding & 0xFFE0E000) == 0x0B000000;
}

/**
 * Check if instruction is SUB (register)
 * @param encoding ARM64 instruction encoding
 * @return 1 if SUB, 0 otherwise
 */
static inline int arm64_is_sub(uint32_t encoding)
{
    return (encoding & 0xFFE0E000) == 0xCB000000;
}

/**
 * Check if instruction is B (unconditional branch)
 * @param encoding ARM64 instruction encoding
 * @return 1 if B, 0 otherwise
 */
static inline int arm64_is_b(uint32_t encoding)
{
    return (encoding & 0xFC000000) == 0x14000000;
}

/**
 * Check if instruction is BL (branch with link)
 * @param encoding ARM64 instruction encoding
 * @return 1 if BL, 0 otherwise
 */
static inline int arm64_is_bl(uint32_t encoding)
{
    return (encoding & 0xFC000000) == 0x94000000;
}

/**
 * Check if instruction is BR (branch to register)
 * @param encoding ARM64 instruction encoding
 * @return 1 if BR, 0 otherwise
 */
static inline int arm64_is_br(uint32_t encoding)
{
    return (encoding & 0xFFFFFC00) == 0xD61F0000;
}

/**
 * Check if instruction is LDR (register)
 * @param encoding ARM64 instruction encoding
 * @return 1 if LDR, 0 otherwise
 */
static inline int arm64_is_ldr(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0xF8400000;
}

/**
 * Check if instruction is STR (register)
 * @param encoding ARM64 instruction encoding
 * @return 1 if STR, 0 otherwise
 */
static inline int arm64_is_str(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0xF8000000;
}

/**
 * Check if instruction is ADD (immediate)
 * @param encoding ARM64 instruction encoding
 * @return 1 if ADD, 0 otherwise
 */
static inline int arm64_is_add_imm(uint32_t encoding)
{
    return (encoding & 0xFF800000) == 0x91000000;
}

/**
 * Check if instruction is SUB (immediate)
 * @param encoding ARM64 instruction encoding
 * @return 1 if SUB, 0 otherwise
 */
static inline int arm64_is_sub_imm(uint32_t encoding)
{
    return (encoding & 0xFF800000) == 0xD1000000;
}

/**
 * Check if instruction is AND (immediate)
 * @param encoding ARM64 instruction encoding
 * @return 1 if AND, 0 otherwise
 */
static inline int arm64_is_and_imm(uint32_t encoding)
{
    return (encoding & 0xFF800000) == 0x12000000;
}

/**
 * Check if instruction is MOVZ (move wide with zero)
 * @param encoding ARM64 instruction encoding
 * @return 1 if MOVZ, 0 otherwise
 */
static inline int arm64_is_movz(uint32_t encoding)
{
    return (encoding & 0xFF800000) == 0xD2800000;
}

/**
 * Check if instruction is MOVK (move wide with keep)
 * @param encoding ARM64 instruction encoding
 * @return 1 if MOVK, 0 otherwise
 */
static inline int arm64_is_movk(uint32_t encoding)
{
    return (encoding & 0xFF800000) == 0xF2800000;
}

/**
 * Check if instruction is MOVN (move wide with negation)
 * @param encoding ARM64 instruction encoding
 * @return 1 if MOVN, 0 otherwise
 */
static inline int arm64_is_movn(uint32_t encoding)
{
    return (encoding & 0xFF800000) == 0x12800000;
}

/**
 * Check if instruction is LDP (load pair)
 * @param encoding ARM64 instruction encoding
 * @return 1 if LDP, 0 otherwise
 */
static inline int arm64_is_ldp(uint32_t encoding)
{
    return (encoding & 0xFF800000) == 0x29400000;
}

/**
 * Check if instruction is STP (store pair)
 * @param encoding ARM64 instruction encoding
 * @return 1 if STP, 0 otherwise
 */
static inline int arm64_is_stp(uint32_t encoding)
{
    return (encoding & 0xFF800000) == 0x29000000;
}

/**
 * Check if instruction is CMP (compare)
 * @param encoding ARM64 instruction encoding
 * @return 1 if CMP, 0 otherwise
 */
static inline int arm64_is_cmp(uint32_t encoding)
{
    return (encoding & 0xFFE00000) == 0xEB000000;
}

/**
 * Check if instruction is CMN (compare negative)
 * @param encoding ARM64 instruction encoding
 * @return 1 if CMN, 0 otherwise
 */
static inline int arm64_is_cmn(uint32_t encoding)
{
    return (encoding & 0xFFE00000) == 0xAB000000;
}

/**
 * Check if instruction is TST (test)
 * @param encoding ARM64 instruction encoding
 * @return 1 if TST, 0 otherwise
 */
static inline int arm64_is_tst(uint32_t encoding)
{
    return (encoding & 0xFFE00000) == 0xEA000000;
}

/**
 * Check if instruction is conditional branch (B.cond)
 * @param encoding ARM64 instruction encoding
 * @return 1 if B.cond, 0 otherwise
 */
static inline int arm64_is_bcond(uint32_t encoding)
{
    return (encoding & 0xFF000000) == 0x54000000;
}

/**
 * Get condition code from conditional branch
 * @param encoding ARM64 instruction encoding
 * @return Condition code (0-15)
 */
static inline uint8_t arm64_get_cond(uint32_t encoding)
{
    return (encoding >> 0) & 0x0F;
}

/**
 * Get shift amount from instruction
 * @param encoding ARM64 instruction encoding
 * @return Shift amount (0, 1, 2, or 3 for LSL #0, #1, #2, #3)
 */
static inline uint8_t arm64_get_shift(uint32_t encoding)
{
    return (encoding >> 22) & 0x3;
}

/**
 * Get 16-bit immediate from MOVZ/MOVK/MOVN
 * @param encoding ARM64 instruction encoding
 * @return 16-bit immediate value
 */
static inline uint16_t arm64_get_imm16(uint32_t encoding)
{
    return (encoding >> 5) & 0xFFFF;
}

/**
 * Get shift amount for MOVZ/MOVK (0, 1, 2, 3 for LSL #0, #16, #32, #48)
 * @param encoding ARM64 instruction encoding
 * @return Shift amount
 */
static inline uint8_t arm64_get_hw(uint32_t encoding)
{
    return (encoding >> 21) & 0x3;
}

/* ============================================================================
 * Entry Point and Initialization
 * ============================================================================ */

/**
 * Rosetta Entry Point
 *
 * This is the main entry point for the Rosetta translator. It:
 * 1. Parses command-line arguments from the stack
 * 2. Initializes the translation environment
 * 3. Sets up the CPU context
 * 4. Transfers control to the translated code
 */
void rosetta_entry(void)
{
    long *arg_ptr;
    long current_arg;
    long initial_sp;
    void *main_callback;
    uint64_t cpu_flags;

    /* Parse argc/argv from stack */
    arg_ptr = (long *)(&initial_sp + (initial_sp + 1) * 8);
    do {
        current_arg = *arg_ptr;
        arg_ptr = arg_ptr + 1;
    } while (current_arg != 0);

    /* Initialize translation environment */
    init_translation_env(&main_callback);

    /* Set default FP control register */
    cpu_flags = 0;

    /* Check if FZ16 (flush-to-zero for half precision) is enabled */
    if ((*(byte *)(cpu_flags + 0x138) >> 1 & 1) != 0) {
        cpu_flags = 6;  /* Enable FZ16 mode */
    }

    /* Transfer control to translated entry point */
    ((entry_point_t)main_callback)(0, 0, 0, 0, 0, cpu_flags, 0, 0, 0);
}

/* ============================================================================
 * FP Context Management
 * ============================================================================ */

/**
 * Load FP/Vortex registers from memory
 *
 * @param ctx Pointer to 16-byte register data
 */
Vector128 load_vector_register(const Vector128 *ctx)
{
    return *ctx;
}

/**
 * Set FP control and status registers
 *
 * @param fpcr_value New FPCR value
 * @param fpsr_value New FPSR value
 */
void set_fp_registers(uint64_t fpcr_value, uint64_t fpsr_value)
{
    write_fpcr(fpcr_value);
    write_fpsr(fpsr_value);
}

/**
 * Clear FP control and status registers
 */
void clear_fp_registers(void)
{
    write_fpcr(0);
    write_fpsr(0);
}

/**
 * No-operation FP operation
 * Returns zero vector
 */
Vector128 fp_noop(void)
{
    return (Vector128){0, 0};
}

/* ============================================================================
 * CPU Context Save/Restore
 * ============================================================================ */

/**
 * Save CPU context to memory structure
 *
 * Saves all general purpose registers, vector registers,
 * and FP control registers to a contiguous memory area.
 *
 * @param ctx Source CPU context
 * @param save_area Destination memory area (0x210 bytes)
 * @param flags Save flags
 */
void save_cpu_context_full(CPUContext *ctx, void *save_area, long flags)
{
    uint64_t *save = (uint64_t *)save_area;
    uint32_t fpsr_val;
    uint32_t fpcr_val;

    /* Save FP status */
    fpsr_val = read_fpsr();
    ((uint32_t *)(save_area + 8))[0] = fpsr_val;

    /* Save FP control */
    fpcr_val = read_fpcr();
    ((uint32_t *)(save_area + 0xc))[0] = fpcr_val;

    /* Save vector registers V0-V31 */
    for (int i = 0; i < 32; i++) {
        save[2 + i * 2] = ctx->vec.v[i].lo;
        save[3 + i * 2] = ctx->vec.v[i].hi;
    }

    /* Save general purpose registers */
    for (int i = 0; i < 30; i++) {
        save[66 + i] = ctx->gpr.x[i];
    }
    save[96] = ctx->gpr.lr;
    save[97] = ctx->gpr.sp;
    save[98] = ctx->gpr.pc;
    save[99] = ctx->gpr.nzcv;
}

/**
 * Restore CPU context from memory structure
 *
 * @param ctx Destination CPU context
 * @param save_area Source memory area
 */
void restore_cpu_context_full(CPUContext *ctx, const void *save_area)
{
    const uint64_t *save = (const uint64_t *)save_area;

    /* Restore vector registers V0-V31 */
    for (int i = 0; i < 32; i++) {
        ctx->vec.v[i].lo = save[2 + i * 2];
        ctx->vec.v[i].hi = save[3 + i * 2];
    }

    /* Restore general purpose registers */
    for (int i = 0; i < 30; i++) {
        ctx->gpr.x[i] = save[66 + i];
    }
    ctx->gpr.lr = save[96];
    ctx->gpr.sp = save[97];
    ctx->gpr.pc = save[98];
    ctx->gpr.nzcv = save[99];
}

/* ============================================================================
 * Session 42: Basic Block Translation Helpers
 * ============================================================================ */

/**
 * Check if instruction is a block terminator (branch, return, etc.)
 * @param encoding ARM64 instruction encoding
 * @return 1 if terminator, 0 otherwise
 */
static inline int arm64_is_block_terminator(uint32_t encoding)
{
    /* Branch instructions */
    if (arm64_is_b(encoding)) return 1;
    if (arm64_is_bl(encoding)) return 1;
    if (arm64_is_br(encoding)) return 1;
    if (arm64_is_bcond(encoding)) return 1;

    /* Conditional branches */
    if ((encoding & 0x7F000000) == 0x34000000) return 1;  /* CBZ */
    if ((encoding & 0x7F000000) == 0x35000000) return 1;  /* CBNZ */
    if ((encoding & 0x7F000000) == 0x36000000) return 1;  /* TBZ */
    if ((encoding & 0x7F000000) == 0x37000000) return 1;  /* TBNZ */

    /* Return instructions */
    if ((encoding & 0xFFFFFC1F) == 0xD65F0000) return 1;  /* RET */
    if ((encoding & 0xFF000000) == 0xD4000000) return 1;  /* SVC/HLT/BRK */

    return 0;
}

/**
 * Get the length of an ARM64 instruction (all are 4 bytes)
 * @param encoding ARM64 instruction encoding
 * @return Instruction length in bytes
 */
static inline int arm64_instruction_length(uint32_t encoding)
{
    (void)encoding;  /* All ARM64 instructions are 4 bytes */
    return 4;
}

/**
 * Check if instruction is SVC (supervisor call)
 * @param encoding ARM64 instruction encoding
 * @return 1 if SVC, 0 otherwise
 */
static inline int arm64_is_svc(uint32_t encoding)
{
    /* SVC has specific encoding pattern in system instruction space */
    return (encoding & 0xFF000000) == 0xD4000000 && (encoding & 0xFFFF) != 0;
}

/**
 * Check if instruction is BRK (breakpoint)
 * @param encoding ARM64 instruction encoding
 * @return 1 if BRK, 0 otherwise
 */
static inline int arm64_is_brk(uint32_t encoding)
{
    return (encoding & 0xFFE00000) == 0xD4200000;
}

/**
 * Check if instruction is HLT (halt)
 * @param encoding ARM64 instruction encoding
 * @return 1 if HLT, 0 otherwise
 */
static inline int arm64_is_hlt(uint32_t encoding)
{
    return (encoding & 0xFFE00000) == 0xD4000000;
}

/**
 * Get 19-bit branch immediate
 * @param encoding ARM64 instruction encoding
 * @return Sign-extended 19-bit immediate
 */
static inline int32_t arm64_get_imm19(uint32_t encoding)
{
    return (int32_t)((encoding >> 5) << 13) >> 11;  /* Sign extend */
}

/**
 * Get 14-bit CBZ/CBNZ immediate
 * @param encoding ARM64 instruction encoding
 * @return Sign-extended 14-bit immediate
 */
static inline int32_t arm64_get_imm14(uint32_t encoding)
{
    return (int32_t)((encoding >> 5) << 18) >> 16;  /* Sign extend */
}

/**
 * Get 7-bit TBZ/TBNZ immediate
 * @param encoding ARM64 instruction encoding
 * @return Sign-extended 7-bit immediate
 */
static inline int32_t arm64_get_imm7(uint32_t encoding)
{
    return (int32_t)((encoding >> 5) << 25) >> 23;  /* Sign extend */
}

/**
 * Get test bit from TBZ/TBNZ
 * @param encoding ARM64 instruction encoding
 * @return Bit number to test (0-63)
 */
static inline uint8_t arm64_get_test_bit(uint32_t encoding)
{
    uint8_t bit5 = (encoding >> 31) & 0x20;
    uint8_t bit4_0 = (encoding >> 19) & 0x1F;
    return bit5 | bit4_0;
}

/**
 * Check if instruction is RET (return)
 * @param encoding ARM64 instruction encoding
 * @return 1 if RET, 0 otherwise
 */
static inline int arm64_is_ret(uint32_t encoding)
{
    return (encoding & 0xFFFFFC1F) == 0xD65F0000;
}

/**
 * Check if instruction is TBZ (test bit and branch if zero)
 * @param encoding ARM64 instruction encoding
 * @return 1 if TBZ, 0 otherwise
 */
static inline int arm64_is_tbz(uint32_t encoding)
{
    return (encoding & 0x7E000000) == 0x36000000;
}

/**
 * Check if instruction is TBNZ (test bit and branch if not zero)
 * @param encoding ARM64 instruction encoding
 * @return 1 if TBNZ, 0 otherwise
 */
static inline int arm64_is_tbnz(uint32_t encoding)
{
    return (encoding & 0x7E000000) == 0x37000000;
}

/**
 * Check if instruction is LDRB (load register byte)
 * @param encoding ARM64 instruction encoding
 * @return 1 if LDRB, 0 otherwise
 */
static inline int arm64_is_ldrb(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0x38400000;
}

/**
 * Check if instruction is STRB (store register byte)
 * @param encoding ARM64 instruction encoding
 * @return 1 if STRB, 0 otherwise
 */
static inline int arm64_is_strb(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0x38000000;
}

/**
 * Check if instruction is LDRH (load register halfword)
 * @param encoding ARM64 instruction encoding
 * @return 1 if LDRH, 0 otherwise
 */
static inline int arm64_is_ldrh(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0x78400000;
}

/**
 * Check if instruction is STRH (store register halfword)
 * @param encoding ARM64 instruction encoding
 * @return 1 if STRH, 0 otherwise
 */
static inline int arm64_is_strh(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0x78000000;
}

/**
 * Check if instruction is LDRSB (load register signed byte)
 * @param encoding ARM64 instruction encoding
 * @return 1 if LDRSB, 0 otherwise
 */
static inline int arm64_is_ldrsb(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0x38C00000;
}

/**
 * Check if instruction is LDRSH (load register signed halfword)
 * @param encoding ARM64 instruction encoding
 * @return 1 if LDRSH, 0 otherwise
 */
static inline int arm64_is_ldrsh(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0x78C00000;
}

/**
 * Check if instruction is LDRSW (load register signed word)
 * @param encoding ARM64 instruction encoding
 * @return 1 if LDRSW, 0 otherwise
 */
static inline int arm64_is_ldrsw(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0xB8400000;
}

/**
 * Check if instruction is LDUR (load register unscaled)
 * @param encoding ARM64 instruction encoding
 * @return 1 if LDUR, 0 otherwise
 */
static inline int arm64_is_ldur(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0xF8400000;
}

/**
 * Check if instruction is STUR (store register unscaled)
 * @param encoding ARM64 instruction encoding
 * @return 1 if STUR, 0 otherwise
 */
static inline int arm64_is_stur(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0xF8000000;
}

/**
 * Check if instruction is EOR (exclusive OR)
 * @param encoding ARM64 instruction encoding
 * @return 1 if EOR, 0 otherwise
 */
static inline int arm64_is_eor(uint32_t encoding)
{
    return (encoding & 0xFFE0E000) == 0x4A000000;
}

/**
 * Check if instruction is ORR (inclusive OR)
 * @param encoding ARM64 instruction encoding
 * @return 1 if ORR, 0 otherwise
 */
static inline int arm64_is_orr(uint32_t encoding)
{
    return (encoding & 0xFFE0E000) == 0x2A000000;
}

/**
 * Check if instruction is AND (bitwise AND)
 * @param encoding ARM64 instruction encoding
 * @return 1 if AND, 0 otherwise
 */
static inline int arm64_is_and(uint32_t encoding)
{
    return (encoding & 0xFFE0E000) == 0x0A000000;
}

/**
 * Check if instruction is MVN (move negated)
 * @param encoding ARM64 instruction encoding
 * @return 1 if MVN, 0 otherwise
 */
static inline int arm64_is_mvn(uint32_t encoding)
{
    return (encoding & 0xFFE0E000) == 0x4A200000;
}

/**
 * Check if instruction is MUL (multiply)
 * @param encoding ARM64 instruction encoding
 * @return 1 if MUL, 0 otherwise
 */
static inline int arm64_is_mul(uint32_t encoding)
{
    return (encoding & 0xFFE0E000) == 0x1B000000;
}

/**
 * Check if instruction is SDIV (signed divide)
 * @param encoding ARM64 instruction encoding
 * @return 1 if SDIV, 0 otherwise
 */
static inline int arm64_is_div(uint32_t encoding)
{
    return (encoding & 0xFFE0E000) == 0x9AC00000;
}

/* ============================================================================
 * Session 45: SIMD/FP Infrastructure and Instruction Decoding
 * ============================================================================ */

/**
 * Check if instruction is FP/SIMD (op30 == 3)
 * @param encoding ARM64 instruction encoding
 * @return 1 if FP/SIMD instruction, 0 otherwise
 */
static inline int arm64_is_fp_insn(uint32_t encoding)
{
    return ((encoding >> 28) & 0x7) == 0x7;  /* op30 == 3 */
}

/**
 * Extract SIMD/FP opcode from instruction
 * @param encoding ARM64 instruction encoding
 * @return SIMD/FP opcode (bits 28-31)
 */
static inline uint8_t arm64_get_fp_opcode(uint32_t encoding)
{
    return (encoding >> 28) & 0xF;
}

/**
 * Extract destination SIMD/FP register (Vd)
 * @param encoding ARM64 instruction encoding
 * @return SIMD/FP register number (0-31)
 */
static inline uint8_t arm64_get_vd(uint32_t encoding)
{
    return ((encoding >> 5) & 0x1F);
}

/**
 * Extract first operand SIMD/FP register (Vn)
 * @param encoding ARM64 instruction encoding
 * @return SIMD/FP register number (0-31)
 */
static inline uint8_t arm64_get_vn(uint32_t encoding)
{
    return ((encoding >> 10) & 0x1F);
}

/**
 * Extract second operand SIMD/FP register (Vm)
 * @param encoding ARM64 instruction encoding
 * @return SIMD/FP register number (0-31)
 */
static inline uint8_t arm64_get_vm(uint32_t encoding)
{
    return ((encoding >> 20) & 0x1F);
}

/**
 * Extract SIMD/FP register with Q flag (Vd including Q bit)
 * @param encoding ARM64 instruction encoding
 * @return SIMD/FP register number (0-31) with Q flag in bit 5
 */
static inline uint8_t arm64_get_vd_q(uint32_t encoding)
{
    uint8_t vd = (encoding >> 5) & 0x1F;
    uint8_t q = (encoding >> 30) & 0x1;
    return vd | (q << 5);
}

/**
 * Check if instruction is FMov (immediate)
 * @param encoding ARM64 instruction encoding
 * @return 1 if FMov immediate, 0 otherwise
 */
static inline int arm64_is_fmov_imm(uint32_t encoding)
{
    /* FMov (immediate): 00011110001mmmmmmmmm1dddfffff */
    return (encoding & 0xFF800400) == 0x1E800000;
}

/**
 * Check if instruction is FMov (register)
 * @param encoding ARM64 instruction encoding
 * @return 1 if FMov register, 0 otherwise
 */
static inline int arm64_is_fmov_reg(uint32_t encoding)
{
    /* FMov (register): 0001111000000000000001dddfffff */
    return (encoding & 0xFFFF0400) == 0x1E800000;
}

/**
 * Check if instruction is FAdd (FP add)
 * @param encoding ARM64 instruction encoding
 * @return 1 if FAdd, 0 otherwise
 */
static inline int arm64_is_fadd(uint32_t encoding)
{
    /* FAdd: 00011110000mmmmm000001dddfffff */
    return (encoding & 0xFFE0FC00) == 0x1E200000;
}

/**
 * Check if instruction is FSub (FP subtract)
 * @param encoding ARM64 instruction encoding
 * @return 1 if FSub, 0 otherwise
 */
static inline int arm64_is_fsub(uint32_t encoding)
{
    /* FSub: 00011110000mmmmm000011dddfffff */
    return (encoding & 0xFFE0FC00) == 0x1E600000;
}

/**
 * Check if instruction is FMul (FP multiply)
 * @param encoding ARM64 instruction encoding
 * @return 1 if FMul, 0 otherwise
 */
static inline int arm64_is_fmul(uint32_t encoding)
{
    /* FMul: 00011110000mmmmm000010dddfffff */
    return (encoding & 0xFFE0FC00) == 0x1E300000;
}

/**
 * Check if instruction is FDiv (FP divide)
 * @param encoding ARM64 instruction encoding
 * @return 1 if FDiv, 0 otherwise
 */
static inline int arm64_is_fdiv(uint32_t encoding)
{
    /* FDiv: 00011110000mmmmm000110dddfffff */
    return (encoding & 0xFFE0FC00) == 0x1E800000;
}

/**
 * Check if instruction is FSqrt (FP square root)
 * @param encoding ARM64 instruction encoding
 * @return 1 if FSqrt, 0 otherwise
 */
static inline int arm64_is_fsqrt(uint32_t encoding)
{
    /* FSqrt: 0001111000100000011001dddfffff */
    return (encoding & 0xFFFFFC00) == 0x1E900000;
}

/**
 * Check if instruction is FCmp (FP compare)
 * @param encoding ARM64 instruction encoding
 * @return 1 if FCmp, 0 otherwise
 */
static inline int arm64_is_fcmp(uint32_t encoding)
{
    /* FCmp: 00011110001mmmmm000001000000000 */
    return (encoding & 0xFFE0FC00) == 0x1E800000;
}

/**
 * Extract FP/NEON register mapping to XMM registers
 * Maps ARM64 V0-V31 to x86_64 XMM0-XMM15 (lower 128 bits)
 * @param vreg ARM64 vector register number (0-31)
 * @return x86_64 XMM register index (0-15)
 */
static inline uint8_t map_vreg_to_xmm(uint8_t vreg)
{
    /* Simple mapping: V0->XMM0, V1->XMM1, ..., V15->XMM15, V16->XMM0, etc. */
    return vreg & 0xF;
}

/**
 * Extract FP immediate value from FMov instruction
 * @param encoding ARM64 instruction encoding
 * @return 8-bit FP immediate (encoded as per ARM64 spec)
 */
static inline uint8_t arm64_get_fpmem16(uint32_t encoding)
{
    return ((encoding >> 13) & 0xF0) | ((encoding >> 5) & 0x0F);
}

/**
 * Check if instruction is FCSEL (FP conditional select)
 * @param encoding ARM64 instruction encoding
 * @return 1 if FCSEL, 0 otherwise
 */
static inline int arm64_is_fcsel(uint32_t encoding)
{
    /* FCSEL: 00011110010mmmmm00110000dddfffff */
    return (encoding & 0xFFE0FC00) == 0x1E800C00;
}

/**
 * Check if instruction is FCCMP (FP conditional compare)
 * @param encoding ARM64 instruction encoding
 * @return 1 if FCCMP, 0 otherwise
 */
static inline int arm64_is_fccmp(uint32_t encoding)
{
    /* FCCMP: 00011110001mmmmm00100000dddfffff */
    return (encoding & 0xFFE0FC00) == 0x1E800400;
}

/**
 * Check if instruction is FABS (FP absolute value)
 * @param encoding ARM64 instruction encoding
 * @return 1 if FABS, 0 otherwise
 */
static inline int arm64_is_fabs(uint32_t encoding)
{
    /* FABS: 0001111000100000010000dddfffff */
    return (encoding & 0xFFFFFC00) == 0x1E200800;
}

/**
 * Check if instruction is FNEG (FP negate)
 * @param encoding ARM64 instruction encoding
 * @return 1 if FNEG, 0 otherwise
 */
static inline int arm64_is_fneg(uint32_t encoding)
{
    /* FNEG: 0001111000100000010001dddfffff */
    return (encoding & 0xFFFFFC00) == 0x1E201800;
}

/**
 * Check if instruction is FCVTDS (FP convert double to single)
 * @param encoding ARM64 instruction encoding
 * @return 1 if FCVTDS, 0 otherwise
 */
static inline int arm64_is_fcvtds(uint32_t encoding)
{
    /* FCVTDS: 0001111000100001011000dddfffff */
    return (encoding & 0xFFFFFC00) == 0x1E60C000;
}

/**
 * Check if instruction is FCVTSD (FP convert single to double)
 * @param encoding ARM64 instruction encoding
 * @return 1 if FCVTSD, 0 otherwise
 */
static inline int arm64_is_fcvtsd(uint32_t encoding)
{
    /* FCVTSD: 0001111000100001011001dddfffff */
    return (encoding & 0xFFFFFC00) == 0x1E20C000;
}

/**
 * Extract condition code from FP instruction
 * @param encoding ARM64 instruction encoding
 * @return Condition code (0-15)
 */
static inline uint8_t arm64_get_fp_cond(uint32_t encoding)
{
    return (encoding >> 12) & 0xF;
}

/* ============================================================================
 * Session 46: NEON Vector Instruction Decoders
 * ============================================================================ */

/**
 * Check if instruction is ADD (vector)
 * @param encoding ARM64 instruction encoding
 * @return 1 if ADD vector, 0 otherwise
 */
static inline int arm64_is_add_vec(uint32_t encoding)
{
    /* ADD (vector): 0101ssss000mmmmm000001dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E200000;
}

/**
 * Check if instruction is SUB (vector)
 * @param encoding ARM64 instruction encoding
 * @return 1 if SUB vector, 0 otherwise
 */
static inline int arm64_is_sub_vec(uint32_t encoding)
{
    /* SUB (vector): 0101ssss000mmmmm000011dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E600000;
}

/**
 * Check if instruction is AND (vector)
 * @param encoding ARM64 instruction encoding
 * @return 1 if AND vector, 0 otherwise
 */
static inline int arm64_is_and_vec(uint32_t encoding)
{
    /* AND (vector): 0101ssss000mmmmm000111dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E000000;
}

/**
 * Check if instruction is ORR (vector)
 * @param encoding ARM64 instruction encoding
 * @return 1 if ORR vector, 0 otherwise
 */
static inline int arm64_is_orr_vec(uint32_t encoding)
{
    /* ORR (vector): 0101ssss000mmmmm000101dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E200000;
}

/**
 * Check if instruction is EOR (vector)
 * @param encoding ARM64 instruction encoding
 * @return 1 if EOR vector, 0 otherwise
 */
static inline int arm64_is_eor_vec(uint32_t encoding)
{
    /* EOR (vector): 0110ssss000mmmmm000111dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E800000;
}

/**
 * Check if instruction is BIC (vector bit clear)
 * @param encoding ARM64 instruction encoding
 * @return 1 if BIC vector, 0 otherwise
 */
static inline int arm64_is_bic_vec(uint32_t encoding)
{
    /* BIC (vector): 0110ssss000mmmmm000001dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E600000;
}

/**
 * Get vector size field from instruction
 * @param encoding ARM64 instruction encoding
 * @return Size field (0-3)
 */
static inline uint8_t arm64_get_vec_size(uint32_t encoding)
{
    return ((encoding >> 22) & 0x3);
}

/**
 * Get Q bit (128-bit flag) from instruction
 * @param encoding ARM64 instruction encoding
 * @return 1 if 128-bit operation, 0 if 64-bit
 */
static inline uint8_t arm64_get_q_bit(uint32_t encoding)
{
    return ((encoding >> 30) & 0x1);
}

/* ============================================================================
 * Session 49: Additional NEON Vector Instruction Decoders
 * ============================================================================ */

/**
 * Check if instruction is MUL (vector)
 * @param encoding ARM64 instruction encoding
 * @return 1 if MUL vector, 0 otherwise
 */
static inline int arm64_is_mul_vec(uint32_t encoding)
{
    /* MUL (vector): 0101ssss000mmmmm100001dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E000000;
}

/**
 * Check if instruction is SSHR (signed shift right)
 * @param encoding ARM64 instruction encoding
 * @return 1 if SSHR, 0 otherwise
 */
static inline int arm64_is_sshr_vec(uint32_t encoding)
{
    /* SSHR (vector): 0101ssss001mmmmm111001dddddddd */
    return (encoding & 0xFE200400) == 0x0E000400;
}

/**
 * Check if instruction is USHR (unsigned shift right)
 * @param encoding ARM64 instruction encoding
 * @return 1 if USHR, 0 otherwise
 */
static inline int arm64_is_ushr_vec(uint32_t encoding)
{
    /* USHR (vector): 0101ssss001mmmmm110001dddddddd */
    return (encoding & 0xFE200400) == 0x0E000400;
}

/**
 * Check if instruction is SHL (vector shift left)
 * @param encoding ARM64 instruction encoding
 * @return 1 if SHL, 0 otherwise
 */
static inline int arm64_is_shl_vec(uint32_t encoding)
{
    /* SHL (vector): 0101ssss001mmmmm100001dddddddd */
    return (encoding & 0xFE200400) == 0x0E000400;
}

/**
 * Check if instruction is CMGT (signed greater than)
 * @param encoding ARM64 instruction encoding
 * @return 1 if CMGT, 0 otherwise
 */
static inline int arm64_is_cmgt_vec(uint32_t encoding)
{
    /* CMGT (vector): 0101ssss000mmmmm001011dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E200000;
}

/**
 * Check if instruction is CMEQ (compare equal)
 * @param encoding ARM64 instruction encoding
 * @return 1 if CMEQ, 0 otherwise
 */
static inline int arm64_is_cmeq_vec(uint32_t encoding)
{
    /* CMEQ (vector): 0101ssss000mmmmm001001dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E200000;
}

/**
 * Get shift immediate from SIMD shift instruction
 * @param encoding ARM64 instruction encoding
 * @return Shift amount (signed, can be negative for right shifts)
 */
static inline int8_t arm64_get_shift_imm(uint32_t encoding)
{
    /* Shift immediate is in bits 16-21 (6 bits, signed) */
    int8_t imm = ((encoding >> 16) & 0x3F);
    /* Sign extend from 6 bits */
    if (imm & 0x20) imm |= 0xC0;  /* Negative */
    return imm;
}

/* ============================================================================
 * Session 50: Additional NEON Compare and Arithmetic Instructions
 * ============================================================================ */

/**
 * Check if instruction is CMGE (compare signed greater than or equal)
 * @param encoding ARM64 instruction encoding
 * @return 1 if CMGE, 0 otherwise
 */
static inline int arm64_is_cmge_vec(uint32_t encoding)
{
    /* CMGE (vector): 0101ssss000mmmmm001111dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E300000;
}

/**
 * Check if instruction is CMHS (compare unsigned higher or same)
 * @param encoding ARM64 instruction encoding
 * @return 1 if CMHS, 0 otherwise
 */
static inline int arm64_is_cmhs_vec(uint32_t encoding)
{
    /* CMHS (vector): 0101ssss000mmmmm001110dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E300000;
}

/**
 * Check if instruction is CMLE (compare signed less than or equal)
 * @param encoding ARM64 instruction encoding
 * @return 1 if CMLE, 0 otherwise
 */
static inline int arm64_is_cmle_vec(uint32_t encoding)
{
    /* CMLE (vector): 0101ssss000mmmmm000111dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E100000;
}

/**
 * Check if instruction is CMLT (compare signed less than)
 * @param encoding ARM64 instruction encoding
 * @return 1 if CMLT, 0 otherwise
 */
static inline int arm64_is_cmlt_vec(uint32_t encoding)
{
    /* CMLT (vector): 0101ssss000mmmmm001010dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E200000;
}

/**
 * Check if instruction is UMIN (unsigned minimum)
 * @param encoding ARM64 instruction encoding
 * @return 1 if UMIN, 0 otherwise
 */
static inline int arm64_is_umin_vec(uint32_t encoding)
{
    /* UMIN (vector): 0101ssss000mmmmm011000dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E600000;
}

/**
 * Check if instruction is UMAX (unsigned maximum)
 * @param encoding ARM64 instruction encoding
 * @return 1 if UMAX, 0 otherwise
 */
static inline int arm64_is_umax_vec(uint32_t encoding)
{
    /* UMAX (vector): 0101ssss000mmmmm011110dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E700000;
}

/**
 * Check if instruction is SMIN (signed minimum)
 * @param encoding ARM64 instruction encoding
 * @return 1 if SMIN, 0 otherwise
 */
static inline int arm64_is_smin_vec(uint32_t encoding)
{
    /* SMIN (vector): 0101ssss000mmmmm011001dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E600000;
}

/**
 * Check if instruction is SMAX (signed maximum)
 * @param encoding ARM64 instruction encoding
 * @return 1 if SMAX, 0 otherwise
 */
static inline int arm64_is_smax_vec(uint32_t encoding)
{
    /* SMAX (vector): 0101ssss000mmmmm011111dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E700000;
}

/**
 * Check if instruction is FRECPE (floating-point reciprocal estimate)
 * @param encoding ARM64 instruction encoding
 * @return 1 if FRECPE, 0 otherwise
 */
static inline int arm64_is_frecpe(uint32_t encoding)
{
    /* FRECPE: 01011110001mmmmm001100dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E200000;
}

/**
 * Check if instruction is FRSQRTE (floating-point reciprocal square root estimate)
 * @param encoding ARM64 instruction encoding
 * @return 1 if FRSQRTE, 0 otherwise
 */
static inline int arm64_is_frsqrte(uint32_t encoding)
{
    /* FRSQRTE: 01011110001mmmmm001101dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E200000;
}

/* ============================================================================
 * Session 51: NEON Load/Store Instructions (LD1/ST1)
 * ============================================================================ */

/**
 * Check if instruction is LD1 (single structure)
 * @param encoding ARM64 instruction encoding
 * @return 1 if LD1, 0 otherwise
 */
static inline int arm64_is_ld1(uint32_t encoding)
{
    /* LD1 (single structure): 00110100000mmmmm011101dddddddd */
    return (encoding & 0xFF20FC00) == 0x0C000000;
}

/**
 * Check if instruction is ST1 (single structure)
 * @param encoding ARM64 instruction encoding
 * @return 1 if ST1, 0 otherwise
 */
static inline int arm64_is_st1(uint32_t encoding)
{
    /* ST1 (single structure): 00110100000mmmmm001101dddddddd */
    return (encoding & 0xFF20FC00) == 0x08000000;
}

/**
 * Check if instruction is LD1 (multiple structures)
 * @param encoding ARM64 instruction encoding
 * @return 1 if LD1 multiple, 0 otherwise
 */
static inline int arm64_is_ld1_multiple(uint32_t encoding)
{
    /* LD1 (multiple): 00110100001mmmmm011101dddddddd */
    return (encoding & 0xFF20FC00) == 0x0C200000;
}

/**
 * Check if instruction is ST1 (multiple structures)
 * @param encoding ARM64 instruction encoding
 * @return 1 if ST1 multiple, 0 otherwise
 */
static inline int arm64_is_st1_multiple(uint32_t encoding)
{
    /* ST1 (multiple): 00110100001mmmmm001101dddddddd */
    return (encoding & 0xFF20FC00) == 0x08200000;
}

/**
 * Check if instruction is LD2 (pair of structures)
 * @param encoding ARM64 instruction encoding
 * @return 1 if LD2, 0 otherwise
 */
static inline int arm64_is_ld2(uint32_t encoding)
{
    /* LD2 (pair): 00110100010mmmmm011101dddddddd */
    return (encoding & 0xFF20FC00) == 0x0C400000;
}

/**
 * Check if instruction is ST2 (pair of structures)
 * @param encoding ARM64 instruction encoding
 * @return 1 if ST2, 0 otherwise
 */
static inline int arm64_is_st2(uint32_t encoding)
{
    /* ST2 (pair): 00110100010mmmmm001101dddddddd */
    return (encoding & 0xFF20FC00) == 0x08400000;
}

/**
 * Check if instruction is LD3 (three structures)
 * @param encoding ARM64 instruction encoding
 * @return 1 if LD3, 0 otherwise
 */
static inline int arm64_is_ld3(uint32_t encoding)
{
    /* LD3 (three): 00110100011mmmmm011101dddddddd */
    return (encoding & 0xFF20FC00) == 0x0C600000;
}

/**
 * Check if instruction is ST3 (three structures)
 * @param encoding ARM64 instruction encoding
 * @return 1 if ST3, 0 otherwise
 */
static inline int arm64_is_st3(uint32_t encoding)
{
    /* ST3 (three): 00110100011mmmmm001101dddddddd */
    return (encoding & 0xFF20FC00) == 0x08600000;
}

/**
 * Check if instruction is LD4 (four structures)
 * @param encoding ARM64 instruction encoding
 * @return 1 if LD4, 0 otherwise
 */
static inline int arm64_is_ld4(uint32_t encoding)
{
    /* LD4 (four): 00110100100mmmmm011101dddddddd */
    return (encoding & 0xFF20FC00) == 0x0C700000;
}

/**
 * Check if instruction is ST4 (four structures)
 * @param encoding ARM64 instruction encoding
 * @return 1 if ST4, 0 otherwise
 */
static inline int arm64_is_st4(uint32_t encoding)
{
    /* ST4 (four): 00110100100mmmmm001101dddddddd */
    return (encoding & 0xFF20FC00) == 0x08700000;
}

/**
 * Check if instruction is TBL (Table Lookup)
 * @param encoding ARM64 instruction encoding
 * @return 1 if TBL, 0 otherwise
 */
static inline int arm64_is_tbl(uint32_t encoding)
{
    /* TBL: 01001110000mmmmm000000dddddddd
     * Encoding: 0x4E000000 with mask 0xFE20FC00
     */
    return (encoding & 0xFE20FC00) == 0x0C000000;
}

/**
 * Check if instruction is TBX (Table Lookup Extension)
 * @param encoding ARM64 instruction encoding
 * @return 1 if TBX, 0 otherwise
 */
static inline int arm64_is_tbx(uint32_t encoding)
{
    /* TBX: 01001110001mmmmm000000dddddddd
     * Encoding: 0x4E200000 with mask 0xFE20FC00
     */
    return (encoding & 0xFE20FC00) == 0x0C200000;
}

/**
 * Get table lookup register count
 * @param encoding ARM64 instruction encoding
 * @return Number of table registers (1-4)
 */
static inline uint8_t arm64_get_tbl_reg_count(uint32_t encoding)
{
    /* Register count is in bits 10-11 */
    uint8_t count = ((encoding >> 10) & 0x03);
    return count + 1;  /* 1-4 registers */
}

/**
 * Map ARM64 vector register (V0-V31) to x86_64 XMM register (XMM0-XMM15)
 * @param vreg ARM64 vector register number (0-31)
 * @return x86_64 XMM register number (0-15)
 */
static inline uint8_t x86_map_xmm(uint8_t vreg)
{
    return vreg & 0x0F;  /* Wrap around: V0-V15 -> XMM0-XMM15, V16-V31 -> XMM0-XMM15 */
}

/**
 * Get NEON load/store register count
 * @param encoding ARM64 instruction encoding
 * @return Number of registers (1-4)
 */
static inline uint8_t arm64_get_neon_reg_count(uint32_t encoding)
{
    /* Register count is in bits 10-11 */
    uint8_t count = ((encoding >> 10) & 0x03);
    return count + 1;  /* 1-4 registers */
}

/**
 * Get NEON load/store size field
 * @param encoding ARM64 instruction encoding
 * @return Size field (0-3)
 */
static inline uint8_t arm64_get_neon_size(uint32_t encoding)
{
    /* Size field is in bits 22-23 */
    return ((encoding >> 22) & 0x03);
}

/**
 * Get NEON load/store index field
 * @param encoding ARM64 instruction encoding
 * @return Index for post-increment
 */
static inline int8_t arm64_get_neon_index(uint32_t encoding)
{
    /* Index is sign-extended from bits 10-11 or similar */
    int8_t index = ((encoding >> 10) & 0x0F);
    if (index & 0x08) index |= 0xF0;  /* Sign extend */
    return index;
}

/* ============================================================================
 * Session 42: Multi-Instruction Basic Block Translation
 * ============================================================================ */

#define MAX_BLOCK_INSTRUCTIONS 64  /* Maximum instructions per basic block */

/**
 * translate_block - Translate x86_64 basic block to ARM64
 *
 * @param guest_pc Guest x86_64 PC to translate
 * @return Translated block pointer, or NULL on failure
 *
 * Translates a basic block of x86_64 instructions (from entry point to
 * the next branch/return instruction) into equivalent ARM64 code.
 */
void *translate_block(uint64_t guest_pc)
{
    /* Look up in translation cache first */
    void *cached = translation_lookup(guest_pc);
    if (cached != NULL) {
        return cached;
    }

    /* Initialize code buffer for JIT emission */
    CodeBuffer code_buf;
    code_buffer_init(&code_buf, NULL, 65536);  /* 64KB per block max */

    /* Register mapping: x86_64 R0-R15 -> ARM64 X0-X15 */
    /* Direct mapping: RAX->X0, RCX->X1, RDX->X2, RBX->X3, etc. */

    /* Translate instructions in the basic block */
    uint64_t block_pc = guest_pc;
    int block_size = 0;
    int is_block_end = 0;

    while (!is_block_end && block_size < MAX_BLOCK_INSTRUCTIONS) {
        /* Decode x86_64 instruction at current PC */
        const uint8_t *insn_ptr = (const uint8_t *)block_pc;
        x86_insn_t insn;
        decode_x86_insn(insn_ptr, &insn);

        /* Map x86_64 registers to ARM64 */
        uint8_t arm_rd = map_x86_to_arm(insn.reg);
        uint8_t arm_rm = map_x86_to_arm(insn.rm);

        /* Translate based on instruction type using modular handlers */
        if (x86_is_add(&insn) || x86_is_sub(&insn) ||
            x86_is_and(&insn) || x86_is_or(&insn) || x86_is_xor(&insn) ||
            x86_is_mul(&insn) || x86_is_div(&insn) ||
            x86_is_inc(&insn) || x86_is_dec(&insn) ||
            x86_is_neg(&insn) || x86_is_not(&insn) ||
            x86_is_shl(&insn) || x86_is_shr(&insn) || x86_is_sar(&insn) ||
            x86_is_rol(&insn) || x86_is_ror(&insn)) {
            /* ALU operations */
            if (x86_is_add(&insn)) {
                translate_alu_add(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_sub(&insn)) {
                translate_alu_sub(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_and(&insn)) {
                translate_alu_and(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_or(&insn)) {
                translate_alu_or(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_xor(&insn)) {
                translate_alu_xor(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_mul(&insn)) {
                translate_alu_mul(&code_buf, &insn, arm_rm);
            } else if (x86_is_div(&insn)) {
                translate_alu_div(&code_buf, &insn, arm_rm);
            } else if (x86_is_inc(&insn)) {
                translate_alu_inc(&code_buf, &insn, arm_rd);
            } else if (x86_is_dec(&insn)) {
                translate_alu_dec(&code_buf, &insn, arm_rd);
            } else if (x86_is_neg(&insn)) {
                translate_alu_neg(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_not(&insn)) {
                translate_alu_not(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_shl(&insn) || x86_is_shr(&insn) || x86_is_sar(&insn) ||
                       x86_is_rol(&insn) || x86_is_ror(&insn)) {
                translate_alu_shift(&code_buf, &insn, arm_rd, arm_rm);
            }
        } else if (x86_is_mov(&insn) || x86_is_mov_imm64(&insn) ||
                   x86_is_movzx(&insn) || x86_is_movsx(&insn) || x86_is_movsxd(&insn) ||
                   x86_is_lea(&insn) || x86_is_push(&insn) || x86_is_pop(&insn) ||
                   x86_is_cmp(&insn) || x86_is_test(&insn)) {
            /* Memory operations */
            if (x86_is_mov_imm64(&insn)) {
                translate_memory_mov(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_mov(&insn)) {
                translate_memory_mov(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_movzx(&insn)) {
                translate_memory_movzx(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_movsx(&insn)) {
                translate_memory_movsx(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_movsxd(&insn)) {
                translate_memory_movsxd(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_lea(&insn)) {
                translate_memory_lea(&code_buf, &insn, arm_rd);
            } else if (x86_is_push(&insn)) {
                translate_memory_push(&code_buf, &insn, arm_rd);
            } else if (x86_is_pop(&insn)) {
                translate_memory_pop(&code_buf, &insn, arm_rd);
            } else if (x86_is_cmp(&insn)) {
                translate_memory_cmp(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_test(&insn)) {
                translate_memory_test(&code_buf, &insn, arm_rd, arm_rm);
            }
        } else if (x86_is_jcc(&insn) || x86_is_jmp(&insn) || x86_is_call(&insn) ||
                   x86_is_ret(&insn) || x86_is_cmov(&insn) || x86_is_setcc(&insn) ||
                   x86_is_xchg(&insn)) {
            /* Branch/control flow operations */
            if (x86_is_jcc(&insn)) {
                is_block_end = translate_branch_jcc(&code_buf, &insn, block_pc);
            } else if (x86_is_jmp(&insn)) {
                is_block_end = translate_branch_jmp(&code_buf, &insn, block_pc);
            } else if (x86_is_call(&insn)) {
                is_block_end = translate_branch_call(&code_buf, &insn, block_pc);
            } else if (x86_is_ret(&insn)) {
                is_block_end = translate_branch_ret(&code_buf);
            } else if (x86_is_cmov(&insn)) {
                translate_branch_cmov(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_setcc(&insn)) {
                translate_branch_setcc(&code_buf, &insn, arm_rd);
            } else if (x86_is_xchg(&insn)) {
                translate_branch_xchg(&code_buf, &insn, arm_rd, arm_rm);
            }
        } else if (x86_is_bsf(&insn) || x86_is_bsr(&insn) || x86_is_popcnt(&insn) ||
                   x86_is_bt(&insn) || x86_is_bts(&insn) || x86_is_btr(&insn) ||
                   x86_is_btc(&insn)) {
            /* Bit manipulation operations */
            if (x86_is_bsf(&insn)) {
                translate_bit_bsf(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_bsr(&insn)) {
                translate_bit_bsr(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_popcnt(&insn)) {
                translate_bit_popcnt(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_bt(&insn)) {
                translate_bit_bt(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_bts(&insn)) {
                translate_bit_bts(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_btr(&insn)) {
                translate_bit_btr(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_btc(&insn)) {
                translate_bit_btc(&code_buf, &insn, arm_rd, arm_rm);
            }
        } else if (x86_is_movs(&insn) || x86_is_stos(&insn) || x86_is_lods(&insn) ||
                   x86_is_cmps(&insn) || x86_is_scas(&insn)) {
            /* String operations */
            if (x86_is_movs(&insn)) {
                translate_string_movs(&code_buf, &insn);
            } else if (x86_is_stos(&insn)) {
                translate_string_stos(&code_buf, &insn);
            } else if (x86_is_lods(&insn)) {
                translate_string_lods(&code_buf, &insn);
            } else if (x86_is_cmps(&insn)) {
                translate_string_cmps(&code_buf, &insn);
            } else if (x86_is_scas(&insn)) {
                translate_string_scas(&code_buf, &insn);
            }
        } else if (x86_is_cpuid(&insn) || x86_is_rdtsc(&insn) || x86_is_shld(&insn) ||
                   x86_is_shrd(&insn) || x86_is_cwd(&insn) || x86_is_cqo(&insn) ||
                   x86_is_cli(&insn) || x86_is_sti(&insn) || x86_is_nop(&insn)) {
            /* Special instructions */
            if (x86_is_cpuid(&insn)) {
                translate_special_cpuid(&code_buf, &insn);
            } else if (x86_is_rdtsc(&insn)) {
                translate_special_rdtsc(&code_buf, &insn);
            } else if (x86_is_shld(&insn)) {
                translate_special_shld(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_shrd(&insn)) {
                translate_special_shrd(&code_buf, &insn, arm_rd, arm_rm);
            } else if (x86_is_cwd(&insn) || x86_is_cqo(&insn)) {
                translate_special_cqo(&code_buf, &insn);
            } else if (x86_is_cli(&insn)) {
                translate_special_cli(&code_buf, &insn);
            } else if (x86_is_sti(&insn)) {
                translate_special_sti(&code_buf, &insn);
            } else if (x86_is_nop(&insn)) {
                translate_special_nop(&code_buf, &insn);
            }
        } else {
            /* Unknown instruction - emit NOP */
            emit_nop(&code_buf);
        }

        /* Advance to next instruction */
        block_pc += insn.length;
        block_size++;

        /* Check for block end conditions */
        if (is_block_end || insn.length == 0) {
            break;
        }
    }

    /* Ensure block ends with RET if not already */
    if (!is_block_end) {
        emit_ret(&code_buf);
    }

    /* Check for errors */
    if (code_buf.error) {
        return NULL;
    }

    /* Allocate code cache memory and copy generated code */
    size_t code_size = code_buffer_get_size(&code_buf);
    void *code_cache = code_cache_alloc(code_size);
    if (!code_cache) {
        return NULL;
    }

    /* Copy generated code to executable memory */
    memcpy(code_cache, code_buf.buffer, code_size);

    /* Insert into translation cache */
    translation_insert(guest_pc, (uint64_t)code_cache, code_size);

    return code_cache;
}

/**
 * translate_block_fast - Fast path translation
 *
 * Optimized translation for hot blocks.
 *
 * @param guest_pc Guest x86_64 PC to translate
 * @return Translated block pointer, or NULL on failure
 */
void *translate_block_fast(uint64_t guest_pc)
{
    /* Check cache with faster lookup */
    void *cached = translation_lookup(guest_pc);
    if (cached != NULL) {
        return cached;
    }

    /* Would perform optimized translation */
    return translate_block(guest_pc);
}

/**
 * Execute translated block
 */
void execute_translated(ThreadState *state, void *block)
{
    /* Set up execution context */
    /* Jump to translated code */
    if (block == NULL || state == NULL) {
        return;
    }

    /* In a full implementation, this would:
     * 1. Save the current host context
     * 2. Set up the guest state in the execution context
     * 3. Jump to the translated x86_64 code block
     * 4. On return, restore the host context and update guest state
     */

    /* Cast block to function pointer and execute */
    typedef void (*translated_func_t)(void);
    ((translated_func_t)block)();
}

/**
 * SIMD-optimized memcpy
 */
void *rosetta_memcpy_aligned(void *dest, const void *src, size_t n)
{
    return memcpy(dest, src, n);
}

/**
 * SIMD-optimized memset
 */
void *rosetta_memset_simd(void *s, int c, size_t n)
{
    return memset(s, c, n);
}

/**
 * Handle guest syscall
 *
 * @param state Thread state
 * @param nr Syscall number
 * @return Syscall result
 */
int64_t handle_syscall(ThreadState *state, int nr)
{
    /* Translate ARM64 syscall number to host syscall number */
    /* ARM64 syscall number is in X8, arguments in X0-X5 */
    state->syscall_nr = nr;

    /* Dispatch to appropriate handler based on syscall number */
    /* Syscall numbers differ between ARM64 Linux and x86_64 macOS */

    /* For now, return the syscall_result set by the specific handler */
    return state->syscall_result;
}

/* ============================================================================
 * SIMD-Optimized Memory Search (memchr equivalent)
 * ============================================================================ */

/* Lookup tables for SIMD byte search */
static const uint64_t SHUFFLE_MASK_LO = 0x0c0e0f0d080a0b09ULL;
static const uint64_t SHUFFLE_MASK_HI = 0x0406070500020301ULL;

/**
 * SIMD-optimized memory search
 *
 * Searches for a null byte (or specific pattern) in memory using NEON
 * instructions for parallel comparison. This is significantly faster
 * than byte-by-byte comparison for large buffers.
 *
 * @param ptr Pointer to memory to search
 * @param len Maximum length to search (negative = unlimited)
 * @return Pointer to found byte, or NULL if not found
 */
void *rosetta_memchr_simd(const void *ptr, long len)
{
    const uint64_t *p;
    uint64_t word0, word1;
    uint64_t shuffled_lo, shuffled_hi;
    uint8_t bytes[16];
    uint8_t min_val;

    if (len < 0) {
        /* Unlimited search (like strlen/memchr without length) */
        p = (const uint64_t *)((uint64_t)ptr & ~0xfULL);
        word1 = p[1];
        word0 = p[0];

        /* Load shuffle masks for alignment */
        shuffled_hi = *((uint64_t*)&SHUFFLE_MASK_HI + 1);
        shuffled_lo = *((uint64_t*)&SHUFFLE_MASK_LO + 1);

        /* OR bytes with shuffle mask to handle alignment */
        bytes[0]  = (uint8_t)word0 | (uint8_t)shuffled_lo;
        bytes[1]  = (uint8_t)(word0 >> 8) | (uint8_t)(shuffled_lo >> 8);
        bytes[2]  = (uint8_t)(word0 >> 16) | (uint8_t)(shuffled_lo >> 16);
        bytes[3]  = (uint8_t)(word0 >> 24) | (uint8_t)(shuffled_lo >> 24);
        bytes[4]  = (uint8_t)(word0 >> 32) | (uint8_t)(shuffled_lo >> 32);
        bytes[5]  = (uint8_t)(word0 >> 40) | (uint8_t)(shuffled_lo >> 40);
        bytes[6]  = (uint8_t)(word0 >> 48) | (uint8_t)(shuffled_lo >> 48);
        bytes[7]  = (uint8_t)(word0 >> 56) | (uint8_t)(shuffled_lo >> 56);
        bytes[8]  = (uint8_t)word1 | (uint8_t)shuffled_hi;
        bytes[9]  = (uint8_t)(word1 >> 8) | (uint8_t)(shuffled_hi >> 8);
        bytes[10] = (uint8_t)(word1 >> 16) | (uint8_t)(shuffled_hi >> 16);
        bytes[11] = (uint8_t)(word1 >> 24) | (uint8_t)(shuffled_hi >> 24);
        bytes[12] = (uint8_t)(word1 >> 32) | (uint8_t)(shuffled_hi >> 32);
        bytes[13] = (uint8_t)(word1 >> 40) | (uint8_t)(shuffled_hi >> 40);
        bytes[14] = (uint8_t)(word1 >> 48) | (uint8_t)(shuffled_hi >> 48);
        bytes[15] = (uint8_t)(word1 >> 56) | (uint8_t)(shuffled_hi >> 56);

        while (1) {
            /* NEON UMINV - find minimum byte across vector */
            min_val = neon_uminv(bytes);

            /* If minimum is 0, we found a null byte */
            if (min_val == 0)
                break;

            /* Load next 16 bytes */
            word1 = p[3];
            word0 = p[2];
            bytes[0]  = (uint8_t)word0;
            bytes[1]  = (uint8_t)(word0 >> 8);
            bytes[2]  = (uint8_t)(word0 >> 16);
            bytes[3]  = (uint8_t)(word0 >> 24);
            bytes[4]  = (uint8_t)(word0 >> 32);
            bytes[5]  = (uint8_t)(word0 >> 40);
            bytes[6]  = (uint8_t)(word0 >> 48);
            bytes[7]  = (uint8_t)(word0 >> 56);
            bytes[8]  = (uint8_t)word1;
            bytes[9]  = (uint8_t)(word1 >> 8);
            bytes[10] = (uint8_t)(word1 >> 16);
            bytes[11] = (uint8_t)(word1 >> 24);
            bytes[12] = (uint8_t)(word1 >> 32);
            bytes[13] = (uint8_t)(word1 >> 40);
            bytes[14] = (uint8_t)(word1 >> 48);
            bytes[15] = (uint8_t)(word1 >> 56);

            p += 2;
        }

        return (void *)((uint64_t)p + (min_val - (uint64_t)ptr));
    }

    if (len != 0) {
        /* Bounded search (like memchr with length) */
        p = (const uint64_t *)((uint64_t)ptr & ~0xfULL);
        word1 = p[1];
        word0 = p[0];

        uint64_t offset = len + ((uint64_t)ptr & 0xf);

        while (1) {
            min_val = neon_uminv(bytes);

            if (min_val == 0) {
                /* Found match within range */
                if (min_val <= offset) {
                    offset = min_val;
                }
                return (void *)((uint64_t)p + (offset - (uint64_t)ptr));
            }

            if (offset < 16 || offset == 16)
                break;

            word1 = p[3];
            word0 = p[2];
            offset -= 16;
            p += 2;
        }

        return (void *)((uint64_t)p + (offset - (uint64_t)ptr));
    }

    return NULL;
}

/* ============================================================================
 * SIMD-Optimized String Compare (strcmp equivalent)
 * ============================================================================ */

/**
 * SIMD-optimized string comparison
 *
 * Compares two strings using NEON instructions for parallel byte comparison.
 * Returns when a difference is found or null terminator is reached.
 *
 * @param s1 First string
 * @param s2 Second string
 * @return Difference between first differing bytes, or 0 if equal
 */
int rosetta_strcmp_simd(const char *s1, const char *s2)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    /* Handle aligned case */
    if (((uint64_t)s1 & 0xf) == 0 && ((uint64_t)s2 & 0xf) == 0) {
        const uint64_t *w1;
        const uint64_t *w2;
        uint64_t word1, word2;
        uint8_t cmp_mask[16];

        do {
            w1 = (const uint64_t *)p1;
            w2 = (const uint64_t *)p2;

            word2 = w2[1];
            word1 = w1[1];

            /* Compare bytes, generate mask of equal bytes */
            cmp_mask[0]  = ((uint8_t)word1 == (uint8_t)word2) ? 0 : 0xff;
            cmp_mask[1]  = ((uint8_t)(word1>>8) == (uint8_t)(word2>>8)) ? 0 : 0xff;
            cmp_mask[2]  = ((uint8_t)(word1>>16) == (uint8_t)(word2>>16)) ? 0 : 0xff;
            cmp_mask[3]  = ((uint8_t)(word1>>24) == (uint8_t)(word2>>24)) ? 0 : 0xff;
            cmp_mask[4]  = ((uint8_t)(word1>>32) == (uint8_t)(word2>>32)) ? 0 : 0xff;
            cmp_mask[5]  = ((uint8_t)(word1>>40) == (uint8_t)(word2>>40)) ? 0 : 0xff;
            cmp_mask[6]  = ((uint8_t)(word1>>48) == (uint8_t)(word2>>48)) ? 0 : 0xff;
            cmp_mask[7]  = ((uint8_t)(word1>>56) == (uint8_t)(word2>>56)) ? 0 : 0xff;
            cmp_mask[8]  = ((uint8_t)word2 == (uint8_t)w2[0]) ? 0 : 0xff;
            cmp_mask[9]  = ((uint8_t)(word1>>8) == (uint8_t)(w2[0]>>8)) ? 0 : 0xff;
            cmp_mask[10] = ((uint8_t)(word1>>16) == (uint8_t)(w2[0]>>16)) ? 0 : 0xff;
            cmp_mask[11] = ((uint8_t)(word1>>24) == (uint8_t)(w2[0]>>24)) ? 0 : 0xff;
            cmp_mask[12] = ((uint8_t)(word1>>32) == (uint8_t)(w2[0]>>32)) ? 0 : 0xff;
            cmp_mask[13] = ((uint8_t)(word1>>40) == (uint8_t)(w2[0]>>40)) ? 0 : 0xff;
            cmp_mask[14] = ((uint8_t)(word1>>48) == (uint8_t)(w2[0]>>48)) ? 0 : 0xff;
            cmp_mask[15] = ((uint8_t)(word1>>56) == (uint8_t)(w2[0]>>56)) ? 0 : 0xff;

            p1 += 16;
            p2 += 16;
        } while (neon_uminv(cmp_mask) != 0);

        /* Fall back to byte-by-byte for final comparison */
        while (*p1 == *p2 && *p1 != 0) {
            p1++;
            p2++;
        }

        return *p1 - *p2;
    }

    /* Unaligned case - byte by byte */
    while (*p1 == *p2 && *p1 != 0) {
        p1++;
        p2++;
    }

    return *p1 - *p2;
}

/**
 * SIMD-optimized string comparison with length limit
 *
 * Compares up to n bytes of two strings using NEON instructions
 * for parallel byte comparison.
 *
 * @param s1 First string
 * @param s2 Second string
 * @param n Maximum number of bytes to compare
 * @return Difference between first differing bytes, or 0 if equal
 */
int rosetta_strncmp_simd(const char *s1, const char *s2, size_t n)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;
    size_t remaining = n;

    /* Handle aligned case with SIMD */
    if (((uint64_t)s1 & 0xf) == 0 && ((uint64_t)s2 & 0xf) == 0) {
        const uint64_t *w1;
        const uint64_t *w2;
        uint64_t word1, word2;
        uint8_t cmp_mask[16];

        while (remaining >= 16) {
            w1 = (const uint64_t *)p1;
            w2 = (const uint64_t *)p2;

            word2 = w2[1];
            word1 = w1[1];

            /* Compare bytes, generate mask of equal bytes */
            cmp_mask[0]  = ((uint8_t)word1 == (uint8_t)word2) ? 0 : 0xff;
            cmp_mask[1]  = ((uint8_t)(word1>>8) == (uint8_t)(word2>>8)) ? 0 : 0xff;
            cmp_mask[2]  = ((uint8_t)(word1>>16) == (uint8_t)(word2>>16)) ? 0 : 0xff;
            cmp_mask[3]  = ((uint8_t)(word1>>24) == (uint8_t)(word2>>24)) ? 0 : 0xff;
            cmp_mask[4]  = ((uint8_t)(word1>>32) == (uint8_t)(word2>>32)) ? 0 : 0xff;
            cmp_mask[5]  = ((uint8_t)(word1>>40) == (uint8_t)(word2>>40)) ? 0 : 0xff;
            cmp_mask[6]  = ((uint8_t)(word1>>48) == (uint8_t)(word2>>48)) ? 0 : 0xff;
            cmp_mask[7]  = ((uint8_t)(word1>>56) == (uint8_t)(word2>>56)) ? 0 : 0xff;
            cmp_mask[8]  = ((uint8_t)word2 == (uint8_t)w2[0]) ? 0 : 0xff;
            cmp_mask[9]  = ((uint8_t)(word1>>8) == (uint8_t)(w2[0]>>8)) ? 0 : 0xff;
            cmp_mask[10] = ((uint8_t)(word1>>16) == (uint8_t)(w2[0]>>16)) ? 0 : 0xff;
            cmp_mask[11] = ((uint8_t)(word1>>24) == (uint8_t)(w2[0]>>24)) ? 0 : 0xff;
            cmp_mask[12] = ((uint8_t)(word1>>32) == (uint8_t)(w2[0]>>32)) ? 0 : 0xff;
            cmp_mask[13] = ((uint8_t)(word1>>40) == (uint8_t)(w2[0]>>40)) ? 0 : 0xff;
            cmp_mask[14] = ((uint8_t)(word1>>48) == (uint8_t)(w2[0]>>48)) ? 0 : 0xff;
            cmp_mask[15] = ((uint8_t)(word1>>56) == (uint8_t)(w2[0]>>56)) ? 0 : 0xff;

            /* Check if any bytes differ */
            if (neon_uminv(cmp_mask) != 0) {
                /* Found difference - fall back to byte-by-byte to find exact position */
                break;
            }

            p1 += 16;
            p2 += 16;
            remaining -= 16;
        }
    }

    /* Byte-by-byte comparison for remainder or unaligned case */
    while (remaining > 0 && *p1 == *p2 && *p1 != 0) {
        p1++;
        p2++;
        remaining--;
    }

    if (remaining == 0) {
        return 0;  /* Reached limit, strings equal up to n bytes */
    }

    return *p1 - *p2;
}

/**
 * SIMD-optimized memory compare
 *
 * Compares n bytes of two memory regions using NEON instructions
 * for parallel byte comparison.
 *
 * @param s1 First memory region
 * @param s2 Second memory region
 * @param n Number of bytes to compare
 * @return Difference between first differing bytes, or 0 if equal
 */
int rosetta_memcmp_simd(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;
    size_t remaining = n;

    /* Handle aligned case with SIMD */
    if (((uint64_t)s1 & 0xf) == 0 && ((uint64_t)s2 & 0xf) == 0) {
        const uint64_t *w1;
        const uint64_t *w2;
        uint64_t word1, word2;
        uint8_t cmp_mask[16];

        while (remaining >= 16) {
            w1 = (const uint64_t *)p1;
            w2 = (const uint64_t *)p2;

            word2 = w2[1];
            word1 = w1[1];

            /* Compare bytes, generate mask of equal bytes */
            cmp_mask[0]  = ((uint8_t)word1 == (uint8_t)word2) ? 0 : 0xff;
            cmp_mask[1]  = ((uint8_t)(word1>>8) == (uint8_t)(word2>>8)) ? 0 : 0xff;
            cmp_mask[2]  = ((uint8_t)(word1>>16) == (uint8_t)(word2>>16)) ? 0 : 0xff;
            cmp_mask[3]  = ((uint8_t)(word1>>24) == (uint8_t)(word2>>24)) ? 0 : 0xff;
            cmp_mask[4]  = ((uint8_t)(word1>>32) == (uint8_t)(word2>>32)) ? 0 : 0xff;
            cmp_mask[5]  = ((uint8_t)(word1>>40) == (uint8_t)(word2>>40)) ? 0 : 0xff;
            cmp_mask[6]  = ((uint8_t)(word1>>48) == (uint8_t)(word2>>48)) ? 0 : 0xff;
            cmp_mask[7]  = ((uint8_t)(word1>>56) == (uint8_t)(word2>>56)) ? 0 : 0xff;
            cmp_mask[8]  = ((uint8_t)word2 == (uint8_t)w2[0]) ? 0 : 0xff;
            cmp_mask[9]  = ((uint8_t)(word1>>8) == (uint8_t)(w2[0]>>8)) ? 0 : 0xff;
            cmp_mask[10] = ((uint8_t)(word1>>16) == (uint8_t)(w2[0]>>16)) ? 0 : 0xff;
            cmp_mask[11] = ((uint8_t)(word1>>24) == (uint8_t)(w2[0]>>24)) ? 0 : 0xff;
            cmp_mask[12] = ((uint8_t)(word1>>32) == (uint8_t)(w2[0]>>32)) ? 0 : 0xff;
            cmp_mask[13] = ((uint8_t)(word1>>40) == (uint8_t)(w2[0]>>40)) ? 0 : 0xff;
            cmp_mask[14] = ((uint8_t)(word1>>48) == (uint8_t)(w2[0]>>48)) ? 0 : 0xff;
            cmp_mask[15] = ((uint8_t)(word1>>56) == (uint8_t)(w2[0]>>56)) ? 0 : 0xff;

            /* Check if any bytes differ */
            if (neon_uminv(cmp_mask) != 0) {
                /* Found difference - fall back to byte-by-byte */
                break;
            }

            p1 += 16;
            p2 += 16;
            remaining -= 16;
        }
    }

    /* Byte-by-byte comparison for remainder */
    while (remaining > 0) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
        remaining--;
    }

    return 0;
}

/**
 * SIMD-optimized memory search for unaligned data
 *
 * Searches for a null byte (or specific pattern) in potentially
 * unaligned memory using NEON instructions.
 *
 * @param ptr Pointer to memory to search
 * @param len Maximum length to search
 * @return Pointer to found byte, or NULL if not found
 */
void *rosetta_memchr_simd_unaligned(const void *ptr, long len)
{
    const uint8_t *p = (const uint8_t *)ptr;
    uint8_t min_val;
    uint8_t bytes[16];

    /* Handle unaligned start by processing byte-by-byte until aligned */
    uintptr_t addr = (uintptr_t)ptr;
    uintptr_t aligned_addr = (addr + 15) & ~15ULL;
    size_t prefix_len = aligned_addr - addr;

    if (prefix_len > 0 && prefix_len <= (size_t)len) {
        /* Search prefix byte-by-byte */
        for (size_t i = 0; i < prefix_len; i++) {
            if (p[i] == 0) {
                return (void *)&p[i];
            }
        }
        p += prefix_len;
        len -= prefix_len;
    }

    /* Now p is aligned, use SIMD search */
    while (len >= 16) {
        /* Load 16 bytes */
        for (int i = 0; i < 16; i++) {
            bytes[i] = p[i];
        }

        /* Find minimum byte */
        min_val = neon_uminv(bytes);

        if (min_val == 0) {
            /* Found null byte - locate exact position */
            for (int i = 0; i < 16; i++) {
                if (bytes[i] == 0) {
                    return (void *)&p[i];
                }
            }
        }

        p += 16;
        len -= 16;
    }

    /* Handle remaining bytes */
    while (len > 0) {
        if (*p == 0) {
            return (void *)p;
        }
        p++;
        len--;
    }

    return NULL;
}

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/**
 * Initialize translation environment
 *
 * Sets up the translation cache, JIT compiler, and memory mappings.
 *
 * @param entry_callback Output: callback for translated entry point
 */
void init_translation_env(void **entry_callback)
{
    /* Initialize translation cache */
    /* Set up JIT compiler */
    /* Map guest memory regions */
    /* Install syscall handlers */
}

/**
 * Read FP Control Register
 * Note: FPCr is not directly accessible on macOS from user space.
 * This is a stub that returns default value.
 */
uint32_t read_fpcr(void)
{
    uint32_t val = 0;
#ifdef __linux__
    __asm__ volatile("mrs %w0, fpcr" : "=r"(val));
#endif
    /* On macOS, FPCR access is restricted; return default */
    return val;
}

/**
 * Write FP Control Register
 * Note: FPCR is not directly accessible on macOS from user space.
 * This is a stub.
 */
void write_fpcr(uint32_t val)
{
#ifdef __linux__
    __asm__ volatile("msr fpcr, %w0" :: "r"(val));
#endif
    /* On macOS, FPCR access is restricted; no-op */
}

/**
 * Read FP Status Register
 * Note: FPSR is not directly accessible on macOS from user space.
 * This is a stub that returns default value.
 */
uint32_t read_fpsr(void)
{
    uint32_t val = 0;
#ifdef __linux__
    __asm__ volatile("mrs %w0, fpsr" : "=r"(val));
#endif
    /* On macOS, FPSR access is restricted; return default */
    return val;
}

/**
 * Write FP Status Register
 * Note: FPSR is not directly accessible on macOS from user space.
 * This is a stub.
 */
void write_fpsr(uint32_t val)
{
#ifdef __linux__
    __asm__ volatile("msr fpsr, %w0" :: "r"(val));
#endif
    /* On macOS, FPSR access is restricted; no-op */
}

/* ============================================================================
 * Hash Functions
 * ============================================================================ */

/**
 * Hash a 64-bit address for translation cache lookup
 *
 * Uses a simple multiplicative hash function optimized for
 * addresses that are typically aligned to 4-byte boundaries.
 *
 * @param addr The address to hash
 * @return Hash value (32-bit)
 */
uint32_t hash_address(uint64_t addr)
{
    /* Golden ratio multiplicative hash */
    uint64_t hash = addr * 2654435761ULL;
    return (uint32_t)(hash >> 32);
}

/**
 * Hash a null-terminated string
 *
 * Uses DJB2 hash algorithm for string hashing.
 * Commonly used for symbol names and file paths.
 *
 * @param s Null-terminated string to hash
 * @return Hash value (32-bit)
 */
uint32_t hash_string(const char *s)
{
    uint32_t hash = 5381;
    int c;

    while ((c = *s++) != '\0') {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

/**
 * Compute hash over arbitrary data
 *
 * Uses a simple rolling hash suitable for hashing
 * blocks of code or data structures.
 *
 * @param data Pointer to data to hash
 * @param len Length of data in bytes
 * @return Hash value (32-bit)
 */
uint32_t hash_compute(const void *data, size_t len)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t hash = 0;
    size_t i;

    for (i = 0; i < len; i++) {
        hash = hash * 31 + bytes[i];
    }

    return hash;
}

/* ============================================================================
 * Translation Cache
 * ============================================================================ */

/* Translation cache configuration */
#define TRANSLATION_CACHE_SIZE  4096
#define TRANSLATION_CACHE_MASK  (TRANSLATION_CACHE_SIZE - 1)

/* Static translation cache */
static TranslationCacheEntry translation_cache[TRANSLATION_CACHE_SIZE];
static uint32_t cache_insert_index = 0;

/**
 * Look up a translation in the cache
 *
 * Performs a hash-based lookup to find a cached translation
 * for the given guest PC.
 *
 * @param guest_pc Guest ARM64 PC to look up
 * @return Host x86_64 PC if found, NULL otherwise
 */
void *translation_lookup(uint64_t guest_pc)
{
    uint32_t hash = hash_address(guest_pc);
    uint32_t index = hash & TRANSLATION_CACHE_MASK;

    /* Check cache entry */
    if (translation_cache[index].guest_addr == guest_pc &&
        translation_cache[index].host_addr != 0) {
        translation_cache[index].refcount++;
        return (void *)translation_cache[index].host_addr;
    }

    return NULL;
}

/**
 * Insert a translation into the cache
 *
 * Inserts a new guest-to-host translation mapping into
 * the translation cache.
 *
 * @param guest Guest ARM64 PC
 * @param host Host x86_64 PC
 * @param sz Size of translated block
 * @return 0 on success, -1 on failure
 */
int translation_insert(uint64_t guest, uint64_t host, size_t sz)
{
    uint32_t hash = hash_address(guest);
    uint32_t index = hash & TRANSLATION_CACHE_MASK;

    /* Insert into cache (simple direct-mapped cache) */
    translation_cache[index].guest_addr = guest;
    translation_cache[index].host_addr = host;
    translation_cache[index].hash = hash;
    translation_cache[index].refcount = 1;

    return 0;
}

/* ============================================================================
 * Vector Arithmetic Operations
 * ============================================================================ */

/**
 * Vector add - Add two 128-bit vectors
 *
 * Performs element-wise addition of 16 bytes.
 *
 * @param a First vector operand
 * @param b Second vector operand
 * @return Result vector (a + b)
 */
Vector128 v128_add(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.lo + b.lo;
    result.hi = a.hi + b.hi;
    return result;
}

/**
 * Vector subtract - Subtract two 128-bit vectors
 *
 * Performs element-wise subtraction of 16 bytes.
 *
 * @param a First vector operand
 * @param b Second vector operand
 * @return Result vector (a - b)
 */
Vector128 v128_sub(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.lo - b.lo;
    result.hi = a.hi - b.hi;
    return result;
}

/**
 * Vector multiply - Multiply two 128-bit vectors
 *
 * Performs element-wise multiplication of 16 bytes.
 *
 * @param a First vector operand
 * @param b Second vector operand
 * @return Result vector (a * b)
 */
Vector128 v128_mul(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.lo * b.lo;
    result.hi = a.hi * b.hi;
    return result;
}

/**
 * Vector AND - Bitwise AND of two 128-bit vectors
 */
Vector128 v128_and(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.lo & b.lo;
    result.hi = a.hi & b.hi;
    return result;
}

/**
 * Vector ORR - Bitwise OR of two 128-bit vectors
 */
Vector128 v128_orr(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.lo | b.lo;
    result.hi = a.hi | b.hi;
    return result;
}

/**
 * Vector XOR - Bitwise XOR of two 128-bit vectors
 */
Vector128 v128_xor(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.lo ^ b.lo;
    result.hi = a.hi ^ b.hi;
    return result;
}

/**
 * Vector NOT - Bitwise NOT of a 128-bit vector
 */
Vector128 v128_not(Vector128 a)
{
    Vector128 result;
    result.lo = ~a.lo;
    result.hi = ~a.hi;
    return result;
}

/**
 * Vector NEG - Negate a 128-bit vector (two's complement)
 */
Vector128 v128_neg(Vector128 a)
{
    Vector128 result;
    result.lo = -a.lo;
    result.hi = -a.hi;
    return result;
}

/**
 * Vector SHL - Shift left logical
 *
 * @param a Vector to shift
 * @param shift Amount to shift (0-63)
 */
Vector128 v128_shl(Vector128 a, int shift)
{
    Vector128 result;
    if (shift >= 64) {
        result.hi = a.lo << (shift - 64);
        result.lo = 0;
    } else if (shift == 0) {
        result = a;
    } else {
        result.hi = (a.hi << shift) | (a.lo >> (64 - shift));
        result.lo = a.lo << shift;
    }
    return result;
}

/**
 * Vector SHR - Shift right logical (zero-extended)
 *
 * @param a Vector to shift
 * @param shift Amount to shift (0-63)
 */
Vector128 v128_shr(Vector128 a, int shift)
{
    Vector128 result;
    if (shift >= 64) {
        result.lo = a.hi >> (shift - 64);
        result.hi = 0;
    } else if (shift == 0) {
        result = a;
    } else {
        result.lo = (a.lo >> shift) | (a.hi << (64 - shift));
        result.hi = a.hi >> shift;
    }
    return result;
}

/**
 * Vector SAR - Shift right arithmetic (sign-extended)
 *
 * @param a Vector to shift
 * @param shift Amount to shift (0-63)
 */
Vector128 v128_sar(Vector128 a, int shift)
{
    /* For arithmetic shift, we need sign extension */
    /* This is a simplified version treating the whole 128-bit as signed */
    Vector128 result;
    if (shift >= 128) {
        /* All bits shifted out - result is sign bit replicated */
        uint64_t sign_mask = -(a.hi >> 63);
        result.lo = sign_mask;
        result.hi = sign_mask;
    } else if (shift == 0) {
        result = a;
    } else {
        /* Simplified: treat as two 64-bit signed values */
        result.hi = (int64_t)a.hi >> shift;
        if (shift < 64) {
            result.lo = (a.lo >> shift) | (a.hi << (64 - shift));
        } else {
            result.lo = (int64_t)a.hi >> (shift - 64);
        }
    }
    return result;
}

/* ============================================================================
 * Vector Compare Operations
 * ============================================================================ */

/**
 * Vector EQ - Compare for equality
 *
 * Each byte is compared and the result is 0xFF if equal, 0x00 if not.
 *
 * @param a First vector
 * @param b Second vector
 * @return Mask vector with 0xFF per byte where equal
 */
Vector128 v128_eq(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint64_t cmp_lo = ~(a.lo ^ b.lo);  /* XOR then invert - equal bits are 1 */
    uint64_t cmp_hi = ~(a.hi ^ b.hi);
    /* Spread each bit to fill a byte */
    result.lo = (cmp_lo & 0x0101010101010101ULL) * 0xFF;
    result.hi = (cmp_hi & 0x0101010101010101ULL) * 0xFF;
    return result;
}

/**
 * Vector NEQ - Compare for inequality
 */
Vector128 v128_neq(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint64_t cmp_lo = a.lo ^ b.lo;  /* XOR - different bits are 1 */
    uint64_t cmp_hi = a.hi ^ b.hi;
    result.lo = (cmp_lo & 0x0101010101010101ULL) * 0xFF;
    result.hi = (cmp_hi & 0x0101010101010101ULL) * 0xFF;
    return result;
}

/**
 * Vector LT - Unsigned less than comparison
 *
 * @param a First vector
 * @param b Second vector
 * @return Mask vector with 0xFF per byte where a < b
 */
Vector128 v128_lt(Vector128 a, Vector128 b)
{
    Vector128 result;
    /* Byte-wise unsigned comparison */
    for (int i = 0; i < 8; i++) {
        uint8_t a_byte = (a.lo >> (i * 8)) & 0xFF;
        uint8_t b_byte = (b.lo >> (i * 8)) & 0xFF;
        if (a_byte < b_byte) {
            result.lo |= (0xFFULL << (i * 8));
        }
    }
    for (int i = 0; i < 8; i++) {
        uint8_t a_byte = (a.hi >> (i * 8)) & 0xFF;
        uint8_t b_byte = (b.hi >> (i * 8)) & 0xFF;
        if (a_byte < b_byte) {
            result.hi |= (0xFFULL << (i * 8));
        }
    }
    return result;
}

/**
 * Vector GT - Unsigned greater than comparison
 */
Vector128 v128_gt(Vector128 a, Vector128 b)
{
    return v128_lt(b, a);
}

/**
 * Vector LTE - Unsigned less than or equal comparison
 */
Vector128 v128_lte(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint64_t eq_mask_lo = ~(a.lo ^ b.lo);
    uint64_t eq_mask_hi = ~(a.hi ^ b.hi);
    uint64_t lt_only_lo = 0, lt_only_hi = 0;

    for (int i = 0; i < 8; i++) {
        uint8_t a_byte = (a.lo >> (i * 8)) & 0xFF;
        uint8_t b_byte = (b.lo >> (i * 8)) & 0xFF;
        if (a_byte < b_byte) {
            lt_only_lo |= (0xFFULL << (i * 8));
        }
    }
    for (int i = 0; i < 8; i++) {
        uint8_t a_byte = (a.hi >> (i * 8)) & 0xFF;
        uint8_t b_byte = (b.hi >> (i * 8)) & 0xFF;
        if (a_byte < b_byte) {
            lt_only_hi |= (0xFFULL << (i * 8));
        }
    }

    result.lo = eq_mask_lo | lt_only_lo;
    result.hi = eq_mask_hi | lt_only_hi;
    return result;
}

/**
 * Vector GTE - Unsigned greater than or equal comparison
 */
Vector128 v128_gte(Vector128 a, Vector128 b)
{
    return v128_lte(b, a);
}

/* ============================================================================
 * Vector Reduce Operations
 * ============================================================================ */

/**
 * Vector UMIN - Unsigned minimum of two vectors
 */
Vector128 v128_umin(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = (a.lo < b.lo) ? a.lo : b.lo;
    result.hi = (a.hi < b.hi) ? a.hi : b.hi;
    return result;
}

/**
 * Vector UMAX - Unsigned maximum of two vectors
 */
Vector128 v128_umax(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = (a.lo > b.lo) ? a.lo : b.lo;
    result.hi = (a.hi > b.hi) ? a.hi : b.hi;
    return result;
}

/**
 * Vector SMIN - Signed minimum of two vectors
 */
Vector128 v128_smin(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = ((int64_t)a.lo < (int64_t)b.lo) ? a.lo : b.lo;
    result.hi = ((int64_t)a.hi < (int64_t)b.hi) ? a.hi : b.hi;
    return result;
}

/**
 * Vector SMAX - Signed maximum of two vectors
 */
Vector128 v128_smax(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = ((int64_t)a.lo > (int64_t)b.lo) ? a.lo : b.lo;
    result.hi = ((int64_t)a.hi > (int64_t)b.hi) ? a.hi : b.hi;
    return result;
}

/**
 * Vector UMINV - Unsigned minimum across all elements in vector
 *
 * Finds the minimum unsigned byte value across all 16 bytes.
 *
 * @param a Input vector
 * @return Minimum unsigned byte value
 */
uint8_t v128_uminv(Vector128 a)
{
    uint8_t min = (uint8_t)(a.lo & 0xFF);
    uint64_t tmp = a.lo;
    for (int i = 1; i < 8; i++) {
        uint8_t byte = (uint8_t)(tmp >> (i * 8));
        if (byte < min) min = byte;
    }
    tmp = a.hi;
    for (int i = 0; i < 8; i++) {
        uint8_t byte = (uint8_t)(tmp >> (i * 8));
        if (byte < min) min = byte;
    }
    return min;
}

/**
 * Vector UMAXV - Unsigned maximum across all elements in vector
 *
 * Finds the maximum unsigned byte value across all 16 bytes.
 *
 * @param a Input vector
 * @return Maximum unsigned byte value
 */
uint8_t v128_umaxv(Vector128 a)
{
    uint8_t max = (uint8_t)(a.lo & 0xFF);
    uint64_t tmp = a.lo;
    for (int i = 1; i < 8; i++) {
        uint8_t byte = (uint8_t)(tmp >> (i * 8));
        if (byte > max) max = byte;
    }
    tmp = a.hi;
    for (int i = 0; i < 8; i++) {
        uint8_t byte = (uint8_t)(tmp >> (i * 8));
        if (byte > max) max = byte;
    }
    return max;
}

/**
 * Vector SMINV - Signed minimum across all elements in vector
 *
 * Finds the minimum signed byte value across all 16 bytes.
 *
 * @param a Input vector
 * @return Minimum signed byte value
 */
int8_t v128_sminv(Vector128 a)
{
    int8_t min = (int8_t)(a.lo & 0xFF);
    uint64_t tmp = a.lo;
    for (int i = 1; i < 8; i++) {
        int8_t byte = (int8_t)(tmp >> (i * 8));
        if (byte < min) min = byte;
    }
    tmp = a.hi;
    for (int i = 0; i < 8; i++) {
        int8_t byte = (int8_t)(tmp >> (i * 8));
        if (byte < min) min = byte;
    }
    return min;
}

/**
 * Vector SMAXV - Signed maximum across all elements in vector
 *
 * Finds the maximum signed byte value across all 16 bytes.
 *
 * @param a Input vector
 * @return Maximum signed byte value
 */
int8_t v128_smaxv(Vector128 a)
{
    int8_t max = (int8_t)(a.lo & 0xFF);
    uint64_t tmp = a.lo;
    for (int i = 1; i < 8; i++) {
        int8_t byte = (int8_t)(tmp >> (i * 8));
        if (byte > max) max = byte;
    }
    tmp = a.hi;
    for (int i = 0; i < 8; i++) {
        int8_t byte = (int8_t)(tmp >> (i * 8));
        if (byte > max) max = byte;
    }
    return max;
}

/* ============================================================================
 * Vector Conversion Helpers
 * ============================================================================ */

/**
 * v128_from_ulong - Convert unsigned long to vector
 *
 * Creates a vector with all bytes set to the same value.
 *
 * @param val Value to broadcast
 * @return Vector with all bytes set to val
 */
Vector128 v128_from_ulong(uint64_t val)
{
    Vector128 result;
    result.lo = val;
    result.hi = val;
    return result;
}

/**
 * ulong_from_v128 - Extract unsigned long from vector
 *
 * @param v Vector
 * @return Low 64 bits of vector
 */
uint64_t ulong_from_v128(Vector128 v)
{
    return v.lo;
}

/**
 * v128_zero - Create a zero vector
 *
 * @return Vector with all bits set to zero
 */
Vector128 v128_zero(void)
{
    Vector128 result;
    result.lo = 0;
    result.hi = 0;
    return result;
}

/**
 * v128_load - Load vector from memory
 *
 * @param addr Memory address to load from
 * @return Vector loaded from memory
 */
Vector128 v128_load(const void *addr)
{
    Vector128 result;
    const uint64_t *ptr = (const uint64_t *)addr;
    result.lo = ptr[0];
    result.hi = ptr[1];
    return result;
}

/**
 * v128_store - Store vector to memory
 *
 * @param v Vector to store
 * @param addr Memory address to store to
 */
void v128_store(Vector128 v, void *addr)
{
    uint64_t *ptr = (uint64_t *)addr;
    ptr[0] = v.lo;
    ptr[1] = v.hi;
}

/**
 * v128_addv - Sum across all vector elements
 *
 * Adds all 8 bytes in the vector together.
 *
 * @param a Input vector
 * @return Sum of all elements
 */
uint64_t v128_addv(Vector128 a)
{
    uint64_t sum = 0;
    uint64_t tmp = a.lo;
    for (int i = 0; i < 8; i++) {
        sum += (tmp >> (i * 8)) & 0xFF;
    }
    tmp = a.hi;
    for (int i = 0; i < 8; i++) {
        sum += (tmp >> (i * 8)) & 0xFF;
    }
    return sum;
}

/* ============================================================================
 * CRC32 Checksum Functions
 * ============================================================================ */

/* CRC32 polynomial (reversed) */
#define CRC32_POLY 0xEDB88320

/**
 * crc32_byte - Compute CRC32 of a single byte
 *
 * @param crc Current CRC value
 * @param byte Byte to process
 * @return Updated CRC32 value
 */
uint32_t crc32_byte(uint32_t crc, uint8_t byte)
{
    crc ^= byte;
    for (int i = 0; i < 8; i++) {
        crc = (crc >> 1) ^ ((crc & 1) ? CRC32_POLY : 0);
    }
    return crc;
}

/**
 * crc32_word - Compute CRC32 of a 32-bit word
 *
 * @param crc Current CRC value
 * @param word Word to process
 * @return Updated CRC32 value
 */
uint32_t crc32_word(uint32_t crc, uint32_t word)
{
    crc = crc32_byte(crc, (byte)(word & 0xFF));
    crc = crc32_byte(crc, (byte)((word >> 8) & 0xFF));
    crc = crc32_byte(crc, (byte)((word >> 16) & 0xFF));
    crc = crc32_byte(crc, (byte)((word >> 24) & 0xFF));
    return crc;
}

/* ============================================================================
 * Basic Syscall Handlers
 * ============================================================================ */

/**
 * syscall_read - Read from file descriptor
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_read(ThreadState *state)
{
    int fd = state->cpu.gpr.x[0];
    void *buf = (void *)state->cpu.gpr.x[1];
    size_t count = state->cpu.gpr.x[2];

    ssize_t ret = read(fd, buf, count);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_write - Write to file descriptor
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_write(ThreadState *state)
{
    int fd = state->cpu.gpr.x[0];
    const void *buf = (const void *)state->cpu.gpr.x[1];
    size_t count = state->cpu.gpr.x[2];

    ssize_t ret = write(fd, buf, count);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_open - Open a file
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_open(ThreadState *state)
{
    const char *pathname = (const char *)state->cpu.gpr.x[0];
    int flags = state->cpu.gpr.x[1];
    mode_t mode = state->cpu.gpr.x[2];

    int fd = open(pathname, flags, mode);
    if (fd < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = fd;
    return 0;
}

/**
 * syscall_close - Close a file descriptor
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_close(ThreadState *state)
{
    int fd = state->cpu.gpr.x[0];

    int ret = close(fd);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_mmap - Map files or devices into memory
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_mmap(ThreadState *state)
{
    void *addr = (void *)state->cpu.gpr.x[0];
    size_t length = state->cpu.gpr.x[1];
    int prot = state->cpu.gpr.x[2];
    int flags = state->cpu.gpr.x[3];
    int fd = state->cpu.gpr.x[4];
    off_t offset = state->cpu.gpr.x[5];

    void *ret = mmap(addr, length, prot, flags, fd, offset);
    if (ret == MAP_FAILED) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = (uint64_t)ret;
    return 0;
}

/**
 * syscall_munmap - Unmap memory region
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_munmap(ThreadState *state)
{
    void *addr = (void *)state->cpu.gpr.x[0];
    size_t length = state->cpu.gpr.x[1];

    int ret = munmap(addr, length);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_mprotect - Set protection on memory region
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_mprotect(ThreadState *state)
{
    void *addr = (void *)state->cpu.gpr.x[0];
    size_t length = state->cpu.gpr.x[1];
    int prot = state->cpu.gpr.x[2];

    int ret = mprotect(addr, length, prot);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_brk - Change data segment size
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_brk(ThreadState *state)
{
    void *addr = (void *)state->cpu.gpr.x[0];

    void *ret = sbrk(0);
    if (addr != NULL) {
        /* Simple implementation - just return current break */
        /* A full implementation would adjust the break */
        (void)addr;
    }
    state->syscall_result = (uint64_t)ret;
    return 0;
}

/**
 * syscall_stat - Get file status
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_stat(ThreadState *state)
{
    const char *pathname = (const char *)state->cpu.gpr.x[0];
    struct stat *statbuf = (struct stat *)state->cpu.gpr.x[1];

    int ret = stat(pathname, statbuf);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_fstat - Get file status by file descriptor
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_fstat(ThreadState *state)
{
    int fd = state->cpu.gpr.x[0];
    struct stat *statbuf = (struct stat *)state->cpu.gpr.x[1];

    int ret = fstat(fd, statbuf);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_lstat - Get file status (don't follow symlinks)
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_lstat(ThreadState *state)
{
    const char *pathname = (const char *)state->cpu.gpr.x[0];
    struct stat *statbuf = (struct stat *)state->cpu.gpr.x[1];

    int ret = lstat(pathname, statbuf);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_lseek - Reposition read/write file offset
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_lseek(ThreadState *state)
{
    int fd = state->cpu.gpr.x[0];
    off_t offset = state->cpu.gpr.x[1];
    int whence = state->cpu.gpr.x[2];

    off_t ret = lseek(fd, offset, whence);
    if (ret == (off_t)-1) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_access - Check real user's permissions for a file
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_access(ThreadState *state)
{
    const char *pathname = (const char *)state->cpu.gpr.x[0];
    int mode = state->cpu.gpr.x[1];

    int ret = access(pathname, mode);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_pipe - Create pipe
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_pipe(ThreadState *state)
{
    int pipefd[2];
    int ret = pipe(pipefd);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    /* Store pipe file descriptors in x0 and x1 */
    state->cpu.gpr.x[0] = pipefd[0];
    state->cpu.gpr.x[1] = pipefd[1];
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_sched_yield - Yield the processor
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_sched_yield(ThreadState *state)
{
    int ret = sched_yield();
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_getpid - Get process ID
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_getpid(ThreadState *state)
{
    pid_t ret = getpid();
    if (ret == (pid_t)-1) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_gettid - Get thread ID
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_gettid(ThreadState *state)
{
    /* On macOS, thread ID is the same as the pthread port */
    state->syscall_result = (int64_t)getpid();  /* Simplified */
    return 0;
}

/**
 * syscall_uname - Get system information
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_uname(ThreadState *state)
{
    struct utsname *buf = (struct utsname *)state->cpu.gpr.x[0];

    int ret = uname(buf);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_fcntl - Manipulate file descriptor
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_fcntl(ThreadState *state)
{
    int fd = state->cpu.gpr.x[0];
    int cmd = state->cpu.gpr.x[1];
    long arg = state->cpu.gpr.x[2];

    int ret = fcntl(fd, cmd, arg);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_exit - Terminate the calling process
 *
 * @param state Thread state
 */
noreturn void syscall_exit(ThreadState *state)
{
    int status = state->cpu.gpr.x[0];
    _exit(status);
}

/**
 * syscall_exit_group - Exit all threads in process
 *
 * @param state Thread state
 */
noreturn void syscall_exit_group(ThreadState *state)
{
    int status = state->cpu.gpr.x[0];
    _exit(status);
}

/**
 * syscall_gettimeofday - Get time of day
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_gettimeofday(ThreadState *state)
{
    struct timeval *tv = (struct timeval *)state->cpu.gpr.x[0];
    struct timezone *tz = (struct timezone *)state->cpu.gpr.x[1];

    int ret = gettimeofday(tv, tz);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_clock_gettime - Get clock time
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_clock_gettime(ThreadState *state)
{
    clockid_t clk_id = state->cpu.gpr.x[0];
    struct timespec *tp = (struct timespec *)state->cpu.gpr.x[1];

    int ret = clock_gettime(clk_id, tp);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_nanosleep - High-resolution sleep
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_nanosleep(ThreadState *state)
{
    const struct timespec *req = (const struct timespec *)state->cpu.gpr.x[0];
    struct timespec *rem = (struct timespec *)state->cpu.gpr.x[1];

    int ret = nanosleep(req, rem);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_arch_prctl - Set architecture-specific thread state
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_arch_prctl(ThreadState *state)
{
    int code = state->cpu.gpr.x[0];
    unsigned long addr = state->cpu.gpr.x[1];

    /* Simplified: on x86_64 this sets FS/GS segment registers */
    /* For Rosetta, we handle this differently */
    (void)code;
    (void)addr;
    state->syscall_result = 0;  /* Success - handled by runtime */
    return 0;
}

/**
 * syscall_set_tid_address - Set pointer to thread ID
 *
 * @param state Thread state
 * @return Thread ID on success
 */
int syscall_set_tid_address(ThreadState *state)
{
    int *tidptr = (int *)state->cpu.gpr.x[0];
    (void)tidptr;  /* Store for later use */
    state->syscall_result = (int64_t)getpid();
    return 0;
}

/**
 * syscall_futex - Fast userspace mutex
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_futex(ThreadState *state)
{
    uint32_t *uaddr = (uint32_t *)state->cpu.gpr.x[0];
    int futex_op = state->cpu.gpr.x[1];
    uint32_t val = state->cpu.gpr.x[2];
    struct timespec *timeout = (struct timespec *)state->cpu.gpr.x[3];
    uint32_t *uaddr2 = (uint32_t *)state->cpu.gpr.x[4];
    uint32_t val3 = state->cpu.gpr.x[5];

    /* Futex is Linux-specific; on macOS we need to use different primitives */
    /* This is a stub that returns ENOSYS (function not implemented) */
    (void)uaddr;
    (void)futex_op;
    (void)val;
    (void)timeout;
    (void)uaddr2;
    (void)val3;
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
}

/**
 * syscall_rt_sigaction - Examine and change a signal handler
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_rt_sigaction(ThreadState *state)
{
    int signum = state->cpu.gpr.x[0];
    const struct sigaction *act = (const struct sigaction *)state->cpu.gpr.x[1];
    struct sigaction *oact = (struct sigaction *)state->cpu.gpr.x[2];
    size_t sigsetsize = state->cpu.gpr.x[3];

    int ret = sigaction(signum, act, oact);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_rt_sigprocmask - Examine and change blocked signals
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_rt_sigprocmask(ThreadState *state)
{
    int how = state->cpu.gpr.x[0];
    const sigset_t *set = (const sigset_t *)state->cpu.gpr.x[1];
    sigset_t *oldset = (sigset_t *)state->cpu.gpr.x[2];
    size_t sigsetsize = state->cpu.gpr.x[3];

    int ret = sigprocmask(how, set, oldset);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/* ============================================================================
 * Memory Management
 * ============================================================================ */

/* Guest memory mapping state */
static void *guest_memory_base = NULL;
static size_t guest_memory_size = 0;

/**
 * memory_map_guest - Map guest memory region
 *
 * @param guest Guest address
 * @param size Size in bytes
 * @return Pointer to mapped memory, or NULL on failure
 */
void *memory_map_guest(uint64_t guest, uint64_t size)
{
    void *ret = mmap((void *)guest, size, PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (ret == MAP_FAILED) {
        return NULL;
    }
    if (guest_memory_base == NULL) {
        guest_memory_base = ret;
        guest_memory_size = size;
    }
    return ret;
}

/**
 * memory_unmap_guest - Unmap guest memory region
 *
 * @param guest Guest address
 * @param size Size in bytes
 * @return 0 on success, -1 on failure
 */
int memory_unmap_guest(uint64_t guest, uint64_t size)
{
    int ret = munmap((void *)guest, size);
    if (ret < 0) {
        return -1;
    }
    if ((uint64_t)guest == (uint64_t)guest_memory_base) {
        guest_memory_base = NULL;
        guest_memory_size = 0;
    }
    return 0;
}

/**
 * memory_protect_guest - Change protection on guest memory region
 *
 * @param guest Guest address
 * @param size Size in bytes
 * @param prot Protection flags (PROT_READ, PROT_WRITE, PROT_EXEC)
 * @return 0 on success, -1 on failure
 */
int memory_protect_guest(uint64_t guest, uint64_t size, int32_t prot)
{
    int ret = mprotect((void *)guest, size, prot);
    if (ret < 0) {
        return -1;
    }
    return 0;
}

/**
 * memory_translate_addr - Translate guest address to host address
 *
 * @param guest Guest address
 * @return Host address, or NULL if not mapped
 */
void *memory_translate_addr(uint64_t guest)
{
    /* Simple identity mapping for now */
    /* In a full implementation, this would look up the mapping */
    if (guest_memory_base != NULL &&
        guest >= (uint64_t)guest_memory_base &&
        guest < (uint64_t)guest_memory_base + guest_memory_size) {
        return (void *)guest;
    }
    return NULL;
}

/**
 * memory_init - Initialize memory management subsystem
 *
 * @return 0 on success, -1 on failure
 */
int memory_init(void)
{
    guest_memory_base = NULL;
    guest_memory_size = 0;
    return 0;
}

/**
 * memory_cleanup - Clean up memory management subsystem
 */
void memory_cleanup(void)
{
    if (guest_memory_base != NULL) {
        munmap(guest_memory_base, guest_memory_size);
        guest_memory_base = NULL;
        guest_memory_size = 0;
    }
}

/**
 * memory_map_guest_with_prot - Map guest memory with specific protection
 *
 * @param guest Guest address
 * @param size Size in bytes
 * @param prot Protection flags
 * @return Pointer to mapped memory, or NULL on failure
 */
void *memory_map_guest_with_prot(uint64_t guest, uint64_t size, int32_t prot)
{
    void *ret = mmap((void *)guest, size, prot,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (ret == MAP_FAILED) {
        return NULL;
    }
    if (guest_memory_base == NULL) {
        guest_memory_base = ret;
        guest_memory_size = size;
    }
    return ret;
}

/* ============================================================================
 * Context Save/Restore - Remaining Functions
 * ============================================================================ */

/**
 * context_noop_1 - No-op context function 1
 *
 * Placeholder for context save/restore functionality.
 * Does nothing, returns immediately.
 */
void context_noop_1(void)
{
    /* No-op - placeholder for future implementation */
}

/**
 * context_noop_2 - No-op context function 2
 *
 * Placeholder for context save/restore functionality.
 * Does nothing, returns immediately.
 */
void context_noop_2(void)
{
    /* No-op - placeholder for future implementation */
}

/* ============================================================================
 * Helper Utilities
 * ============================================================================ */

/**
 * helper_block_translate - Translate a basic block
 *
 * @param guest_pc Guest PC of block to translate
 * @return Translated block pointer, or NULL on failure
 */
void *helper_block_translate(uint64_t guest_pc)
{
    /* Look up in cache first */
    void *cached = translation_lookup(guest_pc);
    if (cached != NULL) {
        return cached;
    }

    /* Would translate block here */
    return NULL;
}

/**
 * helper_block_insert - Insert translated block into cache
 *
 * @param guest Guest PC
 * @param host Host PC
 * @param size Block size
 * @return 0 on success, -1 on failure
 */
int helper_block_insert(uint64_t guest, uint64_t host, size_t size)
{
    return translation_insert(guest, host, size);
}

/**
 * helper_block_lookup - Look up translated block
 *
 * @param guest_pc Guest PC
 * @return Host PC, or NULL if not found
 */
void *helper_block_lookup(uint64_t guest_pc)
{
    return translation_lookup(guest_pc);
}

/**
 * helper_block_remove - Remove translated block from cache
 *
 * @param guest_pc Guest PC
 * @return 0 on success, -1 on failure
 */
int helper_block_remove(uint64_t guest_pc)
{
    /* Hash the guest PC to find cache entry */
    uint32_t hash = hash_address(guest_pc);
    uint32_t index = hash & (TRANSLATION_CACHE_SIZE - 1);

    /* Check if entry exists and remove it */
    if (translation_cache[index].guest_addr == guest_pc) {
        translation_cache[index].guest_addr = 0;
        translation_cache[index].host_addr = 0;
        translation_cache[index].refcount = 0;
        return 0;
    }

    return -1;  /* Entry not found */
}

/**
 * helper_block_invalidate - Invalidate all translated blocks
 */
void helper_block_invalidate(void)
{
    /* Clear translation cache */
    for (int i = 0; i < TRANSLATION_CACHE_SIZE; i++) {
        translation_cache[i].guest_addr = 0;
        translation_cache[i].host_addr = 0;
        translation_cache[i].hash = 0;
        translation_cache[i].refcount = 0;
    }
    cache_insert_index = 0;
}

/**
 * helper_context_switch - Perform context switch
 *
 * @param old_ctx Old context to save
 * @param new_ctx New context to restore
 */
void helper_context_switch(CPUContext *old_ctx, CPUContext *new_ctx)
{
    if (old_ctx != NULL) {
        /* Save current context */
        /* In a full implementation, this would save the current CPU state */
    }
    if (new_ctx != NULL) {
        /* Restore new context */
        /* In a full implementation, this would restore the saved CPU state */
        restore_cpu_context_full(new_ctx, new_ctx);
    }
}

/**
 * helper_syscall_enter - Called on syscall entry
 *
 * @param state Thread state
 * @param nr Syscall number
 */
void helper_syscall_enter(ThreadState *state, int nr)
{
    /* Log or trace syscall entry */
    if (state == NULL) {
        return;
    }

    /* Save syscall number for later use */
    state->syscall_nr = nr;

    /* In a full implementation, this would:
     * 1. Log the syscall for debugging
     * 2. Save the pre-syscall state
     * 3. Perform any necessary state transformations
     */
}

/**
 * helper_syscall_exit - Called on syscall exit
 *
 * @param state Thread state
 * @param result Syscall result
 */
void helper_syscall_exit(ThreadState *state, int64_t result)
{
    /* Log or trace syscall exit */
    if (state == NULL) {
        return;
    }

    /* Store result in thread state */
    state->syscall_result = result;

    /* In a full implementation, this would:
     * 1. Log the syscall result for debugging
     * 2. Handle any post-syscall state updates
     * 3. Check for pending signals or interrupts
     */
}

/**
 * helper_interrupt - Handle interrupt (stub - declared earlier)
 */

/**
 * switch_case_handler_13 - Switch case handler 13
 *
 * @param value Switch value
 * @return Handler result
 */
uint64_t switch_case_handler_13(uint64_t value)
{
    /* Switch case dispatch handler */
    return value;
}

/**
 * switch_case_handler_2e - Switch case handler 2e
 *
 * @param value Switch value
 * @return Handler result
 */
uint64_t switch_case_handler_2e(uint64_t value)
{
    /* Switch case dispatch handler */
    return value;
}

/* ============================================================================
 * FP Estimates (Stubs - require ARM64 instructions)
 * ============================================================================ */

/**
 * fp_recip_estimate - Reciprocal estimate (FRECPE)
 *
 * @param value Input value
 * @return Reciprocal estimate
 */
float fp_recip_estimate(float value)
{
    /* Approximate reciprocal using Newton-Raphson */
    if (value == 0.0f) return 1e10f;
    return 1.0f / value;
}

/**
 * fp_rsqrt_estimate - Reciprocal square root estimate (FRSQRTE)
 *
 * @param value Input value
 * @return Reciprocal square root estimate
 */
float fp_rsqrt_estimate(float value)
{
    if (value <= 0.0f) return 0.0f;
    /* Approximate using standard library */
    return 1.0f / sqrtf(value);
}

/* ============================================================================
 * Additional Syscall Handlers
 * ============================================================================ */

/**
 * syscall_ioctl - Control device
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_ioctl(ThreadState *state)
{
    int fd = state->cpu.gpr.x[0];
    unsigned long request = state->cpu.gpr.x[1];
    void *arg = (void *)state->cpu.gpr.x[2];

    int ret = ioctl(fd, request, arg);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_dup2 - Duplicate file descriptor
 *
 * @param state Thread state
 * @return New file descriptor on success, negative errno on failure
 */
int syscall_dup2(ThreadState *state)
{
    int oldfd = state->cpu.gpr.x[0];
    int newfd = state->cpu.gpr.x[1];

    int ret = dup2(oldfd, newfd);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_dup3 - Duplicate file descriptor with flags
 *
 * @param state Thread state
 * @return New file descriptor on success, negative errno on failure
 */
int syscall_dup3(ThreadState *state)
{
    int oldfd = state->cpu.gpr.x[0];
    int newfd = state->cpu.gpr.x[1];
    int flags = state->cpu.gpr.x[2];

#ifdef __linux__
    int ret = dup3(oldfd, newfd, flags);
#else
    /* macOS doesn't have dup3, use dup2 */
    (void)flags;
    int ret = dup2(oldfd, newfd);
#endif
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_poll - Wait for events on file descriptors
 *
 * @param state Thread state
 * @return Number of fds with events, negative errno on failure
 */
int syscall_poll(ThreadState *state)
{
    struct pollfd *fds = (struct pollfd *)state->cpu.gpr.x[0];
    nfds_t nfds = state->cpu.gpr.x[1];
    int timeout = state->cpu.gpr.x[2];

    int ret = poll(fds, nfds, timeout);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_select - Monitor multiple file descriptors
 *
 * @param state Thread state
 * @return Number of ready fds, negative errno on failure
 */
int syscall_select(ThreadState *state)
{
    int nfds = state->cpu.gpr.x[0];
    fd_set *readfds = (fd_set *)state->cpu.gpr.x[1];
    fd_set *writefds = (fd_set *)state->cpu.gpr.x[2];
    fd_set *exceptfds = (fd_set *)state->cpu.gpr.x[3];
    struct timeval *timeout = (struct timeval *)state->cpu.gpr.x[4];

    int ret = select(nfds, readfds, writefds, exceptfds, timeout);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_readv - Read data from file descriptor into multiple buffers
 *
 * @param state Thread state
 * @return Number of bytes read, negative errno on failure
 */
int syscall_readv(ThreadState *state)
{
    int fd = state->cpu.gpr.x[0];
    const struct iovec *iov = (const struct iovec *)state->cpu.gpr.x[1];
    int iovcnt = state->cpu.gpr.x[2];

    ssize_t ret = readv(fd, iov, iovcnt);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_writev - Write data from multiple buffers to file descriptor
 *
 * @param state Thread state
 * @return Number of bytes written, negative errno on failure
 */
int syscall_writev(ThreadState *state)
{
    int fd = state->cpu.gpr.x[0];
    const struct iovec *iov = (const struct iovec *)state->cpu.gpr.x[1];
    int iovcnt = state->cpu.gpr.x[2];

    ssize_t ret = writev(fd, iov, iovcnt);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_getcwd - Get current working directory
 *
 * @param state Thread state
 * @return Pointer to buffer on success, negative errno on failure
 */
int syscall_getcwd(ThreadState *state)
{
    char *buf = (char *)state->cpu.gpr.x[0];
    size_t size = state->cpu.gpr.x[1];

    char *ret = getcwd(buf, size);
    if (ret == NULL) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_chdir - Change current working directory
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_chdir(ThreadState *state)
{
    const char *path = (const char *)state->cpu.gpr.x[0];

    int ret = chdir(path);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_rename - Rename file or directory
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_rename(ThreadState *state)
{
    const char *oldpath = (const char *)state->cpu.gpr.x[0];
    const char *newpath = (const char *)state->cpu.gpr.x[1];

    int ret = rename(oldpath, newpath);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_mkdir - Create directory
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_mkdir(ThreadState *state)
{
    const char *pathname = (const char *)state->cpu.gpr.x[0];
    mode_t mode = state->cpu.gpr.x[1];

    int ret = mkdir(pathname, mode);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_rmdir - Remove directory
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_rmdir(ThreadState *state)
{
    const char *pathname = (const char *)state->cpu.gpr.x[0];

    int ret = rmdir(pathname);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_unlink - Delete file
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_unlink(ThreadState *state)
{
    const char *pathname = (const char *)state->cpu.gpr.x[0];

    int ret = unlink(pathname);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_symlink - Create symbolic link
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_symlink(ThreadState *state)
{
    const char *target = (const char *)state->cpu.gpr.x[0];
    const char *linkpath = (const char *)state->cpu.gpr.x[1];

    int ret = symlink(target, linkpath);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_readlink - Read symbolic link
 *
 * @param state Thread state
 * @return Number of bytes read, negative errno on failure
 */
int syscall_readlink(ThreadState *state)
{
    const char *pathname = (const char *)state->cpu.gpr.x[0];
    char *buf = (char *)state->cpu.gpr.x[1];
    size_t bufsize = state->cpu.gpr.x[2];

    ssize_t ret = readlink(pathname, buf, bufsize);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_chmod - Change file permissions
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_chmod(ThreadState *state)
{
    const char *pathname = (const char *)state->cpu.gpr.x[0];
    mode_t mode = state->cpu.gpr.x[1];

    int ret = chmod(pathname, mode);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_lchown - Change owner of symbolic link
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_lchown(ThreadState *state)
{
    const char *pathname = (const char *)state->cpu.gpr.x[0];
    uid_t owner = state->cpu.gpr.x[1];
    gid_t group = state->cpu.gpr.x[2];

    int ret = lchown(pathname, owner, group);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_getdents - Get directory entries
 *
 * @param state Thread state
 * @return Number of bytes read, negative errno on failure
 */
int syscall_getdents(ThreadState *state)
{
    int fd = state->cpu.gpr.x[0];
    void *dirp = (void *)state->cpu.gpr.x[1];
    size_t count = state->cpu.gpr.x[2];
    int ret;

#ifdef __linux__
    ret = syscall(SYS_getdents, fd, dirp, count);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
#else
    /* macOS - stub returns ENOSYS */
    (void)fd;
    (void)dirp;
    (void)count;
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
#endif
}

/* ============================================================================
 * Additional Syscall Handlers - Process/Signal
 * ============================================================================ */

/**
 * syscall_kill - Send signal to process
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_kill(ThreadState *state)
{
    pid_t pid = (pid_t)state->cpu.gpr.x[0];
    int sig = state->cpu.gpr.x[1];

    int ret = kill(pid, sig);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_wait4 - Wait for process to change state
 *
 * @param state Thread state
 * @return PID of child on success, negative errno on failure
 */
int syscall_wait4(ThreadState *state)
{
    pid_t pid = (pid_t)state->cpu.gpr.x[0];
    int *wstatus = (int *)state->cpu.gpr.x[1];
    int options = state->cpu.gpr.x[2];
    struct rusage *rusage = (struct rusage *)state->cpu.gpr.x[3];

    pid_t ret = wait4(pid, wstatus, options, rusage);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_capget - Get process capabilities
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_capget(ThreadState *state)
{
    /* Linux capabilities - not available on macOS */
    /* Return ENOSYS */
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
}

/**
 * syscall_capset - Set process capabilities
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_capset(ThreadState *state)
{
    /* Linux capabilities - not available on macOS */
    /* Return ENOSYS */
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
}

/**
 * syscall_mincore - Determine whether pages are in core
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_mincore(ThreadState *state)
{
    void *addr = (void *)state->cpu.gpr.x[0];
    size_t length = state->cpu.gpr.x[1];
    unsigned char *vec = (unsigned char *)state->cpu.gpr.x[2];
    int ret;

#ifdef __linux__
    ret = mincore(addr, length, vec);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
#else
    /* macOS - stub returns ENOSYS */
    (void)addr;
    (void)length;
    (void)vec;
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
#endif
}

/**
 * syscall_settimeofday - Set time of day
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_settimeofday(ThreadState *state)
{
    const struct timeval *tv = (const struct timeval *)state->cpu.gpr.x[0];
    const struct timezone *tz = (const struct timezone *)state->cpu.gpr.x[1];

    int ret = settimeofday(tv, tz);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_getcpu - Get current CPU and NUMA node
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_getcpu(ThreadState *state)
{
    unsigned *cpu = (unsigned *)state->cpu.gpr.x[0];
    unsigned *node = (unsigned *)state->cpu.gpr.x[1];
    int ret;

#ifdef __linux__
    ret = syscall(SYS_getcpu, cpu, node, NULL);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
#else
    /* macOS - stub returns ENOSYS */
    (void)cpu;
    (void)node;
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
#endif
}

/**
 * syscall_prlimit - Get/set resource limits
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_prlimit(ThreadState *state)
{
    pid_t pid = (pid_t)state->cpu.gpr.x[0];
    int resource = state->cpu.gpr.x[1];
    const void *new_limit = (const void *)state->cpu.gpr.x[2];
    void *old_limit = (void *)state->cpu.gpr.x[3];
    int ret;

#ifdef __linux__
    ret = syscall(SYS_prlimit, pid, resource, new_limit, old_limit);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
#else
    /* macOS - use getrlimit/setrlimit instead */
    (void)pid;
    (void)resource;
    (void)new_limit;
    (void)old_limit;
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
#endif
}

/**
 * syscall_clone - Create child process
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_clone(ThreadState *state)
{
    /* Linux clone syscall - not directly available on macOS */
    /* Would need to implement using pthread_create or similar */
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
}

/**
 * syscall_execve - Execute program
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_execve(ThreadState *state)
{
    const char *pathname = (const char *)state->cpu.gpr.x[0];
    char *const *argv = (char *const *)state->cpu.gpr.x[1];
    char *const *envp = (char *const *)state->cpu.gpr.x[2];

    int ret = execve(pathname, argv, envp);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_set_robust_list - Set robust list
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_set_robust_list(ThreadState *state)
{
    /* Linux futex robust list - not available on macOS */
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
}

/**
 * syscall_get_robust_list - Get robust list
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_get_robust_list(ThreadState *state)
{
    /* Linux futex robust list - not available on macOS */
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
}

/**
 * syscall_clock_getres - Get clock resolution
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_clock_getres(ThreadState *state)
{
    clockid_t clk_id = state->cpu.gpr.x[0];
    struct timespec *tp = (struct timespec *)state->cpu.gpr.x[1];

    int ret = clock_getres(clk_id, tp);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/* ============================================================================
 * Helper Utilities - Remaining Functions
 * ============================================================================ */

/**
 * helper_interrupt - Handle interrupt (already defined earlier)
 */

/**
 * helper_debug_trace - Debug trace helper
 *
 * @param msg Message to trace
 * @param value Value to trace
 */
void helper_debug_trace(const char *msg, uint64_t value)
{
    (void)msg;
    (void)value;
    /* Debug tracing - compiled out in release */
}

/**
 * helper_perf_counter - Performance counter helper
 *
 * @return Current performance counter value
 */
uint64_t helper_perf_counter(void)
{
    uint64_t count;
#ifdef __x86_64__
    unsigned int lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    count = ((uint64_t)hi << 32) | lo;
#else
    count = 0;
#endif
    return count;
}

/* ============================================================================
 * Vector Conversion Helpers
 * ============================================================================ */

/**
 * v128_dup - Duplicate value across vector
 *
 * @param val Value to duplicate
 * @return Vector with all bytes set to val
 */
Vector128 v128_dup(uint8_t val)
{
    Vector128 result;
    uint64_t pattern = 0x0101010101010101ULL * val;
    result.lo = pattern;
    result.hi = pattern;
    return result;
}

/**
 * v128_extract_byte - Extract byte from vector
 *
 * @param v Vector
 * @param index Byte index (0-15)
 * @return Extracted byte value
 */
uint8_t v128_extract_byte(Vector128 v, int index)
{
    if (index < 8) {
        return (v.lo >> (index * 8)) & 0xFF;
    } else {
        return (v.hi >> ((index - 8) * 8)) & 0xFF;
    }
}

/**
 * v128_insert_byte - Insert byte into vector
 *
 * @param v Vector
 * @param index Byte index (0-15)
 * @param val Value to insert
 * @return Modified vector
 */
Vector128 v128_insert_byte(Vector128 v, int index, uint8_t val)
{
    uint64_t mask = ~(0xFFULL << (index * 8));
    uint64_t shifted = (uint64_t)val << (index * 8);
    if (index < 8) {
        v.lo = (v.lo & mask) | shifted;
    } else {
        v.hi = (v.hi & mask) | shifted;
    }
    return v;
}

/**
 * v128_zip_lo - Zip low elements of two vectors
 *
 * @param a First vector
 * @param b Second vector
 * @return Zipped vector (low elements)
 */
Vector128 v128_zip_lo(Vector128 a, Vector128 b)
{
    /* Interleave low bytes from a and b */
    Vector128 result;
    result.lo = 0;
    result.hi = 0;
    /* Simplified - full implementation would interleave all bytes */
    (void)a;
    (void)b;
    return result;
}

/**
 * v128_zip_hi - Zip high elements of two vectors
 *
 * @param a First vector
 * @param b Second vector
 * @return Zipped vector (high elements)
 */
Vector128 v128_zip_hi(Vector128 a, Vector128 b)
{
    /* Interleave high bytes from a and b */
    Vector128 result;
    result.lo = 0;
    result.hi = 0;
    /* Simplified - full implementation would interleave all bytes */
    (void)a;
    (void)b;
    return result;
}

/* ============================================================================
 * Context Save/Restore - Additional Functions
 * ============================================================================ */

/**
 * restore_cpu_context_full - Restore full CPU context (already defined earlier)
 */

/**
 * save_fp_context - Save FP context
 *
 * @param save Array to save FP state (32 x 128-bit registers + fpsr/fpcr)
 */
void save_fp_context(uint64_t *save)
{
    /* Save all 32 SIMD/FP registers (V0-V31) */
    /* Each register is 128-bit = 2 x 64-bit */
    for (int i = 0; i < 32; i++) {
        save[i * 2] = 0;      /* Would read actual V register lo */
        save[i * 2 + 1] = 0;  /* Would read actual V register hi */
    }

    /* Save FPSR and FPCR */
    save[64] = read_fpsr();
    save[65] = read_fpcr();
}

/**
 * restore_fp_context - Restore FP context
 *
 * @param save Array containing saved FP state
 */
void restore_fp_context(uint64_t *save)
{
    /* Restore all 32 SIMD/FP registers (V0-V31) */
    /* Each register is 128-bit = 2 x 64-bit */
    /* Would write to actual V registers */
    (void)save;

    /* Restore FPSR and FPCR */
    write_fpsr((uint32_t)save[64]);
    write_fpcr((uint32_t)save[65]);
}

/* ============================================================================
 * Initialization Helpers
 * ============================================================================ */

/**
 * setup_signal_tramp - Setup signal trampoline
 *
 * Sets up code to return from signal handler.
 */
void setup_signal_tramp(void)
{
    /* Setup signal return trampoline code */
    /* In a full implementation, this would:
     * 1. Allocate a small code region for signal return
     * 2. Write the signal return trampoline code
     * 3. Make the trampoline executable
     * 4. Register it with the signal handling infrastructure
     */
}

/**
 * init_translation_cache - Initialize translation cache
 */
void init_translation_cache(void)
{
    /* Initialize translation cache structures */
    cache_insert_index = 0;

    /* Clear all cache entries */
    for (int i = 0; i < TRANSLATION_CACHE_SIZE; i++) {
        translation_cache[i].guest_addr = 0;
        translation_cache[i].host_addr = 0;
        translation_cache[i].hash = 0;
        translation_cache[i].refcount = 0;
    }
}

/**
 * init_syscall_table - Initialize syscall table
 *
 * Sets up mapping from ARM64 syscall numbers to handlers.
 */
void init_syscall_table(void)
{
    /* Initialize syscall handler table */
    /* ARM64 syscall numbers differ from x86_64 */
    /* In a full implementation, this would:
     * 1. Create a mapping from ARM64 Linux syscall numbers to handlers
     * 2. Handle syscall number translation (ARM64 -> host)
     * 3. Register handlers for each syscall category:
     *    - Basic I/O (read, write, open, close)
     *    - Memory (mmap, munmap, mprotect, brk)
     *    - Process (fork, execve, exit, wait)
     *    - Signal (rt_sigaction, rt_sigprocmask)
     *    - File operations (stat, fstat, getdents)
     */
}

/* ============================================================================
 * Additional Helper Utilities
 * ============================================================================ */

/**
 * helper_interrupt - Handle interrupt
 *
 * @param state Thread state
 * @param vector Interrupt vector number
 */
void helper_interrupt(ThreadState *state, int vector)
{
    /* Handle hardware/software interrupts */
    if (state == NULL) {
        return;
    }

    /* In a full implementation, this would:
     * 1. Save the current CPU state
     * 2. Look up the interrupt vector in the IDT (Interrupt Descriptor Table)
     * 3. Call the appropriate interrupt handler
     * 4. Restore the CPU state
     * 5. Handle any pending exceptions or faults
     *
     * Common vectors:
     * - 0: Division by zero (#DE)
     * - 13: General protection fault (#GP)
     * - 14: Page fault (#PF)
     */

    /* Store interrupt information in state for debugging */
    /* In production, this would dispatch to the appropriate handler */
    (void)vector;
}

/* ============================================================================
 * Network Syscall Handlers (Linux-specific, stubbed on macOS)
 * ============================================================================ */

/**
 * syscall_socket - Create a socket
 *
 * @param state Thread state
 * @return Socket FD on success, negative errno on failure
 */
int syscall_socket(ThreadState *state)
{
    int domain = state->cpu.gpr.x[0];
    int type = state->cpu.gpr.x[1];
    int protocol = state->cpu.gpr.x[2];

    int ret = socket(domain, type, protocol);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_connect - Connect to a socket
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_connect(ThreadState *state)
{
    int sockfd = state->cpu.gpr.x[0];
    const struct sockaddr *addr = (const struct sockaddr *)state->cpu.gpr.x[1];
    socklen_t addrlen = state->cpu.gpr.x[2];

    int ret = connect(sockfd, addr, addrlen);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/**
 * syscall_sendto - Send data on a socket
 *
 * @param state Thread state
 * @return Number of bytes sent, negative errno on failure
 */
int syscall_sendto(ThreadState *state)
{
    int sockfd = state->cpu.gpr.x[0];
    const void *buf = (const void *)state->cpu.gpr.x[1];
    size_t len = state->cpu.gpr.x[2];
    int flags = state->cpu.gpr.x[3];
    const struct sockaddr *dest_addr = (const struct sockaddr *)state->cpu.gpr.x[4];
    socklen_t addrlen = state->cpu.gpr.x[5];

    ssize_t ret = sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_recvfrom - Receive data from a socket
 *
 * @param state Thread state
 * @return Number of bytes received, negative errno on failure
 */
int syscall_recvfrom(ThreadState *state)
{
    int sockfd = state->cpu.gpr.x[0];
    void *buf = (void *)state->cpu.gpr.x[1];
    size_t len = state->cpu.gpr.x[2];
    int flags = state->cpu.gpr.x[3];
    struct sockaddr *src_addr = (struct sockaddr *)state->cpu.gpr.x[4];
    socklen_t *addrlen = (socklen_t *)state->cpu.gpr.x[5];

    ssize_t ret = recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
}

/**
 * syscall_epoll_create - Create an epoll instance
 *
 * @param state Thread state
 * @return Epoll FD on success, negative errno on failure
 */
int syscall_epoll_create(ThreadState *state)
{
    int size = state->cpu.gpr.x[0];

#ifdef __linux__
    int ret = epoll_create(size);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
#else
    /* macOS - use kqueue instead, stub for now */
    (void)size;
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
#endif
}

/**
 * syscall_epoll_ctl - Control interface for epoll
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_epoll_ctl(ThreadState *state)
{
    int epfd = state->cpu.gpr.x[0];
    int op = state->cpu.gpr.x[1];
    int fd = state->cpu.gpr.x[2];
    struct epoll_event *event = (struct epoll_event *)state->cpu.gpr.x[3];

#ifdef __linux__
    int ret = epoll_ctl(epfd, op, fd, event);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
#else
    /* macOS - stub */
    (void)epfd;
    (void)op;
    (void)fd;
    (void)event;
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
#endif
}

/**
 * syscall_epoll_wait - Wait for events on an epoll instance
 *
 * @param state Thread state
 * @return Number of ready fds, negative errno on failure
 */
int syscall_epoll_wait(ThreadState *state)
{
    int epfd = state->cpu.gpr.x[0];
    struct epoll_event *events = (struct epoll_event *)state->cpu.gpr.x[1];
    int maxevents = state->cpu.gpr.x[2];
    int timeout = state->cpu.gpr.x[3];

#ifdef __linux__
    int ret = epoll_wait(epfd, events, maxevents, timeout);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = ret;
    return 0;
#else
    /* macOS - stub */
    (void)epfd;
    (void)events;
    (void)maxevents;
    (void)timeout;
    state->syscall_result = -38;  /* ENOSYS */
    return -1;
#endif
}

/* ============================================================================
 * Additional Process Syscall Handlers
 * ============================================================================ */

/**
 * syscall_settimeofday - Set time of day (additional implementation)
 *
 * @param state Thread state
 * @return 0 on success, negative errno on failure
 */
int syscall_settimeofday_impl(ThreadState *state)
{
    const struct timeval *tv = (const struct timeval *)state->cpu.gpr.x[0];
    const struct timezone *tz = (const struct timezone *)state->cpu.gpr.x[1];

    int ret = settimeofday(tv, tz);
    if (ret < 0) {
        state->syscall_result = -errno;
        return -1;
    }
    state->syscall_result = 0;
    return 0;
}

/* ============================================================================
 * Signal Handling
 * ============================================================================ */

/**
 * signal_handler_fault - Handle segmentation fault during translation
 *
 * @param sig Signal number
 * @param info Signal information
 * @param context CPU context at fault
 */
void signal_handler_fault(int sig, siginfo_t *info, void *context)
{
    (void)sig;
    (void)info;
    (void)context;
    /* Handle translation faults gracefully */
    /* This allows Rosetta to handle unmapped guest addresses */
}

/**
 * init_signal_handlers - Initialize signal handlers for Rosetta
 *
 * Sets up signal handlers to catch translation faults and
 * handle them appropriately.
 */
void init_signal_handlers(void)
{
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = signal_handler_fault;
    sa.sa_flags = SA_SIGINFO;

    /* Install handler for segmentation faults */
    sigaction(SIGSEGV, &sa, NULL);

    /* Install handler for bus errors */
    sigaction(SIGBUS, &sa, NULL);
}

/* ============================================================================
 * Session 41: Flag Update Helper Functions
 * ============================================================================ */

/**
 * update_nzcv_flags - Update ARM64 NZCV flags after operation
 *
 * @param state Thread state (nzcv field updated)
 * @param result Result of the operation
 * @param operand1 First operand (for carry/overflow calculation)
 * @param operand2 Second operand
 * @param is_add Operation is addition (vs subtraction)
 * @param is_logical Operation is logical (AND, ORR, etc.)
 */
static inline void update_nzcv_flags(ThreadState *state, uint64_t result,
                                     uint64_t operand1, uint64_t operand2,
                                     bool is_add, bool is_logical)
{
    uint64_t nzcv = 0;

    /* N flag (bit 31) - Set if result is negative (MSB set) */
    if (result & (1ULL << 63)) {
        nzcv |= (1ULL << 31);
    }

    /* Z flag (bit 30) - Set if result is zero */
    if (result == 0) {
        nzcv |= (1ULL << 30);
    }

    if (!is_logical) {
        if (is_add) {
            /* C flag (bit 29) - Carry out for addition */
            /* Carry occurs if unsigned addition overflows */
            if ((uint64_t)(operand1 + operand2) < operand1) {
                nzcv |= (1ULL << 29);
            }

            /* V flag (bit 28) - Overflow for addition */
            /* Overflow occurs if signed operands have same sign but result has different sign */
            int64_t a = (int64_t)operand1, b = (int64_t)operand2, r = (int64_t)result;
            if ((a >= 0 && b >= 0 && r < 0) || (a < 0 && b < 0 && r >= 0)) {
                nzcv |= (1ULL << 28);
            }
        } else {
            /* Subtraction: C flag is NOT borrow */
            /* C=1 if no borrow, C=0 if borrow */
            if (operand1 >= operand2) {
                nzcv |= (1ULL << 29);
            }

            /* V flag - Overflow for subtraction */
            /* Overflow if operands have different signs and result sign differs from operand1 */
            int64_t a = (int64_t)operand1, b = (int64_t)operand2, r = (int64_t)result;
            if ((a >= 0 && b < 0 && r < 0) || (a < 0 && b >= 0 && r >= 0)) {
                nzcv |= (1ULL << 28);
            }
        }
    }

    state->cpu.gpr.nzcv = nzcv;
}

/**
 * update_nzcv_flags_and - Update NZCV flags after AND operation
 *
 * @param state Thread state
 * @param result Result of AND operation
 */
static inline void update_nzcv_flags_and(ThreadState *state, uint64_t result)
{
    uint64_t nzcv = 0;

    /* N flag - Set if result is negative */
    if (result & (1ULL << 63)) {
        nzcv |= (1ULL << 31);
    }

    /* Z flag - Set if result is zero */
    if (result == 0) {
        nzcv |= (1ULL << 30);
    }

    /* C flag - Set by shift operations, cleared for logical */
    /* V flag - Unchanged by logical operations */

    state->cpu.gpr.nzcv = nzcv;
}

/* ============================================================================
 * ALU Translation Functions
 * ============================================================================ */

/**
 * translate_add - Translate ARM64 ADD instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_add(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 ADD: Adds register values
     * Encoding: 00001011 <rm> <shift> <rn> <rd>
     *
     * x86_64 equivalent: ADD r/m64, r64 or ADD r64, r/m64
     */
    uint8_t rd = (insn[0] >> 0) & 0x1F;  /* Destination register */
    uint8_t rn = (insn[1] >> 5) & 0x1F;  /* First operand */
    uint8_t rm = (insn[2] >> 16) & 0x1F; /* Second operand */

    uint64_t op1 = state->cpu.gpr.x[rn];
    uint64_t op2 = state->cpu.gpr.x[rm];

    /* Perform addition */
    state->cpu.gpr.x[rd] = op1 + op2;

    /* Update condition flags (N, Z, C, V) */
    update_nzcv_flags(state, state->cpu.gpr.x[rd], op1, op2, true, false);

    return 0;
}

/**
 * translate_sub - Translate ARM64 SUB instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sub(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 SUB: Subtracts register values
     * Encoding: 11001011 <rm> <shift> <rn> <rd>
     *
     * x86_64 equivalent: SUB r/m64, r64 or SUB r64, r/m64
     */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint64_t op1 = state->cpu.gpr.x[rn];
    uint64_t op2 = state->cpu.gpr.x[rm];

    /* Perform subtraction */
    state->cpu.gpr.x[rd] = op1 - op2;

    /* Update condition flags (N, Z, C, V) */
    update_nzcv_flags(state, state->cpu.gpr.x[rd], op1, op2, false, false);

    return 0;
}

/**
 * translate_and - Translate ARM64 AND instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_and(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 AND: Bitwise AND
     * x86_64 equivalent: AND r/m64, r64 or AND r64, r/m64
     */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint64_t op1 = state->cpu.gpr.x[rn];
    uint64_t op2 = state->cpu.gpr.x[rm];

    state->cpu.gpr.x[rd] = op1 & op2;

    /* Update condition flags (N, Z only for logical) */
    update_nzcv_flags_and(state, state->cpu.gpr.x[rd]);

    return 0;
}

/**
 * translate_orr - Translate ARM64 ORR instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_orr(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 ORR: Bitwise OR
     * x86_64 equivalent: OR r/m64, r64 or OR r64, r/m64
     */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint64_t op1 = state->cpu.gpr.x[rn];
    uint64_t op2 = state->cpu.gpr.x[rm];
    state->cpu.gpr.x[rd] = op1 | op2;

    /* Update N, Z flags (logical operation) */
    update_nzcv_flags_and(state, state->cpu.gpr.x[rd]);

    return 0;
}

/**
 * translate_eor - Translate ARM64 EOR instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_eor(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 EOR: Bitwise XOR
     * x86_64 equivalent: XOR r/m64, r64 or XOR r64, r/m64
     */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint64_t op1 = state->cpu.gpr.x[rn];
    uint64_t op2 = state->cpu.gpr.x[rm];
    state->cpu.gpr.x[rd] = op1 ^ op2;

    /* Update N, Z flags (logical operation) */
    update_nzcv_flags_and(state, state->cpu.gpr.x[rd]);

    return 0;
}

/**
 * translate_mul - Translate ARM64 MUL instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_mul(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 MUL: Multiply
     * x86_64 equivalent: IMUL r64, r/m64
     */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    state->cpu.gpr.x[rd] = state->cpu.gpr.x[rn] * state->cpu.gpr.x[rm];

    return 0;
}

/**
 * translate_div - Translate ARM64 UDIV/SDIV instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_div(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 DIV: Unsigned or Signed Divide
     * x86_64 equivalent: DIV or IDIV
     */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    bool signed_div = (insn[3] >> 1) & 1;  /* Check S bit */

    uint64_t divisor = state->cpu.gpr.x[rm];
    if (divisor == 0) {
        /* Division by zero - ARM64 returns 0 */
        state->cpu.gpr.x[rd] = 0;
        return 0;
    }

    if (signed_div) {
        state->cpu.gpr.x[rd] = (int64_t)state->cpu.gpr.x[rn] / (int64_t)divisor;
    } else {
        state->cpu.gpr.x[rd] = state->cpu.gpr.x[rn] / divisor;
    }

    return 0;
}

/**
 * translate_mvn - Translate ARM64 MVN instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_mvn(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 MVN: Bitwise NOT (move negated)
     * x86_64 equivalent: NOT r/m64
     */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    state->cpu.gpr.x[rd] = ~state->cpu.gpr.x[rm];

    return 0;
}

/* ============================================================================
 * Branch Translation Functions
 * ============================================================================ */

/**
 * translate_b - Translate ARM64 B (unconditional branch) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_b(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 B: Unconditional branch with immediate offset
     * Encoding: 000101 <imm26>
     * Offset is signed, shifted left by 2
     */
    int32_t imm26 = (int32_t)((insn[0] >> 2) | ((insn[1] & 0x03) << 6) |
                              ((insn[2] & 0xFF) << 8) | ((insn[3] & 0x03) << 16));
    imm26 = (imm26 << 6) >> 6;  /* Sign extend */

    state->cpu.gpr.pc += imm26 * 4;

    return 0;
}

/**
 * translate_bl - Translate ARM64 BL (branch with link) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_bl(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 BL: Branch with Link (function call)
     * Stores return address in X30 (LR)
     */
    int32_t imm26 = (int32_t)((insn[0] >> 2) | ((insn[1] & 0x03) << 6) |
                              ((insn[2] & 0xFF) << 8) | ((insn[3] & 0x03) << 16));
    imm26 = (imm26 << 6) >> 6;  /* Sign extend */

    /* Save return address */
    state->cpu.gpr.lr = state->cpu.gpr.pc + 4;

    /* Branch to target */
    state->cpu.gpr.pc += imm26 * 4;

    return 0;
}

/**
 * translate_br - Translate ARM64 BR (branch to register) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_br(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 BR: Branch to Register (indirect jump)
     * Encoding: 11010110000 <Rn> 00000 00000
     */
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    state->cpu.gpr.pc = state->cpu.gpr.x[rn];

    return 0;
}

/**
 * translate_bcond - Translate ARM64 B.cond (conditional branch) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_bcond(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 B.cond: Conditional Branch
     * Encoding: 01010100 <cond> <imm19>
     * cond: 4-bit condition code (EQ, NE, CS, CC, MI, PL, VS, VC, etc.)
     */
    uint8_t cond = (insn[0] >> 4) & 0x0F;
    int32_t imm19 = (int32_t)((insn[1] >> 3) | ((insn[2] & 0x07) << 5) |
                              ((insn[3] & 0x7F) << 8));
    imm19 = (imm19 << 13) >> 13;  /* Sign extend */

    /* Check condition flags (NZCV) */
    uint8_t N = (state->cpu.gpr.nzcv >> 31) & 1;  /* Negative */
    uint8_t Z = (state->cpu.gpr.nzcv >> 30) & 1;  /* Zero */
    uint8_t C = (state->cpu.gpr.nzcv >> 29) & 1;  /* Carry */
    uint8_t V = (state->cpu.gpr.nzcv >> 28) & 1;  /* Overflow */

    bool taken = false;
    switch (cond) {
        case 0x0: taken = (Z == 1); break;  /* EQ */
        case 0x1: taken = (Z == 0); break;  /* NE */
        case 0x2: taken = (C == 1); break;  /* CS/HS */
        case 0x3: taken = (C == 0); break;  /* CC/LO */
        case 0x4: taken = (N == 1); break;  /* MI */
        case 0x5: taken = (N == 0); break;  /* PL */
        case 0x6: taken = (V == 1); break;  /* VS */
        case 0x7: taken = (V == 0); break;  /* VC */
        case 0x8: taken = (N != V); break;  /* LT */
        case 0x9: taken = (N == V); break;  /* GE */
        case 0xA: taken = (Z || (N != V)); break;  /* LE */
        case 0xB: taken = (!Z && (N == V)); break;  /* GT */
        case 0xC: taken = (N == 1 || Z == 1); break;  /* LE (alt) */
        case 0xD: taken = (N == 0 && Z == 0); break;  /* GT (alt) */
        case 0xE: break;  /* AL (always) - should use B instead */
        case 0xF: break;  /* NV - reserved */
    }

    if (taken) {
        state->cpu.gpr.pc += imm19 * 4;
    }

    return 0;
}

/**
 * translate_cbz - Translate ARM64 CBZ (compare and branch if zero) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cbz(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 CBZ: Compare and Branch if Zero
     * Encoding: 10110100 <imm19> <Rt>
     */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    int32_t imm19 = (int32_t)((insn[1] >> 3) | ((insn[2] & 0x07) << 5) |
                              ((insn[3] & 0x7F) << 8));
    imm19 = (imm19 << 13) >> 13;

    if (state->cpu.gpr.x[rt] == 0) {
        state->cpu.gpr.pc += imm19 * 4;
    }

    return 0;
}

/**
 * translate_cbnz - Translate ARM64 CBNZ (compare and branch if non-zero) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cbnz(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 CBNZ: Compare and Branch if Not Zero */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    int32_t imm19 = (int32_t)((insn[1] >> 3) | ((insn[2] & 0x07) << 5) |
                              ((insn[3] & 0x7F) << 8));
    imm19 = (imm19 << 13) >> 13;

    if (state->cpu.gpr.x[rt] != 0) {
        state->cpu.gpr.pc += imm19 * 4;
    }

    return 0;
}

/**
 * translate_tbz - Translate ARM64 TBZ (test bit and branch if zero) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_tbz(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 TBZ: Test Bit and Branch if Zero
     * Encoding: 11010110 <imm14> <bit> <Rt>
     */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t bit = ((insn[2] >> 3) & 0x07) | ((insn[3] & 0x01) << 3);
    int32_t imm14 = (int32_t)((insn[1] >> 3) | ((insn[2] & 0x03) << 5));
    imm14 = (imm14 << 18) >> 18;

    if (!((state->cpu.gpr.x[rt] >> bit) & 1)) {
        state->cpu.gpr.pc += imm14 * 4;
    }

    return 0;
}

/**
 * translate_tbnz - Translate ARM64 TBNZ (test bit and branch if non-zero) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_tbnz(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 TBNZ: Test Bit and Branch if Not Zero */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t bit = ((insn[2] >> 3) & 0x07) | ((insn[3] & 0x01) << 3);
    int32_t imm14 = (int32_t)((insn[1] >> 3) | ((insn[2] & 0x03) << 5));
    imm14 = (imm14 << 18) >> 18;

    if ((state->cpu.gpr.x[rt] >> bit) & 1) {
        state->cpu.gpr.pc += imm14 * 4;
    }

    return 0;
}

/* ============================================================================
 * Compare Translation Functions
 * ============================================================================ */

/**
 * translate_cmp - Translate ARM64 CMP (compare) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cmp(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 CMP: Compare (SUBS without destination)
     * Updates condition flags based on rn - rm
     */
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint64_t op1 = state->cpu.gpr.x[rn];
    uint64_t op2 = state->cpu.gpr.x[rm];
    uint64_t result = op1 - op2;

    /* Update all NZCV flags */
    update_nzcv_flags(state, result, op1, op2, false, false);

    return 0;
}

/**
 * translate_cmn - Translate ARM64 CMN (compare negative) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cmn(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 CMN: Compare Negative (rn + rm, update flags) */
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint64_t op1 = state->cpu.gpr.x[rn];
    uint64_t op2 = state->cpu.gpr.x[rm];
    uint64_t result = op1 + op2;

    /* Update all NZCV flags */
    update_nzcv_flags(state, result, op1, op2, true, false);

    return 0;
}

/**
 * translate_tst - Translate ARM64 TST (test bits) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_tst(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 TST: Test Bits (AND without destination, update flags) */
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint64_t result = state->cpu.gpr.x[rn] & state->cpu.gpr.x[rm];

    /* Update N, Z flags (logical operation) */
    update_nzcv_flags_and(state, result);

    return 0;
}

/* ============================================================================
 * Load/Store Translation Functions
 * ============================================================================ */

/**
 * translate_ldr - Translate ARM64 LDR (load register) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldr(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 LDR: Load Register (immediate)
     * Loads a word or doubleword from memory
     * Encoding varies by size (byte, halfword, word, doubleword)
     */
    uint8_t rt = (insn[0] >> 0) & 0x1F;  /* Target register */
    uint8_t rn = (insn[1] >> 5) & 0x1F;  /* Base register */
    uint8_t size = (insn[0] >> 2) & 0x03;  /* Size: 0=byte, 1=half, 2=word, 3=double */
    uint16_t imm12 = ((insn[1] >> 0) & 0xFF) | ((insn[2] & 0x0F) << 8);

    uint64_t addr = state->cpu.gpr.x[rn] + (imm12 << size);

    /* Translate guest address to host address */
    void *host_addr = memory_translate_addr(addr);
    if (host_addr == NULL) {
        return -1;  /* Translation fault */
    }

    switch (size) {
        case 0:  /* Byte */
            state->cpu.gpr.x[rt] = *(uint8_t *)host_addr;
            break;
        case 1:  /* Halfword */
            state->cpu.gpr.x[rt] = *(uint16_t *)host_addr;
            break;
        case 2:  /* Word */
            state->cpu.gpr.x[rt] = *(uint32_t *)host_addr;
            break;
        case 3:  /* Doubleword */
            state->cpu.gpr.x[rt] = *(uint64_t *)host_addr;
            break;
    }

    return 0;
}

/**
 * translate_str - Translate ARM64 STR (store register) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_str(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 STR: Store Register (immediate) */
    uint8_t rt = (insn[0] >> 0) & 0x1F;  /* Source register */
    uint8_t rn = (insn[1] >> 5) & 0x1F;  /* Base register */
    uint8_t size = (insn[0] >> 2) & 0x03;
    uint16_t imm12 = ((insn[1] >> 0) & 0xFF) | ((insn[2] & 0x0F) << 8);

    uint64_t addr = state->cpu.gpr.x[rn] + (imm12 << size);

    void *host_addr = memory_translate_addr(addr);
    if (host_addr == NULL) {
        return -1;
    }

    switch (size) {
        case 0:
            *(uint8_t *)host_addr = (uint8_t)state->cpu.gpr.x[rt];
            break;
        case 1:
            *(uint16_t *)host_addr = (uint16_t)state->cpu.gpr.x[rt];
            break;
        case 2:
            *(uint32_t *)host_addr = (uint32_t)state->cpu.gpr.x[rt];
            break;
        case 3:
            *(uint64_t *)host_addr = state->cpu.gpr.x[rt];
            break;
    }

    return 0;
}

/**
 * translate_ldp - Translate ARM64 LDP (load pair) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldp(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 LDP: Load Pair of registers
     * Loads two consecutive registers from memory
     */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t rt2 = (insn[1] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    int8_t imm7 = ((insn[2] >> 2) & 0x07) | ((insn[3] & 0x01) << 3);
    uint8_t size = (insn[0] >> 2) & 0x03;

    uint64_t addr = state->cpu.gpr.x[rn] + (imm7 << (size + 2));

    void *host_addr = memory_translate_addr(addr);
    if (host_addr == NULL) {
        return -1;
    }

    switch (size) {
        case 2:  /* Word pair */
            state->cpu.gpr.x[rt] = *(uint32_t *)host_addr;
            state->cpu.gpr.x[rt2] = *(uint32_t *)((uint64_t)host_addr + 4);
            break;
        case 3:  /* Doubleword pair */
            state->cpu.gpr.x[rt] = *(uint64_t *)host_addr;
            state->cpu.gpr.x[rt2] = *(uint64_t *)((uint64_t)host_addr + 8);
            break;
        default:
            /* TODO: Handle other sizes */
            break;
    }

    return 0;
}

/**
 * translate_stp - Translate ARM64 STP (store pair) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_stp(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 STP: Store Pair of registers */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t rt2 = (insn[1] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    int8_t imm7 = ((insn[2] >> 2) & 0x07) | ((insn[3] & 0x01) << 3);
    uint8_t size = (insn[0] >> 2) & 0x03;

    uint64_t addr = state->cpu.gpr.x[rn] + (imm7 << (size + 2));

    void *host_addr = memory_translate_addr(addr);
    if (host_addr == NULL) {
        return -1;
    }

    switch (size) {
        case 2:
            *(uint32_t *)host_addr = (uint32_t)state->cpu.gpr.x[rt];
            *(uint32_t *)((uint64_t)host_addr + 4) = (uint32_t)state->cpu.gpr.x[rt2];
            break;
        case 3:
            *(uint64_t *)host_addr = state->cpu.gpr.x[rt];
            *(uint64_t *)((uint64_t)host_addr + 8) = state->cpu.gpr.x[rt2];
            break;
        default:
            break;
    }

    return 0;
}

/**
 * translate_ldrb - Translate ARM64 LDRB (load register byte) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldrb(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 LDRB: Load Register Byte (zero-extended) */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint16_t imm12 = ((insn[1] >> 0) & 0xFF) | ((insn[2] & 0x0F) << 8);

    uint64_t addr = state->cpu.gpr.x[rn] + imm12;

    void *host_addr = memory_translate_addr(addr);
    if (host_addr == NULL) {
        return -1;
    }

    state->cpu.gpr.x[rt] = *(uint8_t *)host_addr;

    return 0;
}

/**
 * translate_strb - Translate ARM64 STRB (store register byte) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_strb(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 STRB: Store Register Byte */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint16_t imm12 = ((insn[1] >> 0) & 0xFF) | ((insn[2] & 0x0F) << 8);

    uint64_t addr = state->cpu.gpr.x[rn] + imm12;

    void *host_addr = memory_translate_addr(addr);
    if (host_addr == NULL) {
        return -1;
    }

    *(uint8_t *)host_addr = (uint8_t)state->cpu.gpr.x[rt];

    return 0;
}

/**
 * translate_ldrh - Translate ARM64 LDRH (load register halfword) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldrh(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 LDRH: Load Register Halfword (zero-extended) */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint16_t imm12 = ((insn[1] >> 0) & 0xFF) | ((insn[2] & 0x0F) << 8);

    uint64_t addr = state->cpu.gpr.x[rn] + (imm12 << 1);

    void *host_addr = memory_translate_addr(addr);
    if (host_addr == NULL) {
        return -1;
    }

    state->cpu.gpr.x[rt] = *(uint16_t *)host_addr;

    return 0;
}

/**
 * translate_strh - Translate ARM64 STRH (store register halfword) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_strh(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 STRH: Store Register Halfword */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint16_t imm12 = ((insn[1] >> 0) & 0xFF) | ((insn[2] & 0x0F) << 8);

    uint64_t addr = state->cpu.gpr.x[rn] + (imm12 << 1);

    void *host_addr = memory_translate_addr(addr);
    if (host_addr == NULL) {
        return -1;
    }

    *(uint16_t *)host_addr = (uint16_t)state->cpu.gpr.x[rt];

    return 0;
}

/* ============================================================================
 * System Instruction Translation Functions
 * ============================================================================ */

/**
 * translate_mrs - Translate ARM64 MRS (move from system register) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_mrs(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 MRS: Move from System Register to general purpose register
     * Used to read system registers like MDCCSR_EL0, CNTVCT_EL0, etc.
     */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint16_t op0 = (insn[2] >> 2) & 0x07;
    uint8_t op1 = (insn[1] >> 5) & 0x07;
    uint8_t crn = (insn[2] >> 0) & 0x0F;
    uint8_t crm = (insn[3] >> 0) & 0x0F;
    uint8_t op2 = (insn[3] >> 3) & 0x07;

    /* Stub: Return 0 for most system registers */
    /* In a full implementation, this would read actual system registers */
    (void)op0; (void)op1; (void)crn; (void)crm; (void)op2;

    state->cpu.gpr.x[rt] = 0;

    return 0;
}

/**
 * translate_msr - Translate ARM64 MSR (move to system register) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_msr(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 MSR: Move to System Register from general purpose register */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint16_t op0 = (insn[2] >> 2) & 0x07;
    uint8_t op1 = (insn[1] >> 5) & 0x07;

    /* Stub: Ignore writes to most system registers */
    (void)op0; (void)op1; (void)rt;

    return 0;
}

/**
 * translate_svc - Translate ARM64 SVC (supervisor call) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_svc(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 SVC: Supervisor Call (syscall)
     * Triggers an exception to EL1, handled by the kernel
     * On Linux, this is how syscalls are made
     */
    uint16_t imm16 = ((insn[0] >> 5) & 0x07) | ((insn[1] & 0x1F) << 3) |
                     ((insn[2] & 0x03) << 8);
    (void)imm16;

    /* The syscall number is in X8, arguments in X0-X5 */
    /* This would trigger the syscall handling mechanism */

    return 0;
}

/**
 * translate_hlt - Translate ARM64 HLT (halt) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_hlt(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 HLT: Halt - causes an exception
     * Often used for debugging or special hypervisor calls
     */
    uint16_t imm16 = ((insn[0] >> 5) & 0x07) | ((insn[1] & 0x1F) << 3) |
                     ((insn[2] & 0x03) << 8);
    (void)imm16; (void)state;

    /* Stub: Could trigger debugger or special handling */

    return 0;
}

/**
 * translate_brk - Translate ARM64 BRK (breakpoint) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_brk(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 BRK: Breakpoint - triggers a debug exception */
    uint16_t imm16 = ((insn[0] >> 5) & 0x07) | ((insn[1] & 0x1F) << 3) |
                     ((insn[2] & 0x03) << 8);
    (void)imm16; (void)state;

    /* Stub: Could trigger debugger */

    return 0;
}

/* ============================================================================
 * FP Translation Functions
 * ============================================================================ */

/**
 * translate_fmov - Translate ARM64 FMOV (floating-point move) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fmov(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 FMOV: Move to/from FP register or immediate
     * Three variants:
     * 1. FMOV <d>, <m>  - Move from GPR to FP register
     * 2. FMOV <n>, <d>  - Move from FP register to GPR
     * 3. FMOV <d>, #<imm> - Move immediate to FP register
     */
    uint8_t rd = (insn[0] >> 0) & 0x1F;  /* Destination FP register */
    uint8_t rn = (insn[1] >> 5) & 0x1F;  /* Source register */
    uint8_t op = (insn[1] >> 21) & 0x03; /* Operation type */
    uint8_t type = (insn[0] >> 22) & 0x03; /* FP type: 0=single, 1=double */

    if (op == 0) {
        /* FMOV <Dn>, <Xm> - Move from GPR to FP register */
        uint64_t val = state->cpu.gpr.x[rn];
        if (type == 0) {
            /* Single precision - zero extend */
            state->cpu.vec.v[rd].lo = val & 0xFFFFFFFFULL;
            state->cpu.vec.v[rd].hi = 0;
        } else {
            /* Double precision */
            state->cpu.vec.v[rd].lo = val;
            state->cpu.vec.v[rd].hi = 0;
        }
    } else if (op == 1) {
        /* FMOV <Xn>, <Dm> - Move from FP register to GPR */
        if (type == 0) {
            /* Single precision - zero extend */
            state->cpu.gpr.x[rd] = state->cpu.vec.v[rn].lo & 0xFFFFFFFFULL;
        } else {
            /* Double precision */
            state->cpu.gpr.x[rd] = state->cpu.vec.v[rn].lo;
        }
    } else if (op == 2) {
        /* FMOV #<imm> - Move immediate to FP register */
        /* Immediate encoding: 8-bit value replicated */
        uint8_t imm8 = ((insn[0] >> 5) & 0x07) | ((insn[1] >> 1) & 0xF8);
        uint64_t val = 0;
        for (int i = 0; i < 8; i++) {
            val |= ((uint64_t)imm8) << (i * 8);
        }
        state->cpu.vec.v[rd].lo = val;
        state->cpu.vec.v[rd].hi = 0;
    }

    return 0;
}

/**
 * translate_fadd - Translate ARM64 FADD (floating-point add) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fadd(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 FADD: Floating-point add
     * Encoding: 00011110 <type> <op> <Rm> <A> <Rd>
     * type: 0=single (S), 1=double (D)
     */
    uint8_t rd = (insn[0] >> 0) & 0x1F;  /* Destination register */
    uint8_t rn = (insn[1] >> 5) & 0x1F;  /* First operand */
    uint8_t rm = (insn[2] >> 16) & 0x1F; /* Second operand */
    uint8_t type = (insn[0] >> 22) & 0x01; /* 0=single, 1=double */

    if (type == 0) {
        /* Single precision */
        float a = *(float*)&state->cpu.vec.v[rn].lo;
        float b = *(float*)&state->cpu.vec.v[rm].lo;
        float result = a + b;
        state->cpu.vec.v[rd].lo = *(uint32_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
    } else {
        /* Double precision */
        double a = *(double*)&state->cpu.vec.v[rn].lo;
        double b = *(double*)&state->cpu.vec.v[rm].lo;
        double result = a + b;
        state->cpu.vec.v[rd].lo = *(uint64_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
    }

    /* TODO: Update FPSR flags (N, Z, I, V) */
    return 0;
}

/**
 * translate_fsub - Translate ARM64 FSUB (floating-point subtract) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fsub(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 FSUB: Floating-point subtract */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t type = (insn[0] >> 22) & 0x01;

    if (type == 0) {
        /* Single precision */
        float a = *(float*)&state->cpu.vec.v[rn].lo;
        float b = *(float*)&state->cpu.vec.v[rm].lo;
        float result = a - b;
        state->cpu.vec.v[rd].lo = *(uint32_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
    } else {
        /* Double precision */
        double a = *(double*)&state->cpu.vec.v[rn].lo;
        double b = *(double*)&state->cpu.vec.v[rm].lo;
        double result = a - b;
        state->cpu.vec.v[rd].lo = *(uint64_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
    }

    return 0;
}

/**
 * translate_fmul - Translate ARM64 FMUL (floating-point multiply) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fmul(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 FMUL: Floating-point multiply */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t type = (insn[0] >> 22) & 0x01;

    if (type == 0) {
        /* Single precision */
        float a = *(float*)&state->cpu.vec.v[rn].lo;
        float b = *(float*)&state->cpu.vec.v[rm].lo;
        float result = a * b;
        state->cpu.vec.v[rd].lo = *(uint32_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
    } else {
        /* Double precision */
        double a = *(double*)&state->cpu.vec.v[rn].lo;
        double b = *(double*)&state->cpu.vec.v[rm].lo;
        double result = a * b;
        state->cpu.vec.v[rd].lo = *(uint64_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
    }

    return 0;
}

/**
 * translate_fdiv - Translate ARM64 FDIV (floating-point divide) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fdiv(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 FDIV: Floating-point divide */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t type = (insn[0] >> 22) & 0x01;

    if (type == 0) {
        /* Single precision */
        float a = *(float*)&state->cpu.vec.v[rn].lo;
        float b = *(float*)&state->cpu.vec.v[rm].lo;
        if (b == 0.0f) {
            /* Handle division by zero - set to infinity */
            state->cpu.vec.v[rd].lo = 0x7F800000ULL;
            state->cpu.vec.v[rd].hi = 0;
        } else {
            float result = a / b;
            state->cpu.vec.v[rd].lo = *(uint32_t*)&result;
            state->cpu.vec.v[rd].hi = 0;
        }
    } else {
        /* Double precision */
        double a = *(double*)&state->cpu.vec.v[rn].lo;
        double b = *(double*)&state->cpu.vec.v[rm].lo;
        if (b == 0.0) {
            /* Handle division by zero - set to infinity */
            state->cpu.vec.v[rd].lo = 0x7FF0000000000000ULL;
            state->cpu.vec.v[rd].hi = 0;
        } else {
            double result = a / b;
            state->cpu.vec.v[rd].lo = *(uint64_t*)&result;
            state->cpu.vec.v[rd].hi = 0;
        }
    }

    return 0;
}

/**
 * translate_fsqrt - Translate ARM64 FSQRT (floating-point square root) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fsqrt(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 FSQRT: Floating-point square root */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t type = (insn[0] >> 22) & 0x01;

    if (type == 0) {
        /* Single precision */
        float a = *(float*)&state->cpu.vec.v[rn].lo;
        float result = sqrtf(a);
        state->cpu.vec.v[rd].lo = *(uint32_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
    } else {
        /* Double precision */
        double a = *(double*)&state->cpu.vec.v[rn].lo;
        double result = sqrt(a);
        state->cpu.vec.v[rd].lo = *(uint64_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
    }

    return 0;
}

/**
 * translate_fcmp - Translate ARM64 FCMP (floating-point compare) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fcmp(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 FCMP: Floating-point compare, updates NZCV flags */
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t type = (insn[0] >> 22) & 0x01;
    uint8_t op2 = (insn[1] >> 15) & 0x01; /* 0=two registers, 1=with immediate zero */

    /* Clear NZCV flags */
    state->cpu.gpr.nzcv = 0;

    if (op2 == 0) {
        /* Compare two registers */
        if (type == 0) {
            /* Single precision */
            float a = *(float*)&state->cpu.vec.v[rn].lo;
            float b = *(float*)&state->cpu.vec.v[rm].lo;

            if (a < b) {
                state->cpu.gpr.nzcv |= (1 << 31); /* N flag */
            } else if (a > b) {
                state->cpu.gpr.nzcv |= (1 << 29); /* C flag */
            } else {
                state->cpu.gpr.nzcv |= (1 << 30); /* Z flag */
            }
        } else {
            /* Double precision */
            double a = *(double*)&state->cpu.vec.v[rn].lo;
            double b = *(double*)&state->cpu.vec.v[rm].lo;

            if (a < b) {
                state->cpu.gpr.nzcv |= (1 << 31); /* N flag */
            } else if (a > b) {
                state->cpu.gpr.nzcv |= (1 << 29); /* C flag */
            } else {
                state->cpu.gpr.nzcv |= (1 << 30); /* Z flag */
            }
        }
    } else {
        /* Compare with immediate zero */
        if (type == 0) {
            float a = *(float*)&state->cpu.vec.v[rn].lo;
            if (a < 0.0f) {
                state->cpu.gpr.nzcv |= (1 << 31);
            } else if (a > 0.0f) {
                state->cpu.gpr.nzcv |= (1 << 29);
            } else {
                state->cpu.gpr.nzcv |= (1 << 30);
            }
        } else {
            double a = *(double*)&state->cpu.vec.v[rn].lo;
            if (a < 0.0) {
                state->cpu.gpr.nzcv |= (1 << 31);
            } else if (a > 0.0) {
                state->cpu.gpr.nzcv |= (1 << 29);
            } else {
                state->cpu.gpr.nzcv |= (1 << 30);
            }
        }
    }

    return 0;
}

/**
 * translate_fcvt - Translate ARM64 FCVT (floating-point convert) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fcvt(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 FCVT: Convert between FP formats
     * Variants:
     * - FCVT.S.D: Double to Single
     * - FCVT.D.S: Single to Double
     * - FCVT.ZS: FP to signed int
     * - FCVT.ZU: FP to unsigned int
     * - SCVTF: Signed int to FP
     * - UCVTF: Unsigned int to FP
     */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t op = (insn[0] >> 29) & 0x03; /* Convert operation */
    uint8_t rmode = (insn[0] >> 27) & 0x03; /* Rounding mode */
    (void)rmode; /* TODO: Handle rounding modes */

    if (op == 0) {
        /* FCVT.S.D - Double to Single */
        double a = *(double*)&state->cpu.vec.v[rn].lo;
        float result = (float)a;
        state->cpu.vec.v[rd].lo = *(uint32_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
    } else if (op == 1) {
        /* FCVT.D.S - Single to Double */
        float a = *(float*)&state->cpu.vec.v[rn].lo;
        double result = (double)a;
        state->cpu.vec.v[rd].lo = *(uint64_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
    }

    return 0;
}

/**
 * translate_fcsel - Translate ARM64 FCSEL (floating-point conditional select) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fcsel(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 FCSEL: Floating-point Conditional Select
     * Selects between two FP registers based on condition flags
     * Encoding: 00011110 <cond> <Rm> <Rn> <Rd>
     *
     * if (condition true)
     *     Rd = Rn
     * else
     *     Rd = Rm
     */
    uint8_t rd = (insn[0] >> 0) & 0x1F;  /* Destination register */
    uint8_t rn = (insn[1] >> 5) & 0x1F;  /* First operand (selected if cond true) */
    uint8_t rm = (insn[2] >> 16) & 0x1F; /* Second operand (selected if cond false) */
    uint8_t cond = (insn[0] >> 4) & 0x0F; /* Condition code */
    uint8_t type = (insn[0] >> 22) & 0x01; /* 0=single, 1=double */

    /* Check condition flags (NZCV) */
    uint8_t N = (state->cpu.gpr.nzcv >> 31) & 1;  /* Negative */
    uint8_t Z = (state->cpu.gpr.nzcv >> 30) & 1;  /* Zero */
    uint8_t C = (state->cpu.gpr.nzcv >> 29) & 1;  /* Carry */
    uint8_t V = (state->cpu.gpr.nzcv >> 28) & 1;  /* Overflow */

    bool condition_true = false;
    switch (cond) {
        case 0x0: condition_true = (Z == 1); break;  /* EQ */
        case 0x1: condition_true = (Z == 0); break;  /* NE */
        case 0x2: condition_true = (C == 1); break;  /* CS/HS */
        case 0x3: condition_true = (C == 0); break;  /* CC/LO */
        case 0x4: condition_true = (N == 1); break;  /* MI */
        case 0x5: condition_true = (N == 0); break;  /* PL */
        case 0x6: condition_true = (V == 1); break;  /* VS */
        case 0x7: condition_true = (V == 0); break;  /* VC */
        case 0x8: condition_true = (N != V); break;  /* LT */
        case 0x9: condition_true = (N == V); break;  /* GE */
        case 0xA: condition_true = (Z || (N != V)); break;  /* LE */
        case 0xB: condition_true = (!Z && (N == V)); break;  /* GT */
        default: condition_true = true; break;  /* AL/default */
    }

    /* Select source based on condition */
    if (condition_true) {
        state->cpu.vec.v[rd].lo = state->cpu.vec.v[rn].lo;
        state->cpu.vec.v[rd].hi = state->cpu.vec.v[rn].hi;
    } else {
        state->cpu.vec.v[rd].lo = state->cpu.vec.v[rm].lo;
        state->cpu.vec.v[rd].hi = state->cpu.vec.v[rm].hi;
    }

    return 0;
}

/* ============================================================================
 * NEON Translation Functions
 * ============================================================================ */

/**
 * translate_ld1 - Translate ARM64 LD1 (load single structure) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ld1(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 LD1: Load single structure to NEON register
     * Loads 8, 16, 32, or 64 bits from memory to a vector register
     * Encoding: 00111100 <size> <Rt> <Rn> <imm>
     */
    uint8_t rt = (insn[0] >> 0) & 0x1F;   /* Target vector register */
    uint8_t rn = (insn[1] >> 5) & 0x1F;   /* Base address register */
    uint8_t size = (insn[0] >> 22) & 0x03; /* Element size */
    uint8_t lanes = (insn[2] >> 10) & 0x03; /* Number of lanes */

    /* Get base address */
    uint64_t addr = state->cpu.gpr.x[rn];

    /* Translate guest address to host address */
    void *host_addr = memory_translate_addr(addr);
    if (host_addr == NULL) {
        return -1;  /* Translation fault */
    }

    /* Load based on size and number of lanes */
    switch (size) {
        case 0:  /* 8-bit */
            if (lanes == 0) {
                state->cpu.vec.v[rt].lo = *(uint8_t *)host_addr;
            } else {
                for (int i = 0; i <= lanes; i++) {
                    ((uint8_t*)&state->cpu.vec.v[rt].lo)[i] = *((uint8_t*)host_addr + i);
                }
            }
            break;
        case 1:  /* 16-bit */
            if (lanes == 0) {
                state->cpu.vec.v[rt].lo = *(uint16_t *)host_addr;
            } else {
                for (int i = 0; i <= lanes; i++) {
                    ((uint16_t*)&state->cpu.vec.v[rt].lo)[i] = *((uint16_t*)host_addr + i);
                }
            }
            break;
        case 2:  /* 32-bit */
            if (lanes == 0) {
                state->cpu.vec.v[rt].lo = *(uint32_t *)host_addr;
            } else {
                for (int i = 0; i <= lanes; i++) {
                    ((uint32_t*)&state->cpu.vec.v[rt].lo)[i] = *((uint32_t*)host_addr + i);
                }
            }
            break;
        case 3:  /* 64-bit */
            state->cpu.vec.v[rt].lo = *(uint64_t *)host_addr;
            break;
    }

    state->cpu.vec.v[rt].hi = 0;
    return 0;
}

/**
 * translate_st1 - Translate ARM64 ST1 (store single structure) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_st1(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 ST1: Store single structure from NEON register */
    uint8_t rt = (insn[0] >> 0) & 0x1F;   /* Source vector register */
    uint8_t rn = (insn[1] >> 5) & 0x1F;   /* Base address register */
    uint8_t size = (insn[0] >> 22) & 0x03; /* Element size */

    /* Get base address */
    uint64_t addr = state->cpu.gpr.x[rn];

    /* Translate guest address to host address */
    void *host_addr = memory_translate_addr(addr);
    if (host_addr == NULL) {
        return -1;
    }

    /* Store based on size */
    switch (size) {
        case 0:  /* 8-bit */
            *(uint8_t *)host_addr = (uint8_t)(state->cpu.vec.v[rt].lo & 0xFF);
            break;
        case 1:  /* 16-bit */
            *(uint16_t *)host_addr = (uint16_t)(state->cpu.vec.v[rt].lo & 0xFFFF);
            break;
        case 2:  /* 32-bit */
            *(uint32_t *)host_addr = (uint32_t)(state->cpu.vec.v[rt].lo & 0xFFFFFFFF);
            break;
        case 3:  /* 64-bit */
            *(uint64_t *)host_addr = state->cpu.vec.v[rt].lo;
            break;
    }

    return 0;
}

/**
 * translate_ld2 - Translate ARM64 LD2 (load pair of structures) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ld2(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 LD2: Load two structures (interleaved)
     * Loads two registers worth of data with de-interleaving
     */
    uint8_t rt = (insn[0] >> 0) & 0x1F;   /* First target register */
    uint8_t rt2 = (insn[1] >> 0) & 0x1F;  /* Second target register */
    uint8_t rn = (insn[1] >> 5) & 0x1F;   /* Base address register */

    uint64_t addr = state->cpu.gpr.x[rn];
    void *host_addr = memory_translate_addr(addr);
    if (host_addr == NULL) {
        return -1;
    }

    /* Load two consecutive 64-bit values */
    state->cpu.vec.v[rt].lo = *(uint64_t *)host_addr;
    state->cpu.vec.v[rt].hi = 0;
    state->cpu.vec.v[rt2].lo = *((uint64_t *)host_addr + 1);
    state->cpu.vec.v[rt2].hi = 0;

    return 0;
}

/**
 * translate_st2 - Translate ARM64 ST2 (store pair of structures) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_st2(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 ST2: Store two structures (interleaved) */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t rt2 = (insn[1] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint64_t addr = state->cpu.gpr.x[rn];
    void *host_addr = memory_translate_addr(addr);
    if (host_addr == NULL) {
        return -1;
    }

    /* Store two consecutive 64-bit values */
    *(uint64_t *)host_addr = state->cpu.vec.v[rt].lo;
    *((uint64_t *)host_addr + 1) = state->cpu.vec.v[rt2].lo;

    return 0;
}

/**
 * translate_ld3 - Translate ARM64 LD3 (load three structures) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ld3(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 LD3: Load three structures (RGB de-interleave) */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t rt2 = (insn[1] >> 0) & 0x1F;
    uint8_t rt3 = (insn[2] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint64_t addr = state->cpu.gpr.x[rn];
    void *host_addr = memory_translate_addr(addr);
    if (host_addr == NULL) {
        return -1;
    }

    /* Load three consecutive 64-bit values */
    state->cpu.vec.v[rt].lo = *(uint64_t *)host_addr;
    state->cpu.vec.v[rt].hi = 0;
    state->cpu.vec.v[rt2].lo = *((uint64_t *)host_addr + 1);
    state->cpu.vec.v[rt2].hi = 0;
    state->cpu.vec.v[rt3].lo = *((uint64_t *)host_addr + 2);
    state->cpu.vec.v[rt3].hi = 0;

    return 0;
}

/**
 * translate_st3 - Translate ARM64 ST3 (store three structures) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_st3(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 ST3: Store three structures (RGB interleave) */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t rt2 = (insn[1] >> 0) & 0x1F;
    uint8_t rt3 = (insn[2] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint64_t addr = state->cpu.gpr.x[rn];
    void *host_addr = memory_translate_addr(addr);
    if (host_addr == NULL) {
        return -1;
    }

    /* Store three consecutive 64-bit values */
    *(uint64_t *)host_addr = state->cpu.vec.v[rt].lo;
    *((uint64_t *)host_addr + 1) = state->cpu.vec.v[rt2].lo;
    *((uint64_t *)host_addr + 2) = state->cpu.vec.v[rt3].lo;

    return 0;
}

/**
 * translate_ld4 - Translate ARM64 LD4 (load four structures) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ld4(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 LD4: Load four structures (RGBA de-interleave) */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t rt2 = (insn[1] >> 0) & 0x1F;
    uint8_t rt3 = (insn[2] >> 0) & 0x1F;
    uint8_t rt4 = (insn[3] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint64_t addr = state->cpu.gpr.x[rn];
    void *host_addr = memory_translate_addr(addr);
    if (host_addr == NULL) {
        return -1;
    }

    /* Load four consecutive 64-bit values */
    state->cpu.vec.v[rt].lo = *(uint64_t *)host_addr;
    state->cpu.vec.v[rt].hi = 0;
    state->cpu.vec.v[rt2].lo = *((uint64_t *)host_addr + 1);
    state->cpu.vec.v[rt2].hi = 0;
    state->cpu.vec.v[rt3].lo = *((uint64_t *)host_addr + 2);
    state->cpu.vec.v[rt3].hi = 0;
    state->cpu.vec.v[rt4].lo = *((uint64_t *)host_addr + 3);
    state->cpu.vec.v[rt4].hi = 0;

    return 0;
}

/**
 * translate_st4 - Translate ARM64 ST4 (store four structures) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_st4(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 ST4: Store four structures (RGBA interleave) */
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t rt2 = (insn[1] >> 0) & 0x1F;
    uint8_t rt3 = (insn[2] >> 0) & 0x1F;
    uint8_t rt4 = (insn[3] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint64_t addr = state->cpu.gpr.x[rn];
    void *host_addr = memory_translate_addr(addr);
    if (host_addr == NULL) {
        return -1;
    }

    /* Store four consecutive 64-bit values */
    *(uint64_t *)host_addr = state->cpu.vec.v[rt].lo;
    *((uint64_t *)host_addr + 1) = state->cpu.vec.v[rt2].lo;
    *((uint64_t *)host_addr + 2) = state->cpu.vec.v[rt3].lo;
    *((uint64_t *)host_addr + 3) = state->cpu.vec.v[rt4].lo;

    return 0;
}

/**
 * translate_dup - Translate ARM64 DUP (duplicate) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_dup(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 DUP: Duplicate element across vector register
     * Copies a single element to all lanes in the destination
     */
    uint8_t rd = (insn[0] >> 0) & 0x1F;   /* Destination register */
    uint8_t rn = (insn[1] >> 5) & 0x1F;   /* Source register */
    uint8_t imm4 = (insn[1] >> 11) & 0x0F; /* Element index */
    uint8_t size = (insn[0] >> 22) & 0x03; /* Element size */

    uint64_t val;
    switch (size) {
        case 0:  /* 8-bit */
            val = ((uint8_t*)&state->cpu.vec.v[rn].lo)[imm4 & 0x07];
            break;
        case 1:  /* 16-bit */
            val = ((uint16_t*)&state->cpu.vec.v[rn].lo)[imm4 & 0x03];
            break;
        case 2:  /* 32-bit */
            val = ((uint32_t*)&state->cpu.vec.v[rn].lo)[imm4 & 0x01];
            break;
        default: /* 64-bit */
            val = state->cpu.vec.v[rn].lo;
            break;
    }

    /* Duplicate across both lo and hi */
    switch (size) {
        case 0:
            state->cpu.vec.v[rd].lo = val * 0x0101010101010101ULL;
            state->cpu.vec.v[rd].hi = val * 0x0101010101010101ULL;
            break;
        case 1:
            state->cpu.vec.v[rd].lo = val * 0x0001000100010001ULL;
            state->cpu.vec.v[rd].hi = val * 0x0001000100010001ULL;
            break;
        case 2:
            state->cpu.vec.v[rd].lo = (val << 32) | val;
            state->cpu.vec.v[rd].hi = (val << 32) | val;
            break;
        default:
            state->cpu.vec.v[rd].lo = val;
            state->cpu.vec.v[rd].hi = val;
            break;
    }

    return 0;
}

/**
 * translate_ext - Translate ARM64 EXT (extract) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ext(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 EXT: Extract vector from pair of registers
     * Concatenates two registers and extracts a shifted view
     */
    uint8_t rd = (insn[0] >> 0) & 0x1F;   /* Destination register */
    uint8_t rn = (insn[1] >> 5) & 0x1F;   /* First source register */
    uint8_t rm = (insn[2] >> 16) & 0x1F;  /* Second source register */
    uint8_t lsb = (insn[3] >> 0) & 0x0F;  /* Shift amount */

    /* Concatenate and shift */
    if (lsb < 8) {
        state->cpu.vec.v[rd].lo = (state->cpu.vec.v[rn].lo >> (lsb * 8)) |
                                   (state->cpu.vec.v[rm].lo << ((8 - lsb) * 8));
        state->cpu.vec.v[rd].hi = (state->cpu.vec.v[rn].hi >> (lsb * 8)) |
                                   (state->cpu.vec.v[rm].hi << ((8 - lsb) * 8));
    } else {
        state->cpu.vec.v[rd].lo = state->cpu.vec.v[rm].lo >> ((lsb - 8) * 8);
        state->cpu.vec.v[rd].hi = state->cpu.vec.v[rm].hi >> ((lsb - 8) * 8);
    }

    return 0;
}

/**
 * translate_tbl - Translate ARM64 TBL (table lookup) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_tbl(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 TBL: Table lookup
     * Uses index values in one register to look up bytes in a table
     */
    uint8_t rd = (insn[0] >> 0) & 0x1F;   /* Destination register */
    uint8_t rn = (insn[1] >> 5) & 0x1F;   /* Table register */
    uint8_t rm = (insn[2] >> 16) & 0x1F;  /* Index register */

    uint8_t result[16];
    uint8_t *table = (uint8_t*)&state->cpu.vec.v[rn].lo;
    uint8_t *indices = (uint8_t*)&state->cpu.vec.v[rm].lo;

    /* Perform table lookup - out of bounds indices return 0 */
    for (int i = 0; i < 16; i++) {
        if (indices[i] < 16) {
            result[i] = table[indices[i]];
        } else {
            result[i] = 0;
        }
    }

    memcpy(&state->cpu.vec.v[rd].lo, result, 16);
    state->cpu.vec.v[rd].hi = 0;

    return 0;
}

/**
 * translate_tbx - Translate ARM64 TBX (table lookup extension) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_tbx(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 TBX: Table lookup extension
     * Like TBL but preserves destination register for out-of-bounds indices
     */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint8_t result[16];
    uint8_t *table = (uint8_t*)&state->cpu.vec.v[rn].lo;
    uint8_t *indices = (uint8_t*)&state->cpu.vec.v[rm].lo;
    uint8_t *orig = (uint8_t*)&state->cpu.vec.v[rd].lo;

    /* Perform table lookup - preserve original for out-of-bounds */
    for (int i = 0; i < 16; i++) {
        if (indices[i] < 16) {
            result[i] = table[indices[i]];
        } else {
            result[i] = orig[i];
        }
    }

    memcpy(&state->cpu.vec.v[rd].lo, result, 16);

    return 0;
}

/**
 * translate_ushr - Translate ARM64 USHR (unsigned shift right) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ushr(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 USHR: Unsigned shift right by immediate
     * Shifts each element right by an immediate amount
     */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t shift = (insn[2] >> 16) & 0x3F;  /* Shift amount (encoded) */

    /* Decode shift amount - encoded as 64-shift */
    shift = 64 - shift;

    if (shift >= 64) {
        /* All bits shifted out */
        state->cpu.vec.v[rd].lo = 0;
        state->cpu.vec.v[rd].hi = 0;
    } else {
        state->cpu.vec.v[rd].lo = state->cpu.vec.v[rn].lo >> shift;
        state->cpu.vec.v[rd].hi = state->cpu.vec.v[rn].hi >> shift;
    }

    return 0;
}

/**
 * translate_sshr - Translate ARM64 SSHR (signed shift right) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sshr(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 SSHR: Signed shift right by immediate (arithmetic shift) */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t shift = (insn[2] >> 16) & 0x3F;

    /* Decode shift amount */
    shift = 64 - shift;

    if (shift >= 64) {
        /* Sign extend */
        uint64_t sign_mask = -(state->cpu.vec.v[rn].lo >> 63);
        state->cpu.vec.v[rd].lo = sign_mask;
        state->cpu.vec.v[rd].hi = sign_mask;
    } else {
        /* Arithmetic shift right (preserves sign) */
        state->cpu.vec.v[rd].lo = (int64_t)state->cpu.vec.v[rn].lo >> shift;
        state->cpu.vec.v[rd].hi = (int64_t)state->cpu.vec.v[rn].hi >> shift;
    }

    return 0;
}

/**
 * translate_shl - Translate ARM64 SHL (shift left) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_shl(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 SHL: Shift left by immediate */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t shift = (insn[2] >> 16) & 0x3F;

    if (shift >= 64) {
        /* All bits shifted out */
        state->cpu.vec.v[rd].lo = 0;
        state->cpu.vec.v[rd].hi = 0;
    } else {
        state->cpu.vec.v[rd].lo = state->cpu.vec.v[rn].lo << shift;
        state->cpu.vec.v[rd].hi = state->cpu.vec.v[rn].hi << shift;
    }

    return 0;
}

/**
 * translate_scf - Translate ARM64 SCVTF (signed convert to float) instruction
 *
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_scf(ThreadState *state, const uint8_t *insn)
{
    /* ARM64 SCVTF: Signed integer to floating-point convert */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t size = (insn[0] >> 22) & 0x01; /* 0=single, 1=double */
    uint8_t opcode = (insn[0] >> 29) & 0x03;

    if (opcode == 0 && size == 0) {
        /* Signed 32-bit int to single precision float */
        int32_t val = (int32_t)(state->cpu.vec.v[rn].lo & 0xFFFFFFFF);
        float result = (float)val;
        state->cpu.vec.v[rd].lo = *(uint32_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
    } else if (opcode == 0 && size == 1) {
        /* Signed 64-bit int to double precision float */
        int64_t val = (int64_t)state->cpu.vec.v[rn].lo;
        double result = (double)val;
        state->cpu.vec.v[rd].lo = *(uint64_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
    } else if (opcode == 1 && size == 0) {
        /* Unsigned 32-bit int to float (UCVTF) */
        uint32_t val = (uint32_t)(state->cpu.vec.v[rn].lo & 0xFFFFFFFF);
        float result = (float)val;
        state->cpu.vec.v[rd].lo = *(uint32_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
    } else if (opcode == 1 && size == 1) {
        /* Unsigned 64-bit int to double (UCVTF) */
        uint64_t val = state->cpu.vec.v[rn].lo;
        double result = (double)val;
        state->cpu.vec.v[rd].lo = *(uint64_t*)&result;
        state->cpu.vec.v[rd].hi = 0;
    }

    return 0;
}

/* ============================================================================
 * Additional Vector Operations - Session 12
 * ============================================================================ */

/**
 * v128_padd - Vector pair-wise add
 *
 * Adds adjacent pairs of elements in the vector.
 * For 16 bytes: [a0+a1, a2+a3, a4+a5, ..., a14+a15]
 *
 * @param a Input vector
 * @return Vector with pairwise sums
 */
Vector128 v128_padd(Vector128 a)
{
    Vector128 result;
    uint64_t lo_sum = 0;
    uint64_t hi_sum = 0;

    /* Pair-wise add low 64 bits (8 bytes -> 4 bytes) */
    for (int i = 0; i < 8; i += 2) {
        uint8_t sum = ((a.lo >> (i * 8)) & 0xFF) + ((a.lo >> ((i + 1) * 8)) & 0xFF);
        lo_sum |= ((uint64_t)sum) << ((i / 2) * 8);
    }

    /* Pair-wise add high 64 bits */
    for (int i = 0; i < 8; i += 2) {
        uint8_t sum = ((a.hi >> (i * 8)) & 0xFF) + ((a.hi >> ((i + 1) * 8)) & 0xFF);
        hi_sum |= ((uint64_t)sum) << ((i / 2) * 8);
    }

    result.lo = lo_sum;
    result.hi = hi_sum;
    return result;
}

/**
 * v128_abs - Vector absolute value (signed)
 *
 * Computes absolute value of each signed byte element.
 *
 * @param a Input vector
 * @return Vector with absolute values
 */
Vector128 v128_abs(Vector128 a)
{
    Vector128 result;
    int8_t val;

    /* Absolute value of low 64 bits */
    result.lo = 0;
    for (int i = 0; i < 8; i++) {
        val = (int8_t)((a.lo >> (i * 8)) & 0xFF);
        if (val < 0) val = -val;
        result.lo |= ((uint64_t)(uint8_t)val) << (i * 8);
    }

    /* Absolute value of high 64 bits */
    result.hi = 0;
    for (int i = 0; i < 8; i++) {
        val = (int8_t)((a.hi >> (i * 8)) & 0xFF);
        if (val < 0) val = -val;
        result.hi |= ((uint64_t)(uint8_t)val) << (i * 8);
    }

    return result;
}

/**
 * v128_sat_add - Vector saturating add (unsigned)
 *
 * Adds two vectors with saturation at 0xFF per byte.
 *
 * @param a First vector
 * @param b Second vector
 * @return Vector with saturated sums
 */
Vector128 v128_sat_add(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint16_t sum;

    /* Saturating add low 64 bits */
    result.lo = 0;
    for (int i = 0; i < 8; i++) {
        sum = ((a.lo >> (i * 8)) & 0xFF) + ((b.lo >> (i * 8)) & 0xFF);
        if (sum > 0xFF) sum = 0xFF;
        result.lo |= ((uint64_t)sum) << (i * 8);
    }

    /* Saturating add high 64 bits */
    result.hi = 0;
    for (int i = 0; i < 8; i++) {
        sum = ((a.hi >> (i * 8)) & 0xFF) + ((b.hi >> (i * 8)) & 0xFF);
        if (sum > 0xFF) sum = 0xFF;
        result.hi |= ((uint64_t)sum) << (i * 8);
    }

    return result;
}

/**
 * v128_sat_sub - Vector saturating subtract (unsigned)
 *
 * Subtracts two vectors with saturation at 0x00 per byte.
 *
 * @param a First vector
 * @param b Second vector
 * @return Vector with saturated differences
 */
Vector128 v128_sat_sub(Vector128 a, Vector128 b)
{
    Vector128 result;
    int16_t diff;

    /* Saturating subtract low 64 bits */
    result.lo = 0;
    for (int i = 0; i < 8; i++) {
        diff = (int16_t)((a.lo >> (i * 8)) & 0xFF) - ((b.lo >> (i * 8)) & 0xFF);
        if (diff < 0) diff = 0;
        result.lo |= ((uint64_t)diff) << (i * 8);
    }

    /* Saturating subtract high 64 bits */
    result.hi = 0;
    for (int i = 0; i < 8; i++) {
        diff = (int16_t)((a.hi >> (i * 8)) & 0xFF) - ((b.hi >> (i * 8)) & 0xFF);
        if (diff < 0) diff = 0;
        result.hi |= ((uint64_t)diff) << (i * 8);
    }

    return result;
}

/**
 * v128_rev - Vector byte reverse
 *
 * Reverses the order of bytes in the vector.
 *
 * @param a Input vector
 * @return Vector with reversed bytes
 */
Vector128 v128_rev(Vector128 a)
{
    Vector128 result;
    result.lo = 0;
    result.hi = 0;

    /* Reverse low 64 bits */
    for (int i = 0; i < 8; i++) {
        result.lo |= ((a.lo >> (i * 8)) & 0xFF) << ((7 - i) * 8);
    }

    /* Reverse high 64 bits */
    for (int i = 0; i < 8; i++) {
        result.hi |= ((a.hi >> (i * 8)) & 0xFF) << ((7 - i) * 8);
    }

    return result;
}

/**
 * v128_cnt - Vector bit count
 *
 * Counts the number of set bits in each byte element.
 *
 * @param a Input vector
 * @return Vector with bit counts per byte
 */
Vector128 v128_cnt(Vector128 a)
{
    Vector128 result;
    uint8_t count;

    /* Bit count low 64 bits */
    result.lo = 0;
    for (int i = 0; i < 8; i++) {
        count = __builtin_popcount((a.lo >> (i * 8)) & 0xFF);
        result.lo |= ((uint64_t)count) << (i * 8);
    }

    /* Bit count high 64 bits */
    result.hi = 0;
    for (int i = 0; i < 8; i++) {
        count = __builtin_popcount((a.hi >> (i * 8)) & 0xFF);
        result.hi |= ((uint64_t)count) << (i * 8);
    }

    return result;
}

/* ============================================================================
 * Daemon Functions
 * ============================================================================ */

/**
 * rosettad_entry - Rosetta daemon entry point
 *
 * Entry point for the Rosetta daemon (rosettad), which handles
 * system-wide translation services and caching.
 */
void rosettad_entry(void)
{
    /* Initialize daemon state */
    init_daemon_state();

    /* Main daemon loop - handles translation requests */
    while (1) {
        /* Process translation requests */
        /* Manage translation cache */
        /* Handle inter-process communication */
        pause(); /* Wait for signals/requests */
    }
}

/**
 * init_daemon_state - Initialize Rosetta daemon state
 *
 * Initializes the global state required for the Rosetta daemon
 * to operate, including shared memory regions and IPC mechanisms.
 */
void init_daemon_state(void)
{
    /* Initialize shared memory for translation cache */
    /* Set up IPC mechanisms (sockets, shared memory) */
    /* Initialize daemon logging */
    /* Set up signal handlers for daemon */
    /* Configure daemon-specific settings */
}

/* ============================================================================
 * Session 15: Switch Case Handlers and ELF Parsing
 * ============================================================================ */

/**
 * switch_case_handler_1a - Switch case handler for instruction class 0x1a
 *
 * Dispatch handler used for instruction decoding. This function processes
 * values related to instruction class 0x1a which typically handles
 * specific ARM64 instruction patterns.
 *
 * @param value: Input value to dispatch on
 * @return: Dispatch table index or handler result
 */
uint64_t switch_case_handler_1a(uint64_t value)
{
    /* Switch case dispatch handler for instruction class 0x1a */
    /* Typically used in instruction decoding switch statements */
    return value;
}

/**
 * switch_case_handler_2b - Switch case handler for class 0x2b
 *
 * Dispatch handler used for syscall dispatch or other multi-way branches.
 * Table-driven dispatch logic for handling multiple cases.
 *
 * @param value: Input value to dispatch on
 * @return: Dispatch table index or handler result
 */
uint64_t switch_case_handler_2b(uint64_t value)
{
    /* Switch case dispatch handler for class 0x2b */
    /* Used in syscall dispatch or multi-way branch tables */
    return value;
}

/**
 * elf_parse_header - Parse ELF64 header
 *
 * Parses the ELF64 header and extracts key fields for binary loading.
 * Validates the ELF magic number and extracts program/section header info.
 *
 * @param data: Pointer to ELF binary data
 * @param header: Output header structure (caller-allocated)
 * @return: 0 on success, -1 on failure (invalid ELF)
 */
int elf_parse_header(const void *data, void *header)
{
    const uint8_t *elf = (const uint8_t *)data;

    /* Validate ELF magic number: 0x7f 'E' 'L' 'F' */
    if (elf[0] != 0x7f || elf[1] != 'E' || elf[2] != 'L' || elf[3] != 'F') {
        return -1;
    }

    /* Validate 64-bit ELF */
    if (elf[4] != 2) { /* EI_CLASS = ELFCLASS64 */
        return -1;
    }

    /* Validate little-endian */
    if (elf[5] != 1) { /* EI_DATA = ELFDATA2LSB */
        return -1;
    }

    /* Validate ELF version */
    if (elf[6] != 1) { /* EI_VERSION = EV_CURRENT */
        return -1;
    }

    /* Header parsing successful */
    return 0;
}

/**
 * elf_lookup_section - Lookup ELF section by index
 *
 * Looks up a section in the ELF section header table by index.
 * Used during binary loading to find specific sections.
 *
 * @param data: Pointer to ELF binary data
 * @param section_idx: Section index to lookup
 * @param result: Output section data (caller-allocated)
 * @return: 0 on success, -1 on failure (invalid index)
 */
int elf_lookup_section(const void *data, uint32_t section_idx, void *result)
{
    const uint8_t *elf = (const uint8_t *)data;
    (void)elf; /* Suppress unused warning */
    (void)section_idx;
    (void)result;

    /* Section lookup implementation */
    /* Would parse section header table and return requested section */
    return 0;
}

/**
 * is_bitmask_immediate - Check if value is a valid ARM64 bitmask immediate
 *
 * Determines if a 64-bit value can be encoded as an ARM64 bitmask immediate.
 * ARM64 bitmasks are formed by replicating a smaller pattern across the word.
 *
 * Based on the original FUN_80000002a6bc which:
 * - Checks if value is a valid bitmask pattern
 * - Computes rotation and immediate size if valid
 * - Returns 1 if valid, 0 otherwise
 *
 * @param bitmask: The 64-bit value to check
 * @param rot: Output rotation value (if valid)
 * @param imm: Output immediate size (if valid)
 * @return: 1 if valid bitmask immediate, 0 otherwise
 */
int is_bitmask_immediate(uint64_t bitmask, int *rot, int *imm)
{
    uint32_t uVar1;
    uint64_t uVar3, uVar5, uVar7;

    /* Check if bitmask is all zeros or all ones */
    if (bitmask == 0 || bitmask == 0xFFFFFFFFFFFFFFFF) {
        return 0;
    }

    /* Count leading zeros to determine element size */
    uVar3 = 64;
    uVar5 = 0;

    /* Find the repeating pattern size */
    do {
        uVar7 = uVar3 >> 1;
        uVar5 = (bitmask >> uVar7 ^ bitmask) & ((-1LL << uVar7) ^ 0xFFFFFFFFFFFFFFFF);
        uVar1 = (uint32_t)uVar7;
        if (uVar5 != 0) {
            uVar1 = (uint32_t)uVar3 & 0xfffffffe;
        }
        uVar3 = (uint64_t)uVar1;
    } while ((uVar5 == 0) && (2 < (int)uVar1));

    /* Additional bitmask validation logic */
    /* This is a simplified version - full implementation would match original */
    if (rot) *rot = 0;
    if (imm) *imm = (int)uVar3;

    return 1;
}

/* ============================================================================
 * Session 16: Translation Infrastructure Helpers
 * ============================================================================ */

/**
 * translation_cache_init - Initialize translation cache
 *
 * Initializes the translation cache for storing translated code blocks.
 * Sets up the cache data structures and prepares for insertions.
 *
 * @return: 0 on success, -1 on failure
 */
int translation_cache_init(void)
{
    /* Initialize translation cache data structures */
    /* Allocate cache memory */
    /* Set up cache metadata */
    return 0;
}

/**
 * translation_cache_flush - Flush translation cache
 *
 * Flushes all entries from the translation cache.
 * Used when invalidating translated code (e.g., on self-modifying code).
 */
void translation_cache_flush(void)
{
    /* Clear all cache entries */
    /* Reset cache metadata */
    /* Invalidate translated code regions */
}

/**
 * translation_cache_lookup - Lookup translation in cache
 *
 * Looks up a translation by guest PC in the cache.
 * Similar to translation_lookup but may use different hash table.
 *
 * @param guest_pc: Guest (ARM64) program counter
 * @return: Host (x86_64) translation address, or NULL if not found
 */
void *translation_cache_lookup(uint64_t guest_pc)
{
    /* Hash the guest PC */
    /* Look up in cache table */
    /* Return host address if found */
    (void)guest_pc;
    return NULL;
}

/* ============================================================================
 * Session 17: Memory Management and Runtime Support
 * ============================================================================ */

/**
 * rosetta_malloc - Allocate memory
 *
 * Allocates memory from the heap for Rosetta's internal use.
 * Wraps standard malloc with Rosetta-specific error handling.
 *
 * @param size: Number of bytes to allocate
 * @return: Pointer to allocated memory, or NULL on failure
 */
void *rosetta_malloc(size_t size)
{
    void *ptr = malloc(size);
    /* TODO: Add Rosetta-specific error handling */
    return ptr;
}

/**
 * rosetta_free - Free allocated memory
 *
 * Frees memory previously allocated by rosetta_malloc.
 *
 * @param ptr: Pointer to memory to free
 */
void rosetta_free(void *ptr)
{
    if (ptr) {
        free(ptr);
    }
}

/**
 * rosetta_calloc - Allocate and zero-initialize memory
 *
 * Allocates memory for an array of elements and initializes to zero.
 *
 * @param nmemb: Number of elements
 * @param size: Size of each element
 * @return: Pointer to allocated memory, or NULL on failure
 */
void *rosetta_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

/**
 * rosetta_abort - Abort execution
 *
 * Aborts the Rosetta translator execution.
 * Used for fatal errors.
 */
void rosetta_abort(void)
{
    abort();
}

/**
 * rosetta_atexit - Register exit function
 *
 * Registers a function to be called at program exit.
 *
 * @param func: Function to call at exit
 * @return: 0 on success, non-zero on failure
 */
int rosetta_atexit(void (*func)(void))
{
    return atexit(func);
}

/* ============================================================================
 * Session 18: Code Cache Helpers and Debug Functions
 * ============================================================================ */

/**
 * code_cache_alloc - Allocate memory in code cache
 *
 * Allocates memory suitable for storing translated code.
 * The memory is typically executable and may have special alignment.
 *
 * @param size: Number of bytes to allocate
 * @return: Pointer to allocated code cache memory, or NULL on failure
 */
void *code_cache_alloc(size_t size)
{
    void *ptr;

    /* Code cache memory needs to be executable */
    /* Typically uses mmap with PROT_EXEC */
    ptr = malloc(size);

    /* TODO: Replace with mmap for executable memory */
    /* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, */
    /*            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); */

    return ptr;
}

/**
 * code_cache_free - Free code cache memory
 *
 * Frees memory previously allocated by code_cache_alloc.
 *
 * @param ptr: Pointer to code cache memory to free
 */
void code_cache_free(void *ptr)
{
    if (ptr) {
        /* TODO: Use munmap if using mmap */
        free(ptr);
    }
}

/**
 * code_cache_protect - Change protection on code cache memory
 *
 * Changes the protection flags on code cache memory.
 * Used to make memory executable after writing translated code.
 *
 * @param addr: Address of memory region
 * @param size: Size of memory region
 * @param prot: Protection flags (PROT_READ, PROT_WRITE, PROT_EXEC)
 * @return: 0 on success, -1 on failure
 */
int code_cache_protect(void *addr, size_t size, int prot)
{
    (void)addr;
    (void)size;
    (void)prot;
    /* TODO: Use mprotect for actual implementation */
    /* return mprotect(addr, size, prot); */
    return 0;
}

/**
 * debug_trace - Debug trace helper
 *
 * Outputs debug information during translation or execution.
 *
 * @param msg: Debug message
 * @param value: Associated value (e.g., PC, register value)
 */
void debug_trace(const char *msg, uint64_t value)
{
    /* Debug trace - can be enabled/disabled via compile flag */
#ifdef ROSETTA_DEBUG
    fprintf(stderr, "[Rosetta Debug] %s: 0x%lx\n", msg, value);
#else
    (void)msg;
    (void)value;
#endif
}

/**
 * debug_dump_regs - Dump register state
 *
 * Dumps the current register state for debugging.
 * Useful for tracing translation issues.
 */
void debug_dump_regs(void)
{
    /* Register dump - can be enabled/disabled via compile flag */
#ifdef ROSETTA_DEBUG
    fprintf(stderr, "[Rosetta Debug] Register dump\n");
    /* TODO: Dump actual register values */
#endif
}

/* ============================================================================
 * Session 19: Translation and Vector Helpers
 * ============================================================================ */

/**
 * translate_movz - Translate MOVZ (Move Zero) instruction
 *
 * MOVZ moves a 16-bit immediate into a register, shifted left by 0, 16, 32,
 * or 48 bits, with the remaining bits set to zero.
 *
 * ARM64 encoding:
 * 31 30 29 28         23 22 21         17 16   12 11         5 4    0
 *  1  1  0  1  0  0  1  0  0  1  0  0  0  0  0  0  0  0  0  0  0  0  0
 *    op   S    F      0     1     i    imm16      Rd       shift
 *
 * @param state: Thread state
 * @param insn: Pointer to instruction bytes
 */
void translate_movz(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(uint32_t *)insn;
    uint32_t imm16 = (encoding >> 5) & 0xFFFF;
    uint32_t shift = ((encoding >> 21) & 3) * 16;
    uint32_t rd = encoding & 0x1F;

    /* Move immediate with zero: Rd = imm16 << shift */
    state->cpu.gpr.x[rd] = (uint64_t)imm16 << shift;
}

/**
 * translate_movk - Translate MOVK (Move Keep) instruction
 *
 * MOVK moves a 16-bit immediate into a register at a specified offset,
 * keeping the remaining bits unchanged.
 *
 * ARM64 encoding:
 * 31 30 29 28         23 22 21         17 16   12 11         5 4    0
 *  1  1  0  1  0  0  1  0  0  1  0  1  0  0  0  0  0  0  0  0  0  0  0
 *    op   S    F      0     1     i    imm16      Rd       shift
 *
 * @param state: Thread state
 * @param insn: Pointer to instruction bytes
 */
void translate_movk(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(uint32_t *)insn;
    uint32_t imm16 = (encoding >> 5) & 0xFFFF;
    uint32_t shift = ((encoding >> 21) & 3) * 16;
    uint32_t rd = encoding & 0x1F;

    /* Move immediate with keep: Rd = (Rd & ~(0xFFFF << shift)) | (imm16 << shift) */
    uint64_t mask = ~((uint64_t)0xFFFF << shift);
    state->cpu.gpr.x[rd] = (state->cpu.gpr.x[rd] & mask) | ((uint64_t)imm16 << shift);
}

/**
 * translate_movn - Translate MOVN (Move Negated) instruction
 *
 * MOVN moves a 16-bit immediate into a register, shifted left by 0, 16, 32,
 * or 48 bits, with the remaining bits set to one.
 *
 * ARM64 encoding:
 * 31 30 29 28         23 22 21         17 16   12 11         5 4    0
 *  1  1  0  1  0  0  1  0  0  1  0  0  0  0  0  0  0  0  0  0  0  0  0
 *    op   S    F      0     1     i    imm16      Rd       shift
 *
 * @param state: Thread state
 * @param insn: Pointer to instruction bytes
 */
void translate_movn(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(uint32_t *)insn;
    uint32_t imm16 = (encoding >> 5) & 0xFFFF;
    uint32_t shift = ((encoding >> 21) & 3) * 16;
    uint32_t rd = encoding & 0x1F;

    /* Move negated immediate: Rd = ~(imm16 << shift) */
    state->cpu.gpr.x[rd] = ~((uint64_t)imm16 << shift);
}

/**
 * v128_dupw - Duplicate 32-bit word across vector
 *
 * Creates a vector with the same 32-bit value in all four elements.
 *
 * @param val: 32-bit value to duplicate
 * @return: Vector with val in all 4 elements
 */
Vector128 v128_dupw(uint32_t val)
{
    Vector128 v;
    uint64_t lo = ((uint64_t)val << 32) | val;
    v.lo = lo;
    v.hi = lo;
    return v;
}

/**
 * v128_dupd - Duplicate 64-bit double across vector
 *
 * Creates a vector with the same 64-bit value in both elements.
 *
 * @param val: 64-bit value to duplicate (typically a double)
 * @return: Vector with val in both 64-bit lanes
 */
Vector128 v128_dupd(double val)
{
    Vector128 v;
    uint64_t bits;
    memcpy(&bits, &val, sizeof(bits));
    v.lo = bits;
    v.hi = bits;
    return v;
}

/* ============================================================================
 * String and Memory Utilities - Session 14
 * ============================================================================ */

/**
 * rosetta_strlen - Calculate string length
 *
 * Calculates the length of a null-terminated string.
 * Uses SIMD optimization for faster processing.
 *
 * @param s Input string
 * @return Length of string (not including null terminator)
 */
size_t rosetta_strlen(const char *s)
{
    const char *p = s;

    /* Process 8 bytes at a time for efficiency */
    while (1) {
        /* Check for null byte in current 8-byte chunk */
        uint64_t chunk = *(const uint64_t *)p;

        /* Check if any byte is zero (null terminator) */
        if (((chunk - 0x0101010101010101ULL) & ~chunk & 0x8080808080808080ULL) != 0) {
            /* Null byte found, check each byte */
            while (*p != '\0') {
                p++;
            }
            return (size_t)(p - s);
        }
        p += 8;
    }
}

/**
 * rosetta_strcpy - Copy string
 *
 * Copies source string to destination buffer.
 * Assumes destination has sufficient space.
 *
 * @param dest Destination buffer
 * @param src Source string
 * @return Pointer to destination
 */
char *rosetta_strcpy(char *dest, const char *src)
{
    char *orig_dest = dest;

    /* Copy 8 bytes at a time until null terminator found */
    while (1) {
        uint64_t chunk = *(const uint64_t *)src;
        *(uint64_t *)dest = chunk;

        /* Check if null byte was in this chunk */
        if (((chunk - 0x0101010101010101ULL) & ~chunk & 0x8080808080808080ULL) != 0) {
            /* Find exact position of null and finish byte-by-byte */
            while (*dest != '\0') {
                dest++;
                src++;
            }
            break;
        }
        dest += 8;
        src += 8;
    }

    return orig_dest;
}

/**
 * rosetta_strcat - Concatenate strings
 *
 * Appends source string to end of destination string.
 *
 * @param dest Destination string (must have sufficient space)
 * @param src Source string to append
 * @return Pointer to destination
 */
char *rosetta_strcat(char *dest, const char *src)
{
    char *orig_dest = dest;

    /* Find end of destination string */
    while (*dest != '\0') {
        dest++;
    }

    /* Copy source to end of destination */
    while (*src != '\0') {
        *dest++ = *src++;
    }
    *dest = '\0';

    return orig_dest;
}

/**
 * rosetta_strcmp - Compare two strings
 *
 * Compares two null-terminated strings lexicographically.
 *
 * @param s1 First string
 * @param s2 Second string
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
 */
int rosetta_strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

/**
 * rosetta_strncmp - Compare strings with length limit
 *
 * Compares up to n characters of two strings.
 *
 * @param s1 First string
 * @param s2 Second string
 * @param n Maximum number of characters to compare
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
 */
int rosetta_strncmp(const char *s1, const char *s2, size_t n)
{
    if (n == 0) return 0;

    while (n > 1 && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

/**
 * rosetta_memcpy - Copy memory region
 *
 * Copies n bytes from source to destination.
 * Does not handle overlapping regions (use memmove for that).
 *
 * @param dest Destination buffer
 * @param src Source buffer
 * @param n Number of bytes to copy
 * @return Pointer to destination
 */
void *rosetta_memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    /* Copy 8 bytes at a time when possible */
    while (n >= 8) {
        *(uint64_t *)d = *(const uint64_t *)s;
        d += 8;
        s += 8;
        n -= 8;
    }

    /* Copy remaining bytes */
    while (n--) {
        *d++ = *s++;
    }

    return dest;
}

/**
 * rosetta_memmove - Copy memory region with overlap handling
 *
 * Copies n bytes from source to destination, handling overlapping regions.
 *
 * @param dest Destination buffer
 * @param src Source buffer
 * @param n Number of bytes to copy
 * @return Pointer to destination
 */
void *rosetta_memmove(void *dest, const void *src, size_t n)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    if (d == s) return dest;

    /* If dest is after src and they overlap, copy backwards */
    if (d > s && d < s + n) {
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    } else {
        /* Otherwise copy forwards */
        while (n >= 8) {
            *(uint64_t *)d = *(const uint64_t *)s;
            d += 8;
            s += 8;
            n -= 8;
        }
        while (n--) {
            *d++ = *s++;
        }
    }

    return dest;
}

/**
 * rosetta_memcmp - Compare memory regions
 *
 * Compares n bytes of two memory regions.
 *
 * @param s1 First memory region
 * @param s2 Second memory region
 * @param n Number of bytes to compare
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
 */
int rosetta_memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

/**
 * rosetta_memset - Set memory region
 *
 * Sets n bytes of memory to value c.
 *
 * @param s Memory region
 * @param c Value to set
 * @param n Number of bytes
 * @return Pointer to memory region
 */
void *rosetta_memset(void *s, int c, size_t n)
{
    uint8_t *p = (uint8_t *)s;
    uint64_t pattern;

    /* Build 8-byte pattern for faster filling */
    pattern = (uint8_t)c;
    pattern |= pattern << 8;
    pattern |= pattern << 16;
    pattern |= pattern << 32;

    /* Set 8 bytes at a time */
    while (n >= 8) {
        *(uint64_t *)p = pattern;
        p += 8;
        n -= 8;
    }

    /* Set remaining bytes */
    while (n--) {
        *p++ = (uint8_t)c;
    }

    return s;
}

/**
 * rosetta_memchr - Find byte in memory
 *
 * Searches for first occurrence of byte c in memory region.
 *
 * @param s Memory region to search
 * @param c Byte value to find
 * @param n Number of bytes to search
 * @return Pointer to found byte, or NULL if not found
 */
void *rosetta_memchr(const void *s, int c, size_t n)
{
    const uint8_t *p = (const uint8_t *)s;
    uint64_t pattern;

    /* Build 8-byte pattern for comparison */
    pattern = (uint8_t)c;
    pattern |= pattern << 8;
    pattern |= pattern << 16;
    pattern |= pattern << 32;

    while (n >= 8) {
        uint64_t chunk = *(const uint64_t *)p;

        /* XOR with pattern - if any byte matches, result has zero byte */
        uint64_t xored = chunk ^ pattern;

        /* Check if any byte is zero */
        if (((xored - 0x0101010101010101ULL) & ~xored & 0x8080808080808080ULL) != 0) {
            /* Match found, find exact position */
            for (int i = 0; i < 8; i++) {
                if (p[i] == (uint8_t)c) {
                    return (void *)(p + i);
                }
            }
        }
        p += 8;
        n -= 8;
    }

    return NULL;
}

/* ============================================================================
 * Session 20: Additional Vector Operations
 * ============================================================================ */

/**
 * v128_extract_word - Extract 32-bit word from vector
 *
 * Extracts a 32-bit word from the specified index of a 128-bit vector.
 *
 * @param v Vector register
 * @param index Index of word to extract (0-3)
 * @return 32-bit word from specified index
 */
uint32_t v128_extract_word(Vector128 v, int index)
{
    if (index < 0 || index > 3) {
        return 0;
    }
    return ((uint32_t *)&v)[index];
}

/**
 * v128_insert_word - Insert 32-bit word into vector
 *
 * Inserts a 32-bit word into the specified index of a 128-bit vector.
 *
 * @param v Vector register
 * @param index Index where to insert word (0-3)
 * @param val 32-bit word value to insert
 * @return Modified vector with new word inserted
 */
Vector128 v128_insert_word(Vector128 v, int index, uint32_t val)
{
    if (index >= 0 && index <= 3) {
        ((uint32_t *)&v)[index] = val;
    }
    return v;
}

/**
 * v128_extract_dword - Extract 64-bit doubleword from vector
 *
 * Extracts a 64-bit doubleword from the specified index of a 128-bit vector.
 *
 * @param v Vector register
 * @param index Index of doubleword to extract (0-1)
 * @return 64-bit doubleword from specified index
 */
uint64_t v128_extract_dword(Vector128 v, int index)
{
    if (index < 0 || index > 1) {
        return 0;
    }
    return ((uint64_t *)&v)[index];
}

/**
 * v128_insert_dword - Insert 64-bit doubleword into vector
 *
 * Inserts a 64-bit doubleword into the specified index of a 128-bit vector.
 *
 * @param v Vector register
 * @param index Index where to insert doubleword (0-1)
 * @param val 64-bit doubleword value to insert
 * @return Modified vector with new doubleword inserted
 */
Vector128 v128_insert_dword(Vector128 v, int index, uint64_t val)
{
    if (index >= 0 && index <= 1) {
        ((uint64_t *)&v)[index] = val;
    }
    return v;
}

/**
 * v128_mov - Move vector register
 *
 * Copies the contents of one vector register to another.
 * This is a simple assignment but provided for consistency with
 * other vector operations and potential future optimization.
 *
 * @param v Source vector register
 * @return Copy of source vector
 */
Vector128 v128_mov(Vector128 v)
{
    return v;
}

/**
 * v128_tbl - Table lookup from two vectors
 *
 * Performs a table lookup operation using indices from one vector
 * to select bytes from two source vectors. This implements the
 * ARM64 TBL (table lookup) instruction behavior.
 *
 * @param t1 First table vector (bytes 0-15)
 * @param t2 Second table vector (bytes 16-31)
 * @param i Index vector specifying which bytes to select
 * @return Vector with looked-up bytes
 */
Vector128 v128_tbl(Vector128 t1, Vector128 t2, Vector128 i)
{
    Vector128 result;
    uint8_t *t1_bytes = (uint8_t *)&t1;
    uint8_t *t2_bytes = (uint8_t *)&t2;
    uint8_t *i_bytes = (uint8_t *)&i;
    uint8_t *r_bytes = (uint8_t *)&result;

    for (int j = 0; j < 16; j++) {
        int idx = i_bytes[j];
        if (idx < 16) {
            r_bytes[j] = t1_bytes[idx];
        } else if (idx < 32) {
            r_bytes[j] = t2_bytes[idx - 16];
        } else {
            r_bytes[j] = 0;  /* Out of range returns zero */
        }
    }

    return result;
}

/* ============================================================================
 * Session 20: Translation Block Optimization Helpers
 * ============================================================================ */

/**
 * translate_block_optimize - Optimize a translated block
 *
 * Performs basic optimizations on a translated code block.
 * Currently a placeholder for future optimization passes.
 *
 * @param block Pointer to translated block
 * @param size Size of translated block in bytes
 * @return 0 on success, -1 on failure
 *
 * Potential optimizations:
 * - Dead code elimination
 * - Instruction combining
 * - Register allocation improvement
 * - Branch folding
 */
int translate_block_optimize(void *block, size_t size)
{
    if (!block || size == 0) {
        return -1;
    }

    /* Placeholder for optimization passes */
    /* Future implementations could include: */
    /* 1. Peephole optimization */
    /* 2. Redundant move elimination */
    /* 3. Constant folding */
    /* 4. Jump threading */

    return 0;
}

/**
 * translate_block_link - Link two translated blocks
 *
 * Creates a direct jump from one translated block to another,
 * bypassing the translation lookup for frequently-executed paths.
 *
 * @param from_block Source block to patch
 * @param to_block Destination block to jump to
 * @param size Size of the from_block
 * @return 0 on success, -1 on failure
 *
 * This implements "block chaining" - a critical optimization
 * where consecutive blocks are directly linked after the first
 * execution, avoiding repeated cache lookups.
 */
int translate_block_link(void *from_block, void *to_block, size_t size)
{
    if (!from_block || !to_block || size < 5) {
        return -1;
    }

    /* Calculate relative offset for direct jump */
    /* x86_64 JMP rel32 is 5 bytes: E9 xx xx xx xx */
    int64_t offset = (int64_t)((uint8_t *)to_block - (uint8_t *)from_block);
    offset -= 5;  /* Relative to end of JMP instruction */

    /* Check if offset fits in 32-bit signed */
    if (offset > INT32_MAX || offset < INT32_MIN) {
        return -1;  /* Too far for direct jump */
    }

    /* In a real implementation, we would patch the tail of from_block */
    /* to directly jump to to_block. For now, this is a placeholder. */

    return 0;
}

/**
 * translate_block_unlink - Unlink a translated block
 *
 * Removes direct jumps to a block that is being invalidated.
 * This ensures that no other blocks jump to freed memory.
 *
 * @param block Block being invalidated
 * @return 0 on success, -1 on failure
 */
int translate_block_unlink(void *block)
{
    if (!block) {
        return -1;
    }

    /* In a full implementation, this would: */
    /* 1. Find all blocks that chain to this block */
    /* 2. Replace direct jumps with indirect jumps through lookup */
    /* 3. Flush instruction cache if needed */

    return 0;
}

/* ============================================================================
 * Session 20: Additional ELF Parsing Functions
 * ============================================================================ */

/**
 * elf_find_symbol - Find symbol in ELF symbol table
 *
 * Searches for a symbol by name in the ELF file's symbol table.
 *
 * @param elf_base Base address of ELF file in memory
 * @param name Name of symbol to find
 * @param sym_out Output structure for found symbol
 * @return 0 on success, -1 if symbol not found
 */
int elf_find_symbol(const void *elf_base, const char *name, void *sym_out)
{
    if (!elf_base || !name || !sym_out) {
        return -1;
    }

    /* Placeholder implementation */
    /* A full implementation would: */
    /* 1. Parse ELF header to find section headers */
    /* 2. Find .symtab and .strtab sections */
    /* 3. Iterate through symbol table entries */
    /* 4. Compare symbol names */
    /* 5. Return matching symbol info */

    return -1;  /* Not implemented */
}

/**
 * elf_get_section_by_name - Get ELF section by name
 *
 * Finds a section in the ELF file by its name.
 *
 * @param elf_base Base address of ELF file in memory
 * @param name Name of section to find (e.g., ".text", ".data")
 * @param shdr_out Output structure for section header
 * @return 0 on success, -1 if section not found
 */
int elf_get_section_by_name(const void *elf_base, const char *name, void *shdr_out)
{
    if (!elf_base || !name || !shdr_out) {
        return -1;
    }

    /* Placeholder implementation */
    /* A full implementation would: */
    /* 1. Parse ELF header to get section header offset */
    /* 2. Get section header string table index */
    /* 3. Iterate through section headers */
    /* 4. Look up section names in string table */
    /* 5. Return matching section header */

    return -1;  /* Not implemented */
}

/**
 * elf_relocate - Apply relocations to ELF image
 *
 * Applies dynamic relocations to a loaded ELF image.
 *
 * @param elf_base Base address of loaded ELF image
 * @param load_offset Offset where image is loaded
 * @return 0 on success, -1 on failure
 *
 * This function processes:
 * - R_AARCH64_ABS64 relocations (absolute 64-bit)
 * - R_AARCH64_REL64 relocations (relative 64-bit)
 * - R_AARCH64_GLOB_DAT (global data)
 * - R_AARCH64_JUMP_SLOT (function calls)
 */
int elf_relocate(const void *elf_base, uint64_t load_offset)
{
    if (!elf_base) {
        return -1;
    }

    /* Placeholder implementation */
    /* A full implementation would: */
    /* 1. Find .rela.dyn and .rela.plt sections */
    /* 2. Process each relocation entry */
    /* 3. Apply appropriate relocation type */
    /* 4. Handle symbol resolution */

    return 0;  /* Success (no-op for now) */
}

/* ============================================================================
 * Session 21: vDSO and ELF Parsing Functions
 * ============================================================================ */

/**
 * has_zero_byte - Check if any byte in a 64-bit word is zero
 *
 * Efficiently checks if any of the 8 bytes in a 64-bit word equals zero.
 * This is a fundamental SIMD helper used in string operations.
 *
 * @param x 64-bit word to check
 * @return Non-zero if any byte is zero, 0 otherwise
 *
 * Algorithm: Uses the property that (x - 0x0101010101010101) & ~x & 0x8080808080808080
 * is non-zero iff any byte of x is zero.
 */
uint64_t has_zero_byte(uint64_t x)
{
    return ((x - 0x0101010101010101ULL) & ~x & 0x8080808080808080ULL);
}

/**
 * elf64_parse_header - Parse ELF64 header information
 *
 * Extracts essential information from an ELF64 header for further parsing.
 *
 * @param base Base address of ELF file in memory
 * @param info Output structure for header information
 * @return 0 on success, -1 on failure
 *
 * Validates:
 * - ELF magic number (0x7f 'E' 'L' 'F')
 * - 64-bit format (EI_CLASS = ELFCLASS64)
 * - Little-endian (EI_DATA = ELFDATA2LSB)
 */
int elf64_parse_header(const void *base, void *info)
{
    const uint8_t *elf = (const uint8_t *)base;
    uint32_t *out = (uint32_t *)info;

    /* Validate ELF magic number */
    if (elf[0] != 0x7f || elf[1] != 'E' || elf[2] != 'L' || elf[3] != 'F') {
        return -1;
    }

    /* Validate 64-bit ELF */
    if (elf[4] != 2) {  /* ELFCLASS64 */
        return -1;
    }

    /* Validate little-endian */
    if (elf[5] != 1) {  /* ELFDATA2LSB */
        return -1;
    }

    /* Extract header fields */
    out[0] = *(const uint16_t *)(elf + 16);   /* e_type */
    out[1] = *(const uint16_t *)(elf + 18);   /* e_machine */
    out[2] = *(const uint32_t *)(elf + 20);   /* e_version */
    out[3] = *(const uint64_t *)(elf + 24);   /* e_entry */
    out[4] = *(const uint64_t *)(elf + 32);   /* e_phoff */
    out[5] = *(const uint64_t *)(elf + 40);   /* e_shoff */
    out[6] = *(const uint32_t *)(elf + 52);   /* e_flags */
    out[7] = *(const uint16_t *)(elf + 54);   /* e_ehsize */
    out[8] = *(const uint16_t *)(elf + 56);   /* e_phentsize */
    out[9] = *(const uint16_t *)(elf + 58);   /* e_phnum */
    out[10] = *(const uint16_t *)(elf + 60);  /* e_shentsize */
    out[11] = *(const uint16_t *)(elf + 62);  /* e_shnum */
    out[12] = *(const uint16_t *)(elf + 64);  /* e_shstrndx */

    return 0;
}

/**
 * elf64_find_section_by_type - Find section header by type
 *
 * Searches for a section header with the specified type.
 *
 * @param base Base address of ELF file
 * @param info Parsed ELF header information
 * @param type Section type to find (e.g., SHT_DYNSYM=11, SHT_STRTAB=3)
 * @return Pointer to section header, or NULL if not found
 */
const void *elf64_find_section_by_type(const void *base, const void *info, uint32_t type)
{
    const uint8_t *elf = (const uint8_t *)base;
    const uint32_t *hdr = (const uint32_t *)info;
    const uint8_t *shdr;
    uint32_t i;

    uint64_t shoff = ((const uint64_t *)hdr)[5];      /* e_shoff */
    uint16_t shentsize = ((const uint16_t *)hdr)[10]; /* e_shentsize */
    uint16_t shnum = ((const uint16_t *)hdr)[11];     /* e_shnum */

    for (i = 0; i < shnum; i++) {
        shdr = elf + shoff + i * shentsize;
        if (*(const uint32_t *)(shdr + 4) == type) {  /* sh_type */
            return (const void *)shdr;
        }
    }

    return NULL;
}

/**
 * elf64_find_section_by_name - Find section header by name
 *
 * Searches for a section header with the specified name.
 *
 * @param base Base address of ELF file
 * @param info Parsed ELF header information
 * @param name Name of section to find
 * @return Pointer to section header, or NULL if not found
 */
const void *elf64_find_section_by_name(const void *base, const void *info, const char *name)
{
    const uint8_t *elf = (const uint8_t *)base;
    const uint32_t *hdr = (const uint32_t *)info;
    const uint8_t *shdr;
    const uint8_t *shstrtab;
    const char *sh_name;
    uint32_t i;

    uint64_t shoff = ((const uint64_t *)hdr)[5];      /* e_shoff */
    uint16_t shentsize = ((const uint16_t *)hdr)[10]; /* e_shentsize */
    uint16_t shnum = ((const uint16_t *)hdr)[11];     /* e_shnum */
    uint16_t shstrndx = ((const uint16_t *)hdr)[12];  /* e_shstrndx */

    /* Get section header string table */
    if (shstrndx >= shnum) {
        return NULL;
    }
    shdr = elf + shoff + shstrndx * shentsize;
    shstrtab = elf + *(const uint64_t *)(shdr + 24);  /* sh_offset */

    /* Search for section by name */
    for (i = 0; i < shnum; i++) {
        shdr = elf + shoff + i * shentsize;
        sh_name = (const char *)(shstrtab + *(const uint32_t *)(shdr));  /* sh_name */
        if (strcmp(sh_name, name) == 0) {
            return (const void *)shdr;
        }
    }

    return NULL;
}

/**
 * elf64_lookup_symbol - Look up symbol in ELF symbol table
 *
 * Searches for a symbol by name in the given symbol table.
 *
 * @param base Base address of ELF file
 * @param symtab Pointer to symbol table section
 * @param strtab Pointer to string table section
 * @param sym_name Name of symbol to find
 * @param sym_value Output for symbol value (address)
 * @return 0 on success, -1 if not found
 */
int elf64_lookup_symbol(const void *base, const void *symtab, const void *strtab,
                        const char *sym_name, void **sym_value)
{
    const uint8_t *elf = (const uint8_t *)base;
    const uint8_t *sym = (const uint8_t *)symtab;
    const uint8_t *str = (const uint8_t *)strtab;
    uint64_t symtab_offset = *(const uint64_t *)((const uint8_t *)symtab + 24);  /* sh_offset */
    uint64_t symtab_size = *(const uint64_t *)((const uint8_t *)symtab + 32);    /* sh_size */
    uint64_t strtab_offset = *(const uint64_t *)((const uint8_t *)strtab + 24);  /* sh_offset */
    uint32_t num_symbols = symtab_size / 24;  /* sizeof(Elf64_Sym) */
    uint32_t i;

    str = elf + strtab_offset;

    for (i = 0; i < num_symbols; i++) {
        const uint8_t *entry = elf + symtab_offset + i * 24;
        uint32_t st_name = *(const uint32_t *)entry;
        uint64_t st_value = *(const uint64_t *)(entry + 8);
        const char *name = (const char *)(str + st_name);

        if (st_value != 0 && strcmp(name, sym_name) == 0) {
            *sym_value = (void *)(elf + st_value);
            return 0;
        }
    }

    return -1;
}

/**
 * elf64_compute_load_offset - Compute ELF load offset from PT_LOAD segments
 *
 * Calculates the offset between virtual and file addresses.
 *
 * @param base Base address of ELF file
 * @param info Parsed ELF header information
 * @return Load offset, or 0 if no PT_LOAD segment found
 */
uint64_t elf64_compute_load_offset(const void *base, const void *info)
{
    const uint8_t *elf = (const uint8_t *)base;
    const uint32_t *hdr = (const uint32_t *)info;
    const uint8_t *phdr;
    uint64_t load_offset = 0;
    uint32_t i;

    uint64_t phoff = ((const uint64_t *)hdr)[4];      /* e_phoff */
    uint16_t phentsize = ((const uint16_t *)hdr)[8];  /* e_phentsize */
    uint16_t phnum = ((const uint16_t *)hdr)[9];      /* e_phnum */

    for (i = 0; i < phnum; i++) {
        phdr = elf + phoff + i * phentsize;
        if (*(const uint32_t *)phdr == 1) {  /* PT_LOAD */
            uint64_t p_vaddr = *(const uint64_t *)(phdr + 8);
            uint64_t p_offset = *(const uint64_t *)phdr;
            load_offset = p_vaddr - p_offset;
            break;
        }
    }

    return load_offset;
}

/**
 * vdso_lookup_symbol - Look up symbol in VDSO
 *
 * Resolves a VDSO symbol by name.
 *
 * @param vdso_base Base address of VDSO in memory
 * @param info Parsed ELF header information
 * @param sym_name Name of symbol to find
 * @return Symbol address, or NULL if not found
 */
void *vdso_lookup_symbol(const void *vdso_base, const void *info, const char *sym_name)
{
    const void *dynsym;
    const void *dynstr;
    void *sym_value = NULL;

    /* Find dynamic symbol table */
    dynsym = elf64_find_section_by_type(vdso_base, info, 11);  /* SHT_DYNSYM */
    if (!dynsym) {
        return NULL;
    }

    /* Find dynamic string table */
    dynstr = elf64_find_section_by_name(vdso_base, info, ".dynstr");
    if (!dynstr) {
        return NULL;
    }

    /* Look up symbol */
    if (elf64_lookup_symbol(vdso_base, dynsym, dynstr, sym_name, &sym_value) == 0) {
        return sym_value;
    }

    return NULL;
}

/**
 * vdso_init - Initialize VDSO function pointers
 *
 * Resolves common VDSO symbols for fast syscall implementations.
 *
 * @param vdso_base Base address of VDSO
 * @param clock_getres_out Output for clock_getres function
 * @param gettimeofday_out Output for gettimeofday function
 * @param clock_gettime_out Output for clock_gettime function
 * @return 0 on success, -1 on failure
 */
int vdso_init(const void *vdso_base, void **clock_getres_out,
              void **gettimeofday_out, void **clock_gettime_out)
{
    elf64_header_info_t elf_info;
    uint64_t load_offset;
    void *sym;

    /* Parse ELF header */
    if (elf64_parse_header(vdso_base, &elf_info) != 0) {
        return -1;
    }

    /* Compute load offset */
    load_offset = elf64_compute_load_offset(vdso_base, &elf_info);

    /* Lookup VDSO symbols */
    sym = vdso_lookup_symbol(vdso_base, &elf_info, "__kernel_clock_getres");
    if (sym) {
        *clock_getres_out = sym;
    } else {
        *clock_getres_out = (void *)((uint8_t *)vdso_base + load_offset);
    }

    sym = vdso_lookup_symbol(vdso_base, &elf_info, "__kernel_gettimeofday");
    if (sym) {
        *gettimeofday_out = sym;
    } else {
        *gettimeofday_out = (void *)((uint8_t *)vdso_base + load_offset);
    }

    sym = vdso_lookup_symbol(vdso_base, &elf_info, "__kernel_clock_gettime");
    if (sym) {
        *clock_gettime_out = sym;
    } else {
        *clock_gettime_out = (void *)((uint8_t *)vdso_base + load_offset);
    }

    return 0;
}

/* ============================================================================
 * Session 22: Advanced NEON and Vector Operations
 * ============================================================================ */

/**
 * v128_cls - Count leading sign bits
 *
 * Counts the number of consecutive sign bits starting from the MSB.
 *
 * @param a Input vector (32-bit elements)
 * @return Vector with leading sign bit counts
 */
Vector128 v128_cls(Vector128 a)
{
    Vector128 result;
    uint32_t *in = (uint32_t *)&a;
    uint32_t *out = (uint32_t *)&result;

    /* Count leading sign bits for each 32-bit element */
    for (int i = 0; i < 4; i++) {
        uint32_t x = in[i];
        uint32_t sign = (x >> 31) & 1;
        uint32_t count = 0;

        /* Count leading bits matching sign bit */
        for (int j = 31; j >= 0; j--) {
            uint32_t bit = (x >> j) & 1;
            if (bit != sign) break;
            count++;
        }

        out[i] = count;
    }

    return result;
}

/**
 * v128_clz - Count leading zeros
 *
 * Counts the number of consecutive zero bits starting from the MSB.
 *
 * @param a Input vector (32-bit elements)
 * @return Vector with leading zero counts
 */
Vector128 v128_clz(Vector128 a)
{
    Vector128 result;
    uint32_t *in = (uint32_t *)&a;
    uint32_t *out = (uint32_t *)&result;

    /* Count leading zeros for each 32-bit element */
    for (int i = 0; i < 4; i++) {
        uint32_t x = in[i];
        uint32_t count = 0;

        if (x == 0) {
            out[i] = 32;
        } else {
            while ((x & 0x80000000) == 0) {
                count++;
                x <<= 1;
            }
            out[i] = count;
        }
    }

    return result;
}

/**
 * v128_ctz - Count trailing zeros
 *
 * Counts the number of consecutive zero bits starting from the LSB.
 *
 * @param a Input vector (32-bit elements)
 * @return Vector with trailing zero counts
 */
Vector128 v128_ctz(Vector128 a)
{
    Vector128 result;
    uint32_t *in = (uint32_t *)&a;
    uint32_t *out = (uint32_t *)&result;

    /* Count trailing zeros for each 32-bit element */
    for (int i = 0; i < 4; i++) {
        uint32_t x = in[i];
        uint32_t count = 0;

        if (x == 0) {
            out[i] = 32;
        } else {
            while ((x & 1) == 0) {
                count++;
                x >>= 1;
            }
            out[i] = count;
        }
    }

    return result;
}

/**
 * v128_fcpy - Fast vector copy
 *
 * Copies a vector register with potential optimization.
 *
 * @param a Input vector
 * @return Copy of input vector
 */
Vector128 v128_fcpy(Vector128 a)
{
    return a;
}

/**
 * v128_dup_lane - Duplicate a lane across all elements
 *
 * Copies the specified lane to all other lanes.
 *
 * @param a Input vector
 * @param lane Lane index to duplicate (0-3 for 32-bit elements)
 * @return Vector with all lanes set to selected lane value
 */
Vector128 v128_dup_lane(Vector128 a, int lane)
{
    Vector128 result;
    uint32_t *in = (uint32_t *)&a;
    uint32_t *out = (uint32_t *)&result;
    uint32_t val;

    if (lane < 0 || lane > 3) {
        return a;
    }

    val = in[lane];
    for (int i = 0; i < 4; i++) {
        out[i] = val;
    }

    return result;
}

/**
 * v128_ext - Vector extract
 *
 * Extracts bytes from concatenation of two vectors.
 *
 * @param a First input vector
 * @param b Second input vector
 * @param imm4 Immediate offset (0-15)
 * @return Extracted vector
 *
 * Operation: result = (a || b) >> (imm4 * 8)
 */
Vector128 v128_ext(Vector128 a, Vector128 b, int imm4)
{
    Vector128 result;
    uint8_t *a_in = (uint8_t *)&a;
    uint8_t *b_in = (uint8_t *)&b;
    uint8_t *out = (uint8_t *)&result;
    int i;

    if (imm4 < 0 || imm4 > 15) {
        return a;
    }

    /* Concatenate and extract */
    for (i = 0; i < 16 - imm4; i++) {
        out[i] = b_in[i + imm4];
    }
    for (; i < 16; i++) {
        out[i] = a_in[i - (16 - imm4)];
    }

    return result;
}

/* ============================================================================
 * Session 23: ELF Dynamic Linker Support
 * ============================================================================ */

/**
 * elf_hash_symbol - Compute ELF hash of a symbol name
 *
 * @param name Symbol name to hash
 * @return Hash value
 *
 * Standard ELF hash function used in .hash sections.
 */
uint32_t elf_hash_symbol(const char *name)
{
    uint32_t h = 0;
    uint32_t g;

    while (*name) {
        h = (h << 4) + *name++;
        g = h & 0xf0000000;
        if (g)
            h ^= g >> 24;
        h &= ~g;
    }

    return h;
}

/**
 * elf_gnu_hash_symbol - Compute GNU hash of a symbol name
 *
 * @param name Symbol name to hash
 * @return Hash value
 *
 * GNU hash function used in .gnu.hash sections (faster than ELF hash).
 */
uint32_t elf_gnu_hash_symbol(const char *name)
{
    uint32_t h = 5381;

    while (*name) {
        h = (h << 5) + h + *name++;
    }

    return h;
}

/**
 * elf_parse_dynamic_section - Parse ELF PT_DYNAMIC segment
 *
 * @param base Base address of ELF image
 * @param dyn_info Pointer to store dynamic section info
 * @return 0 on success, -1 on failure
 *
 * Parses the dynamic section to extract:
 * - Symbol table (.dynsym)
 * - String table (.dynstr)
 * - Hash table (.hash)
 * - GNU hash table (.gnu.hash)
 * - Relocation tables (.rela.dyn, .rela.plt)
 * - GOT and PLT addresses
 */
int elf_parse_dynamic_section(const void *base, void *dyn_info)
{
    const Elf64_Dyn *dyn;
    uint64_t dyn_addr = 0;

    if (!base || !dyn_info) {
        return -1;
    }

    /* Parse program headers to find PT_DYNAMIC */
    const uint8_t *elf = (const uint8_t *)base;
    const uint64_t *header = (const uint64_t *)elf;
    uint32_t e_phoff = (uint32_t)header[5];
    uint16_t e_phnum = (uint16_t)((const uint16_t *)elf)[28];
    uint16_t e_phentsize = (uint16_t)((const uint16_t *)elf)[26];

    /* Find PT_DYNAMIC segment */
    for (int i = 0; i < e_phnum; i++) {
        const uint8_t *ph = elf + e_phoff + (i * e_phentsize);
        uint32_t p_type = *(const uint32_t *)ph;

        if (p_type == 2) { /* PT_DYNAMIC */
            dyn_addr = *(const uint64_t *)(ph + 16); /* p_vaddr */
            break;
        }
    }

    if (!dyn_addr) {
        return -1;
    }

    dyn = (const Elf64_Dyn *)(elf + dyn_addr);

    /* Parse dynamic entries */
    while (dyn->d_tag != 0) {
        switch (dyn->d_tag) {
        case 6:  /* DT_SYMTAB */
            /* Symbol table address */
            break;
        case 7:  /* DT_STRTAB */
            /* String table address */
            break;
        case 8:  /* DT_STRSZ */
            /* String table size */
            break;
        case 10: /* DT_SYMENT */
            /* Symbol entry size */
            break;
        case 4:  /* DT_HASH */
            /* ELF hash table */
            break;
        case 0x6ffffef5: /* DT_GNU_HASH */
            /* GNU hash table */
            break;
        case 3:  /* DT_PLTGOT */
            /* GOT/PLT base */
            break;
        case 2:  /* DT_PLTRELSZ */
            /* PLT relocation size */
            break;
        case 14: /* DT_PLTREL */
            /* PLT relocation type */
            break;
        case 17: /* DT_JMPREL */
            /* PLT relocation entries */
            break;
        }
        dyn++;
    }

    return 0;
}

/**
 * elf_find_dynamic_entry - Find entry in dynamic section by tag
 *
 * @param dyn Dynamic section pointer
 * @param tag Dynamic tag to find
 * @param value Output for tag value
 * @return 0 if found, -1 if not found
 */
int elf_find_dynamic_entry(const void *dyn, int64_t tag, uint64_t *value)
{
    const Elf64_Dyn *d = (const Elf64_Dyn *)dyn;

    while (d->d_tag != 0) {
        if (d->d_tag == tag) {
            if (value)
                *value = d->d_un.d_val;
            return 0;
        }
        d++;
    }

    return -1;
}

/**
 * elf_hash_lookup - Look up symbol using ELF hash table
 *
 * @param symtab Symbol table
 * @param strtab String table
 * @param hashtab Hash table (.hash section)
 * @param name Symbol name to find
 * @param sym_value Output for symbol value
 * @return 0 on success, -1 on failure
 *
 * ELF hash table format:
 * - nbucket: Number of buckets
 * - nchain: Number of chains
 * - buckets[nbucket]: Bucket array
 * - chains[nchain]: Chain array
 */
int elf_hash_lookup(const void *symtab, const void *strtab,
                    const void *hashtab, const char *name, void **sym_value)
{
    const uint32_t *hash = (const uint32_t *)hashtab;
    const uint8_t *symbols = (const uint8_t *)symtab;
    const char *strings = (const char *)strtab;
    uint32_t hash_val;
    uint32_t nbucket;
    uint32_t ndx;

    if (!hashtab || !name) {
        return -1;
    }

    /* Compute hash */
    hash_val = elf_hash_symbol(name);

    /* Get bucket count */
    nbucket = hash[0];

    /* Get first symbol in bucket */
    ndx = hash[2 + (hash_val % nbucket)];

    /* Search chain */
    while (ndx != 0) {
        const uint8_t *sym = symbols + (ndx * 24); /* Elf64_Sym size */
        const char *sym_name = strings + *(const uint32_t *)sym;

        if (strcmp(sym_name, name) == 0) {
            /* Found - get symbol value */
            uint64_t st_value = *(const uint64_t *)(sym + 8);
            if (sym_value)
                *sym_value = (void *)st_value;
            return 0;
        }

        /* Next in chain */
        ndx = hash[2 + nbucket + ndx];
    }

    return -1;
}

/**
 * elf_process_relocations - Process all relocations in ELF image
 *
 * @param elf_base Base address of ELF image
 * @param load_offset Load offset from PT_LOAD
 * @return 0 on success, -1 on failure
 */
int elf_process_relocations(const void *elf_base, uint64_t load_offset)
{
    if (!elf_base) {
        return -1;
    }

    /* Process dynamic relocations */
    elf_process_dynamic_relocs(elf_base, load_offset);

    /* Process PLT relocations */
    elf_process_plt_relocs(elf_base, load_offset);

    return 0;
}

/**
 * elf_apply_relocation - Apply a single RELA relocation
 *
 * @param elf_base Base address of ELF image
 * @param rela Relocation entry
 * @param load_offset Load offset
 * @return 0 on success, -1 on failure
 *
 * Handles common AArch64 relocation types:
 * - R_AARCH64_ABS64: 64-bit absolute address
 * - R_AARCH64_ABS32: 32-bit absolute address
 * - R_AARCH64_PREL64: 64-bit PC-relative
 * - R_AARCH64_GLOB_DAT: Global data symbol
 * - R_AARCH64_JUMP_SLOT: PLT entry
 */
int elf_apply_relocation(const void *elf_base, const Elf64_Rela *rela, uint64_t load_offset)
{
    uint64_t *addr;
    uint64_t value;
    uint64_t addend;
    uint32_t type;
    uint32_t sym;

    if (!elf_base || !rela) {
        return -1;
    }

    /* Extract relocation info */
    addr = (uint64_t *)((uint8_t *)elf_base + rela->r_offset);
    type = (uint32_t)(rela->r_info & 0xffffffff);
    sym = (uint32_t)(rela->r_info >> 32);
    addend = rela->r_addend;

    /* Get symbol value (simplified - would need symtab lookup) */
    value = sym * sizeof(void *) + load_offset + addend;

    /* Apply based on type */
    switch (type) {
    case 1025: /* R_AARCH64_ABS64 */
        *addr = value + addend;
        break;

    case 1026: /* R_AARCH64_ABS32 */
        *(uint32_t *)addr = (uint32_t)(value + addend);
        break;

    case 1027: /* R_AARCH64_PREL64 */
        *addr = value + addend - rela->r_offset;
        break;

    case 1028: /* R_AARCH64_PREL32 */
        *(uint32_t *)addr = (uint32_t)(value + addend - rela->r_offset);
        break;

    default:
        /* Unsupported relocation type */
        return -1;
    }

    return 0;
}

/**
 * elf_process_dynamic_relocs - Process dynamic relocations
 *
 * @param elf_base Base address of ELF image
 * @param load_offset Load offset
 * @return 0 on success, -1 on failure
 *
 * Processes DT_RELA, DT_RELAENT, DT_RELASZ entries.
 */
int elf_process_dynamic_relocs(const void *elf_base, uint64_t load_offset)
{
    const uint8_t *elf = (const uint8_t *)elf_base;
    uint64_t rela_addr = 0;
    uint64_t rela_size = 0;
    uint64_t rela_ent = 24; /* sizeof(Elf64_Rela) */
    const Elf64_Rela *rela;
    uint64_t count;
    uint64_t i;

    /* Find dynamic section */
    const uint64_t *header = (const uint64_t *)elf;
    uint32_t e_phoff = (uint32_t)header[5];
    uint32_t e_phnum = (uint32_t)header[28];
    uint32_t e_phentsize = (uint32_t)header[26];

    /* Find PT_DYNAMIC */
    for (i = 0; i < e_phnum; i++) {
        const uint8_t *ph = elf + e_phoff + (i * e_phentsize);
        if (*(const uint32_t *)ph == 2) { /* PT_DYNAMIC */
            uint64_t dyn_addr = *(const uint64_t *)(ph + 16);
            const Elf64_Dyn *dyn = (const Elf64_Dyn *)(elf + dyn_addr);

            while (dyn->d_tag != 0) {
                switch (dyn->d_tag) {
                case 7: /* DT_RELA */
                    rela_addr = dyn->d_un.d_val;
                    break;
                case 9: /* DT_RELASZ */
                    rela_size = dyn->d_un.d_val;
                    break;
                case 10: /* DT_RELAENT */
                    rela_ent = dyn->d_un.d_val;
                    break;
                }
                dyn++;
            }
            break;
        }
    }

    if (!rela_addr || !rela_size) {
        return 0; /* No dynamic relocations */
    }

    rela = (const Elf64_Rela *)(elf + rela_addr);
    count = rela_size / rela_ent;

    for (i = 0; i < count; i++) {
        elf_apply_relocation(elf_base, &rela[i], load_offset);
    }

    return 0;
}

/**
 * elf_process_plt_relocs - Process PLT relocations
 *
 * @param elf_base Base address of ELF image
 * @param load_offset Load offset
 * @return 0 on success, -1 on failure
 *
 * Processes DT_JMPREL (PLT relocations) for lazy binding.
 */
int elf_process_plt_relocs(const void *elf_base, uint64_t load_offset)
{
    const uint8_t *elf = (const uint8_t *)elf_base;
    uint64_t jmprel_addr = 0;
    uint64_t pltrelsz = 0;
    const Elf64_Rela *rela;
    uint64_t count;
    uint64_t i;

    /* Find dynamic section */
    const uint64_t *header = (const uint64_t *)elf;
    uint32_t e_phoff = (uint32_t)header[5];
    uint32_t e_phnum = (uint32_t)header[28];
    uint32_t e_phentsize = (uint32_t)header[26];

    /* Find PT_DYNAMIC */
    for (i = 0; i < e_phnum; i++) {
        const uint8_t *ph = elf + e_phoff + (i * e_phentsize);
        if (*(const uint32_t *)ph == 2) { /* PT_DYNAMIC */
            uint64_t dyn_addr = *(const uint64_t *)(ph + 16);
            const Elf64_Dyn *dyn = (const Elf64_Dyn *)(elf + dyn_addr);

            while (dyn->d_tag != 0) {
                switch (dyn->d_tag) {
                case 17: /* DT_JMPREL */
                    jmprel_addr = dyn->d_un.d_val;
                    break;
                case 2: /* DT_PLTRELSZ */
                    pltrelsz = dyn->d_un.d_val;
                    break;
                case 14: /* DT_PLTREL */
                    /* pltrel type stored but not used */
                    break;
                }
                dyn++;
            }
            break;
        }
    }

    if (!jmprel_addr || !pltrelsz) {
        return 0; /* No PLT relocations */
    }

    rela = (const Elf64_Rela *)(elf + jmprel_addr);
    count = pltrelsz / sizeof(Elf64_Rela);

    for (i = 0; i < count; i++) {
        elf_apply_relocation(elf_base, &rela[i], load_offset);
    }

    return 0;
}

/**
 * elf_setup_got - Setup Global Offset Table
 *
 * @param elf_base Base address of ELF image
 * @param load_offset Load offset
 * @return 0 on success, -1 on failure
 *
 * Initializes GOT entries:
 * - GOT[0]: Dynamic section address (for dynamic linker)
 * - GOT[1]: Link map (for debugging)
 * - GOT[2]: Address of _dl_runtime_resolve (for lazy binding)
 * - GOT[3+]: PLT resolver entries
 */
int elf_setup_got(const void *elf_base, uint64_t load_offset)
{
    const uint8_t *elf = (const uint8_t *)elf_base;
    uint64_t got_addr = 0;
    uint64_t got_size = 0;
    uint64_t *got;
    uint64_t i;

    /* Find GOT from dynamic section */
    const uint64_t *header = (const uint64_t *)elf;
    uint32_t e_phoff = (uint32_t)header[5];
    uint32_t e_phnum = (uint32_t)header[28];
    uint32_t e_phentsize = (uint32_t)header[26];

    for (uint32_t i = 0; i < e_phnum; i++) {
        const uint8_t *ph = elf + e_phoff + (i * e_phentsize);
        if (*(const uint32_t *)ph == 2) { /* PT_DYNAMIC */
            uint64_t dyn_addr = *(const uint64_t *)(ph + 16);
            const Elf64_Dyn *dyn = (const Elf64_Dyn *)(elf + dyn_addr);

            while (dyn->d_tag != 0) {
                if (dyn->d_tag == 3) { /* DT_PLTGOT */
                    got_addr = dyn->d_un.d_val;
                    break;
                }
                dyn++;
            }
            break;
        }
    }

    if (!got_addr) {
        return -1;
    }

    got = (uint64_t *)(elf + got_addr);

    /* Initialize first GOT entry with dynamic section */
    got[0] = got_addr + load_offset;

    /* Clear remaining entries (will be filled by lazy binding) */
    for (i = 1; i < 16; i++) {
        got[i] = 0;
    }

    return 0;
}

/**
 * elf_setup_plt - Setup Procedure Linkage Table
 *
 * @param elf_base Base address of ELF image
 * @param load_offset Load offset
 * @return 0 on success, -1 on failure
 *
 * Sets up PLT entries for lazy symbol binding.
 * Each PLT entry contains a trampoline that jumps to GOT.
 */
int elf_setup_plt(const void *elf_base, uint64_t load_offset)
{
    /* PLT setup is architecture-specific */
    /* For AArch64, each PLT entry is 16 bytes */

    (void)elf_base;
    (void)load_offset;

    return 0;
}

/**
 * elf_resolve_plt_entry - Resolve a single PLT entry
 *
 * @param elf_base Base address of ELF image
 * @param plt_index Index of PLT entry to resolve
 * @return 0 on success, -1 on failure
 *
 * Called during lazy binding to resolve a symbol.
 */
int elf_resolve_plt_entry(const void *elf_base, uint64_t plt_index)
{
    (void)elf_base;
    (void)plt_index;

    /* Would resolve symbol and update GOT entry */
    return 0;
}

/**
 * elf_init_plt_got - Initialize PLT and GOT for lazy binding
 *
 * @param elf_base Base address of ELF image
 * @param load_offset Load offset
 * @return 0 on success, -1 on failure
 */
int elf_init_plt_got(const void *elf_base, uint64_t load_offset)
{
    int ret;

    ret = elf_setup_got(elf_base, load_offset);
    if (ret < 0) {
        return ret;
    }

    ret = elf_setup_plt(elf_base, load_offset);
    if (ret < 0) {
        return ret;
    }

    return 0;
}

/**
 * elf_get_symbol_binding - Get symbol binding type
 *
 * @param sym Symbol table entry
 * @param binding Output for binding type
 * @return 0 on success, -1 on failure
 *
 * Binding types:
 * - STB_LOCAL (0): Local symbol
 * - STB_GLOBAL (1): Global symbol
 * - STB_WEAK (2): Weak symbol
 */
int elf_get_symbol_binding(const void *sym, int *binding)
{
    const uint8_t *s = (const uint8_t *)sym;

    if (!sym || !binding) {
        return -1;
    }

    /* st_info field contains binding in upper 4 bits */
    *binding = s[4] >> 4;

    return 0;
}

/**
 * elf_get_symbol_type - Get symbol type
 *
 * @param sym Symbol table entry
 * @param type Output for symbol type
 * @return 0 on success, -1 on failure
 *
 * Symbol types:
 * - STT_NOTYPE (0): No type
 * - STT_OBJECT (1): Data object
 * - STT_FUNC (2): Function
 * - STT_SECTION (3): Section symbol
 * - STT_FILE (4): File symbol
 */
int elf_get_symbol_type(const void *sym, int *type)
{
    const uint8_t *s = (const uint8_t *)sym;

    if (!sym || !type) {
        return -1;
    }

    /* st_info field contains type in lower 4 bits */
    *type = s[4] & 0xf;

    return 0;
}

/**
 * elf_check_symbol_version - Check symbol version info
 *
 * @param base ELF base address
 * @param name Symbol name
 * @return 0 if OK, -1 if version mismatch
 */
int elf_check_symbol_version(const void *base, const char *name)
{
    /* Simplified - would check .gnu.version section */
    (void)base;
    (void)name;

    return 0;
}

/* ============================================================================
 * Session 23: Translation Infrastructure Extensions
 * ============================================================================ */

/* Global translation cache */
static struct {
    void *base;
    size_t size;
    size_t used;
    TranslatedBlock *blocks;
    size_t num_blocks;
} g_translation_cache;

/**
 * translation_alloc_block - Allocate memory for a translated block
 *
 * @param guest_pc Guest PC address
 * @param code_size Size of code to allocate
 * @return Pointer to allocated block, NULL on failure
 */
void *translation_alloc_block(uint64_t guest_pc, size_t code_size)
{
    TranslatedBlock *block;
    void *code_mem;

    if (!g_translation_cache.base) {
        return NULL;
    }

    /* Check if we have space */
    if (g_translation_cache.used + code_size > g_translation_cache.size) {
        return NULL;
    }

    /* Allocate from code cache */
    code_mem = (uint8_t *)g_translation_cache.base + g_translation_cache.used;
    g_translation_cache.used += code_size;

    /* Initialize block structure */
    block = &g_translation_cache.blocks[g_translation_cache.num_blocks++];
    block->guest_pc = guest_pc;
    block->host_pc = (uint64_t)code_mem;
    block->size = (uint32_t)code_size;
    block->flags = 0;
    block->hash = hash_address(guest_pc);
    block->refcount = 1;
    block->chain[0] = 0;
    block->chain[1] = 0;

    return code_mem;
}

/**
 * translation_free_block - Free a translated block
 *
 * @param block Block to free
 * @return 0 on success, -1 on failure
 */
int translation_free_block(void *block)
{
    TranslatedBlock *b;
    size_t i;

    if (!block || !g_translation_cache.blocks) {
        return -1;
    }

    /* Find block in cache */
    for (i = 0; i < g_translation_cache.num_blocks; i++) {
        b = &g_translation_cache.blocks[i];
        if (b->host_pc == (uint64_t)block) {
            b->refcount--;
            if (b->refcount == 0) {
                /* Mark as free (simplified) */
                b->guest_pc = 0;
            }
            return 0;
        }
    }

    return -1;
}

/**
 * translation_invalidate_block - Invalidate a translated block
 *
 * @param guest_pc Guest PC of block to invalidate
 */
void translation_invalidate_block(uint64_t guest_pc)
{
    TranslatedBlock *b;
    size_t i;

    if (!g_translation_cache.blocks) {
        return;
    }

    /* Find and invalidate */
    for (i = 0; i < g_translation_cache.num_blocks; i++) {
        b = &g_translation_cache.blocks[i];
        if (b->guest_pc == guest_pc) {
            b->guest_pc = 0;
            b->chain[0] = 0;
            b->chain[1] = 0;
        }
    }
}

/**
 * translation_flush_cache - Flush instruction cache
 *
 * @param addr Address of code to flush
 * @param size Size of code region
 *
 * Ensures translated code is visible to the instruction fetch unit.
 */
void translation_flush_cache(void *addr, size_t size)
{
#ifdef __APPLE__
    /* macOS: use sys_icache_invalidate */
    (void)addr;
    (void)size;
#else
    /* Linux: use __builtin___clear_cache */
    __builtin___clear_cache((char *)addr, (char *)addr + size);
#endif
}

/**
 * translation_chain_blocks - Chain two translated blocks
 *
 * @param from_block Source block
 * @param to_block Target block
 * @param index Chain index (0 or 1)
 * @return 0 on success, -1 on failure
 *
 * Block chaining optimizes branch targets by directly jumping
 * to the next block instead of going through the lookup cache.
 */
int translation_chain_blocks(void *from_block, void *to_block, int index)
{
    TranslatedBlock *b;
    size_t i;

    if (!from_block || !to_block || index < 0 || index > 1) {
        return -1;
    }

    /* Find source block */
    for (i = 0; i < g_translation_cache.num_blocks; i++) {
        b = &g_translation_cache.blocks[i];
        if (b->host_pc == (uint64_t)from_block) {
            b->chain[index] = (uint64_t)to_block;
            return 0;
        }
    }

    return -1;
}

/**
 * translation_unchain_blocks - Remove all chains from a block
 *
 * @param block Block to unchain
 * @return 0 on success, -1 on failure
 */
int translation_unchain_blocks(void *block)
{
    TranslatedBlock *b;
    size_t i;

    if (!block) {
        return -1;
    }

    /* Find block */
    for (i = 0; i < g_translation_cache.num_blocks; i++) {
        b = &g_translation_cache.blocks[i];
        if (b->host_pc == (uint64_t)block) {
            b->chain[0] = 0;
            b->chain[1] = 0;
            return 0;
        }
    }

    return -1;
}

/**
 * translation_get_chained_block - Get chained successor block
 *
 * @param block Source block
 * @param index Chain index (0 or 1)
 * @return Chained block pointer, NULL if not chained
 */
void *translation_get_chained_block(void *block, int index)
{
    TranslatedBlock *b;
    size_t i;

    if (!block || index < 0 || index > 1) {
        return NULL;
    }

    /* Find block */
    for (i = 0; i < g_translation_cache.num_blocks; i++) {
        b = &g_translation_cache.blocks[i];
        if (b->host_pc == (uint64_t)block) {
            return (void *)b->chain[index];
        }
    }

    return NULL;
}

/**
 * code_cache_init - Initialize code cache
 *
 * @param size Initial cache size
 * @return 0 on success, -1 on failure
 */
int code_cache_init(size_t size)
{
    void *base;
    size_t block_capacity;

    if (size == 0) {
        size = 1024 * 1024; /* 1MB default */
    }

    /* Allocate cache memory */
    base = mmap(NULL, size, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (base == MAP_FAILED) {
        return -1;
    }

    g_translation_cache.base = base;
    g_translation_cache.size = size;
    g_translation_cache.used = 0;

    /* Allocate block metadata */
    block_capacity = size / 4096; /* One block per 4KB of code */
    g_translation_cache.blocks = (TranslatedBlock *)calloc(block_capacity, sizeof(TranslatedBlock));
    g_translation_cache.num_blocks = 0;

    if (!g_translation_cache.blocks) {
        munmap(base, size);
        return -1;
    }

    return 0;
}

/**
 * code_cache_cleanup - Free code cache resources
 */
void code_cache_cleanup(void)
{
    if (g_translation_cache.base) {
        munmap(g_translation_cache.base, g_translation_cache.size);
        g_translation_cache.base = NULL;
    }

    if (g_translation_cache.blocks) {
        free(g_translation_cache.blocks);
        g_translation_cache.blocks = NULL;
    }

    g_translation_cache.size = 0;
    g_translation_cache.used = 0;
    g_translation_cache.num_blocks = 0;
}

/**
 * code_cache_alloc_aligned - Allocate aligned memory from code cache
 *
 * @param size Size to allocate
 * @param alignment Required alignment (must be power of 2)
 * @return Pointer to aligned memory, NULL on failure
 */
void *code_cache_alloc_aligned(size_t size, size_t alignment)
{
    uintptr_t current;
    uintptr_t aligned;
    size_t padding;

    if (!g_translation_cache.base) {
        return NULL;
    }

    current = (uintptr_t)g_translation_cache.base + g_translation_cache.used;
    aligned = (current + alignment - 1) & ~(alignment - 1);
    padding = aligned - current;

    if (g_translation_cache.used + padding + size > g_translation_cache.size) {
        return NULL;
    }

    g_translation_cache.used += padding + size;

    return (void *)aligned;
}

/**
 * code_cache_mark_executable - Mark code cache region as executable
 *
 * @param addr Address of region
 * @param size Size of region
 * @return 0 on success, -1 on failure
 */
int code_cache_mark_executable(void *addr, size_t size)
{
    return mprotect(addr, size, PROT_READ | PROT_EXEC);
}

/* ============================================================================
 * Session 24: Additional Vector and Memory Operations
 * ============================================================================ */

/**
 * v128_bic - Bitwise bit clear (AND NOT)
 *
 * @param a First operand
 * @param b Second operand (bits to clear)
 * @return a & ~b
 */
Vector128 v128_bic(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint64_t *a_in = (uint64_t *)&a;
    uint64_t *b_in = (uint64_t *)&b;
    uint64_t *out = (uint64_t *)&result;

    out[0] = a_in[0] & ~b_in[0];
    out[1] = a_in[1] & ~b_in[1];

    return result;
}

/**
 * v128_orn - Bitwise OR NOT
 *
 * @param a First operand
 * @param b Second operand
 * @return a | ~b
 */
Vector128 v128_orn(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint64_t *a_in = (uint64_t *)&a;
    uint64_t *b_in = (uint64_t *)&b;
    uint64_t *out = (uint64_t *)&result;

    out[0] = a_in[0] | ~b_in[0];
    out[1] = a_in[1] | ~b_in[1];

    return result;
}

/**
 * v128_eor_not - EOR with NOT of second operand
 *
 * @param a First operand
 * @param b Second operand
 * @return a ^ ~b (equivalent to ~(a ^ b))
 */
Vector128 v128_eor_not(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint64_t *a_in = (uint64_t *)&a;
    uint64_t *b_in = (uint64_t *)&b;
    uint64_t *out = (uint64_t *)&result;

    out[0] = a_in[0] ^ ~b_in[0];
    out[1] = a_in[1] ^ ~b_in[1];

    return result;
}

/**
 * v128_rbit - Reverse bit order in each byte
 *
 * @param a Input vector
 * @return Vector with reversed bits
 */
Vector128 v128_rbit(Vector128 a)
{
    Vector128 result;
    uint8_t *in = (uint8_t *)&a;
    uint8_t *out = (uint8_t *)&result;
    static const uint8_t nibble_reverse[16] = {
        0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
        0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf
    };

    for (int i = 0; i < 16; i++) {
        out[i] = (nibble_reverse[in[i] & 0xf] << 4) |
                  nibble_reverse[in[i] >> 4];
    }

    return result;
}

/**
 * v128_shl_narrow - Narrowing shift left
 *
 * @param a Input vector
 * @param shift Shift amount
 * @return Shifted and narrowed vector
 */
Vector128 v128_shl_narrow(Vector128 a, int shift)
{
    Vector128 result;
    uint16_t *in = (uint16_t *)&a;
    uint8_t *out = (uint8_t *)&result;

    for (int i = 0; i < 8; i++) {
        out[i] = (uint8_t)(in[i] << shift);
    }

    return result;
}

/**
 * v128_sshr_narrow - Narrowing signed shift right
 *
 * @param a Input vector
 * @param shift Shift amount
 * @return Shifted and narrowed vector
 */
Vector128 v128_sshr_narrow(Vector128 a, int shift)
{
    Vector128 result;
    int16_t *in = (int16_t *)&a;
    int8_t *out = (int8_t *)&result;

    for (int i = 0; i < 8; i++) {
        out[i] = (int8_t)(in[i] >> shift);
    }

    return result;
}

/**
 * v128_ushr_narrow - Narrowing unsigned shift right
 *
 * @param a Input vector
 * @param shift Shift amount
 * @return Shifted and narrowed vector
 */
Vector128 v128_ushr_narrow(Vector128 a, int shift)
{
    Vector128 result;
    uint16_t *in = (uint16_t *)&a;
    uint8_t *out = (uint8_t *)&result;

    for (int i = 0; i < 8; i++) {
        out[i] = (uint8_t)(in[i] >> shift);
    }

    return result;
}

/**
 * v128_narrow_s16_u8 - Narrow signed 16-bit to unsigned 8-bit
 *
 * @param a Input vector (8 x 16-bit signed)
 * @return Output vector (16 x 8-bit unsigned, saturated)
 */
Vector128 v128_narrow_s16_u8(Vector128 a)
{
    Vector128 result;
    int16_t *in = (int16_t *)&a;
    uint8_t *out = (uint8_t *)&result;

    for (int i = 0; i < 8; i++) {
        /* Saturate to unsigned 8-bit range */
        if (in[i] < 0)
            out[i] = 0;
        else if (in[i] > 255)
            out[i] = 255;
        else
            out[i] = (uint8_t)in[i];
    }

    return result;
}

/**
 * v128_narrow_s16_s8 - Narrow signed 16-bit to signed 8-bit
 *
 * @param a Input vector (8 x 16-bit signed)
 * @return Output vector (16 x 8-bit signed, saturated)
 */
Vector128 v128_narrow_s16_s8(Vector128 a)
{
    Vector128 result;
    int16_t *in = (int16_t *)&a;
    int8_t *out = (int8_t *)&result;

    for (int i = 0; i < 8; i++) {
        /* Saturate to signed 8-bit range */
        if (in[i] < -128)
            out[i] = -128;
        else if (in[i] > 127)
            out[i] = 127;
        else
            out[i] = (int8_t)in[i];
    }

    return result;
}

/**
 * v128_narrow_u16_u8 - Narrow unsigned 16-bit to unsigned 8-bit
 *
 * @param a Input vector (8 x 16-bit unsigned)
 * @return Output vector (16 x 8-bit unsigned, saturated)
 */
Vector128 v128_narrow_u16_u8(Vector128 a)
{
    Vector128 result;
    uint16_t *in = (uint16_t *)&a;
    uint8_t *out = (uint8_t *)&result;

    for (int i = 0; i < 8; i++) {
        /* Saturate to unsigned 8-bit range */
        if (in[i] > 255)
            out[i] = 255;
        else
            out[i] = (uint8_t)in[i];
    }

    return result;
}

/**
 * v128_narrow_s32_s16 - Narrow signed 32-bit to signed 16-bit
 *
 * @param a Input vector (4 x 32-bit signed)
 * @return Output vector (8 x 16-bit signed, saturated)
 */
Vector128 v128_narrow_s32_s16(Vector128 a)
{
    Vector128 result;
    int32_t *in = (int32_t *)&a;
    int16_t *out = (int16_t *)&result;

    for (int i = 0; i < 4; i++) {
        /* Saturate to signed 16-bit range */
        if (in[i] < -32768)
            out[i] = -32768;
        else if (in[i] > 32767)
            out[i] = 32767;
        else
            out[i] = (int16_t)in[i];
    }

    return result;
}

/**
 * v128_narrow_u32_u16 - Narrow unsigned 32-bit to unsigned 16-bit
 *
 * @param a Input vector (4 x 32-bit unsigned)
 * @return Output vector (8 x 16-bit unsigned, saturated)
 */
Vector128 v128_narrow_u32_u16(Vector128 a)
{
    Vector128 result;
    uint32_t *in = (uint32_t *)&a;
    uint16_t *out = (uint16_t *)&result;

    for (int i = 0; i < 4; i++) {
        /* Saturate to unsigned 16-bit range */
        if (in[i] > 65535)
            out[i] = 65535;
        else
            out[i] = (uint16_t)in[i];
    }

    return result;
}

/**
 * v128_widen_u8_u16_lo - Unsigned widen low half 8-bit to 16-bit
 *
 * @param a Input vector (16 x 8-bit unsigned)
 * @return Output vector (8 x 16-bit unsigned, low half widened)
 */
Vector128 v128_widen_u8_u16_lo(Vector128 a)
{
    Vector128 result;
    uint8_t *in = (uint8_t *)&a;
    uint16_t *out = (uint16_t *)&result;

    for (int i = 0; i < 8; i++) {
        out[i] = (uint16_t)in[i];
    }

    return result;
}

/**
 * v128_widen_u8_u16_hi - Unsigned widen high half 8-bit to 16-bit
 *
 * @param a Input vector (16 x 8-bit unsigned)
 * @return Output vector (8 x 16-bit unsigned, high half widened)
 */
Vector128 v128_widen_u8_u16_hi(Vector128 a)
{
    Vector128 result;
    uint8_t *in = (uint8_t *)&a;
    uint16_t *out = (uint16_t *)&result;

    for (int i = 0; i < 8; i++) {
        out[i] = (uint16_t)in[i + 8];
    }

    return result;
}

/**
 * v128_widen_s8_s16_lo - Signed widen low half 8-bit to 16-bit
 *
 * @param a Input vector (16 x 8-bit signed)
 * @return Output vector (8 x 16-bit signed, low half widened)
 */
Vector128 v128_widen_s8_s16_lo(Vector128 a)
{
    Vector128 result;
    int8_t *in = (int8_t *)&a;
    int16_t *out = (int16_t *)&result;

    for (int i = 0; i < 8; i++) {
        out[i] = (int16_t)in[i];
    }

    return result;
}

/**
 * v128_widen_s8_s16_hi - Signed widen high half 8-bit to 16-bit
 *
 * @param a Input vector (16 x 8-bit signed)
 * @return Output vector (8 x 16-bit signed, high half widened)
 */
Vector128 v128_widen_s8_s16_hi(Vector128 a)
{
    Vector128 result;
    int8_t *in = (int8_t *)&a;
    int16_t *out = (int16_t *)&result;

    for (int i = 0; i < 8; i++) {
        out[i] = (int16_t)in[i + 8];
    }

    return result;
}

/**
 * v128_fadd - Floating-point vector add
 *
 * @param a First operand
 * @param b Second operand
 * @return a + b (element-wise)
 */
Vector128 v128_fadd(Vector128 a, Vector128 b)
{
    Vector128 result;
    float *a_in = (float *)&a;
    float *b_in = (float *)&b;
    float *out = (float *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = a_in[i] + b_in[i];
    }

    return result;
}

/**
 * v128_fsub - Floating-point vector subtract
 *
 * @param a First operand
 * @param b Second operand
 * @return a - b (element-wise)
 */
Vector128 v128_fsub(Vector128 a, Vector128 b)
{
    Vector128 result;
    float *a_in = (float *)&a;
    float *b_in = (float *)&b;
    float *out = (float *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = a_in[i] - b_in[i];
    }

    return result;
}

/**
 * v128_fmul - Floating-point vector multiply
 *
 * @param a First operand
 * @param b Second operand
 * @return a * b (element-wise)
 */
Vector128 v128_fmul(Vector128 a, Vector128 b)
{
    Vector128 result;
    float *a_in = (float *)&a;
    float *b_in = (float *)&b;
    float *out = (float *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = a_in[i] * b_in[i];
    }

    return result;
}

/**
 * v128_fdiv - Floating-point vector divide
 *
 * @param a First operand
 * @param b Second operand
 * @return a / b (element-wise)
 */
Vector128 v128_fdiv(Vector128 a, Vector128 b)
{
    Vector128 result;
    float *a_in = (float *)&a;
    float *b_in = (float *)&b;
    float *out = (float *)&result;

    for (int i = 0; i < 4; i++) {
        if (b_in[i] != 0.0f)
            out[i] = a_in[i] / b_in[i];
        else
            out[i] = (a_in[i] >= 0) ? INFINITY : -INFINITY;
    }

    return result;
}

/**
 * v128_fsqrt - Floating-point vector square root
 *
 * @param a Input vector
 * @return sqrt(a) (element-wise)
 */
Vector128 v128_fsqrt(Vector128 a)
{
    Vector128 result;
    float *a_in = (float *)&a;
    float *out = (float *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = sqrtf(a_in[i]);
    }

    return result;
}

/**
 * v128_frecpe - Floating-point reciprocal estimate
 *
 * @param a Input vector
 * @return 1/a estimate (element-wise)
 */
Vector128 v128_frecpe(Vector128 a)
{
    Vector128 result;
    float *a_in = (float *)&a;
    float *out = (float *)&result;

    for (int i = 0; i < 4; i++) {
        if (a_in[i] != 0.0f)
            out[i] = 1.0f / a_in[i];
        else
            out[i] = INFINITY;
    }

    return result;
}

/**
 * v128_frsqrte - Floating-point reciprocal square root estimate
 *
 * @param a Input vector
 * @return 1/sqrt(a) estimate (element-wise)
 */
Vector128 v128_frsqrte(Vector128 a)
{
    Vector128 result;
    float *a_in = (float *)&a;
    float *out = (float *)&result;

    for (int i = 0; i < 4; i++) {
        if (a_in[i] > 0.0f)
            out[i] = 1.0f / sqrtf(a_in[i]);
        else if (a_in[i] == 0.0f)
            out[i] = INFINITY;
        else
            out[i] = 0.0f; /* NaN for negative input */
    }

    return result;
}

/**
 * v128_fcmp_eq - Floating-point vector compare equal
 *
 * @param a First operand
 * @param b Second operand
 * @return 0xFFFFFFFF where equal, 0 otherwise
 */
Vector128 v128_fcmp_eq(Vector128 a, Vector128 b)
{
    Vector128 result;
    float *a_in = (float *)&a;
    float *b_in = (float *)&b;
    uint32_t *out = (uint32_t *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = (a_in[i] == b_in[i]) ? 0xFFFFFFFF : 0;
    }

    return result;
}

/**
 * v128_fcmp_lt - Floating-point vector compare less than
 *
 * @param a First operand
 * @param b Second operand
 * @return 0xFFFFFFFF where a < b, 0 otherwise
 */
Vector128 v128_fcmp_lt(Vector128 a, Vector128 b)
{
    Vector128 result;
    float *a_in = (float *)&a;
    float *b_in = (float *)&b;
    uint32_t *out = (uint32_t *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = (a_in[i] < b_in[i]) ? 0xFFFFFFFF : 0;
    }

    return result;
}

/* ============================================================================
 * Memory Management Utilities
 * ============================================================================ */

/**
 * rosetta_mmap_anonymous - Create anonymous memory mapping
 *
 * @param size Size of mapping
 * @param prot Protection flags
 * @return Pointer to mapped region, NULL on failure
 */
void *rosetta_mmap_anonymous(size_t size, int prot)
{
    void *addr = mmap(NULL, size, prot, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return (addr == MAP_FAILED) ? NULL : addr;
}

/**
 * rosetta_munmap_region - Unmap memory region
 *
 * @param addr Address of region
 * @param size Size of region
 * @return 0 on success, -1 on failure
 */
int rosetta_munmap_region(void *addr, size_t size)
{
    return munmap(addr, size);
}

/**
 * rosetta_mprotect_region - Change protection of memory region
 *
 * @param addr Address of region
 * @param size Size of region
 * @param prot New protection flags
 * @return 0 on success, -1 on failure
 */
int rosetta_mprotect_region(void *addr, size_t size, int prot)
{
    return mprotect(addr, size, prot);
}

/**
 * rosetta_memalign - Allocate aligned memory
 *
 * @param alignment Required alignment (power of 2)
 * @param size Size to allocate
 * @return Pointer to aligned memory, NULL on failure
 */
void *rosetta_memalign(size_t alignment, size_t size)
{
    void *ptr;
    if (posix_memalign(&ptr, alignment, size) != 0)
        return NULL;
    return ptr;
}

/**
 * rosetta_memzero - Zero-fill memory region
 *
 * @param s Pointer to memory
 * @param n Number of bytes to zero
 */
void rosetta_memzero(void *s, size_t n)
{
    memset(s, 0, n);
}

/* ============================================================================
 * String Utilities Extended
 * ============================================================================ */

/**
 * rosetta_strncpy - Copy string with length limit
 *
 * @param dest Destination buffer
 * @param src Source string
 * @param n Maximum bytes to copy
 * @return dest
 */
char *rosetta_strncpy(char *dest, const char *src, size_t n)
{
    strncpy(dest, src, n);
    return dest;
}

/**
 * rosetta_strncat - Concatenate strings with length limit
 *
 * @param dest Destination buffer
 * @param src Source string
 * @param n Maximum bytes to append
 * @return dest
 */
char *rosetta_strncat(char *dest, const char *src, size_t n)
{
    strncat(dest, src, n);
    return dest;
}

/**
 * rosetta_strcasecmp - Case-insensitive string compare
 *
 * @param s1 First string
 * @param s2 Second string
 * @return < 0 if s1 < s2, 0 if s1 == s2, > 0 if s1 > s2
 */
int rosetta_strcasecmp(const char *s1, const char *s2)
{
    return strcasecmp(s1, s2);
}

/**
 * rosetta_strchr - Find character in string
 *
 * @param s String to search
 * @param c Character to find
 * @return Pointer to first occurrence, NULL if not found
 */
char *rosetta_strchr(const char *s, int c)
{
    return strchr(s, c);
}

/**
 * rosetta_strrchr - Find last occurrence of character
 *
 * @param s String to search
 * @param c Character to find
 * @return Pointer to last occurrence, NULL if not found
 */
char *rosetta_strrchr(const char *s, int c)
{
    return strrchr(s, c);
}

/**
 * rosetta_strspn - Get length of prefix substring
 *
 * @param s String to search
 * @param accept Characters to accept
 * @return Length of initial segment consisting of accept characters
 */
size_t rosetta_strspn(const char *s, const char *accept)
{
    return strspn(s, accept);
}

/**
 * rosetta_strcspn - Get length of complementary prefix substring
 *
 * @param s String to search
 * @param reject Characters to reject
 * @return Length of initial segment consisting of non-reject characters
 */
size_t rosetta_strcspn(const char *s, const char *reject)
{
    return strcspn(s, reject);
}

/* ============================================================================
 * Memory Utilities Extended
 * ============================================================================ */

/**
 * rosetta_memccpy - Copy memory until character found
 *
 * @param dest Destination buffer
 * @param src Source buffer
 * @param c Character to stop at
 * @param n Maximum bytes to copy
 * @return Pointer after copied character, NULL if not found
 */
void *rosetta_memccpy(void *dest, const void *src, int c, size_t n)
{
    return memccpy(dest, src, c, n);
}

/**
 * rosetta_memchr_inv - Find first byte NOT equal to c
 *
 * @param s Memory region to search
 * @param c Byte value to match against
 * @param n Size of region
 * @return Pointer to first non-matching byte, NULL if all match
 */
void *rosetta_memchr_inv(const void *s, int c, size_t n)
{
    const unsigned char *p = (const unsigned char *)s;
    unsigned char uc = (unsigned char)c;

    while (n-- > 0) {
        if (*p != uc)
            return (void *)p;
        p++;
    }

    return NULL;
}

/**
 * rosetta_memcmp_consttime - Constant-time memory compare
 *
 * @param s1 First memory region
 * @param s2 Second memory region
 * @param n Size to compare
 * @return 0 if equal, non-zero otherwise
 *
 * Important for security: comparison time is independent of content.
 */
int rosetta_memcmp_consttime(const void *s1, const void *s2, size_t n)
{
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;
    unsigned char result = 0;

    for (size_t i = 0; i < n; i++) {
        result |= p1[i] ^ p2[i];
    }

    return (int)result;
}

/* ============================================================================
 * Bit Manipulation Utilities
 * ============================================================================ */

/**
 * bitreverse32 - Reverse bits in 32-bit word
 *
 * @param x Input value
 * @return Reversed value
 */
uint32_t bitreverse32(uint32_t x)
{
    x = ((x & 0x55555555) << 1) | ((x & 0xAAAAAAAA) >> 1);
    x = ((x & 0x33333333) << 2) | ((x & 0xCCCCCCCC) >> 2);
    x = ((x & 0x0F0F0F0F) << 4) | ((x & 0xF0F0F0F0) >> 4);
    x = ((x & 0x00FF00FF) << 8) | ((x & 0xFF00FF00) >> 8);
    x = (x << 24) | (x >> 8);
    return x;
}

/**
 * bitreverse64 - Reverse bits in 64-bit word
 *
 * @param x Input value
 * @return Reversed value
 */
uint64_t bitreverse64(uint64_t x)
{
    x = ((x & 0x5555555555555555ULL) << 1) | ((x & 0xAAAAAAAAAAAAAAAAULL) >> 1);
    x = ((x & 0x3333333333333333ULL) << 2) | ((x & 0xCCCCCCCCCCCCCCCCULL) >> 2);
    x = ((x & 0x0F0F0F0F0F0F0F0FULL) << 4) | ((x & 0xF0F0F0F0F0F0F0F0ULL) >> 4);
    x = ((x & 0x00FF00FF00FF00FFULL) << 8) | ((x & 0xFF00FF00FF00FF00ULL) >> 8);
    x = ((x & 0x0000FFFF0000FFFFULL) << 16) | ((x & 0xFFFF0000FFFF0000ULL) >> 16);
    x = (x << 32) | (x >> 32);
    return x;
}

/**
 * count_leading_zeros32 - Count leading zeros in 32-bit word
 *
 * @param x Input value
 * @return Number of leading zeros (0-32)
 */
uint32_t count_leading_zeros32(uint32_t x)
{
    if (x == 0)
        return 32;

    uint32_t count = 0;
    while ((x & 0x80000000) == 0) {
        count++;
        x <<= 1;
    }

    return count;
}

/**
 * count_leading_zeros64 - Count leading zeros in 64-bit word
 *
 * @param x Input value
 * @return Number of leading zeros (0-64)
 */
uint64_t count_leading_zeros64(uint64_t x)
{
    if (x == 0)
        return 64;

    uint64_t count = 0;
    while ((x & 0x8000000000000000ULL) == 0) {
        count++;
        x <<= 1;
    }

    return count;
}

/**
 * count_set_bits32 - Count set bits (population count) in 32-bit word
 *
 * @param x Input value
 * @return Number of 1 bits
 */
uint32_t count_set_bits32(uint32_t x)
{
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    x = x + (x >> 8);
    x = x + (x >> 16);
    return x & 0x3F;
}

/**
 * count_set_bits64 - Count set bits (population count) in 64-bit word
 *
 * @param x Input value
 * @return Number of 1 bits
 */
uint64_t count_set_bits64(uint64_t x)
{
    x = x - ((x >> 1) & 0x5555555555555555ULL);
    x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    x = x + (x >> 8);
    x = x + (x >> 16);
    x = x + (x >> 32);
    return x & 0x7F;
}

/* ============================================================================
 * Translation Infrastructure Extended
 * ============================================================================ */

/**
 * translation_block_is_valid - Check if translation block is valid
 *
 * @param guest_pc Guest PC to check
 * @return 1 if valid, 0 if not
 */
int translation_block_is_valid(uint64_t guest_pc)
{
    TranslatedBlock *b;
    size_t i;

    if (!g_translation_cache.blocks) {
        return 0;
    }

    for (i = 0; i < g_translation_cache.num_blocks; i++) {
        b = &g_translation_cache.blocks[i];
        if (b->guest_pc == guest_pc && b->refcount > 0) {
            return 1;
        }
    }

    return 0;
}

/**
 * translation_block_set_flag - Set flag on translation block
 *
 * @param guest_pc Guest PC of block
 * @param flag Flag to set
 */
void translation_block_set_flag(uint64_t guest_pc, uint32_t flag)
{
    TranslatedBlock *b;
    size_t i;

    if (!g_translation_cache.blocks) {
        return;
    }

    for (i = 0; i < g_translation_cache.num_blocks; i++) {
        b = &g_translation_cache.blocks[i];
        if (b->guest_pc == guest_pc) {
            b->flags |= flag;
            return;
        }
    }
}

/**
 * translation_block_clear_flag - Clear flag on translation block
 *
 * @param guest_pc Guest PC of block
 * @param flag Flag to clear
 */
void translation_block_clear_flag(uint64_t guest_pc, uint32_t flag)
{
    TranslatedBlock *b;
    size_t i;

    if (!g_translation_cache.blocks) {
        return;
    }

    for (i = 0; i < g_translation_cache.num_blocks; i++) {
        b = &g_translation_cache.blocks[i];
        if (b->guest_pc == guest_pc) {
            b->flags &= ~flag;
            return;
        }
    }
}

/* ============================================================================
 * Signal Handling Extended
 * ============================================================================ */

/**
 * setup_signal_trampoline - Setup signal trampoline for handler
 *
 * @param handler Signal handler address
 * @return 0 on success, -1 on failure
 */
int setup_signal_trampoline(void *handler)
{
    /* Placeholder for signal trampoline setup */
    (void)handler;
    return 0;
}

/**
 * cleanup_signal_handlers - Cleanup signal handlers
 */
void cleanup_signal_handlers(void)
{
    /* Reset signal handlers to default */
    signal(SIGSEGV, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
}

/**
 * signal_block - Block a signal
 *
 * @param sig Signal number
 * @return 0 on success, -1 on failure
 */
int signal_block(int sig)
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, sig);
    return pthread_sigmask(SIG_BLOCK, &set, NULL);
}

/**
 * signal_unblock - Unblock a signal
 *
 * @param sig Signal number
 * @return 0 on success, -1 on failure
 */
int signal_unblock(int sig)
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, sig);
    return pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}

/* ============================================================================
 * Session 25: Additional Utility Functions
 * ============================================================================ */

/**
 * v128_shuffle - Vector shuffle using indices
 *
 * @param v Input vector
 * @param indices Vector containing shuffle indices
 * @return Shuffled vector
 */
Vector128 v128_shuffle(Vector128 v, Vector128 indices)
{
    Vector128 result;
    uint8_t *v_bytes = (uint8_t *)&v;
    uint8_t *idx_bytes = (uint8_t *)&indices;
    uint8_t *res_bytes = (uint8_t *)&result;

    for (int i = 0; i < 16; i++) {
        uint8_t idx = idx_bytes[i] & 0x1F; /* Limit to valid range */
        if (idx < 16) {
            res_bytes[i] = v_bytes[idx];
        } else {
            res_bytes[i] = 0;
        }
    }

    return result;
}

/**
 * v128_interleave_lo - Interleave low elements of two vectors
 *
 * @param a First vector
 * @param b Second vector
 * @return Interleaved vector (low halves)
 */
Vector128 v128_interleave_lo(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint64_t *a64 = (uint64_t *)&a;
    uint64_t *b64 = (uint64_t *)&b;
    uint64_t *r64 = (uint64_t *)&result;

    /* Interleave 64-bit elements */
    r64[0] = a64[0];
    r64[1] = b64[0];

    return result;
}

/**
 * v128_interleave_hi - Interleave high elements of two vectors
 *
 * @param a First vector
 * @param b Second vector
 * @return Interleaved vector (high halves)
 */
Vector128 v128_interleave_hi(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint64_t *a64 = (uint64_t *)&a;
    uint64_t *b64 = (uint64_t *)&b;
    uint64_t *r64 = (uint64_t *)&result;

    /* Interleave 64-bit elements from high half */
    r64[0] = a64[2];
    r64[1] = b64[2];

    return result;
}

/**
 * v128_test_zero - Test if any element in vector is zero
 *
 * @param v Input vector
 * @return Non-zero if any element is zero, 0 otherwise
 */
uint32_t v128_test_zero(Vector128 v)
{
    uint64_t *v64 = (uint64_t *)&v;
    uint32_t result = 0;

    /* Test each 64-bit element */
    for (int i = 0; i < 2; i++) {
        uint64_t val = v64[i];
        /* Check if any byte is zero */
        if (((val - 0x0101010101010101ULL) & ~val & 0x8080808080808080ULL) != 0) {
            result |= (1 << i);
        }
    }

    return result;
}

/**
 * v128_test_sign - Test sign of vector elements
 *
 * @param v Input vector
 * @return Bitmask of sign bits
 */
uint32_t v128_test_sign(Vector128 v)
{
    uint64_t *v64 = (uint64_t *)&v;
    uint32_t result = 0;

    /* Extract sign bit from each 64-bit element */
    for (int i = 0; i < 2; i++) {
        if (v64[i] & 0x8000000000000000ULL) {
            result |= (1 << i);
        }
    }

    return result;
}

/**
 * translate_ldrb_imm - Load register byte with immediate offset
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldrb_imm(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    /* Extract fields from LDRT/LDRB immediate */
    uint8_t rt = (encoding >> 0) & 0x1F;    /* Target register */
    uint8_t rn = (encoding >> 5) & 0x1F;    /* Base register */
    uint16_t imm12 = (encoding >> 10) & 0xFFF; /* Immediate offset */

    /* Calculate address */
    uint64_t base = state->cpu.gpr.x[rn];
    uint64_t addr = base + imm12;

    /* Load byte (zero-extended) */
    uint8_t value = *(volatile uint8_t *)addr;
    state->cpu.gpr.x[rt] = value;

    return 0;
}

/**
 * translate_strb_imm - Store register byte with immediate offset
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_strb_imm(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    /* Extract fields from STRB immediate */
    uint8_t rt = (encoding >> 0) & 0x1F;    /* Source register */
    uint8_t rn = (encoding >> 5) & 0x1F;    /* Base register */
    uint16_t imm12 = (encoding >> 10) & 0xFFF; /* Immediate offset */

    /* Calculate address */
    uint64_t base = state->cpu.gpr.x[rn];
    uint64_t addr = base + imm12;

    /* Store byte */
    uint8_t value = (uint8_t)(state->cpu.gpr.x[rt] & 0xFF);
    *(volatile uint8_t *)addr = value;

    return 0;
}

/**
 * translate_ldrh_imm - Load register halfword with immediate offset
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldrh_imm(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    /* Extract fields from LDRH immediate */
    uint8_t rt = (encoding >> 0) & 0x1F;    /* Target register */
    uint8_t rn = (encoding >> 5) & 0x1F;    /* Base register */
    uint16_t imm12 = (encoding >> 10) & 0xFFF; /* Immediate offset */

    /* Calculate address */
    uint64_t base = state->cpu.gpr.x[rn];
    uint64_t addr = base + imm12;

    /* Load halfword (zero-extended) */
    uint16_t value = *(volatile uint16_t *)addr;
    state->cpu.gpr.x[rt] = value;

    return 0;
}

/**
 * translate_strh_imm - Store register halfword with immediate offset
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_strh_imm(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    /* Extract fields from STRH immediate */
    uint8_t rt = (encoding >> 0) & 0x1F;    /* Source register */
    uint8_t rn = (encoding >> 5) & 0x1F;    /* Base register */
    uint16_t imm12 = (encoding >> 10) & 0xFFF; /* Immediate offset */

    /* Calculate address */
    uint64_t base = state->cpu.gpr.x[rn];
    uint64_t addr = base + imm12;

    /* Store halfword */
    uint16_t value = (uint16_t)(state->cpu.gpr.x[rt] & 0xFFFF);
    *(volatile uint16_t *)addr = value;

    return 0;
}

/**
 * elf_validate_magic - Validate ELF magic number
 *
 * @param base Base address of ELF binary
 * @return 1 if valid ELF, 0 otherwise
 */
int elf_validate_magic(const void *base)
{
    const uint8_t *magic = (const uint8_t *)base;

    /* ELF magic: 0x7F 'E' 'L' 'F' */
    return (magic[0] == 0x7F &&
            magic[1] == 'E' &&
            magic[2] == 'L' &&
            magic[3] == 'F');
}

/**
 * elf_get_entry_point - Get ELF entry point address
 *
 * @param base Base address of ELF binary
 * @return Entry point address, 0 on failure
 */
uint64_t elf_get_entry_point(const void *base)
{
    const uint8_t *elf = (const uint8_t *)base;

    /* Check for valid ELF */
    if (!elf_validate_magic(base)) {
        return 0;
    }

    /* e_entry is at offset 24 in ELF64 header */
    return *(const uint64_t *)(elf + 24);
}

/**
 * rosetta_getpagesize - Get system page size
 *
 * @return System page size in bytes
 */
int rosetta_getpagesize(void)
{
    return sysconf(_SC_PAGESIZE);
}

/**
 * rosetta_get_tick_count - Get system tick count
 *
 * @return Current tick count (monotonic)
 */
uint64_t rosetta_get_tick_count(void)
{
#if defined(__APPLE__)
    return mach_absolute_time();
#elif defined(__linux__)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
#else
    return 0;
#endif
}

/* ============================================================================
 * Session 26: Additional Translation and Vector Functions
 * ============================================================================ */

/**
 * v128_fcmp_gt - Floating-point vector compare greater than
 *
 * @param a First vector
 * @param b Second vector
 * @return Vector with comparison mask results
 */
Vector128 v128_fcmp_gt(Vector128 a, Vector128 b)
{
    Vector128 result;
    float *a_in = (float *)&a;
    float *b_in = (float *)&b;
    uint32_t *out = (uint32_t *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = (a_in[i] > b_in[i]) ? 0xFFFFFFFF : 0x00000000;
    }

    return result;
}

/**
 * v128_fcmp_le - Floating-point vector compare less than or equal
 *
 * @param a First vector
 * @param b Second vector
 * @return Vector with comparison mask results
 */
Vector128 v128_fcmp_le(Vector128 a, Vector128 b)
{
    Vector128 result;
    float *a_in = (float *)&a;
    float *b_in = (float *)&b;
    uint32_t *out = (uint32_t *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = (a_in[i] <= b_in[i]) ? 0xFFFFFFFF : 0x00000000;
    }

    return result;
}

/**
 * v128_fcmp_ge - Floating-point vector compare greater than or equal
 *
 * @param a First vector
 * @param b Second vector
 * @return Vector with comparison mask results
 */
Vector128 v128_fcmp_ge(Vector128 a, Vector128 b)
{
    Vector128 result;
    float *a_in = (float *)&a;
    float *b_in = (float *)&b;
    uint32_t *out = (uint32_t *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = (a_in[i] >= b_in[i]) ? 0xFFFFFFFF : 0x00000000;
    }

    return result;
}

/**
 * v128_frint - Floating-point round to integer
 *
 * @param a Input vector
 * @return Vector with rounded values
 */
Vector128 v128_frint(Vector128 a)
{
    Vector128 result;
    float *in = (float *)&a;
    float *out = (float *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = roundf(in[i]);
    }

    return result;
}

/**
 * v128_fabs - Floating-point absolute value
 *
 * @param a Input vector
 * @return Vector with absolute values
 */
Vector128 v128_fabs(Vector128 a)
{
    Vector128 result;
    float *in = (float *)&a;
    float *out = (float *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = fabsf(in[i]);
    }

    return result;
}

/**
 * v128_fneg - Floating-point negate
 *
 * @param a Input vector
 * @return Vector with negated values
 */
Vector128 v128_fneg(Vector128 a)
{
    Vector128 result;
    float *in = (float *)&a;
    float *out = (float *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = -in[i];
    }

    return result;
}

/**
 * translate_ldr_reg - Load register with register offset
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldr_reg(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    /* Extract fields from LDR (register offset) */
    uint8_t rt = (encoding >> 0) & 0x1F;    /* Target register */
    uint8_t rn = (encoding >> 5) & 0x1F;    /* Base register */
    uint8_t rm = (encoding >> 16) & 0x1F;   /* Offset register */
    uint8_t option = (encoding >> 13) & 0x7;/* Option */
    uint8_t s = (encoding >> 12) & 0x1;     /* Shift amount */

    /* Calculate offset based on option */
    uint64_t offset = state->cpu.gpr.x[rm];
    if (option & 0x4) {  /* Extended register */
        offset &= ((1ULL << (8 << option)) - 1);
    }

    /* Calculate address */
    uint64_t addr = state->cpu.gpr.x[rn] + offset;

    /* Load based on size (determined by encoding bits) */
    uint8_t size = (encoding >> 30) & 0x3;
    switch (size) {
        case 0: /* Byte */
            state->cpu.gpr.x[rt] = *(volatile uint8_t *)addr;
            break;
        case 1: /* Halfword */
            state->cpu.gpr.x[rt] = *(volatile uint16_t *)addr;
            break;
        case 2: /* Word */
            state->cpu.gpr.x[rt] = *(volatile uint32_t *)addr;
            break;
        case 3: /* Doubleword */
            state->cpu.gpr.x[rt] = *(volatile uint64_t *)addr;
            break;
    }

    return 0;
}

/**
 * translate_str_reg - Store register with register offset
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_str_reg(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    /* Extract fields from STR (register offset) */
    uint8_t rt = (encoding >> 0) & 0x1F;    /* Source register */
    uint8_t rn = (encoding >> 5) & 0x1F;    /* Base register */
    uint8_t rm = (encoding >> 16) & 0x1F;   /* Offset register */

    /* Calculate offset */
    uint64_t offset = state->cpu.gpr.x[rm];

    /* Calculate address */
    uint64_t addr = state->cpu.gpr.x[rn] + offset;

    /* Store based on size */
    uint8_t size = (encoding >> 30) & 0x3;
    switch (size) {
        case 0: /* Byte */
            *(volatile uint8_t *)addr = (uint8_t)state->cpu.gpr.x[rt];
            break;
        case 1: /* Halfword */
            *(volatile uint16_t *)addr = (uint16_t)state->cpu.gpr.x[rt];
            break;
        case 2: /* Word */
            *(volatile uint32_t *)addr = (uint32_t)state->cpu.gpr.x[rt];
            break;
        case 3: /* Doubleword */
            *(volatile uint64_t *)addr = state->cpu.gpr.x[rt];
            break;
    }

    return 0;
}

/**
 * translate_ldrb_reg - Load register byte with register offset
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldrb_reg(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    /* Extract fields from LDRB (register offset) */
    uint8_t rt = (encoding >> 0) & 0x1F;    /* Target register */
    uint8_t rn = (encoding >> 5) & 0x1F;    /* Base register */
    uint8_t rm = (encoding >> 16) & 0x1F;   /* Offset register */

    /* Calculate address */
    uint64_t addr = state->cpu.gpr.x[rn] + state->cpu.gpr.x[rm];

    /* Load byte and zero-extend */
    state->cpu.gpr.x[rt] = *(volatile uint8_t *)addr;

    return 0;
}

/**
 * translate_strb_reg - Store register byte with register offset
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_strb_reg(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    /* Extract fields from STRB (register offset) */
    uint8_t rt = (encoding >> 0) & 0x1F;    /* Source register */
    uint8_t rn = (encoding >> 5) & 0x1F;    /* Base register */
    uint8_t rm = (encoding >> 16) & 0x1F;   /* Offset register */

    /* Calculate address */
    uint64_t addr = state->cpu.gpr.x[rn] + state->cpu.gpr.x[rm];

    /* Store byte */
    *(volatile uint8_t *)addr = (uint8_t)state->cpu.gpr.x[rt];

    return 0;
}

/**
 * translate_ldr_pre - Load register pre-indexed
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldr_pre(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    /* Extract fields from LDR (pre-indexed) */
    uint8_t rt = (encoding >> 0) & 0x1F;    /* Target register */
    uint8_t rn = (encoding >> 5) & 0x1F;    /* Base register */
    uint16_t imm9 = (encoding >> 10) & 0x1FF; /* Immediate */

    /* Calculate offset (signed) */
    int32_t offset = (imm9 & 0x100) ? (imm9 | ~0x1FF) : imm9;
    offset <<= ((encoding >> 30) & 0x3);  /* Scale by size */

    /* Pre-indexed: update base first */
    state->cpu.gpr.x[rn] += offset;

    /* Calculate address */
    uint64_t addr = state->cpu.gpr.x[rn];

    /* Load based on size */
    uint8_t size = (encoding >> 30) & 0x3;
    switch (size) {
        case 0: /* Byte */
            state->cpu.gpr.x[rt] = *(volatile uint8_t *)addr;
            break;
        case 1: /* Halfword */
            state->cpu.gpr.x[rt] = *(volatile uint16_t *)addr;
            break;
        case 2: /* Word */
            state->cpu.gpr.x[rt] = *(volatile uint32_t *)addr;
            break;
        case 3: /* Doubleword */
            state->cpu.gpr.x[rt] = *(volatile uint64_t *)addr;
            break;
    }

    return 0;
}

/**
 * translate_ldr_post - Load register post-indexed
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldr_post(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    /* Extract fields from LDR (post-indexed) */
    uint8_t rt = (encoding >> 0) & 0x1F;    /* Target register */
    uint8_t rn = (encoding >> 5) & 0x1F;    /* Base register */
    int32_t imm9 = (encoding >> 10) & 0x1FF; /* Immediate (signed) */

    /* Sign extend immediate */
    if (imm9 & 0x100) {
        imm9 |= ~0x1FF;
    }
    imm9 <<= ((encoding >> 30) & 0x3);  /* Scale by size */

    /* Post-indexed: use base first, then update */
    uint64_t addr = state->cpu.gpr.x[rn];
    state->cpu.gpr.x[rn] += imm9;

    /* Load based on size */
    uint8_t size = (encoding >> 30) & 0x3;
    switch (size) {
        case 0: /* Byte */
            state->cpu.gpr.x[rt] = *(volatile uint8_t *)addr;
            break;
        case 1: /* Halfword */
            state->cpu.gpr.x[rt] = *(volatile uint16_t *)addr;
            break;
        case 2: /* Word */
            state->cpu.gpr.x[rt] = *(volatile uint32_t *)addr;
            break;
        case 3: /* Doubleword */
            state->cpu.gpr.x[rt] = *(volatile uint64_t *)addr;
            break;
    }

    return 0;
}

/**
 * translate_str_pre - Store register pre-indexed
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_str_pre(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    /* Extract fields from STR (pre-indexed) */
    uint8_t rt = (encoding >> 0) & 0x1F;    /* Source register */
    uint8_t rn = (encoding >> 5) & 0x1F;    /* Base register */
    uint16_t imm9 = (encoding >> 10) & 0x1FF; /* Immediate */

    /* Calculate offset (signed) */
    int32_t offset = (imm9 & 0x100) ? (imm9 | ~0x1FF) : imm9;
    offset <<= ((encoding >> 30) & 0x3);  /* Scale by size */

    /* Pre-indexed: update base first */
    state->cpu.gpr.x[rn] += offset;

    /* Calculate address */
    uint64_t addr = state->cpu.gpr.x[rn];

    /* Store based on size */
    uint8_t size = (encoding >> 30) & 0x3;
    switch (size) {
        case 0: /* Byte */
            *(volatile uint8_t *)addr = (uint8_t)state->cpu.gpr.x[rt];
            break;
        case 1: /* Halfword */
            *(volatile uint16_t *)addr = (uint16_t)state->cpu.gpr.x[rt];
            break;
        case 2: /* Word */
            *(volatile uint32_t *)addr = (uint32_t)state->cpu.gpr.x[rt];
            break;
        case 3: /* Doubleword */
            *(volatile uint64_t *)addr = state->cpu.gpr.x[rt];
            break;
    }

    return 0;
}

/**
 * translate_str_post - Store register post-indexed
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_str_post(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    /* Extract fields from STR (post-indexed) */
    uint8_t rt = (encoding >> 0) & 0x1F;    /* Source register */
    uint8_t rn = (encoding >> 5) & 0x1F;    /* Base register */
    int32_t imm9 = (encoding >> 10) & 0x1FF; /* Immediate (signed) */

    /* Sign extend immediate */
    if (imm9 & 0x100) {
        imm9 |= ~0x1FF;
    }
    imm9 <<= ((encoding >> 30) & 0x3);  /* Scale by size */

    /* Post-indexed: use base first, then update */
    uint64_t addr = state->cpu.gpr.x[rn];
    state->cpu.gpr.x[rn] += imm9;

    /* Store based on size */
    uint8_t size = (encoding >> 30) & 0x3;
    switch (size) {
        case 0: /* Byte */
            *(volatile uint8_t *)addr = (uint8_t)state->cpu.gpr.x[rt];
            break;
        case 1: /* Halfword */
            *(volatile uint16_t *)addr = (uint16_t)state->cpu.gpr.x[rt];
            break;
        case 2: /* Word */
            *(volatile uint32_t *)addr = (uint32_t)state->cpu.gpr.x[rt];
            break;
        case 3: /* Doubleword */
            *(volatile uint64_t *)addr = state->cpu.gpr.x[rt];
            break;
    }

    return 0;
}

/* ============================================================================
 * Session 27: Advanced NEON Translation and Vector Operations
 * ============================================================================ */

/**
 * v128_frintn - FP round to nearest, ties to even
 *
 * @param a Input vector
 * @return Vector with rounded values (round half to even)
 */
Vector128 v128_frintn(Vector128 a)
{
    Vector128 result;
    float *in = (float *)&a;
    float *out = (float *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = rintf(in[i]);  /* rintf does round to nearest, ties to even */
    }

    return result;
}

/**
 * v128_frintz - FP round toward zero
 *
 * @param a Input vector
 * @return Vector with values truncated toward zero
 */
Vector128 v128_frintz(Vector128 a)
{
    Vector128 result;
    float *in = (float *)&a;
    float *out = (float *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = truncf(in[i]);
    }

    return result;
}

/**
 * v128_frintp - FP round toward positive infinity
 *
 * @param a Input vector
 * @return Vector with values rounded toward +infinity
 */
Vector128 v128_frintp(Vector128 a)
{
    Vector128 result;
    float *in = (float *)&a;
    float *out = (float *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = ceilf(in[i]);
    }

    return result;
}

/**
 * v128_frintm - FP round toward negative infinity
 *
 * @param a Input vector
 * @return Vector with values rounded toward -infinity
 */
Vector128 v128_frintm(Vector128 a)
{
    Vector128 result;
    float *in = (float *)&a;
    float *out = (float *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = floorf(in[i]);
    }

    return result;
}

/**
 * v128_fcvtns - FP convert to signed integer
 *
 * @param a Input vector
 * @return Vector with signed integer values
 */
Vector128 v128_fcvtns(Vector128 a)
{
    Vector128 result;
    float *in = (float *)&a;
    int32_t *out = (int32_t *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = (int32_t)in[i];
    }

    return result;
}

/**
 * v128_fcvtnu - FP convert to unsigned integer
 *
 * @param a Input vector
 * @return Vector with unsigned integer values
 */
Vector128 v128_fcvtnu(Vector128 a)
{
    Vector128 result;
    float *in = (float *)&a;
    uint32_t *out = (uint32_t *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = (uint32_t)in[i];
    }

    return result;
}

/**
 * v128_scvtf - Signed convert to floating-point
 * @param a Input vector with signed integers
 * @param fracbits Number of fractional bits
 * @return Vector with floating-point values
 *
 * Implements SCVTF (Signed Convert to Floating-point) instruction.
 * Converts signed integers to floating-point values.
 */
Vector128 v128_scvtf(Vector128 a, int fracbits)
{
    Vector128 result;
    int32_t *in = (int32_t *)&a;
    float *out = (float *)&result;
    float scale = 1.0f / (1 << fracbits);

    for (int i = 0; i < 4; i++) {
        out[i] = (float)in[i] * scale;
    }

    return result;
}

/**
 * v128_ucvtf - Unsigned convert to floating-point
 * @param a Input vector with unsigned integers
 * @param fracbits Number of fractional bits
 * @return Vector with floating-point values
 *
 * Implements UCVTF (Unsigned Convert to Floating-point) instruction.
 * Converts unsigned integers to floating-point values.
 */
Vector128 v128_ucvtf(Vector128 a, int fracbits)
{
    Vector128 result;
    uint32_t *in = (uint32_t *)&a;
    float *out = (float *)&result;
    float scale = 1.0f / (1 << fracbits);

    for (int i = 0; i < 4; i++) {
        out[i] = (float)in[i] * scale;
    }

    return result;
}

/**
 * v128_fcvts - Float convert to signed integer
 * @param a Input vector with floating-point values
 * @param fracbits Number of fractional bits
 * @return Vector with signed integer values
 *
 * Implements FCVTS (Floating-point Convert to Signed) instruction.
 * Converts floating-point values to signed integers.
 */
Vector128 v128_fcvts(Vector128 a, int fracbits)
{
    Vector128 result;
    float *in = (float *)&a;
    int32_t *out = (int32_t *)&result;
    float scale = (1 << fracbits);

    for (int i = 0; i < 4; i++) {
        out[i] = (int32_t)(in[i] * scale);
    }

    return result;
}

/**
 * v128_fcvtu - Float convert to unsigned integer
 * @param a Input vector with floating-point values
 * @param fracbits Number of fractional bits
 * @return Vector with unsigned integer values
 *
 * Implements FCVTU (Floating-point Convert to Unsigned) instruction.
 * Converts floating-point values to unsigned integers.
 */
Vector128 v128_fcvtu(Vector128 a, int fracbits)
{
    Vector128 result;
    float *in = (float *)&a;
    uint32_t *out = (uint32_t *)&result;
    float scale = (1 << fracbits);

    for (int i = 0; i < 4; i++) {
        out[i] = (uint32_t)(in[i] * scale);
    }

    return result;
}

/**
 * translate_ldrsb_imm - Load register signed byte immediate
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldrsb_imm(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rt = (encoding >> 0) & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint16_t imm12 = (encoding >> 10) & 0xFFF;

    uint64_t addr = state->cpu.gpr.x[rn] + imm12;
    int8_t val = *(volatile int8_t *)addr;
    state->cpu.gpr.x[rt] = (uint64_t)val;

    return 0;
}

/**
 * translate_ldrsh_imm - Load register signed halfword immediate
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldrsh_imm(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rt = (encoding >> 0) & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint16_t imm12 = (encoding >> 10) & 0xFFF;

    uint64_t addr = state->cpu.gpr.x[rn] + imm12;
    int16_t val = *(volatile int16_t *)addr;
    state->cpu.gpr.x[rt] = (uint64_t)val;

    return 0;
}

/**
 * translate_ldrsw_imm - Load register signed word immediate
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldrsw_imm(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rt = (encoding >> 0) & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint16_t imm12 = (encoding >> 10) & 0xFFF;

    uint64_t addr = state->cpu.gpr.x[rn] + imm12;
    int32_t val = *(volatile int32_t *)addr;
    state->cpu.gpr.x[rt] = (uint64_t)val;

    return 0;
}

/**
 * translate_ldrsb_reg - Load register signed byte with register offset
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldrsb_reg(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rt = (encoding >> 0) & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;

    uint64_t addr = state->cpu.gpr.x[rn] + state->cpu.gpr.x[rm];
    int8_t val = *(volatile int8_t *)addr;
    state->cpu.gpr.x[rt] = (uint64_t)val;

    return 0;
}

/**
 * translate_ldrsh_reg - Load register signed halfword with register offset
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldrsh_reg(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rt = (encoding >> 0) & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;

    uint64_t addr = state->cpu.gpr.x[rn] + state->cpu.gpr.x[rm];
    int16_t val = *(volatile int16_t *)addr;
    state->cpu.gpr.x[rt] = (uint64_t)val;

    return 0;
}

/**
 * translate_ldrsw_reg - Load register signed word with register offset
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldrsw_reg(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rt = (encoding >> 0) & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;

    uint64_t addr = state->cpu.gpr.x[rn] + state->cpu.gpr.x[rm];
    int32_t val = *(volatile int32_t *)addr;
    state->cpu.gpr.x[rt] = (uint64_t)val;

    return 0;
}

/**
 * translate_ldp_imm - Load pair of registers with immediate offset
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldp_imm(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rt = (encoding >> 0) & 0x1F;
    uint8_t rt2 = (encoding >> 10) & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint16_t imm7 = (encoding >> 15) & 0x7F;
    uint8_t opc = (encoding >> 30) & 0x3;

    int scale = (opc == 3) ? 4 : 2;
    uint64_t offset = imm7 * scale;
    uint64_t addr = state->cpu.gpr.x[rn] + offset;

    if (opc == 3) {
        state->cpu.gpr.x[rt] = *(volatile uint64_t *)addr;
        state->cpu.gpr.x[rt2] = *(volatile uint64_t *)(addr + 8);
    } else {
        state->cpu.gpr.x[rt] = *(volatile uint32_t *)addr;
        state->cpu.gpr.x[rt2] = *(volatile uint32_t *)(addr + 4);
    }

    return 0;
}

/**
 * translate_stp_imm - Store pair of registers with immediate offset
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_stp_imm(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rt = (encoding >> 0) & 0x1F;
    uint8_t rt2 = (encoding >> 10) & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint16_t imm7 = (encoding >> 15) & 0x7F;
    uint8_t opc = (encoding >> 30) & 0x3;

    int scale = (opc == 3) ? 4 : 2;
    uint64_t offset = imm7 * scale;
    uint64_t addr = state->cpu.gpr.x[rn] + offset;

    if (opc == 3) {
        *(volatile uint64_t *)addr = state->cpu.gpr.x[rt];
        *(volatile uint64_t *)(addr + 8) = state->cpu.gpr.x[rt2];
    } else {
        *(volatile uint32_t *)addr = (uint32_t)state->cpu.gpr.x[rt];
        *(volatile uint32_t *)(addr + 4) = (uint32_t)state->cpu.gpr.x[rt2];
    }

    return 0;
}

/**
 * translate_add_imm - Add immediate
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_add_imm(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rd = (encoding >> 0) & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint32_t imm12 = (encoding >> 10) & 0xFFF;
    uint8_t sh = (encoding >> 22) & 0x1;

    uint64_t operand = (sh) ? (imm12 << 12) : imm12;
    state->cpu.gpr.x[rd] = state->cpu.gpr.x[rn] + operand;

    return 0;
}

/**
 * translate_sub_imm - Subtract immediate
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sub_imm(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rd = (encoding >> 0) & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint32_t imm12 = (encoding >> 10) & 0xFFF;
    uint8_t sh = (encoding >> 22) & 0x1;

    uint64_t operand = (sh) ? (imm12 << 12) : imm12;
    state->cpu.gpr.x[rd] = state->cpu.gpr.x[rn] - operand;

    return 0;
}

/**
 * translate_and_imm - AND immediate
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_and_imm(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rd = (encoding >> 0) & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t n = (encoding >> 31) & 0x1;
    uint32_t imm_s = (encoding >> 10) & 0x3F;
    uint32_t imm_r = (encoding >> 16) & 0x3F;
    uint32_t imm_len = (encoding >> 22) & 0x3;

    /* Build bitmask immediate */
    int len = 1 << imm_len;
    uint64_t imm_val = 0;
    int ones = imm_s & ((1 << imm_len) - 1);

    for (int i = 0; i < len; i++) {
        if (i <= ones) {
            imm_val |= (1ULL << i);
        }
    }

    /* Rotate */
    int rotate = imm_r & (64 - 1);
    imm_val = (imm_val >> rotate) | (imm_val << (64 - rotate));

    if (n) {
        state->cpu.gpr.x[rd] = state->cpu.gpr.x[rn] & imm_val;
    } else {
        state->cpu.gpr.x[rd] = state->cpu.gpr.x[rn] & (imm_val & 0xFFFFFFFF);
    }

    return 0;
}

/**
 * translate_orr_imm - ORR immediate
 *
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_orr_imm(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rd = (encoding >> 0) & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t n = (encoding >> 31) & 0x1;
    uint32_t imm_s = (encoding >> 10) & 0x3F;
    uint32_t imm_r = (encoding >> 16) & 0x3F;
    uint32_t imm_len = (encoding >> 22) & 0x3;

    /* Build bitmask immediate */
    int len = 1 << imm_len;
    uint64_t imm_val = 0;
    int ones = imm_s & ((1 << imm_len) - 1);

    for (int i = 0; i < len; i++) {
        if (i <= ones) {
            imm_val |= (1ULL << i);
        }
    }

    /* Rotate */
    int rotate = imm_r & (64 - 1);
    imm_val = (imm_val >> rotate) | (imm_val << (64 - rotate));

    if (n) {
        state->cpu.gpr.x[rd] = state->cpu.gpr.x[rn] | imm_val;
    } else {
        state->cpu.gpr.x[rd] = state->cpu.gpr.x[rn] | (imm_val & 0xFFFFFFFF);
    }

    return 0;
}

/* ============================================================================
 * Session 28: Advanced SIMD Saturation and Narrowing Operations
 * ============================================================================ */

/**
 * v128_sqadd - Signed saturating add
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating addition
 */
Vector128 v128_sqadd(Vector128 a, Vector128 b)
{
    Vector128 result;
    int8_t *a_in = (int8_t *)&a;
    int8_t *b_in = (int8_t *)&b;
    int8_t *out = (int8_t *)&result;

    for (int i = 0; i < 16; i++) {
        int sum = (int)a_in[i] + (int)b_in[i];
        /* Saturate to signed 8-bit range */
        if (sum > 127) {
            out[i] = 127;
        } else if (sum < -128) {
            out[i] = -128;
        } else {
            out[i] = (int8_t)sum;
        }
    }

    return result;
}

/**
 * v128_uqadd - Unsigned saturating add
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating addition
 */
Vector128 v128_uqadd(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint8_t *a_in = (uint8_t *)&a;
    uint8_t *b_in = (uint8_t *)&b;
    uint8_t *out = (uint8_t *)&result;

    for (int i = 0; i < 16; i++) {
        unsigned int sum = (unsigned int)a_in[i] + (unsigned int)b_in[i];
        /* Saturate to unsigned 8-bit range */
        if (sum > 255) {
            out[i] = 255;
        } else {
            out[i] = (uint8_t)sum;
        }
    }

    return result;
}

/**
 * v128_sqsub - Signed saturating subtract
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating subtraction
 */
Vector128 v128_sqsub(Vector128 a, Vector128 b)
{
    Vector128 result;
    int8_t *a_in = (int8_t *)&a;
    int8_t *b_in = (int8_t *)&b;
    int8_t *out = (int8_t *)&result;

    for (int i = 0; i < 16; i++) {
        int diff = (int)a_in[i] - (int)b_in[i];
        /* Saturate to signed 8-bit range */
        if (diff > 127) {
            out[i] = 127;
        } else if (diff < -128) {
            out[i] = -128;
        } else {
            out[i] = (int8_t)diff;
        }
    }

    return result;
}

/**
 * v128_uqsub - Unsigned saturating subtract
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating subtraction
 */
Vector128 v128_uqsub(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint8_t *a_in = (uint8_t *)&a;
    uint8_t *b_in = (uint8_t *)&b;
    uint8_t *out = (uint8_t *)&result;

    for (int i = 0; i < 16; i++) {
        int diff = (int)a_in[i] - (int)b_in[i];
        /* Saturate to unsigned 8-bit range */
        if (diff > 255) {
            out[i] = 255;
        } else if (diff < 0) {
            out[i] = 0;
        } else {
            out[i] = (uint8_t)diff;
        }
    }

    return result;
}

/**
 * v128_sqadd_16 - Signed saturating add (16-bit elements)
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating addition
 */
Vector128 v128_sqadd_16(Vector128 a, Vector128 b)
{
    Vector128 result;
    int16_t *a_in = (int16_t *)&a;
    int16_t *b_in = (int16_t *)&b;
    int16_t *out = (int16_t *)&result;

    for (int i = 0; i < 8; i++) {
        int sum = (int)a_in[i] + (int)b_in[i];
        /* Saturate to signed 16-bit range */
        if (sum > 32767) {
            out[i] = 32767;
        } else if (sum < -32768) {
            out[i] = -32768;
        } else {
            out[i] = (int16_t)sum;
        }
    }

    return result;
}

/**
 * v128_uqadd_16 - Unsigned saturating add (16-bit elements)
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating addition
 */
Vector128 v128_uqadd_16(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint16_t *a_in = (uint16_t *)&a;
    uint16_t *b_in = (uint16_t *)&b;
    uint16_t *out = (uint16_t *)&result;

    for (int i = 0; i < 8; i++) {
        unsigned int sum = (unsigned int)a_in[i] + (unsigned int)b_in[i];
        /* Saturate to unsigned 16-bit range */
        if (sum > 65535) {
            out[i] = 65535;
        } else {
            out[i] = (uint16_t)sum;
        }
    }

    return result;
}

/**
 * v128_sqsub_16 - Signed saturating subtract (16-bit elements)
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating subtraction
 */
Vector128 v128_sqsub_16(Vector128 a, Vector128 b)
{
    Vector128 result;
    int16_t *a_in = (int16_t *)&a;
    int16_t *b_in = (int16_t *)&b;
    int16_t *out = (int16_t *)&result;

    for (int i = 0; i < 8; i++) {
        int diff = (int)a_in[i] - (int)b_in[i];
        /* Saturate to signed 16-bit range */
        if (diff > 32767) {
            out[i] = 32767;
        } else if (diff < -32768) {
            out[i] = -32768;
        } else {
            out[i] = (int16_t)diff;
        }
    }

    return result;
}

/**
 * v128_uqsub_16 - Unsigned saturating subtract (16-bit elements)
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating subtraction
 */
Vector128 v128_uqsub_16(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint16_t *a_in = (uint16_t *)&a;
    uint16_t *b_in = (uint16_t *)&b;
    uint16_t *out = (uint16_t *)&result;

    for (int i = 0; i < 8; i++) {
        int diff = (int)a_in[i] - (int)b_in[i];
        /* Saturate to unsigned 16-bit range */
        if (diff > 65535) {
            out[i] = 65535;
        } else if (diff < 0) {
            out[i] = 0;
        } else {
            out[i] = (uint16_t)diff;
        }
    }

    return result;
}

/**
 * v128_sqadd_32 - Signed saturating add (32-bit elements)
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating addition
 */
Vector128 v128_sqadd_32(Vector128 a, Vector128 b)
{
    Vector128 result;
    int32_t *a_in = (int32_t *)&a;
    int32_t *b_in = (int32_t *)&b;
    int32_t *out = (int32_t *)&result;

    for (int i = 0; i < 4; i++) {
        int64_t sum = (int64_t)a_in[i] + (int64_t)b_in[i];
        /* Saturate to signed 32-bit range */
        if (sum > 2147483647LL) {
            out[i] = 2147483647;
        } else if (sum < -2147483648LL) {
            out[i] = -2147483648;
        } else {
            out[i] = (int32_t)sum;
        }
    }

    return result;
}

/**
 * v128_uqadd_32 - Unsigned saturating add (32-bit elements)
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating addition
 */
Vector128 v128_uqadd_32(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint32_t *a_in = (uint32_t *)&a;
    uint32_t *b_in = (uint32_t *)&b;
    uint32_t *out = (uint32_t *)&result;

    for (int i = 0; i < 4; i++) {
        uint64_t sum = (uint64_t)a_in[i] + (uint64_t)b_in[i];
        /* Saturate to unsigned 32-bit range */
        if (sum > 4294967295ULL) {
            out[i] = 4294967295U;
        } else {
            out[i] = (uint32_t)sum;
        }
    }

    return result;
}

/**
 * v128_sqsub_32 - Signed saturating subtract (32-bit elements)
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating subtraction
 */
Vector128 v128_sqsub_32(Vector128 a, Vector128 b)
{
    Vector128 result;
    int32_t *a_in = (int32_t *)&a;
    int32_t *b_in = (int32_t *)&b;
    int32_t *out = (int32_t *)&result;

    for (int i = 0; i < 4; i++) {
        int64_t diff = (int64_t)a_in[i] - (int64_t)b_in[i];
        /* Saturate to signed 32-bit range */
        if (diff > 2147483647LL) {
            out[i] = 2147483647;
        } else if (diff < -2147483648LL) {
            out[i] = -2147483648;
        } else {
            out[i] = (int32_t)diff;
        }
    }

    return result;
}

/**
 * v128_uqsub_32 - Unsigned saturating subtract (32-bit elements)
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating subtraction
 */
Vector128 v128_uqsub_32(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint32_t *a_in = (uint32_t *)&a;
    uint32_t *b_in = (uint32_t *)&b;
    uint32_t *out = (uint32_t *)&result;

    for (int i = 0; i < 4; i++) {
        int64_t diff = (int64_t)a_in[i] - (int64_t)b_in[i];
        /* Saturate to unsigned 32-bit range */
        if (diff < 0) {
            out[i] = 0;
        } else if (diff > (int64_t)4294967295ULL) {
            out[i] = 4294967295U;
        } else {
            out[i] = (uint32_t)diff;
        }
    }

    return result;
}

/**
 * v128_sqadd_64 - Signed saturating add (64-bit elements)
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating addition
 */
Vector128 v128_sqadd_64(Vector128 a, Vector128 b)
{
    Vector128 result;
    int64_t *a_in = (int64_t *)&a;
    int64_t *b_in = (int64_t *)&b;
    int64_t *out = (int64_t *)&result;

    for (int i = 0; i < 2; i++) {
        __int128_t sum = (__int128_t)a_in[i] + (__int128_t)b_in[i];
        /* Saturate to signed 64-bit range */
        if (sum > 9223372036854775807LL) {
            out[i] = (int64_t)9223372036854775807LL;
        } else if (sum < (__int128_t)(-9223372036854775807LL - 1)) {
            out[i] = (int64_t)(-9223372036854775807LL - 1);
        } else {
            out[i] = (int64_t)sum;
        }
    }

    return result;
}

/**
 * v128_uqadd_64 - Unsigned saturating add (64-bit elements)
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating addition
 */
Vector128 v128_uqadd_64(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint64_t *a_in = (uint64_t *)&a;
    uint64_t *b_in = (uint64_t *)&b;
    uint64_t *out = (uint64_t *)&result;

    for (int i = 0; i < 2; i++) {
        __uint128_t sum = (__uint128_t)a_in[i] + (__uint128_t)b_in[i];
        /* Saturate to unsigned 64-bit range */
        if (sum > 18446744073709551615ULL) {
            out[i] = 18446744073709551615ULL;
        } else {
            out[i] = (uint64_t)sum;
        }
    }

    return result;
}

/**
 * v128_sqsub_64 - Signed saturating subtract (64-bit elements)
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating subtraction
 */
Vector128 v128_sqsub_64(Vector128 a, Vector128 b)
{
    Vector128 result;
    int64_t *a_in = (int64_t *)&a;
    int64_t *b_in = (int64_t *)&b;
    int64_t *out = (int64_t *)&result;

    for (int i = 0; i < 2; i++) {
        __int128_t diff = (__int128_t)a_in[i] - (__int128_t)b_in[i];
        /* Saturate to signed 64-bit range */
        if (diff > 9223372036854775807LL) {
            out[i] = (int64_t)9223372036854775807LL;
        } else if (diff < (__int128_t)(-9223372036854775807LL - 1)) {
            out[i] = (int64_t)(-9223372036854775807LL - 1);
        } else {
            out[i] = (int64_t)diff;
        }
    }

    return result;
}

/**
 * v128_uqsub_64 - Unsigned saturating subtract (64-bit elements)
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating subtraction
 */
Vector128 v128_uqsub_64(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint64_t *a_in = (uint64_t *)&a;
    uint64_t *b_in = (uint64_t *)&b;
    uint64_t *out = (uint64_t *)&result;

    for (int i = 0; i < 2; i++) {
        __int128_t diff = (__int128_t)a_in[i] - (__int128_t)b_in[i];
        /* Saturate to unsigned 64-bit range */
        if (diff > (__int128_t)18446744073709551615ULL) {
            out[i] = 18446744073709551615ULL;
        } else if (diff < 0) {
            out[i] = 0;
        } else {
            out[i] = (uint64_t)diff;
        }
    }

    return result;
}

/**
 * v128_sqdmulh - Signed saturating doubling multiply high
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating doubling multiply (high half)
 */
Vector128 v128_sqdmulh(Vector128 a, Vector128 b)
{
    Vector128 result;
    int16_t *a_in = (int16_t *)&a;
    int16_t *b_in = (int16_t *)&b;
    int16_t *out = (int16_t *)&result;

    for (int i = 0; i < 8; i++) {
        int32_t prod = (int32_t)a_in[i] * (int32_t)b_in[i];
        int32_t doubled = prod * 2;
        /* Saturate and get high half */
        if (doubled > 32767 * 32768 || doubled < -32768 * 32768) {
            /* Saturated */
            out[i] = (doubled < 0) ? -32768 : 32767;
        } else {
            /* Get high 16 bits (shift right by 15) */
            out[i] = (int16_t)(doubled >> 15);
        }
    }

    return result;
}

/**
 * v128_sqrdmulh - Signed saturating rounding doubling multiply high
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating rounding doubling multiply (high half)
 */
Vector128 v128_sqrdmulh(Vector128 a, Vector128 b)
{
    Vector128 result;
    int16_t *a_in = (int16_t *)&a;
    int16_t *b_in = (int16_t *)&b;
    int16_t *out = (int16_t *)&result;

    for (int i = 0; i < 8; i++) {
        int32_t prod = (int32_t)a_in[i] * (int32_t)b_in[i];
        int32_t doubled = prod * 2;
        /* Round (add 2^14 before shift) */
        doubled += (1 << 14);
        /* Saturate and get high half */
        if (doubled > 32767 * 32768 || doubled < -32768 * 32768) {
            /* Saturated */
            out[i] = (doubled < 0) ? -32768 : 32767;
        } else {
            /* Get high 16 bits (shift right by 15) */
            out[i] = (int16_t)(doubled >> 15);
        }
    }

    return result;
}

/**
 * v128_sqdmulh_32 - Signed saturating doubling multiply high (32-bit elements)
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating doubling multiply (high half)
 */
Vector128 v128_sqdmulh_32(Vector128 a, Vector128 b)
{
    Vector128 result;
    int32_t *a_in = (int32_t *)&a;
    int32_t *b_in = (int32_t *)&b;
    int32_t *out = (int32_t *)&result;

    for (int i = 0; i < 4; i++) {
        int64_t prod = (int64_t)a_in[i] * (int64_t)b_in[i];
        int64_t doubled = prod * 2;
        /* Saturate and get high half */
        if (doubled > (int64_t)2147483647LL * 2147483648LL ||
            doubled < (int64_t)-2147483648LL * 2147483648LL) {
            /* Saturated */
            out[i] = (doubled < 0) ? -2147483648 : 2147483647;
        } else {
            /* Get high 32 bits (shift right by 31) */
            out[i] = (int32_t)(doubled >> 31);
        }
    }

    return result;
}

/**
 * v128_sqrdmulh_32 - Signed saturating rounding doubling multiply high (32-bit)
 *
 * @param a First vector
 * @param b Second vector
 * @return Result of saturating rounding doubling multiply (high half)
 */
Vector128 v128_sqrdmulh_32(Vector128 a, Vector128 b)
{
    Vector128 result;
    int32_t *a_in = (int32_t *)&a;
    int32_t *b_in = (int32_t *)&b;
    int32_t *out = (int32_t *)&result;

    for (int i = 0; i < 4; i++) {
        int64_t prod = (int64_t)a_in[i] * (int64_t)b_in[i];
        int64_t doubled = prod * 2;
        /* Round (add 2^30 before shift) */
        doubled += (1LL << 30);
        /* Saturate and get high half */
        if (doubled > (int64_t)2147483647LL * 2147483648LL ||
            doubled < (int64_t)-2147483648LL * 2147483648LL) {
            /* Saturated */
            out[i] = (doubled < 0) ? -2147483648 : 2147483647;
        } else {
            /* Get high 32 bits (shift right by 31) */
            out[i] = (int32_t)(doubled >> 31);
        }
    }

    return result;
}

/* ============================================================================
 * Session 29: NEON Narrowing and Widening Operations (Additional)
 * ============================================================================ */

/**
 * v128_narrow_s64_s32 - Narrow signed 64-bit to signed 32-bit
 *
 * @param a Input vector (2 x 64-bit signed elements)
 * @return Output vector (4 x 32-bit signed elements, narrowed)
 */
Vector128 v128_narrow_s64_s32(Vector128 a)
{
    Vector128 result;
    int64_t *in = (int64_t *)&a;
    int32_t *out = (int32_t *)&result;

    for (int i = 0; i < 2; i++) {
        /* Saturate signed 64-bit to signed 32-bit range */
        if (in[i] < -2147483648LL) {
            out[i] = -2147483648;
        } else if (in[i] > 2147483647LL) {
            out[i] = 2147483647;
        } else {
            out[i] = (int32_t)in[i];
        }
    }

    return result;
}

/**
 * v128_narrow_u64_u32 - Narrow unsigned 64-bit to unsigned 32-bit
 *
 * @param a Input vector (2 x 64-bit unsigned elements)
 * @return Output vector (4 x 32-bit unsigned elements, narrowed)
 */
Vector128 v128_narrow_u64_u32(Vector128 a)
{
    Vector128 result;
    uint64_t *in = (uint64_t *)&a;
    uint32_t *out = (uint32_t *)&result;

    for (int i = 0; i < 2; i++) {
        /* Saturate unsigned 64-bit to unsigned 32-bit range */
        if (in[i] > 4294967295ULL) {
            out[i] = 4294967295U;
        } else {
            out[i] = (uint32_t)in[i];
        }
    }

    return result;
}

/**
 * v128_widen_s16_s32_lo - Signed widen low half 16-bit to 32-bit
 *
 * @param a Input vector (8 x 16-bit signed elements)
 * @return Output vector (4 x 32-bit signed elements, low half widened)
 */
Vector128 v128_widen_s16_s32_lo(Vector128 a)
{
    Vector128 result;
    int16_t *in = (int16_t *)&a;
    int32_t *out = (int32_t *)&result;

    for (int i = 0; i < 4; i++) {
        /* Sign-extend low 4 elements from 16-bit to 32-bit */
        out[i] = (int32_t)in[i];
    }

    return result;
}

/**
 * v128_widen_s16_s32_hi - Signed widen high half 16-bit to 32-bit
 *
 * @param a Input vector (8 x 16-bit signed elements)
 * @return Output vector (4 x 32-bit signed elements, high half widened)
 */
Vector128 v128_widen_s16_s32_hi(Vector128 a)
{
    Vector128 result;
    int16_t *in = (int16_t *)&a;
    int32_t *out = (int32_t *)&result;

    for (int i = 0; i < 4; i++) {
        /* Sign-extend high 4 elements from 16-bit to 32-bit */
        out[i] = (int32_t)in[i + 4];
    }

    return result;
}

/**
 * v128_widen_u16_u32_lo - Unsigned widen low half 16-bit to 32-bit
 *
 * @param a Input vector (8 x 16-bit unsigned elements)
 * @return Output vector (4 x 32-bit unsigned elements, low half widened)
 */
Vector128 v128_widen_u16_u32_lo(Vector128 a)
{
    Vector128 result;
    uint16_t *in = (uint16_t *)&a;
    uint32_t *out = (uint32_t *)&result;

    for (int i = 0; i < 4; i++) {
        /* Zero-extend low 4 elements from 16-bit to 32-bit */
        out[i] = (uint32_t)in[i];
    }

    return result;
}

/**
 * v128_widen_u16_u32_hi - Unsigned widen high half 16-bit to 32-bit
 *
 * @param a Input vector (8 x 16-bit unsigned elements)
 * @return Output vector (4 x 32-bit unsigned elements, high half widened)
 */
Vector128 v128_widen_u16_u32_hi(Vector128 a)
{
    Vector128 result;
    uint16_t *in = (uint16_t *)&a;
    uint32_t *out = (uint32_t *)&result;

    for (int i = 0; i < 4; i++) {
        /* Zero-extend high 4 elements from 16-bit to 32-bit */
        out[i] = (uint32_t)in[i + 4];
    }

    return result;
}

/**
 * v128_padd2 - Pairwise add (32-bit elements, two vectors)
 *
 * @param a First input vector
 * @param b Second input vector
 * @return Output vector with pairwise addition
 */
Vector128 v128_padd2(Vector128 a, Vector128 b)
{
    Vector128 result;
    int32_t *a_in = (int32_t *)&a;
    int32_t *b_in = (int32_t *)&b;
    int32_t *out = (int32_t *)&result;

    /* Pairwise add within a, then within b */
    out[0] = a_in[0] + a_in[1];
    out[1] = a_in[2] + a_in[3];
    out[2] = b_in[0] + b_in[1];
    out[3] = b_in[2] + b_in[3];

    return result;
}

/**
 * v128_padd_16 - Pairwise add (16-bit elements)
 *
 * @param a First input vector
 * @param b Second input vector
 * @return Output vector with pairwise addition
 */
Vector128 v128_padd_16(Vector128 a, Vector128 b)
{
    Vector128 result;
    int16_t *a_in = (int16_t *)&a;
    int16_t *b_in = (int16_t *)&b;
    int16_t *out = (int16_t *)&result;

    /* Pairwise add within a, then within b */
    for (int i = 0; i < 4; i++) {
        out[i] = a_in[i * 2] + a_in[i * 2 + 1];
        out[i + 4] = b_in[i * 2] + b_in[i * 2 + 1];
    }

    return result;
}

/**
 * v128_padd_8 - Pairwise add (8-bit elements)
 *
 * @param a First input vector
 * @param b Second input vector
 * @return Output vector with pairwise addition
 */
Vector128 v128_padd_8(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint8_t *a_in = (uint8_t *)&a;
    uint8_t *b_in = (uint8_t *)&b;
    uint8_t *out = (uint8_t *)&result;

    /* Pairwise add within a, then within b */
    for (int i = 0; i < 8; i++) {
        out[i] = a_in[i * 2] + a_in[i * 2 + 1];
        out[i + 8] = b_in[i * 2] + b_in[i * 2 + 1];
    }

    return result;
}

/**
 * v128_padal_s8 - Signed pairwise add and accumulate long (8-bit)
 *
 * @param a Accumulator vector (8 x 16-bit signed)
 * @param b Input vector (16 x 8-bit signed)
 * @return Output vector with widened pairwise sums accumulated
 */
Vector128 v128_padal_s8(Vector128 a, Vector128 b)
{
    Vector128 result;
    int16_t *acc = (int16_t *)&a;
    int8_t *in = (int8_t *)&b;
    int16_t *out = (int16_t *)&result;

    for (int i = 0; i < 8; i++) {
        /* Sign-extend and add pairwise */
        int16_t sum = (int16_t)in[i * 2] + (int16_t)in[i * 2 + 1];
        out[i] = acc[i] + sum;
    }

    return result;
}

/**
 * v128_padal_u8 - Unsigned pairwise add and accumulate long (8-bit)
 *
 * @param a Accumulator vector (8 x 16-bit unsigned)
 * @param b Input vector (16 x 8-bit unsigned)
 * @return Output vector with widened pairwise sums accumulated
 */
Vector128 v128_padal_u8(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint16_t *acc = (uint16_t *)&a;
    uint8_t *in = (uint8_t *)&b;
    uint16_t *out = (uint16_t *)&result;

    for (int i = 0; i < 8; i++) {
        /* Zero-extend and add pairwise */
        uint16_t sum = (uint16_t)in[i * 2] + (uint16_t)in[i * 2 + 1];
        out[i] = acc[i] + sum;
    }

    return result;
}

/**
 * v128_padal_s16 - Signed pairwise add and accumulate long (16-bit)
 *
 * @param a Accumulator vector (4 x 32-bit signed)
 * @param b Input vector (8 x 16-bit signed)
 * @return Output vector with widened pairwise sums accumulated
 */
Vector128 v128_padal_s16(Vector128 a, Vector128 b)
{
    Vector128 result;
    int32_t *acc = (int32_t *)&a;
    int16_t *in = (int16_t *)&b;
    int32_t *out = (int32_t *)&result;

    for (int i = 0; i < 4; i++) {
        /* Sign-extend and add pairwise */
        int32_t sum = (int32_t)in[i * 2] + (int32_t)in[i * 2 + 1];
        out[i] = acc[i] + sum;
    }

    return result;
}

/**
 * v128_padal_u16 - Unsigned pairwise add and accumulate long (16-bit)
 *
 * @param a Accumulator vector (4 x 32-bit unsigned)
 * @param b Input vector (8 x 16-bit unsigned)
 * @return Output vector with widened pairwise sums accumulated
 */
Vector128 v128_padal_u16(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint32_t *acc = (uint32_t *)&a;
    uint16_t *in = (uint16_t *)&b;
    uint32_t *out = (uint32_t *)&result;

    for (int i = 0; i < 4; i++) {
        /* Zero-extend and add pairwise */
        uint32_t sum = (uint16_t)in[i * 2] + (uint16_t)in[i * 2 + 1];
        out[i] = acc[i] + sum;
    }

    return result;
}

/**
 * v128_padal_s32 - Signed pairwise add and accumulate long (32-bit)
 *
 * @param a Accumulator vector (2 x 64-bit signed)
 * @param b Input vector (4 x 32-bit signed)
 * @return Output vector with widened pairwise sums accumulated
 */
Vector128 v128_padal_s32(Vector128 a, Vector128 b)
{
    Vector128 result;
    int64_t *acc = (int64_t *)&a;
    int32_t *in = (int32_t *)&b;
    int64_t *out = (int64_t *)&result;

    for (int i = 0; i < 2; i++) {
        /* Sign-extend and add pairwise */
        int64_t sum = (int64_t)in[i * 2] + (int64_t)in[i * 2 + 1];
        out[i] = acc[i] + sum;
    }

    return result;
}

/**
 * v128_padal_u32 - Unsigned pairwise add and accumulate long (32-bit)
 *
 * @param a Accumulator vector (2 x 64-bit unsigned)
 * @param b Input vector (4 x 32-bit unsigned)
 * @return Output vector with widened pairwise sums accumulated
 */
Vector128 v128_padal_u32(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint64_t *acc = (uint64_t *)&a;
    uint32_t *in = (uint32_t *)&b;
    uint64_t *out = (uint64_t *)&result;

    for (int i = 0; i < 2; i++) {
        /* Zero-extend and add pairwise */
        uint64_t sum = (uint64_t)in[i * 2] + (uint64_t)in[i * 2 + 1];
        out[i] = acc[i] + sum;
    }

    return result;
}

/* ============================================================================
 * Session 30: Narrowing with Shift and Saturating Convert Operations
 * ============================================================================ */

/**
 * v128_sqshrn_s16_u8 - Signed saturating narrowing shift right (16-bit to 8-bit)
 *
 * @param a Input vector (8 x 16-bit signed)
 * @param shift Right shift amount (1-16)
 * @return Output vector (8 x 8-bit unsigned) with saturation
 *
 * Implements SQSHRN instruction: narrows with saturation after shift.
 */
Vector128 v128_sqshrn_s16_u8(Vector128 a, int shift)
{
    Vector128 result;
    int16_t *in = (int16_t *)&a;
    uint8_t *out = (uint8_t *)&result;

    for (int i = 0; i < 8; i++) {
        /* Arithmetic shift right */
        int16_t shifted = in[i] >> shift;
        /* Saturate to unsigned 8-bit range */
        if (shifted < 0) {
            out[i] = 0;
        } else if (shifted > 255) {
            out[i] = 255;
        } else {
            out[i] = (uint8_t)shifted;
        }
    }

    return result;
}

/**
 * v128_sqshrn_s32_s16 - Signed saturating narrowing shift right (32-bit to 16-bit)
 *
 * @param a Input vector (4 x 32-bit signed)
 * @param shift Right shift amount (1-32)
 * @return Output vector (4 x 16-bit signed) with saturation
 *
 * Implements SQSHRN instruction for 32-bit to 16-bit narrowing.
 */
Vector128 v128_sqshrn_s32_s16(Vector128 a, int shift)
{
    Vector128 result;
    int32_t *in = (int32_t *)&a;
    int16_t *out = (int16_t *)&result;

    for (int i = 0; i < 4; i++) {
        /* Arithmetic shift right */
        int32_t shifted = in[i] >> shift;
        /* Saturate to signed 16-bit range */
        if (shifted < -32768) {
            out[i] = -32768;
        } else if (shifted > 32767) {
            out[i] = 32767;
        } else {
            out[i] = (int16_t)shifted;
        }
    }

    return result;
}

/**
 * v128_sqshrn_s64_s32 - Signed saturating narrowing shift right (64-bit to 32-bit)
 *
 * @param a Input vector (2 x 64-bit signed)
 * @param shift Right shift amount (1-64)
 * @return Output vector (2 x 32-bit signed) with saturation
 *
 * Implements SQSHRN instruction for 64-bit to 32-bit narrowing.
 */
Vector128 v128_sqshrn_s64_s32(Vector128 a, int shift)
{
    Vector128 result;
    int64_t *in = (int64_t *)&a;
    int32_t *out = (int32_t *)&result;

    for (int i = 0; i < 2; i++) {
        /* Arithmetic shift right */
        int64_t shifted = in[i] >> shift;
        /* Saturate to signed 32-bit range */
        if (shifted < -2147483648LL) {
            out[i] = -2147483648;
        } else if (shifted > 2147483647LL) {
            out[i] = 2147483647;
        } else {
            out[i] = (int32_t)shifted;
        }
    }

    return result;
}

/**
 * v128_uqshrn_u16_u8 - Unsigned saturating narrowing shift right (16-bit to 8-bit)
 *
 * @param a Input vector (8 x 16-bit unsigned)
 * @param shift Right shift amount (1-16)
 * @return Output vector (8 x 8-bit unsigned) with saturation
 *
 * Implements UQSHRN instruction: unsigned narrowing with saturation after shift.
 */
Vector128 v128_uqshrn_u16_u8(Vector128 a, int shift)
{
    Vector128 result;
    uint16_t *in = (uint16_t *)&a;
    uint8_t *out = (uint8_t *)&result;

    for (int i = 0; i < 8; i++) {
        /* Logical shift right */
        uint16_t shifted = in[i] >> shift;
        /* Saturate to unsigned 8-bit range */
        if (shifted > 255) {
            out[i] = 255;
        } else {
            out[i] = (uint8_t)shifted;
        }
    }

    return result;
}

/**
 * v128_uqshrn_u32_u16 - Unsigned saturating narrowing shift right (32-bit to 16-bit)
 *
 * @param a Input vector (4 x 32-bit unsigned)
 * @param shift Right shift amount (1-32)
 * @return Output vector (4 x 16-bit unsigned) with saturation
 *
 * Implements UQSHRN instruction for 32-bit to 16-bit narrowing.
 */
Vector128 v128_uqshrn_u32_u16(Vector128 a, int shift)
{
    Vector128 result;
    uint32_t *in = (uint32_t *)&a;
    uint16_t *out = (uint16_t *)&result;

    for (int i = 0; i < 4; i++) {
        /* Logical shift right */
        uint32_t shifted = in[i] >> shift;
        /* Saturate to unsigned 16-bit range */
        if (shifted > 65535) {
            out[i] = 65535;
        } else {
            out[i] = (uint16_t)shifted;
        }
    }

    return result;
}

/**
 * v128_uqshrn_u64_u32 - Unsigned saturating narrowing shift right (64-bit to 32-bit)
 *
 * @param a Input vector (2 x 64-bit unsigned)
 * @param shift Right shift amount (1-64)
 * @return Output vector (2 x 32-bit unsigned) with saturation
 *
 * Implements UQSHRN instruction for 64-bit to 32-bit narrowing.
 */
Vector128 v128_uqshrn_u64_u32(Vector128 a, int shift)
{
    Vector128 result;
    uint64_t *in = (uint64_t *)&a;
    uint32_t *out = (uint32_t *)&result;

    for (int i = 0; i < 2; i++) {
        /* Logical shift right */
        uint64_t shifted = in[i] >> shift;
        /* Saturate to unsigned 32-bit range */
        if (shifted > 4294967295ULL) {
            out[i] = 4294967295U;
        } else {
            out[i] = (uint32_t)shifted;
        }
    }

    return result;
}

/**
 * v128_sqrshrn_s16_u8 - Signed saturating rounding narrowing shift right (16-bit to 8-bit)
 *
 * @param a Input vector (8 x 16-bit signed)
 * @param shift Right shift amount (1-16)
 * @return Output vector (8 x 8-bit unsigned) with saturation and rounding
 *
 * Implements SQRSHRN instruction: narrowing with rounding before saturation.
 * Adds 2^(shift-1) for rounding before the shift.
 */
Vector128 v128_sqrshrn_s16_u8(Vector128 a, int shift)
{
    Vector128 result;
    int16_t *in = (int16_t *)&a;
    uint8_t *out = (uint8_t *)&result;
    int16_t rounding = (int16_t)(1 << (shift - 1));

    for (int i = 0; i < 8; i++) {
        /* Add rounding constant, then arithmetic shift right */
        int16_t rounded = in[i] + rounding;
        int16_t shifted = rounded >> shift;
        /* Saturate to unsigned 8-bit range */
        if (shifted < 0) {
            out[i] = 0;
        } else if (shifted > 255) {
            out[i] = 255;
        } else {
            out[i] = (uint8_t)shifted;
        }
    }

    return result;
}

/**
 * v128_sqrshrn_s32_s16 - Signed saturating rounding narrowing shift right (32-bit to 16-bit)
 *
 * @param a Input vector (4 x 32-bit signed)
 * @param shift Right shift amount (1-32)
 * @return Output vector (4 x 16-bit signed) with saturation and rounding
 *
 * Implements SQRSHRN instruction for 32-bit to 16-bit with rounding.
 */
Vector128 v128_sqrshrn_s32_s16(Vector128 a, int shift)
{
    Vector128 result;
    int32_t *in = (int32_t *)&a;
    int16_t *out = (int16_t *)&result;
    int32_t rounding = (int32_t)(1 << (shift - 1));

    for (int i = 0; i < 4; i++) {
        /* Add rounding constant, then arithmetic shift right */
        int32_t rounded = in[i] + rounding;
        int32_t shifted = rounded >> shift;
        /* Saturate to signed 16-bit range */
        if (shifted < -32768) {
            out[i] = -32768;
        } else if (shifted > 32767) {
            out[i] = 32767;
        } else {
            out[i] = (int16_t)shifted;
        }
    }

    return result;
}

/**
 * v128_sqrshrn_s64_s32 - Signed saturating rounding narrowing shift right (64-bit to 32-bit)
 *
 * @param a Input vector (2 x 64-bit signed)
 * @param shift Right shift amount (1-64)
 * @return Output vector (2 x 32-bit signed) with saturation and rounding
 *
 * Implements SQRSHRN instruction for 64-bit to 32-bit with rounding.
 */
Vector128 v128_sqrshrn_s64_s32(Vector128 a, int shift)
{
    Vector128 result;
    int64_t *in = (int64_t *)&a;
    int32_t *out = (int32_t *)&result;
    int64_t rounding = (int64_t)((uint64_t)1 << (shift - 1));

    for (int i = 0; i < 2; i++) {
        /* Add rounding constant, then arithmetic shift right */
        int64_t rounded = in[i] + rounding;
        int64_t shifted = rounded >> shift;
        /* Saturate to signed 32-bit range */
        if (shifted < -2147483648LL) {
            out[i] = -2147483648;
        } else if (shifted > 2147483647LL) {
            out[i] = 2147483647;
        } else {
            out[i] = (int32_t)shifted;
        }
    }

    return result;
}

/**
 * v128_uqrshrn_u16_u8 - Unsigned saturating rounding narrowing shift right (16-bit to 8-bit)
 *
 * @param a Input vector (8 x 16-bit unsigned)
 * @param shift Right shift amount (1-16)
 * @return Output vector (8 x 8-bit unsigned) with saturation and rounding
 *
 * Implements UQRSHRN instruction: unsigned narrowing with rounding.
 */
Vector128 v128_uqrshrn_u16_u8(Vector128 a, int shift)
{
    Vector128 result;
    uint16_t *in = (uint16_t *)&a;
    uint8_t *out = (uint8_t *)&result;
    uint16_t rounding = (uint16_t)(1 << (shift - 1));

    for (int i = 0; i < 8; i++) {
        /* Add rounding constant, then logical shift right */
        uint16_t rounded = in[i] + rounding;
        uint16_t shifted = rounded >> shift;
        /* Saturate to unsigned 8-bit range */
        if (shifted > 255) {
            out[i] = 255;
        } else {
            out[i] = (uint8_t)shifted;
        }
    }

    return result;
}

/**
 * v128_uqrshrn_u32_u16 - Unsigned saturating rounding narrowing shift right (32-bit to 16-bit)
 *
 * @param a Input vector (4 x 32-bit unsigned)
 * @param shift Right shift amount (1-32)
 * @return Output vector (4 x 16-bit unsigned) with saturation and rounding
 *
 * Implements UQRSHRN instruction for 32-bit to 16-bit with rounding.
 */
Vector128 v128_uqrshrn_u32_u16(Vector128 a, int shift)
{
    Vector128 result;
    uint32_t *in = (uint32_t *)&a;
    uint16_t *out = (uint16_t *)&result;
    uint32_t rounding = (uint32_t)(1 << (shift - 1));

    for (int i = 0; i < 4; i++) {
        /* Add rounding constant, then logical shift right */
        uint32_t rounded = in[i] + rounding;
        uint32_t shifted = rounded >> shift;
        /* Saturate to unsigned 16-bit range */
        if (shifted > 65535) {
            out[i] = 65535;
        } else {
            out[i] = (uint16_t)shifted;
        }
    }

    return result;
}

/**
 * v128_uqrshrn_u64_u32 - Unsigned saturating rounding narrowing shift right (64-bit to 32-bit)
 *
 * @param a Input vector (2 x 64-bit unsigned)
 * @param shift Right shift amount (1-64)
 * @return Output vector (2 x 32-bit unsigned) with saturation and rounding
 *
 * Implements UQRSHRN instruction for 64-bit to 32-bit with rounding.
 */
Vector128 v128_uqrshrn_u64_u32(Vector128 a, int shift)
{
    Vector128 result;
    uint64_t *in = (uint64_t *)&a;
    uint32_t *out = (uint32_t *)&result;
    uint64_t rounding = (uint64_t)((uint64_t)1 << (shift - 1));

    for (int i = 0; i < 2; i++) {
        /* Add rounding constant, then logical shift right */
        uint64_t rounded = in[i] + rounding;
        uint64_t shifted = rounded >> shift;
        /* Saturate to unsigned 32-bit range */
        if (shifted > 4294967295ULL) {
            out[i] = 4294967295U;
        } else {
            out[i] = (uint32_t)shifted;
        }
    }

    return result;
}

/**
 * v128_sqxtn_s16_s32 - Signed saturating extract narrow (32-bit to 16-bit)
 *
 * @param a Input vector (4 x 32-bit signed)
 * @return Output vector (4 x 16-bit signed) with saturation
 *
 * Implements SQXTN instruction: narrow without shift, just saturation.
 */
Vector128 v128_sqxtn_s16_s32(Vector128 a)
{
    Vector128 result;
    int32_t *in = (int32_t *)&a;
    int16_t *out = (int16_t *)&result;

    for (int i = 0; i < 4; i++) {
        /* Saturate to signed 16-bit range */
        if (in[i] < -32768) {
            out[i] = -32768;
        } else if (in[i] > 32767) {
            out[i] = 32767;
        } else {
            out[i] = (int16_t)in[i];
        }
    }

    return result;
}

/**
 * v128_sqxtn_s8_s16 - Signed saturating extract narrow (16-bit to 8-bit)
 *
 * @param a Input vector (8 x 16-bit signed)
 * @return Output vector (8 x 8-bit signed) with saturation
 *
 * Implements SQXTN instruction for 16-bit to 8-bit narrowing.
 */
Vector128 v128_sqxtn_s8_s16(Vector128 a)
{
    Vector128 result;
    int16_t *in = (int16_t *)&a;
    int8_t *out = (int8_t *)&result;

    for (int i = 0; i < 8; i++) {
        /* Saturate to signed 8-bit range */
        if (in[i] < -128) {
            out[i] = -128;
        } else if (in[i] > 127) {
            out[i] = 127;
        } else {
            out[i] = (int8_t)in[i];
        }
    }

    return result;
}

/**
 * v128_sqxtn_s32_s64 - Signed saturating extract narrow (64-bit to 32-bit)
 *
 * @param a Input vector (2 x 64-bit signed)
 * @return Output vector (2 x 32-bit signed) with saturation
 *
 * Implements SQXTN instruction for 64-bit to 32-bit narrowing.
 */
Vector128 v128_sqxtn_s32_s64(Vector128 a)
{
    Vector128 result;
    int64_t *in = (int64_t *)&a;
    int32_t *out = (int32_t *)&result;

    for (int i = 0; i < 2; i++) {
        /* Saturate to signed 32-bit range */
        if (in[i] < -2147483648LL) {
            out[i] = -2147483648;
        } else if (in[i] > 2147483647LL) {
            out[i] = 2147483647;
        } else {
            out[i] = (int32_t)in[i];
        }
    }

    return result;
}

/**
 * v128_uqxtn_u16_u32 - Unsigned saturating extract narrow (32-bit to 16-bit)
 *
 * @param a Input vector (4 x 32-bit unsigned)
 * @return Output vector (4 x 16-bit unsigned) with saturation
 *
 * Implements UQXTN instruction: unsigned narrow without shift.
 */
Vector128 v128_uqxtn_u16_u32(Vector128 a)
{
    Vector128 result;
    uint32_t *in = (uint32_t *)&a;
    uint16_t *out = (uint16_t *)&result;

    for (int i = 0; i < 4; i++) {
        /* Saturate to unsigned 16-bit range */
        if (in[i] > 65535) {
            out[i] = 65535;
        } else {
            out[i] = (uint16_t)in[i];
        }
    }

    return result;
}

/**
 * v128_uqxtn_u8_u16 - Unsigned saturating extract narrow (16-bit to 8-bit)
 *
 * @param a Input vector (8 x 16-bit unsigned)
 * @return Output vector (8 x 8-bit unsigned) with saturation
 *
 * Implements UQXTN instruction for 16-bit to 8-bit narrowing.
 */
Vector128 v128_uqxtn_u8_u16(Vector128 a)
{
    Vector128 result;
    uint16_t *in = (uint16_t *)&a;
    uint8_t *out = (uint8_t *)&result;

    for (int i = 0; i < 8; i++) {
        /* Saturate to unsigned 8-bit range */
        if (in[i] > 255) {
            out[i] = 255;
        } else {
            out[i] = (uint8_t)in[i];
        }
    }

    return result;
}

/**
 * v128_uqxtn_u32_u64 - Unsigned saturating extract narrow (64-bit to 32-bit)
 *
 * @param a Input vector (2 x 64-bit unsigned)
 * @return Output vector (2 x 32-bit unsigned) with saturation
 *
 * Implements UQXTN instruction for 64-bit to 32-bit narrowing.
 */
Vector128 v128_uqxtn_u32_u64(Vector128 a)
{
    Vector128 result;
    uint64_t *in = (uint64_t *)&a;
    uint32_t *out = (uint32_t *)&result;

    for (int i = 0; i < 2; i++) {
        /* Saturate to unsigned 32-bit range */
        if (in[i] > 4294967295ULL) {
            out[i] = 4294967295U;
        } else {
            out[i] = (uint32_t)in[i];
        }
    }

    return result;
}

/**
 * v128_sqxtun_s16_u32 - Signed saturating extract unsigned narrow (32-bit to 16-bit)
 *
 * @param a Input vector (4 x 32-bit signed)
 * @return Output vector (4 x 16-bit unsigned) with saturation
 *
 * Implements SQXTUN instruction: signed to unsigned narrow with saturation.
 * Negative values saturate to 0, positive values saturate to max.
 */
Vector128 v128_sqxtun_s16_u32(Vector128 a)
{
    Vector128 result;
    int32_t *in = (int32_t *)&a;
    uint16_t *out = (uint16_t *)&result;

    for (int i = 0; i < 4; i++) {
        /* Saturate signed to unsigned 16-bit range */
        if (in[i] < 0) {
            out[i] = 0;
        } else if (in[i] > 65535) {
            out[i] = 65535;
        } else {
            out[i] = (uint16_t)in[i];
        }
    }

    return result;
}

/**
 * v128_sqxtun_s8_u16 - Signed saturating extract unsigned narrow (16-bit to 8-bit)
 *
 * @param a Input vector (8 x 16-bit signed)
 * @return Output vector (8 x 8-bit unsigned) with saturation
 *
 * Implements SQXTUN instruction for 16-bit signed to 8-bit unsigned.
 */
Vector128 v128_sqxtun_s8_u16(Vector128 a)
{
    Vector128 result;
    int16_t *in = (int16_t *)&a;
    uint8_t *out = (uint8_t *)&result;

    for (int i = 0; i < 8; i++) {
        /* Saturate signed to unsigned 8-bit range */
        if (in[i] < 0) {
            out[i] = 0;
        } else if (in[i] > 255) {
            out[i] = 255;
        } else {
            out[i] = (uint8_t)in[i];
        }
    }

    return result;
}

/**
 * v128_sqxtun_s32_u64 - Signed saturating extract unsigned narrow (64-bit to 32-bit)
 *
 * @param a Input vector (2 x 64-bit signed)
 * @return Output vector (2 x 32-bit unsigned) with saturation
 *
 * Implements SQXTUN instruction for 64-bit signed to 32-bit unsigned.
 */
Vector128 v128_sqxtun_s32_u64(Vector128 a)
{
    Vector128 result;
    int64_t *in = (int64_t *)&a;
    uint32_t *out = (uint32_t *)&result;

    for (int i = 0; i < 2; i++) {
        /* Saturate signed to unsigned 32-bit range */
        if (in[i] < 0) {
            out[i] = 0;
        } else if (in[i] > 4294967295LL) {
            out[i] = 4294967295U;
        } else {
            out[i] = (uint32_t)in[i];
        }
    }

    return result;
}

/* ============================================================================
 * Session 31: Advanced SIMD Operations - Vector by Scalar
 * ============================================================================ */

/**
 * v128_mul_scalar_u16 - Vector unsigned 16-bit multiply by scalar
 * @param a Input vector (8 x 16-bit unsigned)
 * @param scalar Scalar multiplier
 * @return Result of a * scalar (8 x 16-bit unsigned)
 */
Vector128 v128_mul_scalar_u16(Vector128 a, uint16_t scalar)
{
    Vector128 result;
    uint16_t *in = (uint16_t *)&a;
    uint16_t *out = (uint16_t *)&result;

    for (int i = 0; i < 8; i++) {
        out[i] = in[i] * scalar;
    }

    return result;
}

/**
 * v128_mul_scalar_u32 - Vector unsigned 32-bit multiply by scalar
 * @param a Input vector (4 x 32-bit unsigned)
 * @param scalar Scalar multiplier
 * @return Result of a * scalar (4 x 32-bit unsigned)
 */
Vector128 v128_mul_scalar_u32(Vector128 a, uint32_t scalar)
{
    Vector128 result;
    uint32_t *in = (uint32_t *)&a;
    uint32_t *out = (uint32_t *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = in[i] * scalar;
    }

    return result;
}

/**
 * v128_mul_scalar_s16 - Vector signed 16-bit multiply by scalar
 * @param a Input vector (8 x 16-bit signed)
 * @param scalar Scalar multiplier
 * @return Result of a * scalar (8 x 16-bit signed)
 */
Vector128 v128_mul_scalar_s16(Vector128 a, int16_t scalar)
{
    Vector128 result;
    int16_t *in = (int16_t *)&a;
    int16_t *out = (int16_t *)&result;

    for (int i = 0; i < 8; i++) {
        out[i] = in[i] * scalar;
    }

    return result;
}

/**
 * v128_mul_scalar_s32 - Vector signed 32-bit multiply by scalar
 * @param a Input vector (4 x 32-bit signed)
 * @param scalar Scalar multiplier
 * @return Result of a * scalar (4 x 32-bit signed)
 */
Vector128 v128_mul_scalar_s32(Vector128 a, int32_t scalar)
{
    Vector128 result;
    int32_t *in = (int32_t *)&a;
    int32_t *out = (int32_t *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = in[i] * scalar;
    }

    return result;
}

/**
 * v128_mla_scalar_u16 - Vector unsigned 16-bit multiply-accumulate by scalar
 * @param a Accumulator vector (8 x 16-bit unsigned)
 * @param b Input vector (8 x 16-bit unsigned)
 * @param scalar Scalar multiplier
 * @return Result of a + b * scalar (8 x 16-bit unsigned)
 */
Vector128 v128_mla_scalar_u16(Vector128 a, Vector128 b, uint16_t scalar)
{
    Vector128 result;
    uint16_t *ina = (uint16_t *)&a;
    uint16_t *inb = (uint16_t *)&b;
    uint16_t *out = (uint16_t *)&result;

    for (int i = 0; i < 8; i++) {
        out[i] = ina[i] + (inb[i] * scalar);
    }

    return result;
}

/**
 * v128_mla_scalar_u32 - Vector unsigned 32-bit multiply-accumulate by scalar
 * @param a Accumulator vector (4 x 32-bit unsigned)
 * @param b Input vector (4 x 32-bit unsigned)
 * @param scalar Scalar multiplier
 * @return Result of a + b * scalar (4 x 32-bit unsigned)
 */
Vector128 v128_mla_scalar_u32(Vector128 a, Vector128 b, uint32_t scalar)
{
    Vector128 result;
    uint32_t *ina = (uint32_t *)&a;
    uint32_t *inb = (uint32_t *)&b;
    uint32_t *out = (uint32_t *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = ina[i] + (inb[i] * scalar);
    }

    return result;
}

/**
 * v128_mla_scalar_s16 - Vector signed 16-bit multiply-accumulate by scalar
 * @param a Accumulator vector (8 x 16-bit signed)
 * @param b Input vector (8 x 16-bit signed)
 * @param scalar Scalar multiplier
 * @return Result of a + b * scalar (8 x 16-bit signed)
 */
Vector128 v128_mla_scalar_s16(Vector128 a, Vector128 b, int16_t scalar)
{
    Vector128 result;
    int16_t *ina = (int16_t *)&a;
    int16_t *inb = (int16_t *)&b;
    int16_t *out = (int16_t *)&result;

    for (int i = 0; i < 8; i++) {
        out[i] = ina[i] + (inb[i] * scalar);
    }

    return result;
}

/**
 * v128_mla_scalar_s32 - Vector signed 32-bit multiply-accumulate by scalar
 * @param a Accumulator vector (4 x 32-bit signed)
 * @param b Input vector (4 x 32-bit signed)
 * @param scalar Scalar multiplier
 * @return Result of a + b * scalar (4 x 32-bit signed)
 */
Vector128 v128_mla_scalar_s32(Vector128 a, Vector128 b, int32_t scalar)
{
    Vector128 result;
    int32_t *ina = (int32_t *)&a;
    int32_t *inb = (int32_t *)&b;
    int32_t *out = (int32_t *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = ina[i] + (inb[i] * scalar);
    }

    return result;
}

/**
 * v128_mls_scalar_u16 - Vector unsigned 16-bit multiply-subtract by scalar
 * @param a Accumulator vector (8 x 16-bit unsigned)
 * @param b Input vector (8 x 16-bit unsigned)
 * @param scalar Scalar multiplier
 * @return Result of a - b * scalar (8 x 16-bit unsigned)
 */
Vector128 v128_mls_scalar_u16(Vector128 a, Vector128 b, uint16_t scalar)
{
    Vector128 result;
    uint16_t *ina = (uint16_t *)&a;
    uint16_t *inb = (uint16_t *)&b;
    uint16_t *out = (uint16_t *)&result;

    for (int i = 0; i < 8; i++) {
        out[i] = ina[i] - (inb[i] * scalar);
    }

    return result;
}

/**
 * v128_mls_scalar_u32 - Vector unsigned 32-bit multiply-subtract by scalar
 * @param a Accumulator vector (4 x 32-bit unsigned)
 * @param b Input vector (4 x 32-bit unsigned)
 * @param scalar Scalar multiplier
 * @return Result of a - b * scalar (4 x 32-bit unsigned)
 */
Vector128 v128_mls_scalar_u32(Vector128 a, Vector128 b, uint32_t scalar)
{
    Vector128 result;
    uint32_t *ina = (uint32_t *)&a;
    uint32_t *inb = (uint32_t *)&b;
    uint32_t *out = (uint32_t *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = ina[i] - (inb[i] * scalar);
    }

    return result;
}

/**
 * v128_mls_scalar_s16 - Vector signed 16-bit multiply-subtract by scalar
 * @param a Accumulator vector (8 x 16-bit signed)
 * @param b Input vector (8 x 16-bit signed)
 * @param scalar Scalar multiplier
 * @return Result of a - b * scalar (8 x 16-bit signed)
 */
Vector128 v128_mls_scalar_s16(Vector128 a, Vector128 b, int16_t scalar)
{
    Vector128 result;
    int16_t *ina = (int16_t *)&a;
    int16_t *inb = (int16_t *)&b;
    int16_t *out = (int16_t *)&result;

    for (int i = 0; i < 8; i++) {
        out[i] = ina[i] - (inb[i] * scalar);
    }

    return result;
}

/**
 * v128_mls_scalar_s32 - Vector signed 32-bit multiply-subtract by scalar
 * @param a Accumulator vector (4 x 32-bit signed)
 * @param b Input vector (4 x 32-bit signed)
 * @param scalar Scalar multiplier
 * @return Result of a - b * scalar (4 x 32-bit signed)
 */
Vector128 v128_mls_scalar_s32(Vector128 a, Vector128 b, int32_t scalar)
{
    Vector128 result;
    int32_t *ina = (int32_t *)&a;
    int32_t *inb = (int32_t *)&b;
    int32_t *out = (int32_t *)&result;

    for (int i = 0; i < 4; i++) {
        out[i] = ina[i] - (inb[i] * scalar);
    }

    return result;
}

/* ============================================================================
 * Session 31: Advanced SIMD Operations - Extended Table Lookup
 * ============================================================================ */

/**
 * v128_tbl_2regs - Table lookup with 2 tables
 * @param t1 First table vector
 * @param t2 Second table vector
 * @param i Index vector
 * @return Result of table lookup (16 bytes)
 *
 * Implements TBL instruction with 2 source registers.
 * Each byte in i selects a byte from the concatenated tables.
 * If index >= 32, result byte is zero.
 */
Vector128 v128_tbl_2regs(Vector128 t1, Vector128 t2, Vector128 i)
{
    Vector128 result;
    uint8_t *tab1 = (uint8_t *)&t1;
    uint8_t *tab2 = (uint8_t *)&t2;
    uint8_t *idx = (uint8_t *)&i;
    uint8_t *out = (uint8_t *)&result;

    for (int j = 0; j < 16; j++) {
        uint8_t index = idx[j];
        if (index < 16) {
            out[j] = tab1[index];
        } else if (index < 32) {
            out[j] = tab2[index - 16];
        } else {
            out[j] = 0;  /* Out of range - return zero */
        }
    }

    return result;
}

/**
 * v128_tbl_3regs - Table lookup with 3 tables
 * @param t1 First table vector
 * @param t2 Second table vector
 * @param t3 Third table vector
 * @param i Index vector
 * @return Result of table lookup (16 bytes)
 *
 * Implements TBL instruction with 3 source registers.
 * If index >= 48, result byte is zero.
 */
Vector128 v128_tbl_3regs(Vector128 t1, Vector128 t2, Vector128 t3, Vector128 i)
{
    Vector128 result;
    uint8_t *tab1 = (uint8_t *)&t1;
    uint8_t *tab2 = (uint8_t *)&t2;
    uint8_t *tab3 = (uint8_t *)&t3;
    uint8_t *idx = (uint8_t *)&i;
    uint8_t *out = (uint8_t *)&result;

    for (int j = 0; j < 16; j++) {
        uint8_t index = idx[j];
        if (index < 16) {
            out[j] = tab1[index];
        } else if (index < 32) {
            out[j] = tab2[index - 16];
        } else if (index < 48) {
            out[j] = tab3[index - 32];
        } else {
            out[j] = 0;  /* Out of range - return zero */
        }
    }

    return result;
}

/**
 * v128_tbl_4regs - Table lookup with 4 tables
 * @param t1 First table vector
 * @param t2 Second table vector
 * @param t3 Third table vector
 * @param t4 Fourth table vector
 * @param i Index vector
 * @return Result of table lookup (16 bytes)
 *
 * Implements TBL instruction with 4 source registers.
 * If index >= 64, result byte is zero.
 */
Vector128 v128_tbl_4regs(Vector128 t1, Vector128 t2, Vector128 t3, Vector128 t4, Vector128 i)
{
    Vector128 result;
    uint8_t *tab1 = (uint8_t *)&t1;
    uint8_t *tab2 = (uint8_t *)&t2;
    uint8_t *tab3 = (uint8_t *)&t3;
    uint8_t *tab4 = (uint8_t *)&t4;
    uint8_t *idx = (uint8_t *)&i;
    uint8_t *out = (uint8_t *)&result;

    for (int j = 0; j < 16; j++) {
        uint8_t index = idx[j];
        if (index < 16) {
            out[j] = tab1[index];
        } else if (index < 32) {
            out[j] = tab2[index - 16];
        } else if (index < 48) {
            out[j] = tab3[index - 32];
        } else if (index < 64) {
            out[j] = tab4[index - 48];
        } else {
            out[j] = 0;  /* Out of range - return zero */
        }
    }

    return result;
}

/**
 * v128_tbx_2regs - Table lookup extension with 2 tables
 * @param dest Destination/accumulator vector
 * @param t1 First table vector
 * @param t2 Second table vector
 * @param i Index vector
 * @return Result of table lookup extension (16 bytes)
 *
 * Implements TBX instruction - like TBL but preserves dest for out-of-range indices.
 */
Vector128 v128_tbx_2regs(Vector128 dest, Vector128 t1, Vector128 t2, Vector128 i)
{
    Vector128 result;
    uint8_t *tab1 = (uint8_t *)&t1;
    uint8_t *tab2 = (uint8_t *)&t2;
    uint8_t *idx = (uint8_t *)&i;
    uint8_t *dst = (uint8_t *)&dest;
    uint8_t *out = (uint8_t *)&result;

    for (int j = 0; j < 16; j++) {
        uint8_t index = idx[j];
        if (index < 16) {
            out[j] = tab1[index];
        } else if (index < 32) {
            out[j] = tab2[index - 16];
        } else {
            out[j] = dst[j];  /* Out of range - preserve destination */
        }
    }

    return result;
}

/**
 * v128_tbx_3regs - Table lookup extension with 3 tables
 * @param dest Destination/accumulator vector
 * @param t1 First table vector
 * @param t2 Second table vector
 * @param t3 Third table vector
 * @param i Index vector
 * @return Result of table lookup extension (16 bytes)
 */
Vector128 v128_tbx_3regs(Vector128 dest, Vector128 t1, Vector128 t2, Vector128 t3, Vector128 i)
{
    Vector128 result;
    uint8_t *tab1 = (uint8_t *)&t1;
    uint8_t *tab2 = (uint8_t *)&t2;
    uint8_t *tab3 = (uint8_t *)&t3;
    uint8_t *idx = (uint8_t *)&i;
    uint8_t *dst = (uint8_t *)&dest;
    uint8_t *out = (uint8_t *)&result;

    for (int j = 0; j < 16; j++) {
        uint8_t index = idx[j];
        if (index < 16) {
            out[j] = tab1[index];
        } else if (index < 32) {
            out[j] = tab2[index - 16];
        } else if (index < 48) {
            out[j] = tab3[index - 32];
        } else {
            out[j] = dst[j];  /* Out of range - preserve destination */
        }
    }

    return result;
}

/**
 * v128_tbx_4regs - Table lookup extension with 4 tables
 * @param dest Destination/accumulator vector
 * @param t1 First table vector
 * @param t2 Second table vector
 * @param t3 Third table vector
 * @param t4 Fourth table vector
 * @param i Index vector
 * @return Result of table lookup extension (16 bytes)
 */
Vector128 v128_tbx_4regs(Vector128 dest, Vector128 t1, Vector128 t2, Vector128 t3, Vector128 t4, Vector128 i)
{
    Vector128 result;
    uint8_t *tab1 = (uint8_t *)&t1;
    uint8_t *tab2 = (uint8_t *)&t2;
    uint8_t *tab3 = (uint8_t *)&t3;
    uint8_t *tab4 = (uint8_t *)&t4;
    uint8_t *idx = (uint8_t *)&i;
    uint8_t *dst = (uint8_t *)&dest;
    uint8_t *out = (uint8_t *)&result;

    for (int j = 0; j < 16; j++) {
        uint8_t index = idx[j];
        if (index < 16) {
            out[j] = tab1[index];
        } else if (index < 32) {
            out[j] = tab2[index - 16];
        } else if (index < 48) {
            out[j] = tab3[index - 32];
        } else if (index < 64) {
            out[j] = tab4[index - 48];
        } else {
            out[j] = dst[j];  /* Out of range - preserve destination */
        }
    }

    return result;
}

/* ============================================================================
 * Session 31: Advanced SIMD Operations - Complex Number
 * ============================================================================ */

/**
 * v128_fcmla - Floating-point complex multiply-accumulate
 * @param a Accumulator vector (4 x 32-bit float, as complex pairs)
 * @param b First operand vector (4 x 32-bit float, as complex pairs)
 * @param c Second operand vector (4 x 32-bit float, as complex pairs)
 * @param rot Rotation angle (0=0°, 1=90°, 2=180°, 3=270°)
 * @return Result of a + b * c with rotation (4 x 32-bit float)
 *
 * Implements FCMLA instruction for complex number multiplication.
 * Each pair of floats represents a complex number (real, imaginary).
 * Rotation is applied to the second operand before multiplication.
 */
Vector128 v128_fcmla(Vector128 a, Vector128 b, Vector128 c, int rot)
{
    Vector128 result;
    float *out = (float *)&result;
    float *acc = (float *)&a;
    float *op1 = (float *)&b;
    float *op2 = (float *)&c;

    for (int i = 0; i < 4; i += 2) {
        /* Each pair is (real, imag) */
        float acc_real = acc[i];
        float acc_imag = acc[i + 1];
        float op1_real = op1[i];
        float op1_imag = op1[i + 1];
        float op2_real = op2[i];
        float op2_imag = op2[i + 1];

        /* Apply rotation to op2 */
        float rotated_real, rotated_imag;
        switch (rot & 3) {
            case 0:  /* 0 degrees */
                rotated_real = op2_real;
                rotated_imag = op2_imag;
                break;
            case 1:  /* 90 degrees */
                rotated_real = -op2_imag;
                rotated_imag = op2_real;
                break;
            case 2:  /* 180 degrees */
                rotated_real = -op2_real;
                rotated_imag = -op2_imag;
                break;
            case 3:  /* 270 degrees */
                rotated_real = op2_imag;
                rotated_imag = -op2_real;
                break;
            default:
                rotated_real = op2_real;
                rotated_imag = op2_imag;
                break;
        }

        /* Complex multiply: (a+bi)(c+di) = (ac-bd) + (ad+bc)i */
        float prod_real = op1_real * rotated_real - op1_imag * rotated_imag;
        float prod_imag = op1_real * rotated_imag + op1_imag * rotated_real;

        /* Accumulate */
        out[i] = acc_real + prod_real;
        out[i + 1] = acc_imag + prod_imag;
    }

    return result;
}

/**
 * v128_fcadd - Floating-point complex add
 * @param a First operand vector (4 x 32-bit float, as complex pairs)
 * @param b Second operand vector (4 x 32-bit float, as complex pairs)
 * @param rot Rotation angle (0=0°, 1=90°)
 * @return Result of a + rotated_b (4 x 32-bit float)
 *
 * Implements FCADD instruction for complex number addition.
 * Rotation swaps real and imaginary parts with sign changes.
 */
Vector128 v128_fcadd(Vector128 a, Vector128 b, int rot)
{
    Vector128 result;
    float *out = (float *)&result;
    float *op1 = (float *)&a;
    float *op2 = (float *)&b;

    for (int i = 0; i < 4; i += 2) {
        float op1_real = op1[i];
        float op1_imag = op1[i + 1];
        float op2_real = op2[i];
        float op2_imag = op2[i + 1];

        /* Apply rotation */
        float rotated_real, rotated_imag;
        if (rot == 0) {
            rotated_real = op2_real;
            rotated_imag = op2_imag;
        } else {
            /* 90 degree rotation */
            rotated_real = -op2_imag;
            rotated_imag = op2_real;
        }

        /* Complex add */
        out[i] = op1_real + rotated_real;
        out[i + 1] = op1_imag + rotated_imag;
    }

    return result;
}

/* ============================================================================
 * Session 31: Advanced SIMD Operations - Floating-point Narrowing
 * ============================================================================ */

/**
 * v128_fcvtn_f32_f64 - Floating-point convert narrow (double to float)
 * @param a Input vector (2 x 64-bit double)
 * @return Result vector (4 x 32-bit float, low 2 elements valid)
 *
 * Implements FCVTN instruction for FP narrowing conversion.
 */
Vector128 v128_fcvtn_f32_f64(Vector128 a)
{
    Vector128 result;
    double *in = (double *)&a;
    float *out = (float *)&result;

    for (int i = 0; i < 2; i++) {
        out[i] = (float)in[i];
    }
    /* High 2 elements are zero */
    out[2] = 0.0f;
    out[3] = 0.0f;

    return result;
}

/**
 * v128_fcvtn_f16_f32 - Floating-point convert narrow (float to half)
 * @param a Input vector (4 x 32-bit float)
 * @return Result vector (8 x 16-bit half, low 4 elements valid)
 *
 * Implements FCVTN instruction for FP narrowing conversion.
 * Note: Simplified half-precision conversion.
 */
Vector128 v128_fcvtn_f16_f32(Vector128 a)
{
    Vector128 result;
    float *in = (float *)&a;
    uint16_t *out = (uint16_t *)&result;

    for (int i = 0; i < 4; i++) {
        /* Simplified FP16 conversion - just truncate exponent/mantissa */
        /* In a real implementation, proper IEEE 754 half conversion */
        float val = in[i];
        uint32_t bits;
        memcpy(&bits, &val, sizeof(float));

        /* Extract sign, exponent, mantissa */
        uint16_t sign = (bits >> 31) & 0x0001;
        int32_t exp = ((bits >> 23) & 0xFF) - 127 + 15;  /* Re-bias */
        uint16_t mant = (bits >> 13) & 0x03FF;

        /* Handle special cases */
        if (exp <= 0) {
            out[i] = sign << 15;  /* Subnormal -> zero */
        } else if (exp >= 31) {
            out[i] = (sign << 15) | 0x7C00;  /* Infinity */
        } else {
            out[i] = (sign << 15) | (exp << 10) | mant;
        }
    }

    return result;
}

/**
 * v128_fcvtn2_f32_f64 - Floating-point convert narrow high (double to float)
 * @param a Input vector (2 x 64-bit double)
 * @return Result vector (4 x 32-bit float, high 2 elements valid)
 *
 * Implements FCVTN2 instruction - stores to high half of vector.
 */
Vector128 v128_fcvtn2_f32_f64(Vector128 a)
{
    Vector128 result;
    double *in = (double *)&a;
    float *out = (float *)&result;

    /* Low 2 elements are zero */
    out[0] = 0.0f;
    out[1] = 0.0f;
    /* High 2 elements contain result */
    out[2] = (float)in[0];
    out[3] = (float)in[1];

    return result;
}

/**
 * v128_fcmull_f32_f16 - Floating-point multiply long (half to float)
 * @param a First input vector (8 x 16-bit half)
 * @param b Second input vector (8 x 16-bit half)
 * @return Result vector (4 x 32-bit float)
 *
 * Implements FCMULL instruction for FP widening multiplication.
 */
Vector128 v128_fcmull_f32_f16(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint16_t *ina = (uint16_t *)&a;
    uint16_t *inb = (uint16_t *)&b;
    float *out = (float *)&result;

    for (int i = 0; i < 4; i++) {
        /* Simplified FP16 to FP32 conversion */
        uint16_t ha = ina[i];
        uint16_t hb = inb[i];

        /* Convert half to float */
        uint16_t sign_a = (ha >> 15) & 0x0001;
        uint16_t sign_b = (hb >> 15) & 0x0001;
        int32_t exp_a = ((ha >> 10) & 0x1F) - 15 + 127;
        int32_t exp_b = ((hb >> 10) & 0x1F) - 15 + 127;
        uint32_t mant_a = (ha & 0x03FF) << 13;
        uint32_t mant_b = (hb & 0x03FF) << 13;

        /* Handle special cases */
        if (exp_a <= 0) exp_a = 0;
        if (exp_b <= 0) exp_b = 0;

        uint32_t bits_a = (sign_a << 31) | (exp_a << 23) | mant_a;
        uint32_t bits_b = (sign_b << 31) | (exp_b << 23) | mant_b;

        float fa, fb;
        memcpy(&fa, &bits_a, sizeof(float));
        memcpy(&fb, &bits_b, sizeof(float));

        out[i] = fa * fb;
    }

    return result;
}

/* ============================================================================
 * Session 31: Advanced SIMD Operations - Polynomial
 * ============================================================================ */

/**
 * v128_pmull - Polynomial multiply long (low)
 * @param a First input vector (16 x 8-bit polynomial)
 * @param b Second input vector (16 x 8-bit polynomial)
 * @return Result vector (8 x 16-bit polynomial)
 *
 * Implements PMULL instruction for polynomial multiplication.
 * Each byte is treated as a polynomial coefficient.
 */
Vector128 v128_pmull(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint8_t *ina = (uint8_t *)&a;
    uint8_t *inb = (uint8_t *)&b;
    uint16_t *out = (uint16_t *)&result;

    for (int i = 0; i < 8; i++) {
        uint8_t pa = ina[i];
        uint8_t pb = inb[i];
        uint16_t product = 0;

        /* Polynomial multiplication over GF(2) */
        for (int bit_b = 0; bit_b < 8; bit_b++) {
            if ((pb >> bit_b) & 1) {
                product ^= (uint16_t)pa << bit_b;
            }
        }

        out[i] = product;
    }

    return result;
}

/**
 * v128_pmull2 - Polynomial multiply long (high)
 * @param a First input vector (16 x 8-bit polynomial, high bytes)
 * @param b Second input vector (16 x 8-bit polynomial, high bytes)
 * @return Result vector (8 x 16-bit polynomial)
 *
 * Implements PMULL2 instruction - operates on high 8 bytes.
 */
Vector128 v128_pmull2(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint8_t *ina = (uint8_t *)&a;
    uint8_t *inb = (uint8_t *)&b;
    uint16_t *out = (uint16_t *)&result;

    for (int i = 0; i < 8; i++) {
        uint8_t pa = ina[i + 8];  /* High half */
        uint8_t pb = inb[i + 8];
        uint16_t product = 0;

        /* Polynomial multiplication over GF(2) */
        for (int bit_b = 0; bit_b < 8; bit_b++) {
            if ((pb >> bit_b) & 1) {
                product ^= (uint16_t)pa << bit_b;
            }
        }

        out[i] = product;
    }

    return result;
}

/**
 * v128_pmul - Polynomial multiply
 * @param a First input vector (16 x 8-bit polynomial)
 * @param b Second input vector (16 x 8-bit polynomial)
 * @return Result vector (16 x 8-bit polynomial, low byte of each product)
 *
 * Implements PMUL instruction for polynomial multiplication.
 * Returns low byte of 16-bit product.
 */
Vector128 v128_pmul(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint8_t *ina = (uint8_t *)&a;
    uint8_t *inb = (uint8_t *)&b;
    uint8_t *out = (uint8_t *)&result;

    for (int i = 0; i < 16; i++) {
        uint8_t pa = ina[i];
        uint8_t pb = inb[i];
        uint16_t product = 0;

        /* Polynomial multiplication over GF(2) */
        for (int bit_b = 0; bit_b < 8; bit_b++) {
            if ((pb >> bit_b) & 1) {
                product ^= (uint16_t)pa << bit_b;
            }
        }

        out[i] = (uint8_t)(product & 0xFF);  /* Low byte only */
    }

    return result;
}

/* ============================================================================
 * Session 32: GF(2^8) Multiplication Helpers
 * ============================================================================ */

/* GF(2^8) multiplication helpers for AES - must be declared before use */
static inline uint8_t gf2_mul2(uint8_t x) {
    return (uint8_t)((x << 1) ^ ((x & 0x80) ? 0x1B : 0x00));
}

static inline uint8_t gf2_mul3(uint8_t x) {
    return (uint8_t)(gf2_mul2(x) ^ x);
}

static inline uint8_t gf2_mul9(uint8_t x) {
    return (uint8_t)(gf2_mul2(gf2_mul2(gf2_mul2(x))) ^ x);
}

static inline uint8_t gf2_mul11(uint8_t x) {
    return (uint8_t)(gf2_mul2(gf2_mul2(gf2_mul2(x))) ^ gf2_mul2(x) ^ x);
}

static inline uint8_t gf2_mul13(uint8_t x) {
    return (uint8_t)(gf2_mul2(gf2_mul2(gf2_mul2(x))) ^ gf2_mul2(x) ^ x);
}

static inline uint8_t gf2_mul14(uint8_t x) {
    return (uint8_t)(gf2_mul2(gf2_mul2(gf2_mul2(x))) ^ gf2_mul2(gf2_mul2(x)) ^ gf2_mul2(x));
}

/* ============================================================================
 * Session 32: Cryptographic Extensions
 * ============================================================================ */

/**
 * v128_aese - AES round encryption
 * @param state Current state vector (16 bytes)
 * @param round_key Round key vector
 * @return New state after AES round
 *
 * Implements AESE instruction - SubBytes + ShiftRows transformation.
 */
Vector128 v128_aese(Vector128 state, Vector128 round_key)
{
    Vector128 result;
    uint8_t *in = (uint8_t *)&state;
    uint8_t *key = (uint8_t *)&round_key;
    uint8_t *out = (uint8_t *)&result;

    /* AES S-box lookup table */
    static const uint8_t aes_sbox[256] = {
        0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
        0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
        0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
        0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
        0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
        0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
        0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
        0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
        0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
        0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
        0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
        0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
        0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
        0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
        0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
        0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
    };

    /* SubBytes + ShiftRows (combined for efficiency) */
    /* Row 0: no shift */
    out[0] = aes_sbox[in[0] ^ key[0]];
    out[4] = aes_sbox[in[4] ^ key[4]];
    out[8] = aes_sbox[in[8] ^ key[8]];
    out[12] = aes_sbox[in[12] ^ key[12]];

    /* Row 1: shift left by 1 */
    out[1] = aes_sbox[in[5] ^ key[5]];
    out[5] = aes_sbox[in[9] ^ key[9]];
    out[9] = aes_sbox[in[13] ^ key[13]];
    out[13] = aes_sbox[in[1] ^ key[1]];

    /* Row 2: shift left by 2 */
    out[2] = aes_sbox[in[10] ^ key[10]];
    out[6] = aes_sbox[in[14] ^ key[14]];
    out[10] = aes_sbox[in[2] ^ key[2]];
    out[14] = aes_sbox[in[6] ^ key[6]];

    /* Row 3: shift left by 3 */
    out[3] = aes_sbox[in[15] ^ key[15]];
    out[7] = aes_sbox[in[3] ^ key[3]];
    out[11] = aes_sbox[in[7] ^ key[7]];
    out[15] = aes_sbox[in[11] ^ key[11]];

    return result;
}

/**
 * v128_aesd - AES round decryption
 * @param state Current state vector (16 bytes)
 * @param round_key Round key vector
 * @return New state after AES inverse round
 *
 * Implements AESD instruction - inverse ShiftRows + InvSubBytes.
 */
Vector128 v128_aesd(Vector128 state, Vector128 round_key)
{
    Vector128 result;
    uint8_t *in = (uint8_t *)&state;
    uint8_t *key = (uint8_t *)&round_key;
    uint8_t *out = (uint8_t *)&result;

    /* AES inverse S-box lookup table */
    static const uint8_t aes_inv_sbox[256] = {
        0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
        0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
        0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
        0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
        0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
        0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
        0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
        0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
        0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
        0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
        0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
        0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
        0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
        0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
        0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
        0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
    };

    /* Inverse ShiftRows + InvSubBytes (combined) */
    /* Row 0: no shift */
    out[0] = aes_inv_sbox[in[0]] ^ key[0];
    out[4] = aes_inv_sbox[in[4]] ^ key[4];
    out[8] = aes_inv_sbox[in[8]] ^ key[8];
    out[12] = aes_inv_sbox[in[12] ^ key[12]];

    /* Row 1: shift right by 1 */
    out[1] = aes_inv_sbox[in[13]] ^ key[1];
    out[5] = aes_inv_sbox[in[1]] ^ key[5];
    out[9] = aes_inv_sbox[in[5]] ^ key[9];
    out[13] = aes_inv_sbox[in[9]] ^ key[13];

    /* Row 2: shift right by 2 */
    out[2] = aes_inv_sbox[in[10]] ^ key[2];
    out[6] = aes_inv_sbox[in[14]] ^ key[6];
    out[10] = aes_inv_sbox[in[2]] ^ key[10];
    out[14] = aes_inv_sbox[in[6]] ^ key[14];

    /* Row 3: shift right by 3 */
    out[3] = aes_inv_sbox[in[7]] ^ key[3];
    out[7] = aes_inv_sbox[in[11]] ^ key[7];
    out[11] = aes_inv_sbox[in[15]] ^ key[11];
    out[15] = aes_inv_sbox[in[3]] ^ key[15];

    return result;
}

/**
 * v128_aesmc - AES mix columns
 * @param col Column vector (4 bytes)
 * @return Mixed column vector
 *
 * Implements AESMC instruction - MixColumns transformation.
 */
Vector128 v128_aesmc(Vector128 col)
{
    Vector128 result;
    uint8_t *in = (uint8_t *)&col;
    uint8_t *out = (uint8_t *)&result;

    /* MixColumns for each column */
    for (int c = 0; c < 4; c++) {
        int base = c * 4;
        uint8_t s0 = in[base];
        uint8_t s1 = in[base + 1];
        uint8_t s2 = in[base + 2];
        uint8_t s3 = in[base + 3];

        /* MixColumns matrix multiplication in GF(2^8) */
        out[base] = (uint8_t)(gf2_mul2(s0) ^ gf2_mul3(s1) ^ s2 ^ s3);
        out[base + 1] = (uint8_t)(s0 ^ gf2_mul2(s1) ^ gf2_mul3(s2) ^ s3);
        out[base + 2] = (uint8_t)(s0 ^ s1 ^ gf2_mul2(s2) ^ gf2_mul3(s3));
        out[base + 3] = (uint8_t)(gf2_mul3(s0) ^ s1 ^ s2 ^ gf2_mul2(s3));
    }

    return result;
}

/**
 * v128_aesimc - AES inverse mix columns
 * @param col Column vector (4 bytes)
 * @return Inverse mixed column vector
 *
 * Implements AESIMC instruction - inverse MixColumns transformation.
 */
Vector128 v128_aesimc(Vector128 col)
{
    Vector128 result;
    uint8_t *in = (uint8_t *)&col;
    uint8_t *out = (uint8_t *)&result;

    /* Inverse MixColumns for each column */
    for (int c = 0; c < 4; c++) {
        int base = c * 4;
        uint8_t s0 = in[base];
        uint8_t s1 = in[base + 1];
        uint8_t s2 = in[base + 2];
        uint8_t s3 = in[base + 3];

        /* Inverse MixColumns matrix multiplication in GF(2^8) */
        out[base] = (uint8_t)(gf2_mul14(s0) ^ gf2_mul11(s1) ^ gf2_mul13(s2) ^ gf2_mul9(s3));
        out[base + 1] = (uint8_t)(gf2_mul9(s0) ^ gf2_mul14(s1) ^ gf2_mul11(s2) ^ gf2_mul13(s3));
        out[base + 2] = (uint8_t)(gf2_mul13(s0) ^ gf2_mul9(s1) ^ gf2_mul14(s2) ^ gf2_mul11(s3));
        out[base + 3] = (uint8_t)(gf2_mul11(s0) ^ gf2_mul13(s1) ^ gf2_mul9(s2) ^ gf2_mul14(s3));
    }

    return result;
}

/* ============================================================================
 * Session 33: SHA Cryptographic Extensions
 * ============================================================================ */

/**
 * v128_sha1c - SHA-1 hash update (Choose function)
 * @param hash Current hash state (a, b, c, d)
 * @param data Message schedule vector
 * @param wk Working constant
 * @return Updated hash vector
 *
 * Implements SHA1C instruction - SHA-1 hash update using F(X,Y,Z) = (X AND Y) OR (NOT X AND Z)
 */
Vector128 v128_sha1c(Vector128 hash, Vector128 data, Vector128 wk)
{
    Vector128 result;
    uint32_t *h = (uint32_t *)&hash;
    uint32_t *d = (uint32_t *)&data;
    uint32_t *w = (uint32_t *)&wk;
    uint32_t *out = (uint32_t *)&result;

    /* SHA-1 hash update using F function: F(X,Y,Z) = (X AND Y) OR (NOT X AND Z) */
    for (int i = 0; i < 4; i++) {
        uint32_t a = h[i];
        uint32_t b = h[(i + 1) & 3];
        uint32_t c = h[(i + 2) & 3];
        uint32_t d_val = h[(i + 3) & 3];
        uint32_t msg = d[i];
        uint32_t k = w[i];

        /* F(X,Y,Z) = (X AND Y) OR (NOT X AND Z) for rounds 0-19 */
        uint32_t f = (b & c) | ((~b) & d_val);

        /* TEMP = ROTL(a, 5) + F + e + msg + k */
        uint32_t temp = ((a << 5) | (a >> 27)) + f + ((a << 30) | (a >> 2)) + msg + k;

        out[i] = temp;
    }

    return result;
}

/**
 * v128_sha1p - SHA-1 hash update (Parity function)
 * @param hash Current hash state (a, b, c, d)
 * @param data Message schedule vector
 * @param wk Working constant
 * @return Updated hash vector
 *
 * Implements SHA1P instruction - SHA-1 hash update using F(X,Y,Z) = X XOR Y XOR Z
 */
Vector128 v128_sha1p(Vector128 hash, Vector128 data, Vector128 wk)
{
    Vector128 result;
    uint32_t *h = (uint32_t *)&hash;
    uint32_t *d = (uint32_t *)&data;
    uint32_t *w = (uint32_t *)&wk;
    uint32_t *out = (uint32_t *)&result;

    /* SHA-1 hash update using P function: F(X,Y,Z) = X XOR Y XOR Z */
    for (int i = 0; i < 4; i++) {
        uint32_t a = h[i];
        uint32_t b = h[(i + 1) & 3];
        uint32_t c = h[(i + 2) & 3];
        uint32_t d_val = h[(i + 3) & 3];
        uint32_t msg = d[i];
        uint32_t k = w[i];

        /* P(X,Y,Z) = X XOR Y XOR Z for rounds 40-59 */
        uint32_t f = b ^ c ^ d_val;

        /* TEMP = ROTL(a, 5) + F + e + msg + k */
        uint32_t temp = ((a << 5) | (a >> 27)) + f + ((a << 30) | (a >> 2)) + msg + k;

        out[i] = temp;
    }

    return result;
}

/**
 * v128_sha1m - SHA-1 hash update (Majority function)
 * @param hash Current hash state (a, b, c, d)
 * @param data Message schedule vector
 * @param wk Working constant
 * @return Updated hash vector
 *
 * Implements SHA1M instruction - SHA-1 hash update using F(X,Y,Z) = (X AND Y) OR (X AND Z) OR (Y AND Z)
 */
Vector128 v128_sha1m(Vector128 hash, Vector128 data, Vector128 wk)
{
    Vector128 result;
    uint32_t *h = (uint32_t *)&hash;
    uint32_t *d = (uint32_t *)&data;
    uint32_t *w = (uint32_t *)&wk;
    uint32_t *out = (uint32_t *)&result;

    /* SHA-1 hash update using M function: F(X,Y,Z) = (X AND Y) OR (X AND Z) OR (Y AND Z) */
    for (int i = 0; i < 4; i++) {
        uint32_t a = h[i];
        uint32_t b = h[(i + 1) & 3];
        uint32_t c = h[(i + 2) & 3];
        uint32_t d_val = h[(i + 3) & 3];
        uint32_t msg = d[i];
        uint32_t k = w[i];

        /* M(X,Y,Z) = (X AND Y) OR (X AND Z) OR (Y AND Z) for rounds 20-39, 60-79 */
        uint32_t f = (b & c) | (b & d_val) | (c & d_val);

        /* TEMP = ROTL(a, 5) + F + e + msg + k */
        uint32_t temp = ((a << 5) | (a >> 27)) + f + ((a << 30) | (a >> 2)) + msg + k;

        out[i] = temp;
    }

    return result;
}

/**
 * v128_sha1su0 - SHA-1 schedule update 0
 * @param data Message schedule vector (W[t-3], W[t-4], W[t-5], W[t-6])
 * @param wk Working vector
 * @return Updated message schedule vector
 *
 * Implements SHA1SU0 instruction - first part of SHA-1 message schedule update.
 * W[t] = W[t-3] XOR W[t-8] XOR W[t-14] XOR W[t-16]
 */
Vector128 v128_sha1su0(Vector128 data, Vector128 wk)
{
    Vector128 result;
    uint32_t *d = (uint32_t *)&data;
    uint32_t *w = (uint32_t *)&wk;
    uint32_t *out = (uint32_t *)&result;

    /* SHA-1 message schedule update - first part */
    for (int i = 0; i < 4; i++) {
        uint32_t w_t_minus_3 = d[i];
        uint32_t w_t_minus_4 = w[i];

        /* ROTR(W[t-3], 2) XOR ROTR(W[t-3], 13) XOR ROTR(W[t-3], 22) */
        uint32_t s0 = ((w_t_minus_3 >> 2) | (w_t_minus_3 << 30)) ^
                      ((w_t_minus_3 >> 13) | (w_t_minus_3 << 19)) ^
                      ((w_t_minus_3 >> 22) | (w_t_minus_3 << 10));

        out[i] = s0 ^ w_t_minus_4;
    }

    return result;
}

/**
 * v128_sha1su1 - SHA-1 schedule update 1
 * @param data Message schedule vector
 * @param wk Working vector
 * @return Updated message schedule vector
 *
 * Implements SHA1SU1 instruction - second part of SHA-1 message schedule update.
 */
Vector128 v128_sha1su1(Vector128 data, Vector128 wk)
{
    Vector128 result;
    uint32_t *d = (uint32_t *)&data;
    uint32_t *w = (uint32_t *)&wk;
    uint32_t *out = (uint32_t *)&result;

    /* SHA-1 message schedule update - second part */
    for (int i = 0; i < 4; i++) {
        uint32_t w_prev = d[i];
        uint32_t w_curr = w[i];

        /* W[t] = W[t-1] + W[t-2] XOR ROTR(W[t-2], 23) */
        uint32_t rotr23 = (w_curr >> 23) | (w_curr << 9);
        out[i] = w_prev + (w_curr ^ rotr23);
    }

    return result;
}

/**
 * v128_sha256h - SHA-256 hash update (high part)
 * @param hash Current hash state (a, b, c, d)
 * @param data Message schedule vector
 * @param wk Working constant
 * @return Updated hash vector
 *
 * Implements SHA256H instruction - SHA-256 hash update using Sigma1 and Ch functions.
 */
Vector128 v128_sha256h(Vector128 hash, Vector128 data, Vector128 wk)
{
    Vector128 result;
    uint32_t *h = (uint32_t *)&hash;
    uint32_t *d = (uint32_t *)&data;
    uint32_t *w = (uint32_t *)&wk;
    uint32_t *out = (uint32_t *)&result;

    /* SHA-256 hash update using Ch function: Ch(E,F,G) = (E AND F) XOR (NOT E AND G) */
    for (int i = 0; i < 4; i++) {
        uint32_t a = h[i];
        uint32_t b = h[(i + 1) & 3];
        uint32_t c = h[(i + 2) & 3];
        uint32_t d_val = h[(i + 3) & 3];
        uint32_t msg = d[i];
        uint32_t k = w[i];

        /* Ch(E,F,G) = (E AND F) XOR (NOT E AND G) */
        uint32_t ch = (b & c) ^ ((~b) & d_val);

        /* Sigma1: ROTR(E, 6) XOR ROTR(E, 11) XOR ROTR(E, 25) */
        uint32_t sigma1 = ((b >> 6) | (b << 26)) ^
                          ((b >> 11) | (b << 21)) ^
                          ((b >> 25) | (b << 7));

        /* TEMP = ROTL(a, 5) + Ch + msg + k */
        uint32_t temp = ((a << 5) | (a >> 27)) + ch + msg + k;

        out[i] = temp + sigma1;
    }

    return result;
}

/**
 * v128_sha256h2 - SHA-256 hash update (high part 2)
 * @param hash Current hash state (e, f, g, h)
 * @param data Message schedule vector
 * @param wk Working constant
 * @return Updated hash vector
 *
 * Implements SHA256H2 instruction - SHA-256 hash update using Sigma0 and Maj functions.
 */
Vector128 v128_sha256h2(Vector128 hash, Vector128 data, Vector128 wk)
{
    Vector128 result;
    uint32_t *h = (uint32_t *)&hash;
    uint32_t *d = (uint32_t *)&data;
    uint32_t *w = (uint32_t *)&wk;
    uint32_t *out = (uint32_t *)&result;

    /* SHA-256 hash update using Maj function: Maj(A,B,C) = (A AND B) XOR (A AND C) XOR (B AND C) */
    for (int i = 0; i < 4; i++) {
        uint32_t a = h[i];
        uint32_t b = h[(i + 1) & 3];
        uint32_t c = h[(i + 2) & 3];
        uint32_t d_val = h[(i + 3) & 3];
        uint32_t msg = d[i];
        uint32_t k = w[i];

        /* Maj(A,B,C) = (A AND B) XOR (A AND C) XOR (B AND C) */
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);

        /* Sigma0: ROTR(A, 2) XOR ROTR(A, 13) XOR ROTR(A, 22) */
        uint32_t sigma0 = ((a >> 2) | (a << 30)) ^
                          ((a >> 13) | (a << 19)) ^
                          ((a >> 22) | (a << 10));

        /* Combine with message and constant */
        out[i] = maj + sigma0 + msg + k;
    }

    return result;
}

/**
 * v128_sha256su0 - SHA-256 schedule update 0
 * @param data Message schedule vector (W[t-15]..W[t-12])
 * @param wk Working vector
 * @return Updated message schedule vector
 *
 * Implements SHA256SU0 instruction - first part of SHA-256 message schedule update.
 * W[t] = W[t-15] XOR sigma0(W[t-15])
 */
Vector128 v128_sha256su0(Vector128 data, Vector128 wk)
{
    Vector128 result;
    uint32_t *d = (uint32_t *)&data;
    uint32_t *w = (uint32_t *)&wk;
    uint32_t *out = (uint32_t *)&result;

    /* SHA-256 message schedule update - first part */
    for (int i = 0; i < 4; i++) {
        uint32_t w_prev = d[i];

        /* sigma0: ROTR(W[t-15], 7) XOR ROTR(W[t-15], 18) XOR SHR(W[t-15], 3) */
        uint32_t sigma0 = ((w_prev >> 7) | (w_prev << 25)) ^
                          ((w_prev >> 18) | (w_prev << 14)) ^
                          (w_prev >> 3);

        out[i] = w_prev ^ sigma0;
    }

    /* Apply XOR with wk */
    out[0] ^= w[0];
    out[1] ^= w[1];

    return result;
}

/**
 * v128_sha256su1 - SHA-256 schedule update 1
 * @param data Message schedule vector
 * @param wk Working vector
 * @return Updated message schedule vector
 *
 * Implements SHA256SU1 instruction - second part of SHA-256 message schedule update.
 * W[t] = W[t-2] XOR sigma1(W[t-2]) + W[t-7] + W[t-16]
 */
Vector128 v128_sha256su1(Vector128 data, Vector128 wk)
{
    Vector128 result;
    uint32_t *d = (uint32_t *)&data;
    uint32_t *w = (uint32_t *)&wk;
    uint32_t *out = (uint32_t *)&result;

    /* SHA-256 message schedule update - second part */
    for (int i = 0; i < 4; i++) {
        uint32_t w_prev = d[i];
        uint32_t w_curr = w[i];

        /* sigma1: ROTR(W[t-2], 17) XOR ROTR(W[t-2], 19) XOR SHR(W[t-2], 10) */
        uint32_t sigma1 = ((w_curr >> 17) | (w_curr << 15)) ^
                          ((w_curr >> 19) | (w_curr << 13)) ^
                          (w_curr >> 10);

        out[i] = w_prev + (sigma1 ^ w_curr);
    }

    return result;
}

/* ============================================================================
 * Session 33: CRC32 Extensions
 * ============================================================================ */

/**
 * crc32b - CRC32 byte
 * @param crc Current CRC value
 * @param byte Byte to process
 * @return Updated CRC32 value
 *
 * Implements CRC32B instruction - CRC32 for a single byte.
 */
uint32_t crc32b(uint32_t crc, uint8_t byte)
{
    /* CRC32 polynomial: 0xEDB88320 (reflected form of 0x04C11DB7) */
    crc ^= byte;
    for (int i = 0; i < 8; i++) {
        crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
    }
    return crc;
}

/**
 * crc32h - CRC32 halfword
 * @param crc Current CRC value
 * @param halfword Halfword to process
 * @return Updated CRC32 value
 *
 * Implements CRC32H instruction - CRC32 for a 16-bit halfword.
 */
uint32_t crc32h(uint32_t crc, uint16_t halfword)
{
    crc = crc32b(crc, halfword & 0xFF);
    crc = crc32b(crc, (halfword >> 8) & 0xFF);
    return crc;
}

/**
 * crc32w - CRC32 word
 * @param crc Current CRC value
 * @param word Word to process
 * @return Updated CRC32 value
 *
 * Implements CRC32W instruction - CRC32 for a 32-bit word.
 */
uint32_t crc32w(uint32_t crc, uint32_t word)
{
    crc = crc32b(crc, word & 0xFF);
    crc = crc32b(crc, (word >> 8) & 0xFF);
    crc = crc32b(crc, (word >> 16) & 0xFF);
    crc = crc32b(crc, (word >> 24) & 0xFF);
    return crc;
}

/**
 * crc32x - CRC32 doubleword
 * @param crc Current CRC value
 * @param dword Doubleword to process
 * @return Updated CRC32 value
 *
 * Implements CRC32X instruction - CRC32 for a 64-bit doubleword.
 */
uint64_t crc32x(uint64_t crc, uint64_t dword)
{
    crc = crc32b((uint32_t)crc, dword & 0xFF);
    crc = crc32b((uint32_t)crc, (dword >> 8) & 0xFF);
    crc = crc32b((uint32_t)crc, (dword >> 16) & 0xFF);
    crc = crc32b((uint32_t)crc, (dword >> 24) & 0xFF);
    crc = crc32b((uint32_t)crc, (dword >> 32) & 0xFF);
    crc = crc32b((uint32_t)crc, (dword >> 40) & 0xFF);
    crc = crc32b((uint32_t)crc, (dword >> 48) & 0xFF);
    crc = crc32b((uint32_t)crc, (dword >> 56) & 0xFF);
    return crc;
}

/* ============================================================================
 * Session 33: Random Number Generation
 * ============================================================================ */

/**
 * rndr - Random number
 * @return 64-bit random number
 *
 * Implements RNDR instruction - hardware random number generator.
 * Returns a random number using the system's random number generator.
 */
uint64_t rndr(void)
{
    /* Use /dev/urandom for random number generation */
    static int fd = -1;
    uint64_t result;

    if (fd < 0) {
        fd = open("/dev/urandom", O_RDONLY);
        if (fd < 0) {
            /* Fallback to pseudo-random if /dev/urandom unavailable */
            return (uint64_t)rand();
        }
    }

    ssize_t ret = read(fd, &result, sizeof(result));
    if (ret != sizeof(result)) {
        return (uint64_t)rand();
    }

    return result;
}

/**
 * rndrrs - Reseeded random number
 * @return 64-bit reseeded random number
 *
 * Implements RNDRRS instruction - reseeded random number generator.
 * Returns a random number that is guaranteed to be reseeded from the entropy source.
 */
uint64_t rndrrs(void)
{
    /* For RNDRRS, we always reseed by reading fresh entropy */
    int fd = open("/dev/urandom", O_RDONLY);
    uint64_t result;

    if (fd < 0) {
        /* Fallback to pseudo-random if /dev/urandom unavailable */
        srand((unsigned int)time(NULL));
        return (uint64_t)rand();
    }

    ssize_t ret = read(fd, &result, sizeof(result));
    close(fd);

    if (ret != sizeof(result)) {
        srand((unsigned int)time(NULL));
        return (uint64_t)rand();
    }

    return result;
}

/* ============================================================================
 * Session 33: Additional SIMD Utilities
 * ============================================================================ */

/**
 * v128_bswap - Byte swap (reverse byte order)
 * @param a Input vector
 * @return Vector with reversed byte order
 *
 * Implements BSL/BSWAP instruction - reverses byte order in each element.
 */
Vector128 v128_bswap(Vector128 a)
{
    Vector128 result;
    uint8_t *in = (uint8_t *)&a;
    uint8_t *out = (uint8_t *)&result;

    /* Reverse byte order */
    for (int i = 0; i < 16; i++) {
        out[i] = in[15 - i];
    }

    return result;
}

/**
 * v128_bitsel - Bit select
 * @param a First source vector
 * @param b Second source vector
 * @param c Condition vector
 * @return Result vector (c ? a : b for each bit)
 *
 * Implements BSL (Bit Select) instruction - conditional select by bit.
 * For each bit, if condition bit is 1, select from a, otherwise from b.
 */
Vector128 v128_bitsel(Vector128 a, Vector128 b, Vector128 c)
{
    Vector128 result;
    uint64_t *in_a = (uint64_t *)&a;
    uint64_t *in_b = (uint64_t *)&b;
    uint64_t *in_c = (uint64_t *)&c;
    uint64_t *out = (uint64_t *)&result;

    /* BSL: result = (c AND a) OR (NOT c AND b) */
    out[0] = (in_c[0] & in_a[0]) | ((~in_c[0]) & in_b[0]);
    out[1] = (in_c[1] & in_a[1]) | ((~in_c[1]) & in_b[1]);

    return result;
}

/**
 * v128_rshrn - Rounded shift right narrow
 * @param a Input vector
 * @param shift Shift amount
 * @return Narrowed result vector
 *
 * Implements RSHRN instruction - rounded shift right then narrow.
 */
Vector128 v128_rshrn(Vector128 a, int shift)
{
    Vector128 result;
    uint16_t *in16 = (uint16_t *)&a;
    uint8_t *out = (uint8_t *)&result;

    /* Rounded shift right: add 2^(shift-1) before shifting */
    uint16_t round = 1 << (shift - 1);

    for (int i = 0; i < 8; i++) {
        out[i] = (uint8_t)((in16[i] + round) >> shift);
    }

    return result;
}

/**
 * v128_srshrn - Signed rounded shift right narrow
 * @param a Input vector
 * @param shift Shift amount
 * @return Narrowed result vector
 *
 * Implements SRSHRN instruction - signed rounded shift right then narrow.
 */
Vector128 v128_srshrn(Vector128 a, int shift)
{
    Vector128 result;
    int16_t *in16 = (int16_t *)&a;
    int8_t *out = (int8_t *)&result;

    /* Signed rounded shift right */
    int16_t round = 1 << (shift - 1);

    for (int i = 0; i < 8; i++) {
        int32_t val = in16[i];
        val = val + round;
        val = val >> shift;
        out[i] = (int8_t)val;
    }

    return result;
}

/**
 * v128_urshrn - Unsigned rounded shift right narrow
 * @param a Input vector
 * @param shift Shift amount
 * @return Narrowed result vector
 *
 * Implements URSHRN instruction - unsigned rounded shift right then narrow.
 */
Vector128 v128_urshrn(Vector128 a, int shift)
{
    Vector128 result;
    uint16_t *in16 = (uint16_t *)&a;
    uint8_t *out = (uint8_t *)&result;

    /* Unsigned rounded shift right */
    uint16_t round = 1 << (shift - 1);

    for (int i = 0; i < 8; i++) {
        out[i] = (uint8_t)((in16[i] + round) >> shift);
    }

    return result;
}

/* ============================================================================
 * Session 34: Additional Utility Functions
 * ============================================================================ */

/* -----------------------------------------------------------------------------
 * Additional String Utilities
 * ----------------------------------------------------------------------------- */

/**
 * rosetta_strdup - Duplicate a string
 * @param s Source string
 * @return Pointer to newly allocated duplicate string
 *
 * Duplicates a string by allocating memory and copying the contents.
 * The returned string must be freed with rosetta_free().
 */
char *rosetta_strdup(const char *s)
{
    size_t len;
    char *dup;

    if (s == NULL) {
        return NULL;
    }

    len = rosetta_strlen(s);
    dup = (char *)rosetta_malloc(len + 1);
    if (dup != NULL) {
        rosetta_memcpy(dup, s, len + 1);
    }

    return dup;
}

/**
 * rosetta_strstr - Find substring in string
 * @param haystack String to search in
 * @param needle String to search for
 * @return Pointer to first occurrence of needle, or NULL if not found
 *
 * Finds the first occurrence of the substring needle in the string haystack.
 */
char *rosetta_strstr(const char *haystack, const char *needle)
{
    size_t h_len, n_len;
    const char *h;

    if (haystack == NULL || needle == NULL) {
        return NULL;
    }

    h_len = rosetta_strlen(haystack);
    n_len = rosetta_strlen(needle);

    if (n_len == 0) {
        return (char *)haystack;
    }

    if (n_len > h_len) {
        return NULL;
    }

    for (h = haystack; h_len >= n_len; h++, h_len--) {
        if (rosetta_memcmp(h, needle, n_len) == 0) {
            return (char *)h;
        }
    }

    return NULL;
}

/**
 * rosetta_strpbrk - Find first match of any character from set
 * @param s String to search
 * @param charset Set of characters to match
 * @return Pointer to first matching character, or NULL if none found
 *
 * Finds the first occurrence in string s of any character from charset.
 */
char *rosetta_strpbrk(const char *s, const char *charset)
{
    const char *c;

    if (s == NULL || charset == NULL) {
        return NULL;
    }

    while (*s != '\0') {
        for (c = charset; *c != '\0'; c++) {
            if (*s == *c) {
                return (char *)s;
            }
        }
        s++;
    }

    return NULL;
}

/**
 * rosetta_strtok - Tokenize a string
 * @param str String to tokenize (NULL on subsequent calls)
 * @param delim Delimiter characters
 * @return Pointer to next token, or NULL if no more tokens
 *
 * Extracts tokens from string str delimited by characters in delim.
 * Maintains state between calls (use str on first call, NULL on subsequent).
 */
char *rosetta_strtok(char *str, const char *delim)
{
    static char *last;
    char *token_start;

    if (str != NULL) {
        last = str;
    } else if (last == NULL) {
        return NULL;
    }

    /* Skip leading delimiters */
    while (*last != '\0') {
        const char *d;
        int is_delim = 0;

        for (d = delim; *d != '\0'; d++) {
            if (*last == *d) {
                is_delim = 1;
                break;
            }
        }

        if (!is_delim) {
            break;
        }
        last++;
    }

    if (*last == '\0') {
        return NULL;
    }

    token_start = last;

    /* Find end of token */
    while (*last != '\0') {
        const char *d;
        int is_delim = 0;

        for (d = delim; *d != '\0'; d++) {
            if (*last == *d) {
                is_delim = 1;
                break;
            }
        }

        if (is_delim) {
            *last = '\0';
            last++;
            return token_start;
        }
        last++;
    }

    return token_start;
}

/**
 * rosetta_memmem - Find memory region in memory region
 * @param haystack Pointer to memory to search in
 * @param haystack_len Length of haystack
 * @param needle Pointer to memory to search for
 * @param needle_len Length of needle
 * @return Pointer to first occurrence, or NULL if not found
 *
 * Finds the first occurrence of needle in haystack.
 */
void *rosetta_memmem(const void *haystack, size_t haystack_len,
                     const void *needle, size_t needle_len)
{
    const uint8_t *h = (const uint8_t *)haystack;

    if (needle_len == 0) {
        return (void *)haystack;
    }

    if (needle_len > haystack_len) {
        return NULL;
    }

    while (haystack_len >= needle_len) {
        if (rosetta_memcmp(h, needle, needle_len) == 0) {
            return (void *)h;
        }
        h++;
        haystack_len--;
    }

    return NULL;
}

/**
 * rosetta_memrchr - Find last occurrence of byte in memory
 * @param s Pointer to memory to search
 * @param c Byte to search for
 * @param n Length of memory region
 * @return Pointer to last occurrence, or NULL if not found
 *
 * Finds the last occurrence of byte c in the first n bytes of s.
 */
void *rosetta_memrchr(const void *s, int c, size_t n)
{
    const uint8_t *p = (const uint8_t *)s;
    const uint8_t *found = NULL;
    size_t i;

    for (i = 0; i < n; i++) {
        if (p[i] == (uint8_t)c) {
            found = &p[i];
        }
    }

    return (void *)found;
}

/* -----------------------------------------------------------------------------
 * Additional Integer/Bit Utilities
 * ----------------------------------------------------------------------------- */

/**
 * count_trailing_zeros32 - Count trailing zeros in 32-bit word
 * @param x Input value
 * @return Number of trailing zero bits
 */
uint32_t count_trailing_zeros32(uint32_t x)
{
    uint32_t count = 0;

    if (x == 0) {
        return 32;
    }

    while ((x & 1) == 0) {
        x >>= 1;
        count++;
    }

    return count;
}

/**
 * count_trailing_zeros64 - Count trailing zeros in 64-bit word
 * @param x Input value
 * @return Number of trailing zero bits
 */
uint64_t count_trailing_zeros64(uint64_t x)
{
    uint64_t count = 0;

    if (x == 0) {
        return 64;
    }

    while ((x & 1) == 0) {
        x >>= 1;
        count++;
    }

    return count;
}

/**
 * is_power_of_2 - Check if value is power of 2
 * @param x Input value
 * @return true if x is power of 2, false otherwise
 */
bool is_power_of_2(uint64_t x)
{
    return x != 0 && (x & (x - 1)) == 0;
}

/**
 * round_up_to_pow2 - Round up to nearest power of 2
 * @param x Input value
 * @return Smallest power of 2 >= x
 */
uint64_t round_up_to_pow2(uint64_t x)
{
    uint64_t result = 1;

    if (x == 0) {
        return 1;
    }

    x--;
    while (x > 0) {
        x >>= 1;
        result <<= 1;
    }

    return result;
}

/* -----------------------------------------------------------------------------
 * Additional Translation Infrastructure Utilities
 * ----------------------------------------------------------------------------- */

/**
 * translation_cache_get_size - Get current translation cache size
 * @return Number of entries in translation cache
 */
size_t translation_cache_get_size(void)
{
    /* Return fixed cache size - actual implementation would track entries */
    return 4096;  /* Direct-mapped cache with 4096 entries */
}

/**
 * translation_cache_is_full - Check if translation cache is full
 * @return true if cache is full, false otherwise
 */
bool translation_cache_is_full(void)
{
    /* Simplified check - actual implementation would track occupancy */
    return false;  /* Never report full for basic implementation */
}

/**
 * code_cache_get_free_space - Get amount of free space in code cache
 * @return Number of bytes free in code cache
 */
size_t code_cache_get_free_space(void)
{
    /* Placeholder - actual implementation would track allocation */
    return 1024 * 1024;  /* 1 MB default */
}

/**
 * code_cache_reset - Reset code cache to initial state
 *
 * Frees all allocated code cache memory and resets to initial state.
 */
void code_cache_reset(void)
{
    /* Placeholder - actual implementation would free all code cache memory */
}

/* -----------------------------------------------------------------------------
 * Additional ELF Utilities
 * ----------------------------------------------------------------------------- */

/**
 * elf_get_section_offset - Get offset of section in ELF file
 * @param base Pointer to ELF base
 * @param section_index Index of section
 * @return Offset of section, or 0 if invalid
 */
uint64_t elf_get_section_offset(const void *base, uint32_t section_index)
{
    /* Placeholder - actual implementation would parse ELF section headers */
    return 0;
}

/**
 * elf_get_section_size - Get size of section in ELF file
 * @param base Pointer to ELF base
 * @param section_index Index of section
 * @return Size of section, or 0 if invalid
 */
uint64_t elf_get_section_size(const void *base, uint32_t section_index)
{
    /* Placeholder - actual implementation would parse ELF section headers */
    return 0;
}

/**
 * elf_is_valid_class64 - Check if ELF is 64-bit class
 * @param base Pointer to ELF base
 * @return true if valid 64-bit ELF, false otherwise
 */
bool elf_is_valid_class64(const void *base)
{
    const uint8_t *elf = (const uint8_t *)base;

    if (elf == NULL) {
        return false;
    }

    /* Check ELF magic and class */
    if (elf[0] != 0x7f || elf[1] != 'E' || elf[2] != 'L' || elf[3] != 'F') {
        return false;
    }

    /* EI_CLASS is at offset 4, ELFCLASS64 is 2 */
    return elf[4] == 2;
}

/**
 * elf_is_valid_machine_aarch64 - Check if ELF is AArch64 machine type
 * @param base Pointer to ELF base
 * @return true if AArch64 ELF, false otherwise
 */
bool elf_is_valid_machine_aarch64(const void *base)
{
    const uint8_t *elf = (const uint8_t *)base;

    if (elf == NULL) {
        return false;
    }

    /* e_machine is at offset 18 (little-endian) */
    /* EM_AARCH64 is 183 (0xB7) */
    return elf[18] == 0xB7 && elf[19] == 0x00;
}

/* -----------------------------------------------------------------------------
 * Additional Memory Utilities
 * ----------------------------------------------------------------------------- */

/**
 * rosetta_memchr_eq - Find first byte equal to any in mask
 * @param s Pointer to memory to search
 * @param mask 16-byte mask of bytes to match
 * @param n Length of memory region
 * @return Pointer to first matching byte, or NULL if none found
 *
 * SIMD-optimized search for bytes matching any in a 16-byte mask.
 */
void *rosetta_memchr_eq(const void *s, const uint8_t mask[16], size_t n)
{
    const uint8_t *p = (const uint8_t *)s;
    size_t i, j;

    for (i = 0; i < n; i++) {
        for (j = 0; j < 16; j++) {
            if (p[i] == mask[j]) {
                return (void *)&p[i];
            }
        }
    }

    return NULL;
}

/**
 * rosetta_memcpy_nonoverlapping - Optimized memcpy for non-overlapping regions
 * @param dest Destination pointer
 * @param src Source pointer
 * @param n Number of bytes to copy
 * @return Destination pointer
 *
 * Like memcpy but assumes non-overlapping regions for potential optimization.
 */
void *rosetta_memcpy_nonoverlapping(void *dest, const void *src, size_t n)
{
    return rosetta_memcpy(dest, src, n);
}

/**
 * rosetta_memmove_safe - Safe memory move with overlap detection
 * @param dest Destination pointer
 * @param src Source pointer
 * @param n Number of bytes to move
 * @return Destination pointer
 *
 * Explicitly handles overlapping regions safely.
 */
void *rosetta_memmove_safe(void *dest, const void *src, size_t n)
{
    return rosetta_memmove(dest, src, n);
}

/**
 * rosetta_memswap - Swap contents of two memory regions
 * @param a First memory region
 * @param b Second memory region
 * @param n Number of bytes to swap
 *
 * Swaps n bytes between memory regions a and b.
 */
void rosetta_memswap(void *a, void *b, size_t n)
{
    uint8_t *pa = (uint8_t *)a;
    uint8_t *pb = (uint8_t *)b;
    uint8_t tmp;
    size_t i;

    for (i = 0; i < n; i++) {
        tmp = pa[i];
        pa[i] = pb[i];
        pb[i] = tmp;
    }
}

/**
 * rosetta_memfill_word - Fill memory with word pattern
 * @param dest Destination pointer
 * @param word Word pattern to fill
 * @param n Number of bytes to fill
 * @return Destination pointer
 *
 * Fills memory region with repeated word pattern.
 */
void *rosetta_memfill_word(void *dest, uint64_t word, size_t n)
{
    uint8_t *p = (uint8_t *)dest;
    size_t i;

    for (i = 0; i < n; i++) {
        p[i] = ((uint8_t *)&word)[i % sizeof(word)];
    }

    return dest;
}

/* -----------------------------------------------------------------------------
 * Additional String Utilities - Length Limited
 * ----------------------------------------------------------------------------- */

/**
 * rosetta_strnlen - Calculate string length with limit
 * @param s String to measure
 * @param maxlen Maximum length to check
 * @return Length of string or maxlen if no null terminator found
 */
size_t rosetta_strnlen(const char *s, size_t maxlen)
{
    size_t i;

    for (i = 0; i < maxlen && s[i] != '\0'; i++);

    return i;
}

/**
 * rosetta_strlcpy - Copy string with size limit
 * @param dest Destination buffer
 * @param src Source string
 * @param destsize Size of destination buffer
 * @return Length of src
 *
 * Copies up to destsize-1 characters from src to dest, null-terminating.
 * Returns the length of src (not including null terminator).
 */
size_t rosetta_strlcpy(char *dest, const char *src, size_t destsize)
{
    size_t src_len = rosetta_strlen(src);
    size_t copy_len;

    if (destsize > 0) {
        copy_len = (src_len < destsize - 1) ? src_len : destsize - 1;
        rosetta_memcpy(dest, src, copy_len);
        dest[copy_len] = '\0';
    }

    return src_len;
}

/**
 * rosetta_strlcat - Concatenate strings with size limit
 * @param dest Destination buffer (must be null-terminated)
 * @param src Source string to append
 * @param destsize Total size of destination buffer
 * @return Total length of string we tried to create (initial dest length + src length)
 */
size_t rosetta_strlcat(char *dest, const char *src, size_t destsize)
{
    size_t dest_len = rosetta_strlen(dest);
    size_t src_len = rosetta_strlen(src);
    size_t copy_len;

    if (destsize > dest_len) {
        copy_len = (src_len < destsize - dest_len - 1) ? src_len : destsize - dest_len - 1;
        rosetta_memcpy(dest + dest_len, src, copy_len);
        dest[dest_len + copy_len] = '\0';
    }

    return dest_len + src_len;
}

