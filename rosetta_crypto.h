/* ============================================================================
 * Rosetta Translator - Cryptographic Extensions Header
 * ============================================================================
 *
 * This module provides ARM64 cryptographic extension translation functions
 * for AES, SHA, and CRC operations.
 * ============================================================================ */

#ifndef ROSETTA_CRYPTO_H
#define ROSETTA_CRYPTO_H

#include <stdint.h>
#include "rosetta_types.h"
#include "rosetta_refactored_vector.h"

/* ============================================================================
 * AES Cryptographic Extensions
 * ============================================================================ */

/**
 * translate_aese - Translate ARM64 AESE (AES round encryption)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_aese(ThreadState *state, const uint8_t *insn);

/**
 * translate_aesd - Translate ARM64 AESD (AES round decryption)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_aesd(ThreadState *state, const uint8_t *insn);

/**
 * translate_aesmc - Translate ARM64 AESMC (AES mix columns)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_aesmc(ThreadState *state, const uint8_t *insn);

/**
 * translate_aesimc - Translate ARM64 AESIMC (AES inverse mix columns)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_aesimc(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * SHA Cryptographic Extensions
 * ============================================================================ */

/**
 * translate_sha1c - Translate ARM64 SHA1C (SHA1 hash update choose)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sha1c(ThreadState *state, const uint8_t *insn);

/**
 * translate_sha1p - Translate ARM64 SHA1P (SHA1 hash update parity)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sha1p(ThreadState *state, const uint8_t *insn);

/**
 * translate_sha1m - Translate ARM64 SHA1M (SHA1 hash update majority)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sha1m(ThreadState *state, const uint8_t *insn);

/**
 * translate_sha1h - Translate ARM64 SHA1H (SHA1 hash update)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sha1h(ThreadState *state, const uint8_t *insn);

/**
 * translate_sha1su0 - Translate ARM64 SHA1SU0 (SHA1 schedule update 0)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sha1su0(ThreadState *state, const uint8_t *insn);

/**
 * translate_sha1su1 - Translate ARM64 SHA1SU1 (SHA1 schedule update 1)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sha1su1(ThreadState *state, const uint8_t *insn);

/**
 * translate_sha256h - Translate ARM64 SHA256H (SHA256 hash update)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sha256h(ThreadState *state, const uint8_t *insn);

/**
 * translate_sha256h2 - Translate ARM64 SHA256H2 (SHA256 hash update 2)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sha256h2(ThreadState *state, const uint8_t *insn);

/**
 * translate_sha256su0 - Translate ARM64 SHA256SU0 (SHA256 schedule update)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sha256su0(ThreadState *state, const uint8_t *insn);

/**
 * translate_sha256su1 - Translate ARM64 SHA256SU1 (SHA256 schedule update 1)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sha256su1(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * CRC32 Extensions
 * ============================================================================ */

/**
 * translate_crc32b - Translate ARM64 CRC32B (CRC32 byte)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_crc32b(ThreadState *state, const uint8_t *insn);

/**
 * translate_crc32h - Translate ARM64 CRC32H (CRC32 halfword)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_crc32h(ThreadState *state, const uint8_t *insn);

/**
 * translate_crc32w - Translate ARM64 CRC32W (CRC32 word)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_crc32w(ThreadState *state, const uint8_t *insn);

/**
 * translate_crc32x - Translate ARM64 CRC32X (CRC32 doubleword)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_crc32x(ThreadState *state, const uint8_t *insn);

/**
 * translate_crc32cb - Translate ARM64 CRC32CB (CRC32C byte)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_crc32cb(ThreadState *state, const uint8_t *insn);

/**
 * translate_crc32ch - Translate ARM64 CRC32CH (CRC32C halfword)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_crc32ch(ThreadState *state, const uint8_t *insn);

/**
 * translate_crc32cw - Translate ARM64 CRC32CW (CRC32C word)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_crc32cw(ThreadState *state, const uint8_t *insn);

/**
 * translate_crc32cx - Translate ARM64 CRC32CX (CRC32C doubleword)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_crc32cx(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Polynomial Multiplication
 * ============================================================================ */

