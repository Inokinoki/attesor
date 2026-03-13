/* Decoder robustness and error handling tests */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rosetta_types.h"
#include "rosetta_x86_decode.h"

typedef struct {
    const char *name;
    uint8_t bytes[16];
    int length;
    int should_decode;
    const char *description;
} robustness_test_t;

int main() {
    int passed = 0, failed = 0;

    robustness_test_t tests[] = {
        /* Valid instructions that should decode */
        { "Valid NOP", {0x90}, 1, 1, "Single byte NOP" },
        { "Valid INT3", {0xCC}, 1, 1, "Breakpoint" },
        { "Valid RET", {0xC3}, 1, 1, "Near return" },
        { "Valid CALL", {0xE8, 0x00, 0x00, 0x00, 0x00}, 5, 1, "Relative call" },
        { "Valid JMP", {0xE9, 0x00, 0x00, 0x00, 0x00}, 5, 1, "Relative jump" },
        
        /* Boundary cases - minimum length instructions */
        { "Single byte HLT", {0xF4}, 1, 1, "Halt processor" },
        { "Single byte CMC", {0xF5}, 1, 1, "Complement carry" },
        { "Single byte CLC", {0xF8}, 1, 1, "Clear carry" },
        { "Single byte STC", {0xF9}, 1, 1, "Set carry" },
        { "Single byte CLI", {0xFA}, 1, 1, "Clear interrupt" },
        { "Single byte STI", {0xFB}, 1, 1, "Set interrupt" },
        { "Single byte CLD", {0xFC}, 1, 1, "Clear direction" },
        { "Single byte STD", {0xFD}, 1, 1, "Set direction" },
        { "Single byte LAHF", {0x9F}, 1, 1, "Load flags to AH" },
        { "Single byte SAHF", {0x9E}, 1, 1, "Store AH to flags" },
        { "Single byte PUSHF", {0x9C}, 1, 1, "Push flags" },
        { "Single byte POPF", {0x9D}, 1, 1, "Pop flags" },
        { "Single byte CBW", {0x66, 0x98}, 2, 1, "Convert byte to word" },
        { "Single byte CWDE", {0x98}, 1, 1, "Convert word to dword" },
        { "Single byte CDQE", {0x48, 0x98}, 2, 1, "Convert dword to qword" },
        
        /* Single-byte push/pop registers */
        { "PUSH RAX", {0x50}, 1, 1, "Push RAX" },
        { "PUSH RCX", {0x51}, 1, 1, "Push RCX" },
        { "PUSH RDX", {0x52}, 1, 1, "Push RDX" },
        { "PUSH RBX", {0x53}, 1, 1, "Push RBX" },
        { "PUSH RSP", {0x54}, 1, 1, "Push RSP" },
        { "PUSH RBP", {0x55}, 1, 1, "Push RBP" },
        { "PUSH RSI", {0x56}, 1, 1, "Push RSI" },
        { "PUSH RDI", {0x57}, 1, 1, "Push RDI" },
        { "POP RAX", {0x58}, 1, 1, "Pop RAX" },
        { "POP RCX", {0x59}, 1, 1, "Pop RCX" },
        { "POP RDX", {0x5A}, 1, 1, "Pop RDX" },
        { "POP RBX", {0x5B}, 1, 1, "Pop RBX" },
        { "POP RSP", {0x5C}, 1, 1, "Pop RSP" },
        { "POP RBP", {0x5D}, 1, 1, "Pop RBP" },
        { "POP RSI", {0x5E}, 1, 1, "Pop RSI" },
        { "POP RDI", {0x5F}, 1, 1, "Pop RDI" },
        
        /* XCHG with EAX (special single-byte encodings) */
        { "XCHG EAX, ECX", {0x91}, 1, 1, "Exchange EAX and ECX" },
        { "XCHG EAX, EDX", {0x92}, 1, 1, "Exchange EAX and EDX" },
        { "XCHG EAX, EBX", {0x93}, 1, 1, "Exchange EAX and EBX" },
        { "XCHG EAX, ESP", {0x94}, 1, 1, "Exchange EAX and ESP" },
        { "XCHG EAX, EBP", {0x95}, 1, 1, "Exchange EAX and EBP" },
        { "XCHG EAX, ESI", {0x96}, 1, 1, "Exchange EAX and ESI" },
        { "XCHG EAX, EDI", {0x97}, 1, 1, "Exchange EAX and EDI" },
        
        /* Conditional jump boundaries */
        { "JO short", {0x70, 0x00}, 2, 1, "Jump if overflow" },
        { "NO short", {0x71, 0x00}, 2, 1, "Jump if not overflow" },
        { "JB short", {0x72, 0x00}, 2, 1, "Jump if below" },
        { "NB short", {0x73, 0x00}, 2, 1, "Jump if not below" },
        { "JZ short", {0x74, 0x00}, 2, 1, "Jump if zero" },
        { "JNZ short", {0x75, 0x00}, 2, 1, "Jump if not zero" },
        { "JBE short", {0x76, 0x00}, 2, 1, "Jump if below or equal" },
        { "JA short", {0x77, 0x00}, 2, 1, "Jump if above" },
        { "JS short", {0x78, 0x00}, 2, 1, "Jump if sign" },
        { "JNS short", {0x79, 0x00}, 2, 1, "Jump if not sign" },
        { "JP short", {0x7A, 0x00}, 2, 1, "Jump if parity" },
        { "JNP short", {0x7B, 0x00}, 2, 1, "Jump if not parity" },
        { "JL short", {0x7C, 0x00}, 2, 1, "Jump if less" },
        { "JGE short", {0x7D, 0x00}, 2, 1, "Jump if greater or equal" },
        { "JLE short", {0x7E, 0x00}, 2, 1, "Jump if less or equal" },
        { "JG short", {0x7F, 0x00}, 2, 1, "Jump if greater" },
        
        /* Loop instructions */
        { "LOOP", {0xE2, 0x00}, 2, 1, "Loop while CX != 0" },
        { "LOOPE", {0xE1, 0x00}, 2, 1, "Loop while equal" },
        { "LOOPNE", {0xE0, 0x00}, 2, 1, "Loop while not equal" },
        { "JECXZ", {0x67, 0xE3, 0x00}, 3, 1, "Jump if ECX zero (32-bit)" },
        { "JRCXZ", {0xE3, 0x00}, 2, 1, "Jump if RCX zero (64-bit)" },
        
        /* MOV immediate (sign-extended) */
        { "MOV AL, imm8", {0xB0, 0x00}, 2, 1, "Move imm8 to AL" },
        { "MOV CL, imm8", {0xB1, 0x00}, 2, 1, "Move imm8 to CL" },
        { "MOV DL, imm8", {0xB2, 0x00}, 2, 1, "Move imm8 to DL" },
        { "MOV BL, imm8", {0xB3, 0x00}, 2, 1, "Move imm8 to BL" },
        { "MOV AH, imm8", {0xB4, 0x00}, 2, 1, "Move imm8 to AH" },
        { "MOV CH, imm8", {0xB5, 0x00}, 2, 1, "Move imm8 to CH" },
        { "MOV DH, imm8", {0xB6, 0x00}, 2, 1, "Move imm8 to DH" },
        { "MOV BH, imm8", {0xB7, 0x00}, 2, 1, "Move imm8 to BH" },
        { "MOV EAX, imm32", {0xB8, 0x00, 0x00, 0x00, 0x00}, 5, 1, "Move imm32 to EAX" },
        { "MOV ECX, imm32", {0xB9, 0x00, 0x00, 0x00, 0x00}, 5, 1, "Move imm32 to ECX" },
        { "MOV EDX, imm32", {0xBA, 0x00, 0x00, 0x00, 0x00}, 5, 1, "Move imm32 to EDX" },
        { "MOV EBX, imm32", {0xBB, 0x00, 0x00, 0x00, 0x00}, 5, 1, "Move imm32 to EBX" },
        { "MOV ESP, imm32", {0xBC, 0x00, 0x00, 0x00, 0x00}, 5, 1, "Move imm32 to ESP" },
        { "MOV EBP, imm32", {0xBD, 0x00, 0x00, 0x00, 0x00}, 5, 1, "Move imm32 to EBP" },
        { "MOV ESI, imm32", {0xBE, 0x00, 0x00, 0x00, 0x00}, 5, 1, "Move imm32 to ESI" },
        { "MOV EDI, imm32", {0xBF, 0x00, 0x00, 0x00, 0x00}, 5, 1, "Move imm32 to EDI" },
        
        /* DEC/INC (64-bit) - must use ModR/M form */
        { "INC RAX", {0x48, 0xFF, 0xC0}, 3, 1, "Increment RAX" },
        { "INC RCX", {0x48, 0xFF, 0xC1}, 3, 1, "Increment RCX" },
        { "INC RDX", {0x48, 0xFF, 0xC2}, 3, 1, "Increment RDX" },
        { "INC RBX", {0x48, 0xFF, 0xC3}, 3, 1, "Increment RBX" },
        { "INC RSP", {0x48, 0xFF, 0xC4}, 3, 1, "Increment RSP" },
        { "INC RBP", {0x48, 0xFF, 0xC5}, 3, 1, "Increment RBP" },
        { "INC RSI", {0x48, 0xFF, 0xC6}, 3, 1, "Increment RSI" },
        { "INC RDI", {0x48, 0xFF, 0xC7}, 3, 1, "Increment RDI" },
        { "DEC RAX", {0x48, 0xFF, 0xC8}, 3, 1, "Decrement RAX" },
        { "DEC RCX", {0x48, 0xFF, 0xC9}, 3, 1, "Decrement RCX" },
        { "DEC RDX", {0x48, 0xFF, 0xCA}, 3, 1, "Decrement RDX" },
        { "DEC RBX", {0x48, 0xFF, 0xCB}, 3, 1, "Decrement RBX" },
        { "DEC RSP", {0x48, 0xFF, 0xCC}, 3, 1, "Decrement RSP" },
        { "DEC RBP", {0x48, 0xFF, 0xCD}, 3, 1, "Decrement RBP" },
        { "DEC RSI", {0x48, 0xFF, 0xCE}, 3, 1, "Decrement RSI" },
        { "DEC RDI", {0x48, 0xFF, 0xCF}, 3, 1, "Decrement RDI" },
        
        /* PUSH/POP imm8 */
        { "PUSH imm8", {0x6A, 0x00}, 2, 1, "Push imm8" },
        { "PUSH imm32", {0x68, 0x00, 0x00, 0x00, 0x00}, 5, 1, "Push imm32" },
        
        /* Special NOP encodings */
        { "NOP dword [RAX]", {0x0F, 0x1F, 0x00}, 3, 1, "Multi-byte NOP" },
        { "NOP dword [RAX+0]", {0x0F, 0x1F, 0x40, 0x00}, 4, 1, "Multi-byte NOP with disp8" },
        { "NOP dword [RAX+0x00]", {0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00}, 7, 1, "Multi-byte NOP with disp32" },
        { "NOP dword [EAX+0x00]", {0x67, 0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00}, 8, 1, "Multi-byte NOP with address size" },
        
        /* Arithmetic with imm8 */
        { "ADD AL, imm8", {0x04, 0x00}, 2, 1, "Add imm8 to AL" },
        { "ADD CL, imm8", {0x0C, 0x00}, 2, 1, "Add imm8 to CL" },
        { "OR AL, imm8", {0x0C, 0x00}, 2, 1, "OR imm8 with AL" },
        { "ADC AL, imm8", {0x14, 0x00}, 2, 1, "Add imm8 with carry to AL" },
        { "SBB AL, imm8", {0x1C, 0x00}, 2, 1, "Sub imm8 with borrow from AL" },
        { "AND AL, imm8", {0x24, 0x00}, 2, 1, "AND imm8 with AL" },
        { "SUB AL, imm8", {0x2C, 0x00}, 2, 1, "Sub imm8 from AL" },
        { "XOR AL, imm8", {0x34, 0x00}, 2, 1, "XOR imm8 with AL" },
        { "CMP AL, imm8", {0x3C, 0x00}, 2, 1, "Compare imm8 with AL" },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    printf("Testing %d decoder robustness cases:\\n\\n", num_tests);

    for (int i = 0; i < num_tests; i++) {
        robustness_test_t *t = &tests[i];
        x86_insn_t insn;
        memset(&insn, 0, sizeof(insn));

        int decoded = decode_x86_insn(t->bytes, &insn);

        printf("[%3d] %-25s ", i+1, t->name);
        for (int j = 0; j < t->length && j < 8; j++) {
            printf("%02X ", t->bytes[j]);
        }
        if (t->length > 8) {
            printf("... ");
        }
        printf("\\n");

        printf("     %s\\n", t->description);
        
        int test_passed = 0;
        if (t->should_decode) {
            printf("     Length: %d (expected %d)", decoded, t->length);
            test_passed = (decoded == t->length);
        } else {
            printf("     Decoded: %d bytes (expected failure)", decoded);
            test_passed = (decoded == 0 || decoded != t->length);
        }

        if (test_passed) {
            printf(" ✓ PASS\\n\\n");
            passed++;
        } else {
            printf(" ✗ FAIL\\n\\n");
            failed++;
        }
    }

    printf("==================================================\\n");
    printf("Robustness Test Results: %d/%d tests passed (%.1f%%)\\n",
           passed, num_tests, (100.0 * passed) / num_tests);
    printf("Failed: %d\\n", failed);
    printf("==================================================\\n");

    return failed > 0 ? 1 : 0;
}
