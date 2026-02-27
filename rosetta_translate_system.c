/* ============================================================================
 * Rosetta Translator - System Instruction Translation Implementation
 * ============================================================================
 *
 * This module implements translation of ARM64 system instructions to x86_64.
 * Supported instructions: SVC, BRK, HLT, MRS, MSR
 * ============================================================================ */

#include "rosetta_translate_system.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h>

/* External register mapping function from ALU module */
extern uint8_t translate_get_x86_reg(uint8_t arm_reg);

/**
 * translate_emulate_syscall - Emulate a system call
 * @syscall_num: System call number
 * @args: Array of 6 arguments
 * @result: Pointer to store result
 * Returns: 0 on success, -1 on error
 */
int translate_emulate_syscall(uint64_t syscall_num, uint64_t *args, uint64_t *result)
{
    long ret;

    switch (syscall_num) {
        case SYS_EXIT:
            _exit((int)args[0]);
            /* Never returns */
            break;

        case SYS_WRITE: {
            /* write(fd, buf, count) */
            ret = write((int)args[0], (const void *)args[1], (size_t)args[2]);
            *result = (ret < 0) ? (uint64_t)-1 : (uint64_t)ret;
            break;
        }

        case SYS_READ: {
            /* read(fd, buf, count) */
            ret = read((int)args[0], (void *)args[1], (size_t)args[2]);
            *result = (ret < 0) ? (uint64_t)-1 : (uint64_t)ret;
            break;
        }

        case SYS_MMAP: {
            /* mmap(addr, length, prot, flags, fd, offset) */
            void *addr = (void *)args[0];
            size_t length = args[1];
            int prot = (int)args[2];
            int flags = (int)args[3];
            int fd = (int)args[4];
            off_t offset = (off_t)args[5];

            ret = (long)syscall(SYS_mmap, addr, length, prot, flags, fd, offset);
            *result = (ret < 0) ? (uint64_t)-1 : (uint64_t)ret;
            break;
        }

        case SYS_MUNMAP: {
            /* munmap(addr, length) */
            ret = munmap((void *)args[0], (size_t)args[1]);
            *result = (ret < 0) ? (uint64_t)-1 : 0;
            break;
        }

        default:
            fprintf(stderr, "Unknown syscall: %llu\n", (unsigned long long)syscall_num);
            *result = (uint64_t)-1;
            return -1;
    }

    return 0;
}

/**
 * translate_system_svc - Translate SVC (supervisor call) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_system_svc(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint16_t imm16 = (encoding >> 5) & 0xFFFF;
    (void)imm16;  /* SVC immediate usually contains syscall number hint */

    /* On ARM64 Linux, syscall number is in x8, args in x0-x5 */
    uint64_t syscall_num = guest_state[8];
    uint64_t args[6];
    uint64_t result;
    int i;

    for (i = 0; i < 6; i++) {
        args[i] = guest_state[i];
    }

    /* Emulate the syscall */
    if (translate_emulate_syscall(syscall_num, args, &result) == 0) {
        /* Set return value in x0 */
        guest_state[0] = result;
    }

    /* Emit x86_64 code - for now just NOP */
    /* TODO: Emit proper syscall emulation code */
    emit_x86_nop(code_buf);

    return 0;
}

/**
 * translate_system_brk - Translate BRK (breakpoint) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_system_brk(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint16_t imm16 = (encoding >> 5) & 0xFFFF;

    fprintf(stderr, "BRK instruction encountered (imm16=%u)\n", imm16);

    /* Emit INT3 for debugging */
    emit_x86_int3(code_buf);

    return 0;
}

/**
 * translate_system_hlt - Translate HLT (halt) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_system_hlt(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    (void)encoding;
    (void)guest_state;

    fprintf(stderr, "HLT instruction encountered - halting emulation\n");

    /* Emit INT3 to stop execution */
    emit_x86_int3(code_buf);

    return 0;
}

/**
 * translate_system_mrs - Translate MRS (move from system register) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 *
 * MRS Rd, sysreg - Read system register to general-purpose register
 *
 * Common system registers:
 * - MDCCSR_EL0 (Debug Status)
 * - FPCR (Floating-point Control Register)
 * - FPSR (Floating-point Status Register)
 * - NZCV (Condition Flags)
 * - CTR_EL0 (Cache Type Register)
 * - PMCCNTR_EL0 (Performance Counter)
 */
