#ifndef ROSETTA_CODEGEN_H
#define ROSETTA_CODEGEN_H

/* ============================================================================
 * Rosetta Binary Translator - x86_64 Code Generation
 * ============================================================================ */

#include "rosetta_types.h"

/* ============================================================================
 * Code Buffer Management
 * ============================================================================ */

/**
 * Initialize code buffer
 * @param buf Code buffer to initialize
 * @param buffer Optional pre-allocated buffer (NULL for internal allocation)
 * @param size Buffer size
 * @return ROSETTA_OK on success
 */
int code_buffer_init(code_buffer_t *buf, u8 *buffer, u32 size);

/**
 * Free code buffer resources
 * @param buf Code buffer to free
 */
void code_buffer_cleanup(code_buffer_t *buf);

/**
 * Emit a single byte
 * @param buf Code buffer
 * @param byte Byte to emit
 */
void emit_byte(code_buffer_t *buf, u8 byte);

/**
 * Emit a 32-bit word
 * @param buf Code buffer
 * @param word Word to emit
 */
void emit_word32(code_buffer_t *buf, u32 word);

/**
 * Emit a 64-bit word
 * @param buf Code buffer
 * @param word Word to emit
 */
void emit_word64(code_buffer_t *buf, u64 word);

/**
 * Get current code pointer
 * @param buf Code buffer
 * @return Current write position
 */
u8 *code_buffer_get_ptr(code_buffer_t *buf);

/**
 * Get current code size
 * @param buf Code buffer
 * @return Number of bytes emitted
 */
u32 code_buffer_get_size(code_buffer_t *buf);

/* ============================================================================
 * x86_64 Register Allocation
 * ============================================================================ */

/**
 * Map ARM64 register to x86_64 register
 * @param arm64_reg ARM64 register number (0-30)
 * @return x86_64 register number
 */
u8 x86_map_gpr(u8 arm64_reg);

/**
 * Map ARM64 vector register to x86_64 XMM register
 * @param arm64_vreg ARM64 vector register number (0-31)
 * @return x86_64 XMM register number
 */
u8 x86_map_xmm(u8 arm64_vreg);

/* ============================================================================
 * General Purpose Register Instructions
 * ============================================================================ */

/**
 * Emit MOV reg64, imm64
 * @param buf Code buffer
 * @param dst Destination register
 * @param imm 64-bit immediate
 */
void emit_mov_reg_imm64(code_buffer_t *buf, u8 dst, u64 imm);

/**
 * Emit MOV reg64, reg64
 * @param buf Code buffer
 * @param dst Destination register
 * @param src Source register
 */
