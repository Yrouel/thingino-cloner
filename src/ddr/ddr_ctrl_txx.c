/**
 * TXX-specific DDRC (DDR Controller) hardware register generation
 * Based on TXX_DDRBaseParam::ddrc_config_creator @ 0x004711c0
 *
 * This generates the actual DDRC hardware registers at obj[0x7c-0xcc]
 * which are then written to the DDRC section (0x04-0xBF) of the output binary.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "ddr_types.h"
#include "ddr_phy_common.h"

// Forward declaration
int ddr_generate_ddrc_timing_txx_ddr2(const ddr_config_t *config, uint8_t *obj_buffer);

/**
 * Generate TXX DDRC hardware registers
 * Populates obj[0x7c-0xcc] with DDRC hardware register values
 *
 * @param config DDR configuration
 * @param obj_buffer Object buffer (obj[0x7c-0xcc] = DDRC registers, obj[0x154+] = input params)
 * @return 0 on success, non-zero on error
 */
int ddr_generate_ddrc_txx_ddr2(const ddr_config_t *config, uint8_t *obj_buffer) {
    (void)config;  // Suppress unused parameter warning

    // DDRC register structure starts at obj[0x7c]
    uint8_t *ddrc_reg = obj_buffer + 0x7c;

    // Input parameters structure starts at obj[0x154]
    // Based on ddr_params structure from Ghidra decompilation
    uint32_t *params = (uint32_t *)(obj_buffer + 0x154);

    // Extract parameters from obj[0x154+]
    uint32_t ddr_type = params[0];           // obj[0x154] - DDR type (0=DDR2, 1=DDR3, etc.)
    uint32_t cs0_en = params[3];             // obj[0x160] - CS0 enable
    uint32_t cs1_en = params[4];             // obj[0x164] - CS1 enable
    uint32_t data_width = params[8];         // obj[0x174] - Data width (4=x8, 8=x16)
    uint32_t bank_bits = params[10];         // obj[0x17c] - Bank address bits
    uint32_t row_bits = params[12];          // obj[0x184] - Row address bits
    uint32_t cs0_row = params[14];           // obj[0x18c] - CS0 row bits
    uint32_t cs0_col = params[15];           // obj[0x190] - CS0 column bits

    (void)cs0_row;  // Will be used for memory mapping
    (void)cs0_col;  // Will be used for memory mapping

    // Clear DDRC register area (obj[0x7c-0xcc] = 80 bytes)
    memset(ddrc_reg, 0, 80);

    // ========================================
    // Generate DDRC hardware registers
    // Based on TXX_DDRBaseParam::ddrc_config_creator @ 0x004711c0
    // ========================================

    // ========================================
    // DDRC CFG Register (obj[0x7c])
    // Based on Ingenic U-Boot ddr_params_creator.c lines 190-232
    // ========================================
    uint32_t cfg = 0;

    // DW: Data width (bit 0): 0=16-bit, 1=32-bit
    // data_width in params is 8 for x16, 4 for x8
    cfg |= (data_width == 8) ? 0 : 1;

    // BA0: Bank bits for CS0 (bit 1): 0=4 banks, 1=8 banks
    cfg |= (bank_bits == 3) ? (1 << 1) : 0;

    // CL: CAS Latency (bits 2-5): Not used in this version, set to 0
    // cfg |= (cas_latency & 0xF) << 2;

    // CS0EN: CS0 Enable (bit 6)
    cfg |= (cs0_en & 1) << 6;

    // CS1EN: CS1 Enable (bit 7)
    cfg |= (cs1_en & 1) << 7;

    // COL0: Column bits for CS0 (bits 8-10): col_bits - 8
    uint32_t col_bits = params[13];  // obj[0x188]
    cfg |= ((col_bits - 8) & 0x7) << 8;

    // ROW0: Row bits for CS0 (bits 11-13): row_bits - 12
    cfg |= ((row_bits - 12) & 0x7) << 11;

    // bit 14: reserved

    // MISPE: bit 15: Set to 1
    cfg |= 1 << 15;

    // ODTEN: ODT Enable (bit 16): 0=disabled
    // cfg |= 0 << 16;

    // TYPE: DDR type (bits 17-19)
    // DDR2=4 (0b100), DDR3=6 (0b110), LPDDR=3 (0b011), LPDDR2=5 (0b101)
    uint32_t type_field = (ddr_type == 0) ? 4 : ddr_type;  // DDR2=4
    cfg |= (type_field & 0x7) << 17;

    // bit 20: reserved

    // BSL: Burst length (bit 21): 0=4, 1=8
    // Get burst length from config (default to 8 for DDR2)
    uint32_t burst_length = 8;  // TODO: Get from config
    cfg |= (burst_length == 8) ? (1 << 21) : 0;

    // IMBA: bit 22: Set to 1 for T31X
    cfg |= 1 << 22;

    // BA1: Bank bits for CS1 (bit 23): Same as BA0
    cfg |= (bank_bits == 3) ? (1 << 23) : 0;

    // COL1: Column bits for CS1 (bits 24-26): Same as COL0
    cfg |= ((col_bits - 8) & 0x7) << 24;

    // ROW1: Row bits for CS1 (bits 27-29): Same as ROW0
    cfg |= ((row_bits - 12) & 0x7) << 27;

    // bits 30-31: reserved

    *(uint32_t *)(ddrc_reg + 0) = cfg;  // obj[0x7c]

    // ========================================
    // DDRC CTRL Register (obj[0x80])
    // Based on Ingenic U-Boot ddr_params_creator.c lines 234-243
    // ========================================
    // CTRL register bit definitions from U-Boot arch/mips/include/asm/ddr_dwc.h:
    #define DDRC_CTRL_ACTPD   (1 << 15)  // Precharge all banks before power-down
    #define DDRC_CTRL_PDT_64  (4 << 12)  // Enter power-down after 64 tCK idle
    #define DDRC_CTRL_ACTSTP  (1 << 11)  // Active stop
    #define DDRC_CTRL_PRET_8  (1 << 8)   // Precharge active bank after 8 tCK idle
    #define DDRC_CTRL_UNALIGN (1 << 4)   // Enable unaligned transfer on AXI BUS
    #define DDRC_CTRL_ALH     (1 << 3)   // Advanced Latency Hiding
    #define DDRC_CTRL_RDC     (1 << 2)   // Read data cache enable
    #define DDRC_CTRL_CKE     (1 << 1)   // Set CKE Pin High

    uint32_t ctrl = DDRC_CTRL_ACTPD | DDRC_CTRL_PDT_64 | DDRC_CTRL_ACTSTP
                  | DDRC_CTRL_PRET_8 | DDRC_CTRL_UNALIGN
                  | DDRC_CTRL_ALH | DDRC_CTRL_RDC | DDRC_CTRL_CKE;

    *(uint32_t *)(ddrc_reg + 4) = ctrl;  // obj[0x80]

    #undef DDRC_CTRL_ACTPD
    #undef DDRC_CTRL_PDT_64
    #undef DDRC_CTRL_ACTSTP
    #undef DDRC_CTRL_PRET_8
    #undef DDRC_CTRL_UNALIGN
    #undef DDRC_CTRL_ALH
    #undef DDRC_CTRL_RDC
    #undef DDRC_CTRL_CKE

    // Memory mapping registers (obj[0x90-0x97])
    // Get CS0 and CS1 sizes from ddr_params (in bytes)
    uint32_t cs0_size = params[14];  // obj[0x18c]
    uint32_t cs1_size = params[15];  // obj[0x190]
    uint32_t total_size = cs0_size + cs1_size;

    uint32_t cs0_map, cs1_map;

    // dmmap table lookup (from TXX_DDRBaseParam::ddrc_config_creator)
    // Table format: [total_size_mb, cs1_size_bytes, cs0_map, cs1_map]
    // Note: cs1_size is in BYTES, not MB!
    static const uint32_t dmmap[][4] = {
        {128, 0, 0x00000000, 0x00000001},  // 128MB single CS (from reference)
        {256, 0, 0x00000000, 0x00000002},  // 256MB single CS (guess)
        {512, 0, 0x00000000, 0x00000004},  // 512MB single CS (guess)
        {256, 128 * 1024 * 1024, 0x00000001, 0x00000002},  // 256MB dual CS (guess)
    };

    // Lookup logic from Ghidra: entry[0] << 20 == total_size_bytes && entry[1] == cs1_size_bytes
    int found = 0;
    for (int i = 0; i < 4; i++) {
        if ((dmmap[i][0] << 20) == total_size && dmmap[i][1] == cs1_size) {
            cs0_map = dmmap[i][2];
            cs1_map = dmmap[i][3];
            found = 1;
            break;
        }
    }

    if (!found) {
        // Fallback calculation if not in table
        if (total_size < 0x20000001) {  // < 512MB
            cs0_map = 0x2000 | ((-(cs0_size >> 24)) & 0xff);
            cs1_map = (((cs0_size + 0x20000000) >> 24) << 8) | ((-(cs1_size >> 24)) & 0xff);
        } else if (cs1_size == 0) {  // Single CS, >= 512MB
            cs0_map = 0;
            cs1_map = 0xff00 | ((-(cs0_size * 2 >> 24)) & 0xff);
        } else {  // Dual CS, >= 512MB
            uint32_t mask = ~(total_size >> 24);
            cs0_map = ((-(cs1_size >> 24)) & 0xff & mask);
            cs1_map = ((-(cs0_size >> 24)) & 0xff & mask) | ((cs1_size >> 24) << 8);
        }
    }

    *(uint32_t *)(ddrc_reg + 0x14) = cs0_map;  // obj[0x90]
    *(uint32_t *)(ddrc_reg + 0x18) = cs1_map;  // obj[0x94]

    // ========================================
    // Timing registers are now hardcoded above
    // TODO: Re-enable this once we implement proper timing calculations from Ghidra
    // ========================================
    // return ddr_generate_ddrc_timing_txx_ddr2(config, obj_buffer);
    return 0;
}

