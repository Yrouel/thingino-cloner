#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "ddr/ddr_types.h"
#include "ddr/ddr_controller.h"
#include "ddr/ddr_phy.h"

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
    printf("=== DDR Mapping Analysis ===\n\n");
    
    // Load reference binary
    size_t ref_size = 0;
    uint8_t *ref = load_binary_file("references/ddr_extracted.bin", &ref_size);
    if (!ref) {
        printf("[ERROR] Failed to load reference binary\n");
        return 1;
    }
    
    // Create test configuration matching the reference
    ddr_config_t config = {
        .type = DDR_TYPE_DDR2,
        .clock_mhz = 400,
        .cas_latency = 7,
        .tWR = 15,
        .tRAS = 45,
        .tRP = 16,
        .tRCD = 16,
        .tRC = 57,
        .tRRD = 10,
        .tWTR = 8,
        .tRFC = 128,
        .tXP = 8,
        .tCKE = 8,
        .tRL = 7,
        .tWL = 6,
        .tREFI = 7800,
        .banks = 8,
        .row_bits = 13,
        .col_bits = 10,
        .data_width = 16,
        .total_size_bytes = 128 * 1024 * 1024,
    };
    
    // Create object buffer and populate it
    uint8_t obj_buffer[0x300];
    memset(obj_buffer, 0, sizeof(obj_buffer));
    
    // Initialize and generate
    ddr_init_object_buffer(&config, obj_buffer);
    
    uint8_t ddrc_regs[0xbc];
    uint8_t ddrp_regs[0x80];
    ddr_generate_ddrc_with_object(&config, obj_buffer, ddrc_regs);
    ddr_generate_ddrp_with_object(&config, obj_buffer, ddrp_regs);
    
    // Analyze object buffer contents
    printf("Object Buffer Key Values:\n");
    printf("  [0x7c] = 0x%08x\n", *(uint32_t *)(obj_buffer + 0x7c));
    printf("  [0x80] = 0x%08x\n", *(uint32_t *)(obj_buffer + 0x80));
    printf("  [0x88] = 0x%08x\n", *(uint32_t *)(obj_buffer + 0x88));
    printf("  [0x90] = 0x%08x\n", *(uint32_t *)(obj_buffer + 0x90));
    printf("  [0x94] = 0x%08x\n", *(uint32_t *)(obj_buffer + 0x94));
    printf("  [0xac] = 0x%08x\n", *(uint32_t *)(obj_buffer + 0xac));
    printf("  [0xc4] = 0x%08x\n", *(uint32_t *)(obj_buffer + 0xc4));
    printf("  [0xcc] = 0x%08x\n", *(uint32_t *)(obj_buffer + 0xcc));
    printf("  [0xd0] = 0x%08x\n", *(uint32_t *)(obj_buffer + 0xd0));
    printf("  [0xd4] = 0x%08x\n", *(uint32_t *)(obj_buffer + 0xd4));
    printf("  [0xd8] = 0x%08x\n", *(uint32_t *)(obj_buffer + 0xd8));
    printf("  [0xdc] = 0x%08x\n", *(uint32_t *)(obj_buffer + 0xdc));
    printf("  [0xe4] = 0x%08x\n", *(uint32_t *)(obj_buffer + 0xe4));
    printf("  [0xf0] = 0x%08x\n", *(uint32_t *)(obj_buffer + 0xf0));
    printf("  [0xf4] = 0x%08x\n", *(uint32_t *)(obj_buffer + 0xf4));
    printf("  [0xf8] = 0x%08x\n", *(uint32_t *)(obj_buffer + 0xf8));
    printf("  [0xfc] = 0x%08x\n", *(uint32_t *)(obj_buffer + 0xfc));
    
    // Compare reference DDRP with object buffer to find mapping
    printf("\nDDRP Mapping Analysis (comparing reference DDRP with object buffer):\n");
    printf("Reference DDRP[0x00-0x0F] (file offset 0xC4-0xD3):\n");
    for (int i = 0; i < 16; i += 4) {
        uint32_t ref_val = *(uint32_t *)(ref + 0xC4 + i);
        printf("  DDRP[0x%02x] = 0x%08x\n", i, ref_val);
    }
    
    printf("\nLooking for these values in object buffer...\n");
    for (int ddrp_off = 0; ddrp_off < 16; ddrp_off += 4) {
        uint32_t ref_val = *(uint32_t *)(ref + 0xC4 + ddrp_off);
        printf("  DDRP[0x%02x] = 0x%08x -> searching in obj_buffer...\n", ddrp_off, ref_val);
        
        // Search for this value in object buffer
        int found = 0;
        for (int obj_off = 0; obj_off < 0x200; obj_off += 4) {
            uint32_t obj_val = *(uint32_t *)(obj_buffer + obj_off);
            if (obj_val == ref_val) {
                printf("    Found at obj_buffer[0x%02x]\n", obj_off);
                found = 1;
            }
        }
        if (!found) {
            printf("    NOT FOUND in object buffer (might be a constant or calculated value)\n");
        }
    }
    
    free(ref);
    return 0;
}

