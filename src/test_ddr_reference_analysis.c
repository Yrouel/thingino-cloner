#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Helper function to load binary file
static uint8_t* load_binary_file(const char *path, size_t *size) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        printf("[ERROR] Cannot open file: %s\n", path);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    uint8_t *buffer = (uint8_t *)malloc(file_size);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);
    
    if (bytes_read != (size_t)file_size) {
        free(buffer);
        return NULL;
    }
    
    *size = bytes_read;
    return buffer;
}

int main(void) {
    printf("=== DDR Reference Binary Analysis ===\n\n");
    
    // Load reference binary
    size_t ref_size = 0;
    uint8_t *ref = load_binary_file("references/ddr_extracted.bin", &ref_size);
    if (!ref) {
        printf("[ERROR] Failed to load reference binary\n");
        return 1;
    }
    printf("[OK] Loaded reference binary: %zu bytes\n\n", ref_size);
    
    // Analyze structure
    printf("Binary Structure:\n");
    printf("  FIDB marker: %c%c%c%c\n", ref[0], ref[1], ref[2], ref[3]);
    printf("  RDD marker at 0xC0: %02x %c%c%c\n", ref[0xc0], ref[0xc1], ref[0xc2], ref[0xc3]);
    
    // Extract DDRC values (first 16 bytes after FIDB)
    printf("\nDDRC Section (first 16 bytes at 0x04-0x13):\n");
    for (int i = 0; i < 4; i++) {
        uint32_t val = *(uint32_t *)(ref + 0x04 + i*4);
        printf("  [0x%02x] = 0x%08x\n", 0x04 + i*4, val);
    }
    
    // Extract DDRP values (first 64 bytes after RDD)
    printf("\nDDRP Section (first 64 bytes at 0xC4-0x103):\n");
    for (int i = 0; i < 16; i++) {
        uint32_t val = *(uint32_t *)(ref + 0xC4 + i*4);
        printf("  [0x%02x] = 0x%08x\n", 0xC4 + i*4, val);
    }
    
    // Analyze DDRP pattern
    printf("\nDDRP Pattern Analysis:\n");
    printf("  First value at 0xC4: 0x%08x\n", *(uint32_t *)(ref + 0xC4));
    printf("  This looks like it might be an offset or size marker\n");
    
    // Check for DDR type encoding
    uint32_t ddrp_type_field = *(uint32_t *)(ref + 0xCC);
    printf("\n  Value at 0xCC (DDR type + CAS): 0x%08x\n", ddrp_type_field);
    uint8_t type_bits = ddrp_type_field & 0x7;
    uint8_t cas_bits = (ddrp_type_field >> 3) & 0x1f;
    printf("    Type field (bits 0-2): %d ", type_bits);
    if (type_bits == 3) printf("(DDR2)\n");
    else if (type_bits == 0) printf("(DDR3)\n");
    else if (type_bits == 4) printf("(LPDDR/LPDDR2)\n");
    else if (type_bits == 2) printf("(LPDDR3)\n");
    else printf("(Unknown)\n");
    printf("    CAS latency (bits 3+): %d\n", cas_bits);
    
    // Look for timing values in DDRP
    printf("\nDDRP Timing Values:\n");
    printf("  [0xE0] = 0x%08x\n", *(uint32_t *)(ref + 0xE0));
    printf("  [0xE4] = 0x%08x\n", *(uint32_t *)(ref + 0xE4));
    printf("  [0xE8] = 0x%08x\n", *(uint32_t *)(ref + 0xE8));
    printf("  [0xEC] = 0x%08x\n", *(uint32_t *)(ref + 0xEC));
    printf("  [0xF0] = 0x%08x\n", *(uint32_t *)(ref + 0xF0));
    printf("  [0xF4] = 0x%08x\n", *(uint32_t *)(ref + 0xF4));
    printf("  [0xF8] = 0x%08x\n", *(uint32_t *)(ref + 0xF8));
    printf("  [0xFC] = 0x%08x\n", *(uint32_t *)(ref + 0xFC));
    
    // Hex dump of DDRP section
    printf("\nDDRP Section Hex Dump (0xC4-0x143):\n");
    for (int i = 0; i < 0x80; i += 16) {
        printf("  %04x: ", 0xC4 + i);
        for (int j = 0; j < 16 && (i + j) < 0x80; j++) {
            printf("%02x ", ref[0xC4 + i + j]);
        }
        printf("\n");
    }
    
    free(ref);
    return 0;
}