/**
 * Generate TXX DDRC timing registers
 * Populates obj[0xac-0xc4] with DDRC timing values
 * Called after ddrc_config_creator has set up the basic registers
 *
 * Based on TXX_DDRBaseParam::ddrc_generate_register @ 0x00471890
 * and TXX_DDR2Param::ddrc_generate_register @ 0x00473460
 */
int ddr_generate_ddrc_timing_txx_ddr2(const ddr_config_t *config, uint8_t *obj_buffer) {
    (void)config;  // Suppress unused parameter warning
    uint32_t clock_period_ps = *(uint32_t *)(obj_buffer + 0x22c);

    // Helper function for ps to cycles conversion with divisor support
    #define PS2CYCLE_DIV(ps, div) (((ps) + (clock_period_ps * (div)) - 1) / (clock_period_ps * (div)))
    #define PS2CYCLE(ps) PS2CYCLE_DIV(ps, 1)

    // ========================================
    // TXX_DDRBaseParam::ddrc_generate_register @ 0x00471890
    // ========================================

    // obj[0xad] = tWR cycles (from obj[0x168])
    uint32_t tWR_ps = *(uint32_t *)(obj_buffer + 0x168);
    uint32_t tWR_cycles = PS2CYCLE(tWR_ps);
    obj_buffer[0xad] = (obj_buffer[0xad] & 0xc0) | (tWR_cycles & 0x3f);

    // obj[0xac] = tRTP cycles (from obj[0x184])
    uint32_t tRTP_ps = *(uint32_t *)(obj_buffer + 0x184);
    uint32_t tRTP_cycles = PS2CYCLE(tRTP_ps);
    obj_buffer[0xac] = (obj_buffer[0xac] & 0xc0) | (tRTP_cycles & 0x3f);

    // obj[0xb2] = tCCD cycles (from obj[0x158])
    uint32_t tCCD_ps = *(uint32_t *)(obj_buffer + 0x158);
    uint32_t tCCD_cycles = PS2CYCLE(tCCD_ps);
    obj_buffer[0xb2] = (obj_buffer[0xb2] & 0xc0) | (tCCD_cycles & 0x3f);

    // obj[0xb1] = tRAS cycles (from obj[0x160])
    uint32_t tRAS_ps = *(uint32_t *)(obj_buffer + 0x160);
    uint32_t tRAS_cycles = PS2CYCLE(tRAS_ps);
    obj_buffer[0xb1] = (obj_buffer[0xb1] & 0xc0) | (tRAS_cycles & 0x3f);

    // obj[0xb0] = tRC cycles (from obj[0x180])
    uint32_t tRC_ps = *(uint32_t *)(obj_buffer + 0x180);
    uint32_t tRC_cycles = PS2CYCLE(tRC_ps);
    obj_buffer[0xb0] = (obj_buffer[0xb0] & 0xc0) | (tRC_cycles & 0x3f);

    // obj[0xb7] bits [6:3] = 0x4 (constant)
    obj_buffer[0xb7] = (obj_buffer[0xb7] & 0x87) | 0x20;

    // obj[0xb6] = tRCD cycles (from obj[0x15c])
    uint32_t tRCD_ps = *(uint32_t *)(obj_buffer + 0x15c);
    uint32_t tRCD_cycles = PS2CYCLE(tRCD_ps);
    obj_buffer[0xb6] = (obj_buffer[0xb6] & 0xc0) | (tRCD_cycles & 0x3f);

    // obj[0xb5] = tRRD cycles (from obj[0x16c])
    uint32_t tRRD_ps = *(uint32_t *)(obj_buffer + 0x16c);
    uint32_t tRRD_cycles = PS2CYCLE(tRRD_ps);
    obj_buffer[0xb5] = (obj_buffer[0xb5] & 0xc0) | (tRRD_cycles & 0x3f);

    // obj[0xb4] = tRP cycles (from obj[0x164])
    uint32_t tRP_ps = *(uint32_t *)(obj_buffer + 0x164);
    uint32_t tRP_cycles = PS2CYCLE(tRP_ps);
    obj_buffer[0xb4] = (obj_buffer[0xb4] & 0xc0) | (tRP_cycles & 0x3f);

    // obj[0xbb] = tRTW cycles (from obj[0x174])
    uint32_t tRTW_ps = *(uint32_t *)(obj_buffer + 0x174);
    uint32_t tRTW_cycles = ((tRTW_ps - 1 + clock_period_ps * 2) / (clock_period_ps * 2));
    tRTW_cycles = (tRTW_cycles / 2) - 1;
    obj_buffer[0xbb] = (obj_buffer[0xbb] & 0xc0) | (tRTW_cycles & 0x3f);

    // obj[0xba] bits [6:5] = 0x3, bits [2:0] = tWTR+1 (from obj[0x17c])
    obj_buffer[0xba] = (obj_buffer[0xba] & 7) | 0x60;
    uint32_t tWTR_ps = *(uint32_t *)(obj_buffer + 0x17c);
    uint32_t tWTR_cycles = PS2CYCLE(tWTR_ps) + 1;
    obj_buffer[0xba] = (obj_buffer[0xba] & 0xf8) | (tWTR_cycles & 7);

    // obj[0xb8] bits [6:4] = tRTR (from obj[0x178])
    uint32_t tRTR_ps = *(uint32_t *)(obj_buffer + 0x178);
    uint32_t tRTR_cycles = PS2CYCLE(tRTR_ps);
    obj_buffer[0xb8] = (obj_buffer[0xb8] & 0x8f) | ((tRTR_cycles & 7) << 4);

    // obj[0xbf] = 0xff (constant)
    obj_buffer[0xbf] = 0xff;

    // obj[0xc1] = 5 (constant)
    obj_buffer[0xc1] = (obj_buffer[0xc1] & 0xc0) | 5;

    // obj[0xc0] = 5 (constant)
    obj_buffer[0xc0] = (obj_buffer[0xc0] & 0xc0) | 5;

    // obj[0x88] is set by ddrc_config_creator, don't overwrite it here!
    // (Previously was incorrectly set to 0x00000001)

    // obj[0xc4] = 0 (enable flag)
    *(uint32_t *)(obj_buffer + 0xc4) = 0;

    // ========================================
    // TXX_DDR2Param::ddrc_generate_register @ 0x00473460 (DDR2-specific overrides)
    // ========================================

    // obj[0xaf] = tWR cycles (from obj[0x1a4])
    uint32_t tWR_ddr2_ps = *(uint32_t *)(obj_buffer + 0x1a4);
    uint32_t tWR_ddr2_cycles = PS2CYCLE(tWR_ddr2_ps);
    obj_buffer[0xaf] = (obj_buffer[0xaf] & 0xc0) | (tWR_ddr2_cycles & 0x3f);

    // obj[0xae] = tWL + CL - 1 + data_width/2
    uint32_t tWL_ps = *(uint32_t *)(obj_buffer + 0x170);
    uint32_t tWL_cycles = PS2CYCLE(tWL_ps);
    uint32_t CL = *(uint32_t *)(obj_buffer + 0x130);
    uint32_t data_width = *(uint32_t *)(obj_buffer + 0x138);
    uint32_t ae_value = tWL_cycles + CL - 1 + (data_width / 2);
    obj_buffer[0xae] = (obj_buffer[0xae] & 0xc0) | (ae_value & 0x3f);

    // obj[0xbe] = data width encoding (4 or 6)
    if (data_width == 4) {
        obj_buffer[0xbe] = (obj_buffer[0xbe] & 0xc0) | 4;
    } else if (data_width == 8) {
        obj_buffer[0xbe] = (obj_buffer[0xbe] & 0xc0) | 6;
    }

    // obj[0xbc] = obj[0xac] - 1 (tRTP - 1)
    uint8_t tRTP_val = obj_buffer[0xac] & 0x3f;
    if (tRTP_val > 0) {
        obj_buffer[0xbc] = (obj_buffer[0xbc] & 0xc0) | ((tRTP_val - 1) & 0x3f);
    }

    // obj[0xbd] = obj[0xb0] - 3 (tRC - 3)
    uint8_t tRC_val = obj_buffer[0xb0] & 0x3f;
    if (tRC_val >= 2) {
        obj_buffer[0xbd] = (obj_buffer[0xbd] & 0xc0) | ((tRC_val - 3) & 0x3f);
    }

    // obj[0xb3] = tWTR cycles (from obj[0x1a8])
    uint32_t tWTR_ddr2_ps = *(uint32_t *)(obj_buffer + 0x1a8);
    uint32_t tWTR_ddr2_cycles = PS2CYCLE(tWTR_ddr2_ps);
    obj_buffer[0xb3] = (obj_buffer[0xb3] & 0xc0) | (tWTR_ddr2_cycles & 0x3f);

    // obj[0xb7] bits[2:0] = 0 (clear lower 3 bits)
    obj_buffer[0xb7] = obj_buffer[0xb7] & 0xf8;

    // obj[0xb8] bits[1:0] = (tRFC cycles - 1) & 3
    uint32_t tRFC_ps = *(uint32_t *)(obj_buffer + 0x1b0);
    uint32_t tRFC_cycles = PS2CYCLE(tRFC_ps);
    uint8_t tRFC_bits = ((tRFC_cycles - 1) & 3);
    obj_buffer[0xb8] = (obj_buffer[0xb8] & 0xfc) | tRFC_bits;

    // obj[0xc3] = max(tRAS, tRC) / 4
    uint32_t tRAS_ddr2_ps = *(uint32_t *)(obj_buffer + 0x194);
    uint32_t tRC_ddr2_ps = *(uint32_t *)(obj_buffer + 0x1a0);
    uint32_t tRAS_div4 = PS2CYCLE_DIV(tRAS_ddr2_ps, 4);
    uint32_t tRC_div4 = PS2CYCLE_DIV(tRC_ddr2_ps, 4);
    uint32_t max_val = (tRAS_div4 > tRC_div4) ? tRAS_div4 : tRC_div4;
    obj_buffer[0xc3] = (uint8_t)max_val;

    // obj[0xb9] bits[3:0] = (tRFC / 8 - 1) & 0xf
    uint32_t tRFC_div8 = PS2CYCLE_DIV(tRFC_ps, 8);
    uint8_t tRFC_div8_bits = ((tRFC_div8 - 1) & 0xf);
    obj_buffer[0xb9] = (obj_buffer[0xb9] & 0xf0) | tRFC_div8_bits;

    // obj[0xc2] = tRRD cycles (from obj[0x1ac])
    uint32_t tRRD_ddr2_ps = *(uint32_t *)(obj_buffer + 0x1ac);
    uint32_t tRRD_ddr2_cycles = PS2CYCLE(tRRD_ddr2_ps);
    obj_buffer[0xc2] = (obj_buffer[0xc2] & 0xc0) | (tRRD_ddr2_cycles & 0x3f);

    return 0;
}

