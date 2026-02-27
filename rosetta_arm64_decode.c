/* ============================================================================
 * Rosetta Translator - ARM64 Instruction Decoder Implementation
 * ============================================================================
 *
 * This module implements comprehensive ARM64 instruction decoding functions
 * for the Rosetta binary translation layer.
 * ============================================================================ */

#include "rosetta_types.h"
#include <stdio.h>
#include <string.h>

/* Forward declarations for helper functions */
static const char *get_condition_name(uint8_t cond);

/* Instruction classes */
typedef enum {
    ARM64_INS_UNKNOWN = 0,
    ARM64_INS_ALU,
    ARM64_INS_BRANCH,
    ARM64_INS_COMPARE,
    ARM64_INS_MOV,
    ARM64_INS_LOAD,
    ARM64_INS_STORE,
    ARM64_INS_SYSTEM,
    ARM64_INS_FP,
    ARM64_INS_SIMD
} arm64_insn_class_t;

/* ALU instruction types */
typedef enum {
    ARM64_ALU_ADD,
    ARM64_ALU_SUB,
    ARM64_ALU_AND,
    ARM64_ALU_ORR,
    ARM64_ALU_EOR,
    ARM64_ALU_MVN,
    ARM64_ALU_MUL
} arm64_alu_type_t;

/* Branch instruction types */
typedef enum {
    ARM64_BRANCH_UNCOND,
    ARM64_BRANCH_LINK,
    ARM64_BRANCH_REG,
    ARM64_BRANCH_RET,
    ARM64_BRANCH_COND,
    ARM64_BRANCH_CBZ,
    ARM64_BRANCH_CBNZ,
    ARM64_BRANCH_TBZ,
    ARM64_BRANCH_TBNZ
} arm64_branch_type_t;

/* Compare instruction types */
typedef enum {
    ARM64_CMP_CMP,
    ARM64_CMP_CMN,
    ARM64_CMP_TST
} arm64_compare_type_t;

/* MOV instruction types */
typedef enum {
    ARM64_MOV_MOVZ,
    ARM64_MOV_MOVK,
    ARM64_MOV_MOVN
} arm64_mov_type_t;

/* Load/Store instruction types */
typedef enum {
    ARM64_LDR_IMM,
    ARM64_LDR_REG,
    ARM64_STR_IMM,
    ARM64_STR_REG,
    ARM64_LDP,
    ARM64_STP
} arm64_mem_type_t;

/* System instruction types */
typedef enum {
    ARM64_SYS_SVC,
    ARM64_SYS_BRK,
    ARM64_SYS_HLT
} arm64_system_type_t;

/* FP instruction types */
typedef enum {
    ARM64_FP_FADD,
    ARM64_FP_FSUB,
    ARM64_FP_FMUL,
    ARM64_FP_FDIV,
    ARM64_FP_FSQRT,
    ARM64_FP_FABS,
    ARM64_FP_FNEG,
    ARM64_FP_FCSEL,
    ARM64_FCMP
} arm64_fp_type_t;

/* SIMD instruction types */
typedef enum {
    ARM64_SIMD_ADD,
    ARM64_SIMD_SUB,
    ARM64_SIMD_AND,
    ARM64_SIMD_ORR,
    ARM64_SIMD_EOR
} arm64_simd_type_t;

/* Decoded instruction structure */
typedef struct {
    uint32_t encoding;
    arm64_insn_class_t class;
    union {
        arm64_alu_type_t alu;
        arm64_branch_type_t branch;
        arm64_compare_type_t compare;
        arm64_mov_type_t mov;
        arm64_mem_type_t mem;
        arm64_system_type_t system;
        arm64_fp_type_t fp;
        arm64_simd_type_t simd;
        uint32_t type;
    };
    uint8_t rd, rn, rm, rt2;
    uint8_t vd, vn, vm;
    uint8_t cond;
    uint8_t access_size;
    uint8_t shift_type;
    uint8_t shift_amount;
    uint8_t q;
    uint8_t test_bit;
    int32_t imm;
    uint8_t insn_size;
    uint8_t shift;  /* Shift for MOV instructions */
} arm64_insn_t;

