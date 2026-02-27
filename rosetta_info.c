/* ============================================================================
 * Rosetta Binary Translator - Information Module Implementation
 * ============================================================================
 *
 * This module provides version, capability, and configuration information
 * for the Rosetta translation layer.
 * ============================================================================ */

#include "rosetta_info.h"
#include "rosetta_refactored_types.h"

/* ============================================================================
 * Version Information
 * ============================================================================ */

#define ROSETTA_VERSION_MAJOR   2
#define ROSETTA_VERSION_MINOR   0
#define ROSETTA_VERSION_PATCH   0
#define ROSETTA_VERSION_STRING  "2.0.0-refactored"

const char *rosetta_version(void)
{
    return ROSETTA_VERSION_STRING;
}

void rosetta_version_components(int *major, int *minor, int *patch)
{
    if (major) *major = ROSETTA_VERSION_MAJOR;
    if (minor) *minor = ROSETTA_VERSION_MINOR;
    if (patch) *patch = ROSETTA_VERSION_PATCH;
}

/* ============================================================================
 * Capability Flags
 * ============================================================================ */

uint32_t rosetta_capabilities(void)
{
    return ROSETTA_CAP_ALU |
           ROSETTA_CAP_MEMORY |
           ROSETTA_CAP_BRANCH |
           ROSETTA_CAP_SYSCALL |
           ROSETTA_CAP_CACHE |
           ROSETTA_CAP_STATS |
           ROSETTA_CAP_DEBUG |
           ROSETTA_CAP_INTERPRETER |
           ROSETTA_CAP_JIT |
           ROSETTA_CAP_NEON |
           ROSETTA_CAP_FP |
           ROSETTA_CAP_BIT_OPS |
           ROSETTA_CAP_STRING_OPS;
}

int rosetta_has_capability(uint32_t cap)
{
    return (rosetta_capabilities() & cap) != 0;
}

static const char *g_capabilities_desc =
    "ALU, Memory, Branch, Syscall, Cache, Stats, Debug, "
    "Interpreter, JIT, NEON, FP, BitOps, StringOps";

const char *rosetta_capabilities_string(void)
{
    return g_capabilities_desc;
}

/* ============================================================================
 * Instruction Support Information
 * ============================================================================ */

#define ROSETTA_INSN_CLASS_ALU          (1ULL << 0)
#define ROSETTA_INSN_CLASS_MEMORY       (1ULL << 1)
#define ROSETTA_INSN_CLASS_BRANCH       (1ULL << 2)
#define ROSETTA_INSN_CLASS_MOV          (1ULL << 3)
#define ROSETTA_INSN_CLASS_COMPARE      (1ULL << 4)
#define ROSETTA_INSN_CLASS_SYSTEM       (1ULL << 5)
#define ROSETTA_INSN_CLASS_BIT          (1ULL << 6)
#define ROSETTA_INSN_CLASS_STRING       (1ULL << 7)
#define ROSETTA_INSN_CLASS_NEON         (1ULL << 8)
#define ROSETTA_INSN_CLASS_FP           (1ULL << 9)
#define ROSETTA_INSN_CLASS_MULTIPLY     (1ULL << 10)
#define ROSETTA_INSN_CLASS_DIVIDE       (1ULL << 11)
#define ROSETTA_INSN_CLASS_SHIFT        (1ULL << 12)
#define ROSETTA_INSN_CLASS_CONDITIONAL  (1ULL << 13)

int rosetta_get_supported_instructions(void)
{
    /* Approximate count based on implemented instruction classes */
    return 150;
}

uint64_t rosetta_get_instruction_classes(void)
{
    return ROSETTA_INSN_CLASS_ALU |
           ROSETTA_INSN_CLASS_MEMORY |
           ROSETTA_INSN_CLASS_BRANCH |
           ROSETTA_INSN_CLASS_MOV |
           ROSETTA_INSN_CLASS_COMPARE |
           ROSETTA_INSN_CLASS_SYSTEM |
           ROSETTA_INSN_CLASS_BIT |
           ROSETTA_INSN_CLASS_STRING |
           ROSETTA_INSN_CLASS_NEON |
           ROSETTA_INSN_CLASS_FP |
           ROSETTA_INSN_CLASS_MULTIPLY |
           ROSETTA_INSN_CLASS_DIVIDE |
           ROSETTA_INSN_CLASS_SHIFT |
           ROSETTA_INSN_CLASS_CONDITIONAL;
}

int rosetta_supports_instruction_class(int class)
{
    uint64_t classes = rosetta_get_instruction_classes();

    if (class < 0 || class >= 64) {
        return 0;
    }

    return (classes & (1ULL << class)) != 0;
}

/* ============================================================================
 * Build Information
 * ============================================================================ */

const char *rosetta_build_date(void)
{
    return __DATE__;
}

const char *rosetta_build_target(void)
{
#if defined(__APPLE__) && defined(__aarch64__)
    return "arm64-darwin";
#elif defined(__APPLE__) && defined(__x86_64__)
    return "x86_64-darwin";
#elif defined(__linux__) && defined(__aarch64__)
    return "arm64-linux";
#elif defined(__linux__) && defined(__x86_64__)
    return "x86_64-linux";
#else
    return "unknown";
#endif
}

const char *rosetta_compiler_info(void)
{
#if defined(__clang__)
    return "clang";
#elif defined(__GNUC__)
    return "gcc";
#elif defined(_MSC_VER)
    return "msvc";
#else
    return "unknown";
#endif
}

