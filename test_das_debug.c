/* Debug DAS instruction */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rosetta_types.h"
#include "rosetta_x86_decode.h"

int main() {
    /* DAS: 0x2F */
    uint8_t das[] = {0x2F};
    x86_insn_t insn;
    memset(&insn, 0, sizeof(insn));

    printf("DAS instruction bytes: %02X\\n", das[0]);

    int decoded = decode_x86_insn(das, &insn);
    printf("Decoded length: %d (expected 1)\\n", decoded);
    printf("opcode: 0x%02X, opcode2: 0x%02X\\n", insn.opcode, insn.opcode2);
    printf("has_modrm: %d\\n", insn.has_modrm);
    if (insn.has_modrm) {
        printf("modrm: 0x%02X\\n", insn.modrm);
    }

    return 0;
}