/* Include inline helpers directly to avoid redefinition issues */
#include "rosetta_arm64_decode_helpers.h"

/* ============================================================================
 * Instruction Classification
 * ============================================================================ */

/**
 * arm64_decode_instruction - Decode ARM64 instruction
 * @encoding: ARM64 instruction encoding
 * @decoded: Pointer to decoded instruction structure
 * Returns: 0 on success, -1 on unknown instruction
 */
int arm64_decode_instruction(uint32_t encoding, arm64_insn_t *decoded)
{
    if (!decoded) return -1;

    memset(decoded, 0, sizeof(*decoded));
    decoded->encoding = encoding;
    decoded->insn_size = 4;  /* All ARM64 instructions are 4 bytes */

    /* Decode common fields */
    decoded->rd = arm64_get_rd(encoding);
    decoded->rn = arm64_get_rn(encoding);
    decoded->rm = arm64_get_rm(encoding);
    decoded->cond = arm64_get_cond(encoding);

    /* Classify instruction */
    if (arm64_is_b(encoding)) {
        decoded->class = ARM64_INS_BRANCH;
        decoded->type = ARM64_BRANCH_UNCOND;
        decoded->imm = arm64_get_imm26(encoding);
        return 0;
    }

    if (arm64_is_bl(encoding)) {
        decoded->class = ARM64_INS_BRANCH;
        decoded->type = ARM64_BRANCH_LINK;
        decoded->imm = arm64_get_imm26(encoding);
        return 0;
    }

    if (arm64_is_br(encoding)) {
        decoded->class = ARM64_INS_BRANCH;
        decoded->type = ARM64_BRANCH_REG;
        return 0;
    }

    if (arm64_is_ret(encoding)) {
        decoded->class = ARM64_INS_BRANCH;
        decoded->type = ARM64_BRANCH_RET;
        return 0;
    }

    if (arm64_is_bcond(encoding)) {
        decoded->class = ARM64_INS_BRANCH;
        decoded->type = ARM64_BRANCH_COND;
        decoded->imm = arm64_get_imm19(encoding);
        return 0;
    }

    if (arm64_is_cbz(encoding)) {
        decoded->class = ARM64_INS_BRANCH;
        decoded->type = ARM64_BRANCH_CBZ;
        decoded->imm = arm64_get_imm14(encoding);
        return 0;
    }

    if (arm64_is_cbnz(encoding)) {
        decoded->class = ARM64_INS_BRANCH;
        decoded->type = ARM64_BRANCH_CBNZ;
        decoded->imm = arm64_get_imm14(encoding);
        return 0;
    }

    if (arm64_is_tbz(encoding)) {
        decoded->class = ARM64_INS_BRANCH;
        decoded->type = ARM64_BRANCH_TBZ;
        decoded->imm = arm64_get_imm7(encoding);
        decoded->test_bit = arm64_get_test_bit(encoding);
        return 0;
    }

    if (arm64_is_tbnz(encoding)) {
        decoded->class = ARM64_INS_BRANCH;
        decoded->type = ARM64_BRANCH_TBNZ;
        decoded->imm = arm64_get_imm7(encoding);
        decoded->test_bit = arm64_get_test_bit(encoding);
        return 0;
    }

    /* ALU instructions */
    if (arm64_is_add(encoding)) {
        decoded->class = ARM64_INS_ALU;
        decoded->type = ARM64_ALU_ADD;
        decoded->shift_type = (encoding >> 22) & 0x3;
        decoded->shift_amount = (encoding >> 10) & 0x3F;
        return 0;
    }

    if (arm64_is_sub(encoding)) {
        decoded->class = ARM64_INS_ALU;
        decoded->type = ARM64_ALU_SUB;
        decoded->shift_type = (encoding >> 22) & 0x3;
        decoded->shift_amount = (encoding >> 10) & 0x3F;
        return 0;
    }

    if (arm64_is_and(encoding)) {
        decoded->class = ARM64_INS_ALU;
        decoded->type = ARM64_ALU_AND;
        decoded->shift_type = (encoding >> 22) & 0x3;
        decoded->shift_amount = (encoding >> 10) & 0x3F;
        return 0;
    }

    if (arm64_is_orr(encoding)) {
        decoded->class = ARM64_INS_ALU;
        decoded->type = ARM64_ALU_ORR;
        decoded->shift_type = (encoding >> 22) & 0x3;
        decoded->shift_amount = (encoding >> 10) & 0x3F;
        return 0;
    }

    if (arm64_is_eor(encoding)) {
        decoded->class = ARM64_INS_ALU;
        decoded->type = ARM64_ALU_EOR;
        decoded->shift_type = (encoding >> 22) & 0x3;
        decoded->shift_amount = (encoding >> 10) & 0x3F;
        return 0;
    }

    if (arm64_is_mvn(encoding)) {
        decoded->class = ARM64_INS_ALU;
        decoded->type = ARM64_ALU_MVN;
        return 0;
    }

    if (arm64_is_mul(encoding)) {
        decoded->class = ARM64_INS_ALU;
        decoded->type = ARM64_ALU_MUL;
        return 0;
    }

    /* Compare instructions */
    if (arm64_is_cmp(encoding)) {
        decoded->class = ARM64_INS_COMPARE;
        decoded->type = ARM64_CMP_CMP;
        return 0;
    }

    if (arm64_is_cmn(encoding)) {
        decoded->class = ARM64_INS_COMPARE;
        decoded->type = ARM64_CMP_CMN;
        return 0;
    }

    if (arm64_is_tst(encoding)) {
        decoded->class = ARM64_INS_COMPARE;
        decoded->type = ARM64_CMP_TST;
        return 0;
    }

    /* MOV wide instructions */
    if (arm64_is_movz(encoding)) {
        decoded->class = ARM64_INS_MOV;
        decoded->type = ARM64_MOV_MOVZ;
        decoded->imm = arm64_get_imm16(encoding);
        decoded->shift = arm64_get_hw(encoding) * 16;
        return 0;
    }

    if (arm64_is_movk(encoding)) {
        decoded->class = ARM64_INS_MOV;
        decoded->type = ARM64_MOV_MOVK;
        decoded->imm = arm64_get_imm16(encoding);
        decoded->shift = arm64_get_hw(encoding) * 16;
        return 0;
    }

    if (arm64_is_movn(encoding)) {
        decoded->class = ARM64_INS_MOV;
        decoded->type = ARM64_MOV_MOVN;
        decoded->imm = arm64_get_imm16(encoding);
        decoded->shift = arm64_get_hw(encoding) * 16;
        return 0;
    }

    /* Load/Store instructions */
    if (arm64_is_ldr(encoding)) {
        decoded->class = ARM64_INS_LOAD;
        decoded->type = ARM64_LDR_IMM;
        decoded->imm = arm64_get_imm12(encoding);
        decoded->access_size = (encoding >> 30) & 0x3;
        return 0;
    }

    if (arm64_is_str(encoding)) {
        decoded->class = ARM64_INS_STORE;
        decoded->type = ARM64_STR_IMM;
        decoded->imm = arm64_get_imm12(encoding);
        decoded->access_size = (encoding >> 30) & 0x3;
        return 0;
    }

    if (arm64_is_ldp(encoding)) {
        decoded->class = ARM64_INS_LOAD;
        decoded->type = ARM64_LDP;
        decoded->rt2 = (encoding >> 10) & 0x1F;
        decoded->imm = ((encoding >> 15) & 0x7F) << 3;
        return 0;
    }

    if (arm64_is_stp(encoding)) {
        decoded->class = ARM64_INS_STORE;
        decoded->type = ARM64_STP;
        decoded->rt2 = (encoding >> 10) & 0x1F;
        decoded->imm = ((encoding >> 15) & 0x7F) << 3;
        return 0;
    }

    /* System instructions */
    if (arm64_is_svc(encoding)) {
        decoded->class = ARM64_INS_SYSTEM;
        decoded->type = ARM64_SYS_SVC;
        decoded->imm = encoding & 0xFFFF;
        return 0;
    }

    if (arm64_is_brk(encoding)) {
        decoded->class = ARM64_INS_SYSTEM;
        decoded->type = ARM64_SYS_BRK;
        decoded->imm = (encoding >> 5) & 0xFFFF;
        return 0;
    }

    /* FP instructions */
    if (arm64_is_fp_insn(encoding)) {
        decoded->class = ARM64_INS_FP;
        decoded->vd = arm64_get_vd(encoding);
        decoded->vn = arm64_get_vn(encoding);
        decoded->vm = arm64_get_vm(encoding);

        if (arm64_is_fadd(encoding)) {
            decoded->type = ARM64_FP_FADD;
            return 0;
        }
        if (arm64_is_fsub(encoding)) {
            decoded->type = ARM64_FP_FSUB;
            return 0;
        }
        if (arm64_is_fmul(encoding)) {
            decoded->type = ARM64_FP_FMUL;
            return 0;
        }
        if (arm64_is_fdiv(encoding)) {
            decoded->type = ARM64_FP_FDIV;
            return 0;
        }
        if (arm64_is_fsqrt(encoding)) {
            decoded->type = ARM64_FP_FSQRT;
            return 0;
        }
        if (arm64_is_fcmp(encoding)) {
            decoded->type = ARM64_FCMP;
            return 0;
        }
        if (arm64_is_fcsel(encoding)) {
            decoded->type = ARM64_FP_FCSEL;
            return 0;
        }
        if (arm64_is_fabs(encoding)) {
            decoded->type = ARM64_FP_FABS;
            return 0;
        }
        if (arm64_is_fneg(encoding)) {
            decoded->type = ARM64_FP_FNEG;
            return 0;
        }
    }

    /* SIMD/NEON instructions */
    if (arm64_is_add_vec(encoding)) {
        decoded->class = ARM64_INS_SIMD;
        decoded->type = ARM64_SIMD_ADD;
        decoded->vd = arm64_get_vd(encoding);
        decoded->vn = arm64_get_vn(encoding);
        decoded->vm = arm64_get_vm(encoding);
        decoded->q = arm64_get_q_bit(encoding);
        return 0;
    }

    if (arm64_is_sub_vec(encoding)) {
        decoded->class = ARM64_INS_SIMD;
        decoded->type = ARM64_SIMD_SUB;
        decoded->vd = arm64_get_vd(encoding);
        decoded->vn = arm64_get_vn(encoding);
        decoded->vm = arm64_get_vm(encoding);
        decoded->q = arm64_get_q_bit(encoding);
        return 0;
    }

    if (arm64_is_and_vec(encoding)) {
        decoded->class = ARM64_INS_SIMD;
        decoded->type = ARM64_SIMD_AND;
        decoded->vd = arm64_get_vd(encoding);
        decoded->vn = arm64_get_vn(encoding);
        decoded->vm = arm64_get_vm(encoding);
        return 0;
    }

    if (arm64_is_orr_vec(encoding)) {
        decoded->class = ARM64_INS_SIMD;
        decoded->type = ARM64_SIMD_ORR;
        decoded->vd = arm64_get_vd(encoding);
        decoded->vn = arm64_get_vn(encoding);
        decoded->vm = arm64_get_vm(encoding);
        return 0;
    }

    if (arm64_is_eor_vec(encoding)) {
        decoded->class = ARM64_INS_SIMD;
        decoded->type = ARM64_SIMD_EOR;
        decoded->vd = arm64_get_vd(encoding);
        decoded->vn = arm64_get_vn(encoding);
        decoded->vm = arm64_get_vm(encoding);
        return 0;
    }

    /* Unknown instruction */
    decoded->class = ARM64_INS_UNKNOWN;
    return -1;
}