int translate_system_mrs(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint16_t op1 = (encoding >> 16) & 0x7;
    uint16_t crn = (encoding >> 12) & 0xF;
    uint16_t crm = (encoding >> 1) & 0xF;
    uint8_t op2 = (encoding >> 14) & 0x7;

    /* Encode the system register identifier */
    uint16_t sysreg = (op1 << 12) | (crn << 8) | (crm << 4) | op2;

    /* Handle common system registers */
    switch (sysreg) {
        case 0x3000:  /* FPCR - Floating Point Control Register */
            guest_state[rd] = 0;  /* Default FPCR value */
            break;

        case 0x3001:  /* FPSR - Floating Point Status Register */
            guest_state[rd] = 0;  /* Default FPSR value */
            break;

        case 0x3200:  /* NZCV - Condition Flags */
            guest_state[rd] = guest_state[32];  /* Assume flags stored in x32 */
            break;

        case 0x3300:  /* CTR_EL0 - Cache Type Register */
            /* Return typical cache line size info */
            guest_state[rd] = 0x8000001A;  /* Typical ARM64 cache config */
            break;

        case 0x3320:  /* DCZID_EL0 - Data Cache Zero ID Register */
            guest_state[rd] = 0;  /* DC ZVA supported, block size 0 */
            break;

        case 0x3401:  /* PMCCNTR_EL0 - Performance Monitors Cycle Counter */
            guest_state[rd] = 0;  /* Return 0 for performance counter */
            break;

        case 0x4000:  /* MDCCSR_EL0 - Debug Core Status Register */
            guest_state[rd] = 0;
            break;

        case 0x5000:  /* TPIDR_EL0 - Thread Private ID Register */
            /* Thread-local storage base - return 0 for now */
            guest_state[rd] = 0;
            break;

        case 0x5020:  /* TPIDRRO_EL0 - Thread Private ID Read-Only */
            guest_state[rd] = 0;
            break;

        default:
            /* Unknown system register - return 0 */
            fprintf(stderr, "MRS: Unknown system register op1=%d crn=%d crm=%d op2=%d\n",
                    op1, crn, crm, op2);
            guest_state[rd] = 0;
            break;
    }

    /* Emit x86_64 code - set register to the value */
    emit_x86_mov_reg_imm64(code_buf, translate_get_x86_reg(rd), guest_state[rd]);

    return 0;
}

/**
 * translate_system_msr - Translate MSR (move to system register) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 *
 * MSR sysreg, Rn - Write general-purpose register to system register
 *
 * Common system registers:
 * - FPCR (Floating-point Control Register)
 * - FPSR (Floating-point Status Register)
 * - NZCV (Condition Flags)
 * - PMCCNTR_EL0 (Performance Counter)
 * - TPIDR_EL0 (Thread Private ID Register)
 */
int translate_system_msr(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint16_t op1 = (encoding >> 16) & 0x7;
    uint16_t crn = (encoding >> 12) & 0xF;
    uint16_t crm = (encoding >> 1) & 0xF;
    uint8_t op2 = (encoding >> 14) & 0x7;

    /* Encode the system register identifier */
    uint16_t sysreg = (op1 << 12) | (crn << 8) | (crm << 4) | op2;

    /* Get the value to write */
    uint64_t value = guest_state[rn];

    /* Handle common system registers */
    switch (sysreg) {
        case 0x3000:  /* FPCR - Floating Point Control Register */
            /* Store FPCR value in guest state */
            guest_state[33] = value;  /* Assume FPCR stored at index 33 */
            break;

        case 0x3001:  /* FPSR - Floating Point Status Register */
            /* Store FPSR value in guest state */
            guest_state[34] = value;  /* Assume FPSR stored at index 34 */
            break;

        case 0x3200:  /* NZCV - Condition Flags */
            /* Update condition flags in guest state */
            guest_state[32] = value;  /* Assume flags stored at index 32 */
            break;

        case 0x3320:  /* DCZID_EL0 - Data Cache Zero ID Register */
            /* Read-only register - ignore write */
            break;

        case 0x3401:  /* PMCCNTR_EL0 - Performance Monitors Cycle Counter */
            /* Store performance counter value */
            guest_state[35] = value;  /* Assume PMCCNTR stored at index 35 */
            break;

        case 0x5000:  /* TPIDR_EL0 - Thread Private ID Register */
            /* Thread-local storage base */
            guest_state[36] = value;  /* Assume TPIDR stored at index 36 */
            break;

        case 0x5020:  /* TPIDRRO_EL0 - Thread Private ID Read-Only */
            /* Read-only - ignore write */
            break;

        default:
            /* Unknown system register - silently ignore */
            fprintf(stderr, "MSR: Unknown system register op1=%d crn=%d crm=%d op2=%d (val=0x%llx)\n",
                    op1, crn, crm, op2, (unsigned long long)value);
            break;
    }

    /* Emit x86_64 code - NOP since system register is emulated */
    emit_x86_nop(code_buf);

    (void)rn; (void)value;  /* Suppress unused warnings */

    return 0;
}

/**
 * translate_system_dispatch - Dispatch system instruction based on encoding
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 if instruction handled, -1 otherwise
 */
int translate_system_dispatch(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    if ((encoding & SVC_MASK) == SVC_VAL) {
        return translate_system_svc(encoding, code_buf, guest_state);
    } else if ((encoding & BRK_MASK) == BRK_VAL) {
        return translate_system_brk(encoding, code_buf, guest_state);
    } else if ((encoding & HLT_MASK) == HLT_VAL) {
        return translate_system_hlt(encoding, code_buf, guest_state);
    } else if ((encoding & MRS_MASK) == MRS_VAL) {
        return translate_system_mrs(encoding, code_buf, guest_state);
    } else if ((encoding & MSR_MASK) == MSR_VAL) {
        return translate_system_msr(encoding, code_buf, guest_state);
    }

    return -1;  /* Not a system instruction */
}
