#include <stdio.h>
#include <stdint.h>

int main() {
    /* PINSRB: 66 0F 3A 20 /r ib */
    uint8_t pinsrb[] = {0x66, 0x0F, 0x3A, 0x20, 0xC1, 0x00};

    /* SHA256RNDS2: 0F 38 CB /r */
    uint8_t sha[] = {0x0F, 0x38, 0xCB, 0xC1};

    printf("PINSRB: ");
    for (int i = 0; i < 6; i++) printf("%02X ", pinsrb[i]);
    printf("\n");

    printf("SHA256RNDS2: ");
    for (int i = 0; i < 4; i++) printf("%02X ", sha[i]);
    printf("\n");

    return 0;
}