/**
 * arm64_get_instruction_name - Get human-readable instruction name
 * @decoded: Decoded instruction
 * Returns: Instruction name string
 */
const char *arm64_get_instruction_name(const arm64_insn_t *decoded)
{
    if (!decoded) return "unknown";

    switch (decoded->class) {
        case ARM64_INS_ALU:
            switch (decoded->type) {
                case ARM64_ALU_ADD: return "ADD";
                case ARM64_ALU_SUB: return "SUB";
                case ARM64_ALU_AND: return "AND";
                case ARM64_ALU_ORR: return "ORR";
                case ARM64_ALU_EOR: return "EOR";
                case ARM64_ALU_MVN: return "MVN";
                case ARM64_ALU_MUL: return "MUL";
                default: return "ALU";
            }
        case ARM64_INS_BRANCH:
            switch (decoded->type) {
                case ARM64_BRANCH_UNCOND: return "B";
                case ARM64_BRANCH_LINK: return "BL";
                case ARM64_BRANCH_REG: return "BR";
                case ARM64_BRANCH_RET: return "RET";
                case ARM64_BRANCH_COND: return "B.cond";
                case ARM64_BRANCH_CBZ: return "CBZ";
                case ARM64_BRANCH_CBNZ: return "CBNZ";
                case ARM64_BRANCH_TBZ: return "TBZ";
                case ARM64_BRANCH_TBNZ: return "TBNZ";
                default: return "BRANCH";
            }
        case ARM64_INS_COMPARE:
            switch (decoded->type) {
                case ARM64_CMP_CMP: return "CMP";
                case ARM64_CMP_CMN: return "CMN";
                case ARM64_CMP_TST: return "TST";
                default: return "COMPARE";
            }
        case ARM64_INS_MOV:
            switch (decoded->type) {
                case ARM64_MOV_MOVZ: return "MOVZ";
                case ARM64_MOV_MOVK: return "MOVK";
                case ARM64_MOV_MOVN: return "MOVN";
                default: return "MOV";
            }
        case ARM64_INS_LOAD:
            switch (decoded->type) {
                case ARM64_LDR_IMM: return "LDR";
                case ARM64_LDP: return "LDP";
                default: return "LOAD";
            }
        case ARM64_INS_STORE:
            switch (decoded->type) {
                case ARM64_STR_IMM: return "STR";
                case ARM64_STP: return "STP";
                default: return "STORE";
            }
        case ARM64_INS_SYSTEM:
            switch (decoded->type) {
                case ARM64_SYS_SVC: return "SVC";
                case ARM64_SYS_BRK: return "BRK";
                default: return "SYSTEM";
            }
        case ARM64_INS_FP:
            switch (decoded->type) {
                case ARM64_FP_FADD: return "FADD";
                case ARM64_FP_FSUB: return "FSUB";
                case ARM64_FP_FMUL: return "FMUL";
                case ARM64_FP_FDIV: return "FDIV";
                case ARM64_FP_FSQRT: return "FSQRT";
                case ARM64_FP_FABS: return "FABS";
                case ARM64_FP_FNEG: return "FNEG";
                case ARM64_FP_FCSEL: return "FCSEL";
                case ARM64_FCMP: return "FCMP";
                default: return "FP";
            }
        case ARM64_INS_SIMD:
            switch (decoded->type) {
                case ARM64_SIMD_ADD: return "ADD (vector)";
                case ARM64_SIMD_SUB: return "SUB (vector)";
                case ARM64_SIMD_AND: return "AND (vector)";
                case ARM64_SIMD_ORR: return "ORR (vector)";
                case ARM64_SIMD_EOR: return "EOR (vector)";
                default: return "SIMD";
            }
        default:
            return "UNKNOWN";
    }
}