/**
 * translate_pmull - Translate ARM64 PMULL (polynomial multiply long)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_pmull(ThreadState *state, const uint8_t *insn);

/**
 * translate_pmull2 - Translate ARM64 PMULL2 (polynomial multiply long high)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_pmull2(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Crypto Helper Functions
 * ============================================================================ */

/**
 * crypto_aes_encrypt_round - Perform AES encryption round
 * @param state Current state vector
 * @param round_key Round key vector
 * @return New state after AES round
 */
Vector128 crypto_aes_encrypt_round(Vector128 state, Vector128 round_key);

/**
 * crypto_aes_decrypt_round - Perform AES decryption round
 * @param state Current state vector
 * @param round_key Round key vector
 * @return New state after AES round
 */
Vector128 crypto_aes_decrypt_round(Vector128 state, Vector128 round_key);

/**
 * crypto_aes_mix_columns - AES MixColumns transformation
 * @param state State vector
 * @return Transformed state
 */
Vector128 crypto_aes_mix_columns(Vector128 state);

/**
 * crypto_aes_inv_mix_columns - AES Inverse MixColumns transformation
 * @param state State vector
 * @return Transformed state
 */
Vector128 crypto_aes_inv_mix_columns(Vector128 state);

/**
 * crypto_sha1_choose - SHA-1 Choose function
 * @param x First input
 * @param y Second input
 * @param z Third input
 * @return Result of F(x,y,z) = (x AND y) OR (NOT x AND z)
 */
uint32_t crypto_sha1_choose(uint32_t x, uint32_t y, uint32_t z);

/**
 * crypto_sha1_parity - SHA-1 Parity function
 * @param x First input
 * @param y Second input
 * @param z Third input
 * @return Result of F(x,y,z) = x XOR y XOR z
 */
uint32_t crypto_sha1_parity(uint32_t x, uint32_t y, uint32_t z);

/**
 * crypto_sha1_majority - SHA-1 Majority function
 * @param x First input
 * @param y Second input
 * @param z Third input
 * @return Result of F(x,y,z) = (x AND y) OR (x AND z) OR (y AND z)
 */
uint32_t crypto_sha1_majority(uint32_t x, uint32_t y, uint32_t z);

/**
 * crypto_sha256_sigma0 - SHA-256 Sigma0 function
 * @param x Input value
 * @return Result of ROTR(x,2) XOR ROTR(x,13) XOR ROTR(x,22)
 */
uint32_t crypto_sha256_sigma0(uint32_t x);

/**
 * crypto_sha256_sigma1 - SHA-256 Sigma1 function
 * @param x Input value
 * @return Result of ROTR(x,6) XOR ROTR(x,11) XOR ROTR(x,25)
 */
uint32_t crypto_sha256_sigma1(uint32_t x);

/**
 * crypto_sha256_gamma0 - SHA-256 Gamma0 function
 * @param x Input value
 * @return Result of ROTR(x,7) XOR ROTR(x,18) XOR SHR(x,3)
 */
uint32_t crypto_sha256_gamma0(uint32_t x);

/**
 * crypto_sha256_gamma1 - SHA-256 Gamma1 function
 * @param x Input value
 * @return Result of ROTR(x,17) XOR ROTR(x,19) XOR SHR(x,10)
 */
uint32_t crypto_sha256_gamma1(uint32_t x);

/**
 * crypto_crc32_update - Update CRC32 with a byte
 * @param crc Current CRC value
 * @param byte Byte to process
 * @return Updated CRC32 value
 */
uint32_t crypto_crc32_update(uint32_t crc, uint8_t byte);

/**
 * crypto_crc32c_update - Update CRC32C with a byte
 * @param crc Current CRC value
 * @param byte Byte to process
 * @return Updated CRC32C value
 */
uint32_t crypto_crc32c_update(uint32_t crc, uint8_t byte);

/**
 * crypto_gf2_mul2 - GF(2^8) multiply by 2
 * @param x Input byte
 * @return Result of x * 2 in GF(2^8)
 */
uint8_t crypto_gf2_mul2(uint8_t x);

/**
 * crypto_gf2_mul3 - GF(2^8) multiply by 3
 * @param x Input byte
 * @return Result of x * 3 in GF(2^8)
 */
uint8_t crypto_gf2_mul3(uint8_t x);

#endif /* ROSETTA_CRYPTO_H */
