/* Debug LOCK prefix issue */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rosetta_types.h"
#include "rosetta_x86_decode.h"

int main() {
    /* LOCK ADD RAX, RBX: F0 48 01 C3 */
    uint8_t lock_add[] = {0xF0, 0x48, 0x01, 0xC3};
    x86_insn_t insn;
    memset(&insn, 0, sizeof(insn));

    printf("LOCK ADD instruction bytes: ");
    for (int i = 0; i < 4; i++) {
        printf("%02X ", lock_add[i]);
    }
    printf("\n");

    int decoded = decode_x86_insn(lock_add, &insn);
    printf("Decoded length: %d (expected 4)\n", decoded);
    printf("opcode: 0x%02X, opcode2: 0x%02X\n", insn.opcode, insn.opcode2);
    printf("rex: 0x%02X\n", insn.rex);
    printf("has_lock: %d\n", insn.has_lock);
    printf("has_modrm: %d\n", insn.has_modrm);
    printf("is_64bit: %d\n", insn.is_64bit);

    return 0;
}
