/* ============================================================================
 * Rosetta Refactored - Exception Handling Header
 * ============================================================================
 *
 * This module provides exception handling functions for the Rosetta
 * translation layer, including trap, fault, and undefined instruction
 * handling.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_EXCEPTION_H
#define ROSETTA_REFACTORED_EXCEPTION_H

#include <stdint.h>
#include <stdbool.h>
#include <signal.h>

/* ============================================================================
 * Exception Types
 * ============================================================================ */

/* Exception classes */
typedef enum {
    ROS_EXCEPTION_NONE = 0,
    ROS_EXCEPTION_TRAP,         /* Software trap (BRK, HLT) */
    ROS_EXCEPTION_UNDEFINED,    /* Undefined instruction */
    ROS_EXCEPTION_DAT_ABORT,    /* Data abort */
    ROS_EXCEPTION_PRE_ABORT,    /* Prefetch abort */
    ROS_EXCEPTION_ALIGNMENT,    /* Alignment fault */
    ROS_EXCEPTION_FP,           /* Floating point exception */
    ROS_EXCEPTION_SYS,          /* Syscall (SVC) */
    ROS_EXCEPTION_STEP,         /* Single step */
    ROS_EXCEPTION_WATCHPOINT,   /* Watchpoint hit */
    ROS_EXCEPTION_BREAKPOINT    /* Breakpoint hit */
} ros_exception_type_t;

/* Exception flags */
#define ROS_EXCEPTION_FLAG_NONE     0x00
#define ROS_EXCEPTION_FLAG_FATAL    0x01  /* Cannot recover */
#define ROS_EXCEPTION_FLAG_USER     0x02  /* User-generated */
#define ROS_EXCEPTION_FLAG_KERNEL   0x04  /* Kernel-generated */

/* Exception information structure */
typedef struct {
    ros_exception_type_t type;    /* Exception type */
    uint32_t flags;                /* Exception flags */
    uint64_t pc;                   /* PC at exception */
    uint64_t addr;                 /* Faulting address (for aborts) */
    uint32_t insn;                 /* Faulting instruction */
    uint32_t syndrome;             /* Exception syndrome */
    int sig;                       /* Signal number */
    int code;                      /* Signal code */
} ros_exception_info_t;

/* ============================================================================
 * Exception Handler Types
 * ============================================================================ */

/* Exception handler function type */
typedef int (*ros_exception_handler_t)(ros_exception_info_t *info, void *context);

/* ============================================================================
 * Exception Initialization
 * ============================================================================ */

/**
 * rosetta_exception_init - Initialize exception handling
 * Returns: 0 on success, -1 on error
 */
int rosetta_exception_init(void);

/**
 * rosetta_exception_cleanup - Cleanup exception handling
 */
void rosetta_exception_cleanup(void);

/* ============================================================================
 * Exception Handler Registration
 * ============================================================================ */

/**
 * rosetta_exception_register_handler - Register exception handler
 * @type: Exception type
 * @handler: Handler function
 * Returns: 0 on success, -1 on error
 */
int rosetta_exception_register_handler(ros_exception_type_t type,
                                        ros_exception_handler_t handler);

/**
 * rosetta_exception_unregister_handler - Unregister exception handler
 * @type: Exception type
 * Returns: 0 on success, -1 on error
 */
int rosetta_exception_unregister_handler(ros_exception_type_t type);

/**
 * rosetta_set_fault_handler - Set handler for memory faults
 * @handler: Handler function
 * Returns: 0 on success, -1 on error
 */
int rosetta_set_fault_handler(ros_exception_handler_t handler);

/* ============================================================================
 * Exception Generation
 * ============================================================================ */

/**
 * rosetta_raise_exception - Raise an exception
 * @type: Exception type
 * @pc: PC at exception
 * @insn: Faulting instruction
 * Returns: 0 if handled, -1 if fatal
 */
int rosetta_raise_exception(ros_exception_type_t type, uint64_t pc, uint32_t insn);

/**
 * rosetta_raise_trap - Raise a trap exception
 * @pc: PC at trap
 * @insn: Trap instruction
 * Returns: 0 if handled, -1 if fatal
 */
int rosetta_raise_trap(uint64_t pc, uint32_t insn);

/**
 * rosetta_raise_undefined - Raise undefined instruction exception
 * @pc: PC at undefined instruction
 * @insn: Undefined instruction
 * Returns: 0 if handled, -1 if fatal
 */
int rosetta_raise_undefined(uint64_t pc, uint32_t insn);

/**
 * rosetta_raise_data_abort - Raise data abort exception
 * @pc: PC at abort
 * @addr: Faulting address
 * @insn: Faulting instruction
 * Returns: 0 if handled, -1 if fatal
 */
int rosetta_raise_data_abort(uint64_t pc, uint64_t addr, uint32_t insn);

/**
 * rosetta_raise_syscall - Raise syscall exception
 * @pc: PC at syscall
 * @nr: Syscall number
 * Returns: 0 if handled, -1 if fatal
 */
int rosetta_raise_syscall(uint64_t pc, int nr);

/* ============================================================================
 * Exception Handling
 * ============================================================================ */

/**
 * rosetta_handle_exception - Handle an exception
 * @info: Exception information
 * @context: Machine context
 * Returns: 0 if handled, -1 if fatal
 */
int rosetta_handle_exception(ros_exception_info_t *info, void *context);

/**
 * rosetta_handle_undefined_insn - Handle undefined instruction
 * @pc: PC at undefined instruction
 * @insn: Undefined instruction encoding
 * Returns: 0 if handled, -1 if fatal
 */
int rosetta_handle_undefined_insn(uint64_t pc, uint32_t insn);

/**
 * rosetta_handle_brk - Handle BRK instruction
 * @pc: PC at BRK
 * @imm: BRK immediate value
 * Returns: 0 if handled, -1 if fatal
 */
int rosetta_handle_brk(uint64_t pc, uint32_t imm);

/**
 * rosetta_handle_hlt - Handle HLT instruction
 * @pc: PC at HLT
 * @imm: HLT immediate value
 * Returns: 0 if handled, -1 if fatal
 */
int rosetta_handle_hlt(uint64_t pc, uint32_t imm);

/**
 * rosetta_handle_svc - Handle SVC instruction
 * @pc: PC at SVC
 * @imm: SVC immediate value (syscall number)
 * Returns: 0 if handled, -1 if fatal
 */
int rosetta_handle_svc(uint64_t pc, uint32_t imm);

/* ============================================================================
 * Exception State
 * ============================================================================ */

/**
 * rosetta_exception_get_info - Get current exception info
 * Returns: Pointer to exception info, or NULL if none
 */
ros_exception_info_t *rosetta_exception_get_info(void);

/**
 * rosetta_exception_clear - Clear exception state
 */
void rosetta_exception_clear(void);

/**
 * rosetta_exception_occurred - Check if exception occurred
 * Returns: true if exception pending, false otherwise
 */
bool rosetta_exception_occurred(void);

/* ============================================================================
 * Signal-to-Exception Mapping
 * ============================================================================ */

/**
 * rosetta_signal_to_exception - Convert signal to exception type
 * @sig: Signal number
 * @code: Signal code
 * Returns: Exception type
 */
ros_exception_type_t rosetta_signal_to_exception(int sig, int code);

/**
 * rosetta_exception_to_signal - Convert exception type to signal
 * @type: Exception type
 * Returns: Signal number
 */
int rosetta_exception_to_signal(ros_exception_type_t type);

#endif /* ROSETTA_REFACTORED_EXCEPTION_H */
