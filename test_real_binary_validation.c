/* Validate decoder against real x86_64 binaries */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "rosetta_types.h"
#include "rosetta_x86_decode.h"

typedef struct {
    const char *name;
    const char *filename;
} binary_test_t;

int main() {
    binary_test_t tests[] = {
        { "simple_x86_pure", "simple_x86_pure.x86_64" },
        { "simple_x86_test", "simple_x86_test.x86_64" },
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    
    printf("Real Binary Validation Test\\n");
    printf("==========================\\n\\n");
    
    int total_passed = 0;
    int total_failed = 0;
    
    for (int i = 0; i < num_tests; i++) {
        binary_test_t *t = &tests[i];
        
        printf("[%d] Testing binary: %s\\n", i+1, t->filename);
        printf("     Description: %s\\n", t->name);
        
        /* Read binary file */
        FILE *f = fopen(t->filename, "rb");
        if (!f) {
            printf("     ✗ FAIL: Could not open file\\n\\n");
            total_failed++;
            continue;
        }
        
        /* Get file size */
        fseek(f, 0, SEEK_END);
        long file_size = ftell(f);
        fseek(f, 0, SEEK_SET);
        
        /* Read entire file */
        uint8_t *buffer = (uint8_t *)malloc(file_size);
        if (!buffer) {
            printf("     ✗ FAIL: Could not allocate memory\\n\\n");
            fclose(f);
            total_failed++;
            continue;
        }
        
        size_t read_size = fread(buffer, 1, file_size, f);
        fclose(f);
        
        if (read_size != file_size) {
            printf("     ✗ FAIL: Could not read file\\n\\n");
            free(buffer);
            total_failed++;
            continue;
        }
        
        /* Decode all instructions in the binary */
        int offset = 0;
        int insn_count = 0;
        int decode_errors = 0;
        
        while (offset < file_size) {
            x86_insn_t insn;
            memset(&insn, 0, sizeof(insn));
            
            int decoded = decode_x86_insn(buffer + offset, &insn);
            
            if (decoded == 0 || decoded > 15) {
                /* Invalid instruction or too long */
                decode_errors++;
                if (decode_errors <= 5) {
                    printf("     Warning: Cannot decode at offset 0x%lx\\n", offset);
                }
                break;
            }
            
            if (offset + decoded > file_size) {
                /* Overrun - likely end of code */
                break;
            }
            
            offset += decoded;
            insn_count++;
            
            /* Safety limit */
            if (insn_count > 100000) {
                printf("     Stopped after %d instructions (safety limit)\\n", insn_count);
                break;
            }
        }
        
        free(buffer);
        
        printf("     File size: %ld bytes\\n", file_size);
        printf("     Instructions decoded: %d\\n", insn_count);
        printf("     Decode errors: %d\\n", decode_errors);
        
        if (decode_errors == 0 && insn_count > 0) {
            printf("     ✅ PASS: Successfully decoded binary\\n\\n");
            total_passed++;
        } else {
            printf("     ✗ FAIL: Had errors decoding binary\\n\\n");
            total_failed++;
        }
    }
    
    printf("==========================\\n");
    printf("Real Binary Validation Results: %d/%d tests passed\\n",
           total_passed, num_tests);
    printf("Failed: %d\\n", total_failed);
    printf("==========================\\n");
    
    return total_failed > 0 ? 1 : 0;
}
