/* ============================================================================
 * Rosetta Refactored - Exception Handling Implementation
 * ============================================================================
 *
 * This module implements exception handling functions for the Rosetta
 * translation layer.
 * ============================================================================ */

#include "rosetta_refactored_exception.h"
#include "rosetta_refactored_reg.h"
#include "rosetta_refactored_syscall.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * Exception Handler Table
 * ============================================================================ */

#define MAX_EXCEPTION_HANDLERS 16

static ros_exception_handler_t g_exception_handlers[MAX_EXCEPTION_HANDLERS];
static bool g_exception_initialized = false;

/* Current exception info */
static ros_exception_info_t g_current_exception;
static bool g_exception_pending = false;

/* Default fault handler */
static ros_exception_handler_t g_fault_handler = NULL;

/* ============================================================================
 * Exception Initialization
 * ============================================================================ */

/**
 * rosetta_exception_init - Initialize exception handling
 */
int rosetta_exception_init(void)
{
    if (g_exception_initialized) {
        rosetta_exception_cleanup();
    }

    memset(g_exception_handlers, 0, sizeof(g_exception_handlers));
    memset(&g_current_exception, 0, sizeof(g_current_exception));
    g_exception_pending = false;
    g_fault_handler = NULL;
    g_exception_initialized = true;

    return 0;
}

/**
 * rosetta_exception_cleanup - Cleanup exception handling
 */
void rosetta_exception_cleanup(void)
{
    if (!g_exception_initialized) {
        return;
    }

    memset(g_exception_handlers, 0, sizeof(g_exception_handlers));
    memset(&g_current_exception, 0, sizeof(g_current_exception));
    g_exception_pending = false;
    g_fault_handler = NULL;
    g_exception_initialized = false;
}

/* ============================================================================
 * Exception Handler Registration
 * ============================================================================ */

/**
 * rosetta_exception_register_handler - Register exception handler
 */
int rosetta_exception_register_handler(ros_exception_type_t type,
                                        ros_exception_handler_t handler)
{
    if (type <= ROS_EXCEPTION_NONE || type >= MAX_EXCEPTION_HANDLERS) {
        return -1;
    }

    if (!g_exception_initialized) {
        rosetta_exception_init();
    }

    g_exception_handlers[type] = handler;
    return 0;
}

/**
 * rosetta_exception_unregister_handler - Unregister exception handler
 */
int rosetta_exception_unregister_handler(ros_exception_type_t type)
{
    if (type <= ROS_EXCEPTION_NONE || type >= MAX_EXCEPTION_HANDLERS) {
        return -1;
    }

    g_exception_handlers[type] = NULL;
    return 0;
}

/**
 * rosetta_set_fault_handler - Set handler for memory faults
 */
int rosetta_set_fault_handler(ros_exception_handler_t handler)
{
    if (!g_exception_initialized) {
        rosetta_exception_init();
    }

    g_fault_handler = handler;
    return 0;
}

/* ============================================================================
 * Exception Generation
 * ============================================================================ */

/**
 * rosetta_raise_exception - Raise an exception
 */
