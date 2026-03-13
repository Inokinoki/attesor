/* Test AND/OR with actual decoder */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    uint8_t opcode;
    uint8_t opcode2;
    uint8_t opcode3;
    uint8_t rex;
    uint8_t modrm;
    uint8_t mod;
    uint8_t reg;
    uint8_t rm;
    uint8_t is_64bit;
    uint8_t has_modrm;
    uint8_t length;
} x86_insn_t;

/* Import the fixed function from rosetta_x86_decode.c */
extern int decode_x86_insn(const uint8_t *insn_ptr, x86_insn_t *insn);

int main() {
    /* Test AND instructions */
    uint8_t and_insn1[] = {0x48, 0x21, 0xC8};  /* AND RAX,RCX */
    uint8_t and_insn2[] = {0x48, 0x23, 0xC8};  /* AND RAX,RCX (different encoding) */

    /* Test OR instructions */
    uint8_t or_insn1[] = {0x48, 0x09, 0xC3};   /* OR RBX,RAX */
    uint8_t or_insn2[] = {0x48, 0x0B, 0xC3};   /* OR RBX,RAX (different encoding) */

    printf("Testing AND instructions:\n");
    x86_insn_t insn;
    int len;

    len = decode_x86_insn(and_insn1, &insn);
    printf("  AND RAX,RCX (0x48 0x21 0xC8): length=%d, opcode=0x%02X, modrm=0x%02X\n",
           len, insn.opcode, insn.modrm);

    len = decode_x86_insn(and_insn2, &insn);
    printf("  AND RAX,RCX (0x48 0x23 0xC8): length=%d, opcode=0x%02X, modrm=0x%02X\n",
           len, insn.opcode, insn.modrm);

    printf("\nTesting OR instructions:\n");
    len = decode_x86_insn(or_insn1, &insn);
    printf("  OR RBX,RAX (0x48 0x09 0xC3): length=%d, opcode=0x%02X, modrm=0x%02X\n",
           len, insn.opcode, insn.modrm);

    len = decode_x86_insn(or_insn2, &insn);
    printf("  OR RBX,RAX (0x48 0x0B 0xC3): length=%d, opcode=0x%02X, modrm=0x%02X\n",
           len, insn.opcode, insn.modrm);

    return 0;
}
