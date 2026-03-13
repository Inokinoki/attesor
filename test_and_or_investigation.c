/* Investigate AND/OR fast-path detection */
#include <stdio.h>
#include <stdint.h>

static inline int is_reg_to_reg_alu(const uint8_t *p)
{
    uint8_t first = p[0];
    uint8_t second = p[1];

    /* Check for REX prefix */
    int has_rex = ((first & 0xF0) == 0x40);
    uint8_t opcode = has_rex ? second : first;

    /* ALU opcodes with register operands (0x01-0x03, 0x29-0x2B, 0x31, 0x39-0x3B) */
    if ((opcode >= 0x01 && opcode <= 0x03) ||
        (opcode >= 0x29 && opcode <= 0x2B) ||
        opcode == 0x31 ||
        (opcode >= 0x39 && opcode <= 0x3B)) {

        /* Check ModR/M for register-to-register (mod = 11b) */
        uint8_t modrm = has_rex ? p[2] : p[1];
        if ((modrm & 0xC0) == 0xC0) {  /* mod = 11b = register-to-register */
            return 1;
        }
    }

    return 0;
}

int main() {
    /* Test AND instructions */
    uint8_t and_insn1[] = {0x48, 0x21, 0xC8};  /* AND RAX,RCX */
    uint8_t and_insn2[] = {0x48, 0x23, 0xC8};  /* AND RAX,RCX (different encoding) */

    /* Test OR instructions */
    uint8_t or_insn1[] = {0x48, 0x09, 0xC3};   /* OR RBX,RAX */
    uint8_t or_insn2[] = {0x48, 0x0B, 0xC3};   /* OR RBX,RAX (different encoding) */

    printf("Testing AND instructions:\n");
    printf("  AND RAX,RCX (0x48 0x21 0xC8): opcode=0x%02X, fastpath=%d\n",
           and_insn1[1], is_reg_to_reg_alu(and_insn1));
    printf("  AND RAX,RCX (0x48 0x23 0xC8): opcode=0x%02X, fastpath=%d\n",
           and_insn2[1], is_reg_to_reg_alu(and_insn2));

    printf("\nTesting OR instructions:\n");
    printf("  OR RBX,RAX (0x48 0x09 0xC3): opcode=0x%02X, fastpath=%d\n",
           or_insn1[1], is_reg_to_reg_alu(or_insn1));
    printf("  OR RBX,RAX (0x48 0x0B 0xC3): opcode=0x%02X, fastpath=%d\n",
           or_insn2[1], is_reg_to_reg_alu(or_insn2));

    /* Check which opcodes are covered */
    printf("\nOpcode coverage:\n");
    printf("  0x21 (AND r/m, r): %s\n", (0x21 >= 0x20 && 0x21 <= 0x23) ? "covered" : "NOT covered");
    printf("  0x23 (AND r, r/m): %s\n", (0x23 >= 0x20 && 0x23 <= 0x23) ? "covered" : "NOT covered");
    printf("  0x09 (OR r/m, r):  %s\n", (0x09 >= 0x08 && 0x09 <= 0x0B) ? "covered" : "NOT covered");
    printf("  0x0B (OR r, r/m):  %s\n", (0x0B >= 0x08 && 0x0B <= 0x0B) ? "covered" : "NOT covered");

    return 0;
}