int rosetta_raise_exception(ros_exception_type_t type, uint64_t pc, uint32_t insn)
{
    ros_exception_info_t info;

    if (type <= ROS_EXCEPTION_NONE) {
        return -1;
    }

    memset(&info, 0, sizeof(info));
    info.type = type;
    info.pc = pc;
    info.insn = insn;
    info.flags = ROS_EXCEPTION_FLAG_NONE;

    /* Set default signal mapping */
    switch (type) {
        case ROS_EXCEPTION_TRAP:
            info.sig = SIGTRAP;
            info.code = 0;
            break;
        case ROS_EXCEPTION_UNDEFINED:
            info.sig = SIGILL;
            info.code = 2;  /* ILL_ILLTRP */
            break;
        case ROS_EXCEPTION_DAT_ABORT:
            info.sig = SIGSEGV;
            info.code = 1;  /* SEGV_MAPERR */
            break;
        case ROS_EXCEPTION_PRE_ABORT:
            info.sig = SIGSEGV;
            info.code = 2;  /* SEGV_ACCERR */
            break;
        case ROS_EXCEPTION_FP:
            info.sig = SIGFPE;
            info.code = 0;
            break;
        case ROS_EXCEPTION_SYS:
            info.sig = 0;  /* Handled specially */
            info.code = 0;
            break;
        default:
            info.sig = SIGTRAP;
            info.code = 0;
            break;
    }

    /* Save as current exception */
    g_current_exception = info;
    g_exception_pending = true;

    /* Try to handle with registered handler */
    if (type < MAX_EXCEPTION_HANDLERS && g_exception_handlers[type] != NULL) {
        return g_exception_handlers[type](&info, NULL);
    }

    /* Default handling based on type */
    switch (type) {
        case ROS_EXCEPTION_TRAP:
        case ROS_EXCEPTION_BREAKPOINT:
            /* Debug trap - can usually continue */
            return 0;

        case ROS_EXCEPTION_UNDEFINED:
            /* Undefined instruction - fatal */
            info.flags |= ROS_EXCEPTION_FLAG_FATAL;
            return -1;

        case ROS_EXCEPTION_DAT_ABORT:
        case ROS_EXCEPTION_PRE_ABORT:
        case ROS_EXCEPTION_ALIGNMENT:
            /* Memory faults - try fault handler */
            if (g_fault_handler != NULL) {
                return g_fault_handler(&info, NULL);
            }
            info.flags |= ROS_EXCEPTION_FLAG_FATAL;
            return -1;

        case ROS_EXCEPTION_SYS:
            /* Syscall - handled separately */
            return 0;

        default:
            info.flags |= ROS_EXCEPTION_FLAG_FATAL;
            return -1;
    }
}

/**
 * rosetta_raise_trap - Raise a trap exception
 */
int rosetta_raise_trap(uint64_t pc, uint32_t insn)
{
    return rosetta_raise_exception(ROS_EXCEPTION_TRAP, pc, insn);
}

/**
 * rosetta_raise_undefined - Raise undefined instruction exception
 */
int rosetta_raise_undefined(uint64_t pc, uint32_t insn)
{
    return rosetta_raise_exception(ROS_EXCEPTION_UNDEFINED, pc, insn);
}

/**
 * rosetta_raise_data_abort - Raise data abort exception
 */
int rosetta_raise_data_abort(uint64_t pc, uint64_t addr, uint32_t insn)
{
    int result = rosetta_raise_exception(ROS_EXCEPTION_DAT_ABORT, pc, insn);
    g_current_exception.addr = addr;
    return result;
}

/**
 * rosetta_raise_syscall - Raise syscall exception
 */
int rosetta_raise_syscall(uint64_t pc, int nr)
{
    int result = rosetta_raise_exception(ROS_EXCEPTION_SYS, pc, 0xD4000000);
    if (result == 0) {
        result = rosetta_handle_syscall(nr, NULL);
    }
    return result;
}

/* ============================================================================
 * Exception Handling
 * ============================================================================ */

/**
 * rosetta_handle_exception - Handle an exception
 */
int rosetta_handle_exception(ros_exception_info_t *info, void *context)
{
    (void)context;

    if (!info) {
        return -1;
    }

    /* Try registered handler first */
    if (info->type < MAX_EXCEPTION_HANDLERS &&
        g_exception_handlers[info->type] != NULL) {
        return g_exception_handlers[info->type](info, NULL);
    }

    /* Default handling */
    switch (info->type) {
        case ROS_EXCEPTION_TRAP:
        case ROS_EXCEPTION_BREAKPOINT:
            printf("[ROSETTA] Trap at PC=0x%016llx\n",
                   (unsigned long long)info->pc);
            return 0;

        case ROS_EXCEPTION_UNDEFINED:
            printf("[ROSETTA] Undefined instruction at PC=0x%016llx: 0x%08x\n",
                   (unsigned long long)info->pc, info->insn);
            return -1;

        case ROS_EXCEPTION_DAT_ABORT:
            printf("[ROSETTA] Data abort at PC=0x%016llx, addr=0x%016llx\n",
                   (unsigned long long)info->pc,
                   (unsigned long long)info->addr);
            return -1;

        case ROS_EXCEPTION_SYS:
            /* Handled by syscall handler */
            return 0;

        default:
            printf("[ROSETTA] Unknown exception %d at PC=0x%016llx\n",
                   info->type, (unsigned long long)info->pc);
            return -1;
    }
}