/**
 * arm64_format_instruction - Format decoded instruction as string
 * @decoded: Decoded instruction
 * @buf: Output buffer
 * @buf_size: Buffer size
 * Returns: 0 on success
 */
int arm64_format_instruction(const arm64_insn_t *decoded, char *buf, size_t buf_size)
{
    if (!decoded || !buf || buf_size == 0) return -1;

    const char *name = arm64_get_instruction_name(decoded);
    int written = 0;

    switch (decoded->class) {
        case ARM64_INS_ALU:
        case ARM64_INS_COMPARE:
            written = snprintf(buf, buf_size, "%s W%d, W%d, W%d",
                              name, decoded->rd, decoded->rn, decoded->rm);
            break;
        case ARM64_INS_BRANCH:
            switch (decoded->type) {
                case ARM64_BRANCH_UNCOND:
                case ARM64_BRANCH_LINK:
                    written = snprintf(buf, buf_size, "%s #0x%x", name, decoded->imm);
                    break;
                case ARM64_BRANCH_REG:
                case ARM64_BRANCH_RET:
                    written = snprintf(buf, buf_size, "%s X%d", name, decoded->rd);
                    break;
                case ARM64_BRANCH_COND:
                    written = snprintf(buf, buf_size, "%s.%s #0x%x", "B",
                                      get_condition_name(decoded->cond), decoded->imm);
                    break;
                case ARM64_BRANCH_CBZ:
                case ARM64_BRANCH_CBNZ:
                    written = snprintf(buf, buf_size, "%s W%d, #0x%x", name, decoded->rd, decoded->imm);
                    break;
                default:
                    written = snprintf(buf, buf_size, "%s", name);
            }
            break;
        case ARM64_INS_MOV:
            written = snprintf(buf, buf_size, "%s X%d, #0x%x, LSL #%d",
                              name, decoded->rd, decoded->imm, decoded->shift);
            break;
        case ARM64_INS_LOAD:
        case ARM64_INS_STORE:
            written = snprintf(buf, buf_size, "%s X%d, [X%d, #0x%x]",
                              name, decoded->rd, decoded->rn, decoded->imm);
            break;
        case ARM64_INS_SYSTEM:
            written = snprintf(buf, buf_size, "%s #0x%x", name, decoded->imm);
            break;
        default:
            written = snprintf(buf, buf_size, "%s 0x%08x", name, decoded->encoding);
    }

    if (written < 0 || (size_t)written >= buf_size) {
        buf[buf_size - 1] = '\0';
    }

    return 0;
}