void emit_mov_reg_reg(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit MOV [mem], reg64 (store)
 * @param buf Code buffer
 * @param dst_reg Destination register (for address calculation)
 * @param src_reg Source register to store
 * @param disp Optional displacement
 */
void emit_mov_mem_reg(code_buffer_t *buf, u8 dst_reg, u8 src_reg, s32 disp);

/**
 * Emit MOV reg64, [mem] (load)
 * @param buf Code buffer
 * @param dst_reg Destination register
 * @param src_reg Source register (for address calculation)
 * @param disp Optional displacement
 */
void emit_mov_reg_mem(code_buffer_t *buf, u8 dst_reg, u8 src_reg, s32 disp);

/**
 * Emit ADD reg64, reg64
 * @param buf Code buffer
 * @param dst Destination/source register
 * @param src Source register
 */
void emit_add_reg_reg(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit ADD reg64, imm32
 * @param buf Code buffer
 * @param dst Destination register
 * @param imm 32-bit immediate
 */
void emit_add_reg_imm32(code_buffer_t *buf, u8 dst, u32 imm);

/**
 * Emit SUB reg64, reg64
 * @param buf Code buffer
 * @param dst Destination/source register
 * @param src Source register
 */
void emit_sub_reg_reg(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit SUB reg64, imm32
 * @param buf Code buffer
 * @param dst Destination register
 * @param imm 32-bit immediate
 */
void emit_sub_reg_imm32(code_buffer_t *buf, u8 dst, u32 imm);

/**
 * Emit AND reg64, reg64
 * @param buf Code buffer
 * @param dst Destination/source register
 * @param src Source register
 */
void emit_and_reg_reg(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit AND reg64, imm32
 * @param buf Code buffer
 * @param dst Destination register
 * @param imm 32-bit immediate
 */
void emit_and_reg_imm32(code_buffer_t *buf, u8 dst, u32 imm);

/**
 * Emit ORR reg64, reg64
 * @param buf Code buffer
 * @param dst Destination/source register
 * @param src Source register
 */
void emit_orr_reg_reg(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit ORR reg64, imm32
 * @param buf Code buffer
 * @param dst Destination register
 * @param imm 32-bit immediate
 */
void emit_orr_reg_imm32(code_buffer_t *buf, u8 dst, u32 imm);

/**
 * Emit XOR reg64, reg64
 * @param buf Code buffer
 * @param dst Destination/source register
 * @param src Source register
 */
void emit_xor_reg_reg(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit XOR reg64, imm32
 * @param buf Code buffer
 * @param dst Destination register
 * @param imm 32-bit immediate
 */
void emit_xor_reg_imm32(code_buffer_t *buf, u8 dst, u32 imm);

/**
 * Emit MVN reg64, reg64 (NOT)
 * @param buf Code buffer
 * @param dst Destination register
 * @param src Source register
 */
void emit_mvn_reg_reg(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit MUL reg64, reg64
 * @param buf Code buffer
 * @param dst Destination register (also first operand)
 * @param src Source register
 */
void emit_mul_reg(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit DIV reg64, reg64
 * @param buf Code buffer
 * @param src Source register (divisor)
 */
void emit_div_reg(code_buffer_t *buf, u8 src);

/**
 * Emit CMP reg64, reg64
 * @param buf Code buffer
 * @param op1 First operand register
 * @param op2 Second operand register
 */
void emit_cmp_reg_reg(code_buffer_t *buf, u8 op1, u8 op2);

/**
 * Emit CMP reg64, imm32
 * @param buf Code buffer
 * @param reg Register to compare
 * @param imm Immediate value
 */
void emit_cmp_reg_imm32(code_buffer_t *buf, u8 reg, u32 imm);

/**
 * Emit TEST reg64, reg64
 * @param buf Code buffer
 * @param op1 First operand register
 * @param op2 Second operand register
 */
void emit_test_reg_reg(code_buffer_t *buf, u8 op1, u8 op2);

/**
 * Emit TEST reg64, imm32
 * @param buf Code buffer
 * @param reg Register to test
 * @param imm Immediate value
 */
void emit_test_reg_imm32(code_buffer_t *buf, u8 reg, u32 imm);

/**
 * Emit LEA reg64, [reg64 + disp]
 * @param buf Code buffer
 * @param dst Destination register
 * @param base Base register
 * @param disp Displacement
 */
void emit_lea_reg_disp(code_buffer_t *buf, u8 dst, u8 base, s32 disp);

/**
 * Emit PUSH reg64
 * @param buf Code buffer
 * @param reg Register to push
 */
void emit_push_reg(code_buffer_t *buf, u8 reg);

/**
 * Emit POP reg64
 * @param buf Code buffer
 * @param reg Register to pop
 */
void emit_pop_reg(code_buffer_t *buf, u8 reg);

/* ============================================================================
 * Control Flow Instructions
 * ============================================================================ */

/**
 * Emit JMP rel32
 * @param buf Code buffer
 * @return Offset to patch
 */
u32 emit_jmp_rel32(code_buffer_t *buf);

/**
 * Emit JE rel32 (jump if equal)
 * @param buf Code buffer
 * @return Offset to patch
 */
u32 emit_je_rel32(code_buffer_t *buf);

/**
 * Emit JNE rel32 (jump if not equal)
 * @param buf Code buffer
 * @return Offset to patch
 */
u32 emit_jne_rel32(code_buffer_t *buf);

/**
 * Emit JL rel32 (jump if less than)
 * @param buf Code buffer
 * @return Offset to patch
 */
u32 emit_jl_rel32(code_buffer_t *buf);

/**
 * Emit JGE rel32 (jump if greater or equal)
 * @param buf Code buffer
 * @return Offset to patch
 */
u32 emit_jge_rel32(code_buffer_t *buf);

/**
 * Emit JLE rel32 (jump if less or equal)
 * @param buf Code buffer
 * @return Offset to patch
 */
u32 emit_jle_rel32(code_buffer_t *buf);

/**
 * Emit JG rel32 (jump if greater than)
 * @param buf Code buffer
 * @return Offset to patch
 */
u32 emit_jg_rel32(code_buffer_t *buf);

/**
 * Emit JAE rel32 (jump if above or equal)
 * @param buf Code buffer
 * @return Offset to patch
 */
u32 emit_jae_rel32(code_buffer_t *buf);

/**
 * Emit JB rel32 (jump if below)
 * @param buf Code buffer
 * @return Offset to patch
 */
u32 emit_jb_rel32(code_buffer_t *buf);

/**
 * Emit JS rel32 (jump if sign)
 * @param buf Code buffer
 * @return Offset to patch
 */
u32 emit_js_rel32(code_buffer_t *buf);

/**
 * Emit JNS rel32 (jump if not sign)
 * @param buf Code buffer
 * @return Offset to patch
 */
u32 emit_jns_rel32(code_buffer_t *buf);

/**
 * Emit JO rel32 (jump if overflow)
 * @param buf Code buffer
 * @return Offset to patch
 */
u32 emit_jo_rel32(code_buffer_t *buf);

/**
 * Emit JNO rel32 (jump if not overflow)
 * @param buf Code buffer
 * @return Offset to patch
 */
u32 emit_jno_rel32(code_buffer_t *buf);

/**
 * Emit JA rel32 (jump if above)
 * @param buf Code buffer
 * @return Offset to patch
 */
u32 emit_ja_rel32(code_buffer_t *buf);

/**
 * Emit JBE rel32 (jump if below or equal)
 * @param buf Code buffer
 * @return Offset to patch
 */
u32 emit_jbe_rel32(code_buffer_t *buf);

/**
 * Patch relative offset at given position
 * @param buf Code buffer
 * @param offset Offset to patch location
 * @param target Target address
 */
void emit_patch_rel32(code_buffer_t *buf, u32 offset, u32 target);

/**
 * Emit conditional branch based on ARM64 condition code
 * @param buf Code buffer
 * @param cond ARM64 condition code
 * @return Offset to patch (0 for always/nver)
 */
u32 emit_cond_branch(code_buffer_t *buf, arm64_cond_t cond);

/**
 * Emit CALL reg64
 * @param buf Code buffer
 * @param reg Register containing call target
 */
void emit_call_reg(code_buffer_t *buf, u8 reg);

/**
 * Emit CALL rel32
 * @param buf Code buffer
 * @return Offset to patch
 */
u32 emit_call_rel32(code_buffer_t *buf);

/**
 * Emit RET
 * @param buf Code buffer
 */
void emit_ret(code_buffer_t *buf);

/**
 * Emit NOP
 * @param buf Code buffer
 */
void emit_nop(code_buffer_t *buf);

/**
 * Emit UD2 (undefined instruction)
 * @param buf Code buffer
 */
void emit_ud2(code_buffer_t *buf);

/* ============================================================================
 * Flag Handling
 * ============================================================================ */

/**
 * Emit code to read x86_64 EFLAGS and store in ARM64 NZCV format
 * @param buf Code buffer
 * @param nzcv_reg Destination register for NZCV flags
 */
void emit_read_flags_to_nzcv(code_buffer_t *buf, u8 nzcv_reg);

/**
 * Update NZCV flags after ADD instruction
 * @param buf Code buffer
 * @param dst Destination register (contains result)
 * @param op1 First operand
 * @param op2 Second operand
 */
void emit_update_flags_add(code_buffer_t *buf, u8 dst, u8 op1, u8 op2);

/* ============================================================================
 * SSE/SSSE3 Instructions (Scalar FP)
 * ============================================================================ */

/**
 * Emit MOVSS xmm1, xmm2 (Move Scalar Single-Precision)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 */
void emit_movss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit MOVSD xmm1, xmm2 (Move Scalar Double-Precision)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 */
void emit_movsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit ADDSS xmm1, xmm2 (Add Scalar Single-Precision)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_addss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit ADDSD xmm1, xmm2 (Add Scalar Double-Precision)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_addsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit SUBSS xmm1, xmm2 (Subtract Scalar Single-Precision)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_subss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit SUBSD xmm1, xmm2 (Subtract Scalar Double-Precision)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_subsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit MULSS xmm1, xmm2 (Multiply Scalar Single-Precision)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_mulss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit MULSD xmm1, xmm2 (Multiply Scalar Double-Precision)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_mulsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit DIVSS xmm1, xmm2 (Divide Scalar Single-Precision)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_divss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit DIVSD xmm1, xmm2 (Divide Scalar Double-Precision)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_divsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit SQRTSS xmm1 (Square Root Scalar Single-Precision)
 * @param buf Code buffer
 * @param dst Destination XMM register
 */
void emit_sqrtss_xmm(code_buffer_t *buf, u8 dst);

/**
 * Emit SQRSD xmm1 (Square Root Scalar Double-Precision)
 * @param buf Code buffer
 * @param dst Destination XMM register
 */
void emit_sqrtsd_xmm(code_buffer_t *buf, u8 dst);

/**
 * Emit UCOMISS xmm1, xmm2 (Compare Scalar Single-Precision)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 */
void emit_ucomiss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit UCOMISD xmm1, xmm2 (Compare Scalar Double-Precision)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 */
void emit_ucomisd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit CVTSS2SD xmm1, xmm2 (Convert Single to Double)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 */
void emit_cvtss2sd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit CVTSD2SS xmm1, xmm2 (Convert Double to Single)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 */
void emit_cvtsd2ss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit RCPSS xmm1, xmm2 (Reciprocal Scalar Single-Precision)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 */
void emit_rcpss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit RSQRTPS xmm1, xmm2 (Reciprocal Square Root Packed Single-Precision)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 */
void emit_rsqrtps_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit XORPS xmm1, xmm2 (XOR Packed Single-Precision)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 */
void emit_xorps_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit XORPD xmm1, xmm2 (XOR Packed Double-Precision)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 */
void emit_xorpd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/* ============================================================================
 * SSE2/SSSE3 Instructions (Vector/Packed)
 * ============================================================================ */

/**
 * Emit PADDD xmm1, xmm2 (Packed Add Doublewords)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_paddd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PADDQ xmm1, xmm2 (Packed Add Quadwords)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_paddq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PSUBD xmm1, xmm2 (Packed Subtract Doublewords)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_psubd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PSUBQ xmm1, xmm2 (Packed Subtract Quadwords)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_psubq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PAND xmm1, xmm2 (Vector AND)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_pand_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit POR xmm1, xmm2 (Vector OR)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_por_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PXOR xmm1, xmm2 (Vector XOR)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_pxor_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PANDN xmm1, xmm2 (Vector AND NOT)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_pandn_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PMULUDQ xmm1, xmm2 (Multiply Packed Unsigned DW to QW)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_pmuludq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PMINUD xmm1, xmm2 (Packed Unsigned Integer Minimum)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_pminud_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PMAXUD xmm1, xmm2 (Packed Unsigned Integer Maximum)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_pmaxud_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PMINSD xmm1, xmm2 (Packed Signed Integer Minimum)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_pminsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PMAXSD xmm1, xmm2 (Packed Signed Integer Maximum)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_pmaxsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PCMPGTD xmm1, xmm2 (Compare Greater Than DW)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_pcmpgtd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PCMPEQD xmm1, xmm2 (Compare Equal DW)
 * @param buf Code buffer
 * @param dst Destination/source XMM register
 * @param src Source XMM register
 */
void emit_pcmpeqd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/* ============================================================================
 * Shift Instructions (Vector)
 * ============================================================================ */

/**
 * Emit PSLLD xmm1, imm8 (Shift Left Logical DW by immediate)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param imm Shift amount
 */
void emit_pslld_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm);

/**
 * Emit PSLLQ xmm1, imm8 (Shift Left Logical QW by immediate)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param imm Shift amount
 */
void emit_psllq_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm);

/**
 * Emit PSRLD xmm1, imm8 (Shift Right Logical DW by immediate)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param imm Shift amount
 */
void emit_psrld_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm);

/**
 * Emit PSRLQ xmm1, imm8 (Shift Right Logical QW by immediate)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param imm Shift amount
 */
void emit_psrlq_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm);

/**
 * Emit PSRAD xmm1, imm8 (Shift Right Arithmetic DW by immediate)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param imm Shift amount
 */
void emit_psrad_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm);

/**
 * Emit PSRAQ xmm1, imm8 (Shift Right Arithmetic QW by immediate)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param imm Shift amount
 */
void emit_psraq_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm);

/* ============================================================================
 * Memory Operations (RIP-relative)
 * ============================================================================ */

/**
 * Emit MOVDQU xmm1, [mem] (Move Unaligned Packed Integer Values - load)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param addr Memory address
 */
void emit_movdqu_xmm_mem(code_buffer_t *buf, u8 dst, u32 addr);

/**
 * Emit MOVDQU [mem], xmm1 (Move Unaligned Packed Integer Values - store)
 * @param buf Code buffer
 * @param addr Memory address
 * @param src Source XMM register
 */
void emit_movdqu_mem_xmm(code_buffer_t *buf, u32 addr, u8 src);

/**
 * Emit MOVUPS xmm1, [mem] (Move Unaligned Packed Single-Precision - load)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param addr Memory address
 */
void emit_movups_xmm_mem(code_buffer_t *buf, u8 dst, u32 addr);

/**
 * Emit MOVUPS [mem], xmm1 (Move Unaligned Packed Single-Precision - store)
 * @param buf Code buffer
 * @param addr Memory address
 * @param src Source XMM register
 */
void emit_movups_mem_xmm(code_buffer_t *buf, u32 addr, u8 src);

/**
 * Emit MOVAPS xmm1, [mem] (Move Aligned Packed Single-Precision - load)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param addr Memory address (should be 16-byte aligned)
 */
void emit_movaps_xmm_mem(code_buffer_t *buf, u8 dst, u32 addr);

/**
 * Emit MOVAPS [mem], xmm1 (Move Aligned Packed Single-Precision - store)
 * @param buf Code buffer
 * @param addr Memory address (should be 16-byte aligned)
 * @param src Source XMM register
 */
void emit_movaps_mem_xmm(code_buffer_t *buf, u32 addr, u8 src);

/* ============================================================================
 * Memory Operations (Register Indirect)
 * ============================================================================ */

/**
 * Emit MOVDQU xmm, [rax] (Move Unaligned - register indirect load)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param base Base register for address
 */
void emit_movdqu_xmm_mem_reg(code_buffer_t *buf, u8 dst, u8 base);

/**
 * Emit MOVDQU [rax], xmm (Move Unaligned - register indirect store)
 * @param buf Code buffer
 * @param base Base register for address
 * @param src Source XMM register
 */
void emit_movdqu_mem_reg_xmm(code_buffer_t *buf, u8 base, u8 src);

/**
 * Emit MOVUPS xmm, [rax] (Move Unaligned FP - register indirect load)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param base Base register for address
 */
void emit_movups_xmm_mem_reg(code_buffer_t *buf, u8 dst, u8 base);

/**
 * Emit MOVUPS [rax], xmm (Move Unaligned FP - register indirect store)
 * @param buf Code buffer
 * @param base Base register for address
 * @param src Source XMM register
 */
void emit_movups_mem_reg_xmm(code_buffer_t *buf, u8 base, u8 src);

/**
 * Emit MOVAPS xmm, [rax] (Move Aligned FP - register indirect load)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param base Base register for address
 */
void emit_movaps_xmm_mem_reg(code_buffer_t *buf, u8 dst, u8 base);

/**
 * Emit MOVAPS [rax], xmm (Move Aligned FP - register indirect store)
 * @param buf Code buffer
 * @param base Base register for address
 * @param src Source XMM register
 */
void emit_movaps_mem_reg_xmm(code_buffer_t *buf, u8 base, u8 src);

/* ============================================================================
 * NEON/SSSE3 Shuffle and Permutation
 * ============================================================================ */

/**
 * Emit PUNPCKLBW xmm1, xmm2 (Unpack Low Bytes)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 */
void emit_punpcklbw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PUNPCKHBW xmm1, xmm2 (Unpack High Bytes)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 */
void emit_punpckhbw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PUNPCKLWD xmm1, xmm2 (Unpack Low Words)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 */
void emit_punpcklwd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PUNPCKHWD xmm1, xmm2 (Unpack High Words)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 */
void emit_punpckhwd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PUNPCKLDQ xmm1, xmm2 (Unpack Low Doublewords)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 */
void emit_punpckldq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PUNPCKHDQ xmm1, xmm2 (Unpack High Doublewords)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 */
void emit_punpckhdq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PALIGNR xmm1, xmm2, imm8 (Packed Align Right)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 * @param imm Align immediate
 */
void emit_palignr_xmm_xmm_imm(code_buffer_t *buf, u8 dst, u8 src, u8 imm);

/**
 * Emit PSHUFB xmm1, xmm2 (Packed Shuffle Bytes)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register (control)
 */
void emit_pshufb_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PSIGNB xmm1, xmm2 (Packed Sign Byte)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 */
void emit_psignb_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PSIGNW xmm1, xmm2 (Packed Sign Word)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 */
void emit_psignw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PSIGND xmm1, xmm2 (Packed Sign Doubleword)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source XMM register
 */
void emit_psignd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * Emit PEXTRB r32, xmm, imm8 (Extract Byte)
 * @param buf Code buffer
 * @param dst Destination GPR
 * @param src Source XMM register
 * @param imm Index
 */
void emit_pextrb_reg_xmm_imm(code_buffer_t *buf, u8 dst, u8 src, u8 imm);

/**
 * Emit PINSRB xmm, r32, imm8 (Insert Byte)
 * @param buf Code buffer
 * @param dst Destination XMM register
 * @param src Source GPR
 * @param imm Index
 */
void emit_pinsrb_xmm_reg_imm(code_buffer_t *buf, u8 dst, u8 src, u8 imm);

#endif /* ROSETTA_CODEGEN_H */
