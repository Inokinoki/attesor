/* ============================================================================
 * Rosetta Translator - FP Helper Functions
 * ============================================================================
 *
 * This module provides FP scalar helper functions for JIT compilation.
 * ============================================================================ */

#ifndef ROSETTA_FP_HELPERS_H
#define ROSETTA_FP_HELPERS_H

#include "rosetta_refactored_types.h"

/* ============================================================================
 * FP Scalar Helper Functions
 * ============================================================================ */

/**
 * Emit FABS (Floating-point Absolute Value) for scalar
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 * @param is_double 1 for double-precision, 0 for single-precision
 */
void emit_fabs_scalar(CodeBuffer *buf, uint8_t dst, uint8_t src, int is_double);

/**
 * Emit FNEG (Floating-point Negate) for scalar
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 * @param is_double 1 for double-precision, 0 for single-precision
 */
void emit_fneg_scalar(CodeBuffer *buf, uint8_t dst, uint8_t src, int is_double);

/**
 * Emit FCSEL (Floating-point Conditional Select) for scalar
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register for TRUE condition (0-15)
 * @param src2 Source XMM register for FALSE condition (0-15)
 * @param cond ARM64 condition code (0-15)
 */
void emit_fcsel_scalar(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t src2, uint8_t cond);

#endif /* ROSETTA_FP_HELPERS_H */
