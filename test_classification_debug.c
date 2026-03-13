/* Debug classification issues */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rosetta_types.h"
#include "rosetta_x86_decode.h"

void debug_instruction(const char *name, uint8_t *bytes, int length) {
    x86_insn_t insn;
    memset(&insn, 0, sizeof(insn));

    int decoded = decode_x86_insn(bytes, &insn);

    printf("%s\\n", name);
    printf("  Bytes: ");
    for (int j = 0; j < length; j++) {
        printf("%02X ", bytes[j]);
    }
    printf("\\n");
    printf("  Decoded: %d bytes\\n", decoded);
    printf("  opcode: 0x%02X, opcode2: 0x%02X\\n", insn.opcode, insn.opcode2);
    printf("  simd_prefix: 0x%02X, vex_prefix: %d\\n", insn.simd_prefix, insn.vex_prefix);
    printf("  reg: %d, mod: %d, rm: %d\\n", insn.reg, insn.mod, insn.rm);
    printf("  has_modrm: %d\\n\\n", insn.has_modrm);
}

int main() {
    printf("Debugging classification failures:\\n\\n");

    debug_instruction("INC EAX", (uint8_t[]){0xFF, 0xC0}, 2);
    debug_instruction("DEC EAX", (uint8_t[]){0xFF, 0xC8}, 2);
    debug_instruction("MOVZX EAX, BL", (uint8_t[]){0x0F, 0xB6, 0xC3}, 3);
    debug_instruction("MOVSX EAX, BL", (uint8_t[]){0x0F, 0xBE, 0xC3}, 3);
    debug_instruction("XCHG EAX, EBX", (uint8_t[]){0x93}, 1);
    debug_instruction("BT EAX, 3", (uint8_t[]){0x0F, 0xA3, 0xC3}, 3);
    debug_instruction("CPUID", (uint8_t[]){0x0F, 0xA2}, 2);
    debug_instruction("RDTSC", (uint8_t[]){0x0F, 0x31}, 2);
    debug_instruction("SYSCALL", (uint8_t[]){0x0F, 0x05}, 2);
    debug_instruction("ADDPS XMM0, XMM1", (uint8_t[]){0x0F, 0x58, 0xC1}, 3);
    debug_instruction("MOVAPS XMM0, XMM1", (uint8_t[]){0x0F, 0x28, 0xC1}, 3);

    return 0;
}