/**
 * rosetta_handle_undefined_insn - Handle undefined instruction
 */
int rosetta_handle_undefined_insn(uint64_t pc, uint32_t insn)
{
    return rosetta_raise_undefined(pc, insn);
}

/**
 * rosetta_handle_brk - Handle BRK instruction
 */
int rosetta_handle_brk(uint64_t pc, uint32_t imm)
{
    (void)imm;  /* BRK immediate can be used for debug levels */

#ifdef DEBUG
    printf("[ROSETTA] BRK instruction at PC=0x%016llx (imm=%u)\n",
           (unsigned long long)pc, imm);
#endif

    return rosetta_raise_trap(pc, 0xD4200000 | (imm << 5));
}

/**
 * rosetta_handle_hlt - Handle HLT instruction
 */
int rosetta_handle_hlt(uint64_t pc, uint32_t imm)
{
#ifdef DEBUG
    printf("[ROSETTA] HLT instruction at PC=0x%016llx (imm=%u)\n",
           (unsigned long long)pc, imm);
#endif

    /* HLT with imm=0 is often used for graceful termination */
    if (imm == 0) {
        printf("[ROSETTA] HLT encountered - stopping execution\n");
        return -1;
    }

    return rosetta_raise_trap(pc, 0xD4000000 | (imm << 5));
}

/**
 * rosetta_handle_svc - Handle SVC instruction
 */
int rosetta_handle_svc(uint64_t pc, uint32_t imm)
{
    /* SVC immediate contains syscall number on some ABIs */
    int syscall_nr = imm & 0xFFFF;

#ifdef DEBUG
    printf("[ROSETTA] SVC instruction at PC=0x%016llx (nr=%d)\n",
           (unsigned long long)pc, syscall_nr);
#endif

    return rosetta_raise_syscall(pc, syscall_nr);
}

/* ============================================================================
 * Exception State
 * ============================================================================ */

/**
 * rosetta_exception_get_info - Get current exception info
 */
ros_exception_info_t *rosetta_exception_get_info(void)
{
    if (!g_exception_pending) {
        return NULL;
    }

    return &g_current_exception;
}

/**
 * rosetta_exception_clear - Clear exception state
 */
void rosetta_exception_clear(void)
{
    g_exception_pending = false;
    memset(&g_current_exception, 0, sizeof(g_current_exception));
}

/**
 * rosetta_exception_occurred - Check if exception occurred
 */
bool rosetta_exception_occurred(void)
{
    return g_exception_pending;
}

/* ============================================================================
 * Signal-to-Exception Mapping
 * ============================================================================ */

/**
 * rosetta_signal_to_exception - Convert signal to exception type
 */
ros_exception_type_t rosetta_signal_to_exception(int sig, int code)
{
    (void)code;

    switch (sig) {
        case SIGTRAP:
            return ROS_EXCEPTION_TRAP;
        case SIGILL:
            return ROS_EXCEPTION_UNDEFINED;
        case SIGSEGV:
        case SIGBUS:
            return ROS_EXCEPTION_DAT_ABORT;
        case SIGFPE:
            return ROS_EXCEPTION_FP;
        default:
            return ROS_EXCEPTION_NONE;
    }
}

/**
 * rosetta_exception_to_signal - Convert exception type to signal
 */
int rosetta_exception_to_signal(ros_exception_type_t type)
{
    switch (type) {
        case ROS_EXCEPTION_TRAP:
        case ROS_EXCEPTION_BREAKPOINT:
        case ROS_EXCEPTION_STEP:
        case ROS_EXCEPTION_WATCHPOINT:
            return SIGTRAP;
        case ROS_EXCEPTION_UNDEFINED:
            return SIGILL;
        case ROS_EXCEPTION_DAT_ABORT:
        case ROS_EXCEPTION_PRE_ABORT:
        case ROS_EXCEPTION_ALIGNMENT:
            return SIGSEGV;
        case ROS_EXCEPTION_FP:
            return SIGFPE;
        case ROS_EXCEPTION_SYS:
            return 0;  /* No signal for syscall */
        default:
            return SIGTRAP;
    }
}
