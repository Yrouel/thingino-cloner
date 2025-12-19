#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Analyze what values the TXX mapping expects in the object buffer

int main() {
    // Load reference binary
    FILE *fp = fopen("references/ddr_extracted.bin", "rb");
    if (!fp) {
        printf("Failed to open reference binary\n");
        return 1;
    }
    
    uint8_t reference[324];
    size_t ref_size = fread(reference, 1, 324, fp);
    fclose(fp);
    
    if (ref_size != 324) {
        printf("Failed to read reference binary\n");
        return 1;
    }
    
    printf("=== TXX Mapping Analysis ===\n\n");
    printf("Reference DDRP section starts at file offset 0xC4\n");
    printf("DDRP[0x00-0x03] = size marker\n");
    printf("DDRP[0x04+] = TXX mapping data\n\n");
    
    // TXX mapping (from Ghidra @ 0x0046ba40):
    // param_2[0x00] = obj[0x7c]   → DDRP[0x04]
    // param_2[0x01] = obj[0x80]   → DDRP[0x08]
    // param_2[0x02] = obj[0x8c]   → DDRP[0x0C]
    // param_2[0x03] = obj[0x84]   → DDRP[0x10]
    // param_2[0x04] = obj[0x90]   → DDRP[0x14]
    // param_2[0x05] = obj[0x94]   → DDRP[0x18]
    // param_2[0x06] = obj[0x88]   → DDRP[0x1C]
    // param_2[0x07] = obj[0xac]   → DDRP[0x20]
    // param_2[0x08] = obj[0xb0]   → DDRP[0x24]
    // param_2[0x09] = obj[0xb4]   → DDRP[0x28]
    // param_2[0x0a] = obj[0xb8]   → DDRP[0x2C]
    // param_2[0x0b] = obj[0xbc]   → DDRP[0x30]
    // param_2[0x0c] = obj[0xc0]   → DDRP[0x34]
    // param_2[0x0d] = obj[0xc4]   → DDRP[0x38]
    // param_2[0x0e] = obj[0xd0]   → DDRP[0x3C]
    // param_2[0x0f] = obj[0xd8]   → DDRP[0x40]
    // param_2[0x10] = obj[0xdc]   → DDRP[0x44]
    // param_2[0x11] = obj[0x1d4]  → DDRP[0x48]
    // param_2[0x12] = obj[0x1dc]  → DDRP[0x4C]
    // param_2[0x13] = obj[0x1e4]  → DDRP[0x50]
    // param_2[0x14] = obj[0x1e8]  → DDRP[0x54]
    // param_2[0x15] = obj[0x1ec]  → DDRP[0x58]
    // param_2[0x16] = obj[0x1f0]  → DDRP[0x5C]
    // param_2[0x17] = obj[0x1f4]  → DDRP[0x60]
    // param_2[0x18] = obj[0x150]  → DDRP[0x64]
    // param_2[0x19] = obj[0x154]  → DDRP[0x68]
    // param_2[0x1a] = obj[0x1c0]  → DDRP[0x6C]
    // param_2[0x1b] = obj[0x1c4]  → DDRP[0x70]
    // param_2[0x1c] = obj[0x1c8]  → DDRP[0x74]
    // param_2[0x1d] = obj[0x1cc]  → DDRP[0x78]
    // param_2[0x1e] = obj[0x1d0]  → DDRP[0x7C]
    
    uint16_t obj_offsets[] = {
        0x7c, 0x80, 0x8c, 0x84, 0x90, 0x94, 0x88, 0xac,
        0xb0, 0xb4, 0xb8, 0xbc, 0xc0, 0xc4, 0xd0, 0xd8,
        0xdc, 0x1d4, 0x1dc, 0x1e4, 0x1e8, 0x1ec, 0x1f0, 0x1f4,
        0x150, 0x154, 0x1c0, 0x1c4, 0x1c8, 0x1cc, 0x1d0
    };
    
    printf("Object buffer values needed (from reference DDRP):\n\n");
    
    for (int i = 0; i < 31; i++) {
        uint16_t ddrp_offset = 0x04 + (i * 4);
        uint16_t file_offset = 0xC4 + ddrp_offset;
        uint32_t value = *(uint32_t *)(reference + file_offset);
        
        printf("obj[0x%03x] = 0x%08x  (DDRP[0x%02x] = file[0x%03x])\n",
               obj_offsets[i], value, ddrp_offset, file_offset);
    }
    
    printf("\n=== Key Values ===\n");
    printf("obj[0xd0]  = 0x%08x  (DDR2 algorithm writes width+CAS here)\n", 
           *(uint32_t *)(reference + 0xC4 + 0x3C));
    printf("obj[0x1d4] = 0x%08x  (TXX mapping expects width+CAS here)\n",
           *(uint32_t *)(reference + 0xC4 + 0x48));
    printf("obj[0x154] = 0x%08x  (DDR type)\n",
           *(uint32_t *)(reference + 0xC4 + 0x68));
    
    return 0;
}