/**
 * get_condition_name - Get condition name string
 * @cond: Condition code (0-15)
 * Returns: Condition name string
 */
const char *get_condition_name(uint8_t cond)
{
    static const char *const cond_names[16] = {
        "eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
        "hi", "ls", "ge", "lt", "gt", "le", "al", "nv"
    };
    return (cond < 16) ? cond_names[cond] : "??";
}

/**
 * arm64_is_load_store - Check if instruction is load/store
 * @encoding: ARM64 instruction encoding
 * Returns: 1 if load/store, 0 otherwise
 */
int arm64_is_load_store(uint32_t encoding)
{
    return arm64_is_ldr(encoding) ||
           arm64_is_str(encoding) ||
           arm64_is_ldp(encoding) ||
           arm64_is_stp(encoding) ||
           arm64_is_ldrb(encoding) ||
           arm64_is_strb(encoding) ||
           arm64_is_ldrh(encoding) ||
           arm64_is_strh(encoding) ||
           arm64_is_ldrsb(encoding) ||
           arm64_is_ldrsh(encoding) ||
           arm64_is_ldrsw(encoding);
}

/**
 * arm64_get_load_store_size - Get load/store access size
 * @encoding: ARM64 instruction encoding
 * Returns: Size in bytes (1, 2, 4, or 8)
 */
int arm64_get_load_store_size(uint32_t encoding)
{
    uint8_t size = (encoding >> 30) & 0x3;
    switch (size) {
        case 0: return 1;
        case 1: return 2;
        case 2: return 4;
        case 3: return 8;
        default: return 4;
    }
}

/**
 * arm64_compute_branch_target - Compute branch target address
 * @encoding: ARM64 instruction encoding
 * @pc: Current PC
 * Returns: Branch target address
 */
uint64_t arm64_compute_branch_target(uint32_t encoding, uint64_t pc)
{
    int32_t imm = 0;

    if (arm64_is_b(encoding) || arm64_is_bl(encoding)) {
        imm = arm64_get_imm26(encoding);
        return pc + imm;
    }

    if (arm64_is_bcond(encoding) ||
        arm64_is_cbz(encoding) ||
        arm64_is_cbnz(encoding)) {
        imm = arm64_get_imm19(encoding);
        return pc + imm;
    }

    if (arm64_is_tbz(encoding) || arm64_is_tbnz(encoding)) {
        imm = arm64_get_imm7(encoding);
        return pc + imm;
    }

    return 0;  /* Register-indirect branch - can't compute statically */
}
