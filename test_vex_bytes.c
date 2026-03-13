/* Analyze VEX byte structure */
#include <stdio.h>
#include <stdint.h>

int main() {
    /* Test VEX.C5: {0xC5, 0xF8, 0x59, 0xC1} */
    uint8_t vex_c5_bytes[] = {0xC5, 0xF8, 0x59, 0xC1};

    printf("VEX.C5 Analysis:\n");
    printf("Byte 0: 0x%02X (%08b) - VEX.C5 prefix\n", vex_c5_bytes[0], vex_c5_bytes[0]);
    printf("Byte 1: 0x%02X (%08b) - VEX byte 1\n", vex_c5_bytes[1], vex_c5_bytes[1]);
    printf("  Bit 7 (~R): %d\n", (vex_c5_bytes[1] >> 7) & 1);
    printf("  Bits 6-2: %05b\n", (vex_c5_bytes[1] >> 2) & 0x1F);
    printf("  Bit 2 (L): %d\n", (vex_c5_bytes[1] >> 2) & 1);
    printf("  Bits 1-0 (pp): %d\n", vex_c5_bytes[1] & 0x03);
    printf("Byte 2: 0x%02X (%08b) - Opcode\n", vex_c5_bytes[2], vex_c5_bytes[2]);
    printf("Byte 3: 0x%02X (%08b) - ModR/M\n", vex_c5_bytes[3], vex_c5_bytes[3]);

    printf("\n");

    /* Test VEX.C4: {0xC4, 0xE1, 0x10, 0xC1} */
    uint8_t vex_c4_bytes[] = {0xC4, 0xE1, 0x10, 0xC1};

    printf("VEX.C4 Analysis:\n");
    printf("Byte 0: 0x%02X (%08b) - VEX.C4 prefix\n", vex_c4_bytes[0], vex_c4_bytes[0]);
    printf("Byte 1: 0x%02X (%08b) - VEX byte 1\n", vex_c4_bytes[1], vex_c4_bytes[1]);
    printf("  Bit 7 (~R): %d\n", (vex_c4_bytes[1] >> 7) & 1);
    printf("  Bits 6-2 (map): %05b\n", vex_c4_bytes[1] & 0x1F);
    printf("Byte 2: 0x%02X (%08b) - VEX byte 2\n", vex_c4_bytes[2], vex_c4_bytes[2]);
    printf("  Bit 7 (W): %d\n", (vex_c4_bytes[2] >> 7) & 1);
    printf("  Bits 6-3 (~vvvv): %04b\n", (vex_c4_bytes[2] >> 3) & 0x0F);
    printf("  Bit 2 (L): %d\n", (vex_c4_bytes[2] >> 2) & 1);
    printf("  Bits 1-0 (pp): %d\n", vex_c4_bytes[2] & 0x03);
    printf("Byte 3: 0x%02X (%08b) - Opcode or ModR/M?\n", vex_c4_bytes[3], vex_c4_bytes[3]);

    return 0;
}
