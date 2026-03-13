/* Debug end-to-end test failures */
#include <stdio.h>
#include <stdint.h>
#include "rosetta_x86_decode.h"

int main() {
    /* Test 1: MOV EAX, EBX */
    uint8_t mov1[] = {0x8B, 0xC3, 0x00};
    /* Test 2: CPUID */
    uint8_t cpuid[] = {0x0F, 0xA2};
    /* Test 3: RDTSC */
    uint8_t rdtsc[] = {0x0F, 0x31};
    /* Test 4: SYSENTER */
    uint8_t sysenter[] = {0x0F, 0x34};

    x86_insn_t insn;

    printf("Test 1: MOV EAX, EBX\\n");
    memset(&insn, 0, sizeof(insn));
    int len1 = decode_x86_insn(mov1, &insn);
    printf("  Bytes: %02X %02X %02X\\n", mov1[0], mov1[1], mov1[2]);
    printf("  Decoded: %d bytes (expected 2)\\n", len1);
    printf("  opcode: 0x%02X, opcode2: 0x%02X, has_modrm: %d\\n\\n", insn.opcode, insn.opcode2, insn.has_modrm);

    printf("Test 2: CPUID\\n");
    memset(&insn, 0, sizeof(insn));
    int len2 = decode_x86_insn(cpuid, &insn);
    printf("  Bytes: %02X %02X\\n", cpuid[0], cpuid[1]);
    printf("  Decoded: %d bytes (expected 2)\\n", len2);
    printf("  opcode: 0x%02X, opcode2: 0x%02X, has_modrm: %d\\n\\n", insn.opcode, insn.opcode2, insn.has_modrm);

    printf("Test 3: RDTSC\\n");
    memset(&insn, 0, sizeof(insn));
    int len3 = decode_x86_insn(rdtsc, &insn);
    printf("  Bytes: %02X %02X\\n", rdtsc[0], rdtsc[1]);
    printf("  Decoded: %d bytes (expected 2)\\n", len3);
    printf("  opcode: 0x%02X, opcode2: 0x%02X, has_modrm: %d\\n\\n", insn.opcode, insn.opcode2, insn.has_modrm);

    printf("Test 4: SYSENTER\\n");
    memset(&insn, 0, sizeof(insn));
    int len4 = decode_x86_insn(sysenter, &insn);
    printf("  Bytes: %02X %02X\\n", sysenter[0], sysenter[1]);
    printf("  Decoded: %d bytes (expected 2)\\n", len4);
    printf("  opcode: 0x%02X, opcode2: 0x%02X, has_modrm: %d\\n", insn.opcode, insn.opcode2, insn.has_modrm);

    return 0;
}
