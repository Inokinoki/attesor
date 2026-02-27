/* ============================================================================
 * Rosetta Translator - Bit Manipulation Implementation
 * ============================================================================ */

#include "rosetta_trans_bit.h"

int translate_bfm(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t imm6 = ((insn[1] >> 0) & 0x3F);
    uint8_t immr = (insn[2] >> 16) & 0x3F;
    uint8_t imms = (insn[3] >> 10) & 0x3F;

    uint64_t src = state->guest.x[rn];
    uint64_t dst = state->guest.x[rd];
    uint64_t result = dst;

    /* Calculate width and shift */
    int width = imms - immr + 1;
    if (width < 0) width = 0;
    if (width > 64) width = 64;

    uint64_t mask = (1ULL << width) - 1;
    uint64_t bits = (src >> immr) & mask;

    result &= ~(mask << imm6);
    result |= bits << imm6;

    state->guest.x[rd] = result;

    return 0;
}

int translate_bfi(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t lsb = ((insn[1] >> 0) & 0x3F);
    uint8_t width = ((insn[3] >> 10) & 0x3F) - lsb + 1;

    uint64_t src = state->guest.x[rn];
    uint64_t dst = state->guest.x[rd];

    uint64_t mask = (1ULL << width) - 1;
    dst &= ~(mask << lsb);
    dst |= (src & mask) << lsb;

    state->guest.x[rd] = dst;

    return 0;
}

int translate_bfx(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t lsb = ((insn[1] >> 0) & 0x3F);
    uint8_t width = ((insn[3] >> 10) & 0x3F) - lsb + 1;

    uint64_t src = state->guest.x[rn];
    uint64_t mask = (1ULL << width) - 1;

    state->guest.x[rd] = (src >> lsb) & mask;

    return 0;
}

int translate_rbit(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint64_t src = state->guest.x[rn];
    uint64_t result = 0;

    for (int i = 0; i < 64; i++) {
        if (src & (1ULL << i)) {
            result |= (1ULL << (63 - i));
        }
    }

    state->guest.x[rd] = result;

    return 0;
}

int translate_rev(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint64_t src = state->guest.x[rn];
    uint64_t result = 0;

    result |= (src & 0xFF) << 56;
    result |= ((src >> 8) & 0xFF) << 48;
    result |= ((src >> 16) & 0xFF) << 40;
    result |= ((src >> 24) & 0xFF) << 32;
    result |= ((src >> 32) & 0xFF) << 24;
    result |= ((src >> 40) & 0xFF) << 16;
    result |= ((src >> 48) & 0xFF) << 8;
    result |= ((src >> 56) & 0xFF);

    state->guest.x[rd] = result;

    return 0;
}

int translate_rev16(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint64_t src = state->guest.x[rn];
    uint64_t result = 0;

    for (int i = 0; i < 4; i++) {
        uint16_t half = (src >> (i * 16)) & 0xFFFF;
        result |= ((half >> 8) | ((half & 0xFF) << 8)) << (i * 16);
    }

    state->guest.x[rd] = result;

    return 0;
}

int translate_rev32(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint64_t src = state->guest.x[rn];
    uint64_t result = 0;

    uint32_t lo = src & 0xFFFFFFFF;
    uint32_t hi = (src >> 32) & 0xFFFFFFFF;

    lo = ((lo >> 24) | ((lo >> 8) & 0xFF00) | ((lo & 0xFF00) << 8) | (lo << 24));
    hi = ((hi >> 24) | ((hi >> 8) & 0xFF00) | ((hi & 0xFF00) << 8) | (hi << 24));

    result = ((uint64_t)lo << 32) | hi;

    state->guest.x[rd] = result;

    return 0;
}

int translate_clz(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint64_t src = state->guest.x[rn];
    int count = 0;

    if (src == 0) {
        count = 64;
    } else {
        while ((src & (1ULL << 63)) == 0) {
            count++;
            src <<= 1;
        }
    }

    state->guest.x[rd] = count;

    return 0;
}

int translate_cls(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    int64_t src = (int64_t)state->guest.x[rn];
    int count = 0;
    int64_t sign_bit = src < 0 ? 1 : 0;

    for (int i = 62; i >= 0; i--) {
        int64_t bit = (src >> i) & 1;
        if (bit != sign_bit) {
            break;
        }
        count++;
    }

    state->guest.x[rd] = count;

    return 0;
}

int translate_popcnt(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint64_t src = state->guest.x[rn];
    int count = 0;

    while (src) {
        count += src & 1;
        src >>= 1;
    }

    state->guest.x[rd] = count;

    return 0;
}
