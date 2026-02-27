/* ============================================================================
 * Rosetta Binary Translator - Information Module Header
 * ============================================================================
 *
 * This module provides version, capability, and configuration information
 * for the Rosetta translation layer.
 * ============================================================================ */

#ifndef ROSETTA_INFO_H
#define ROSETTA_INFO_H

#include <stdint.h>

/* ============================================================================
 * Version Information
 * ============================================================================ */

/**
 * Get Rosetta version string
 * Returns: Version string (e.g., "2.0.0-refactored")
 */
const char *rosetta_version(void);

/**
 * Get Rosetta version components
 * @major: Output major version
 * @minor: Output minor version
 * @patch: Output patch version
 */
void rosetta_version_components(int *major, int *minor, int *patch);

/* ============================================================================
 * Capability Flags
 * ============================================================================ */

#define ROSETTA_CAP_ALU             (1 << 0)   /* ALU translation */
#define ROSETTA_CAP_MEMORY          (1 << 1)   /* Memory translation */
#define ROSETTA_CAP_BRANCH          (1 << 2)   /* Branch translation */
#define ROSETTA_CAP_SYSCALL         (1 << 3)   /* System call handling */
#define ROSETTA_CAP_CACHE           (1 << 4)   /* Translation caching */
#define ROSETTA_CAP_STATS           (1 << 5)   /* Statistics tracking */
#define ROSETTA_CAP_DEBUG           (1 << 6)   /* Debug support */
#define ROSETTA_CAP_INTERPRETER     (1 << 7)   /* Interpreter mode */
#define ROSETTA_CAP_JIT             (1 << 8)   /* JIT compilation */
#define ROSETTA_CAP_NEON            (1 << 9)   /* NEON/SIMD support */
#define ROSETTA_CAP_FP              (1 << 10)  /* Floating point support */
#define ROSETTA_CAP_BIT_OPS         (1 << 11)  /* Bit manipulation */
#define ROSETTA_CAP_STRING_OPS      (1 << 12)  /* String operations */

/**
 * Get Rosetta capabilities flags
 * Returns: Capabilities bitmask
 */
uint32_t rosetta_capabilities(void);

/**
 * Check if a specific capability is supported
 * @cap: Capability flag to check
 * Returns: 1 if supported, 0 otherwise
 */
int rosetta_has_capability(uint32_t cap);

/**
 * Get Rosetta capabilities as string description
 * Returns: String describing capabilities
 */
const char *rosetta_capabilities_string(void);

/* ============================================================================
 * Instruction Support Information
 * ============================================================================ */

/**
 * Get count of supported instructions
 * Returns: Number of supported ARM64 instructions
 */
int rosetta_get_supported_instructions(void);

/**
 * Get supported instruction classes
 * Returns: Bitmask of supported instruction classes
 */
uint64_t rosetta_get_instruction_classes(void);

/**
 * Check if an instruction class is supported
 * @class: Instruction class (ALU=0, MEM=1, BRANCH=2, etc.)
 * Returns: 1 if supported, 0 otherwise
 */
int rosetta_supports_instruction_class(int class);

/* ============================================================================
 * Build Information
 * ============================================================================ */

/**
 * Get build date string
 * Returns: Build date string
 */
const char *rosetta_build_date(void);

/**
 * Get build target architecture
 * Returns: Target architecture string (e.g., "x86_64-darwin")
 */
const char *rosetta_build_target(void);

/**
 * Get compiler information
 * Returns: Compiler string
 */
const char *rosetta_compiler_info(void);

#endif /* ROSETTA_INFO_H */

