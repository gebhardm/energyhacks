/*
 Control Commands of HopeRF RFM01 modules
 See http://www.hoperf.com/upload/rf/RFM01.pdf for details
*/

// Configuration setting command
#define RFM01_CFG 0x8000
#define RFM01_B1 12   // frequency band: 00:315, 01:433, 10:868, 11:915 MHz
#define RFM01_B0 11
#define RFM01_EB 10   // enable low battery detect
#define RFM01_ET 9    // enable wake-up timer
#define RFM01_EX 8    // enable crystal during sleep
#define RFM01_X3 7    // crystal load cap: 0:8.5pF .. F:16pF in 0.5pF steps
#define RFM01_X2 6
#define RFM01_X1 5
#define RFM01_X0 4
#define RFM01_I2 3    // bandwidth: 001:400 .. 110:67 kHz in ~60kHz steps
#define RFM01_I1 2
#define RFM01_I0 1
#define RFM01_DC 0    // disable clock output


// Frequency setting command
#define RFM01_FRQ 0xA000
#define RFM01_F11 11  // F = 4 * (f0 (kHz) - 430000) / 10 for 433 MHz band
#define RFM01_F10 10
#define RFM01_F09 9
#define RFM01_F08 8
#define RFM01_F07 7
#define RFM01_F06 6
#define RFM01_F05 5
#define RFM01_F04 4
#define RFM01_F03 3
#define RFM01_F02 2
#define RFM01_F01 1
#define RFM01_F00 0

// Receiver setting command
#define RFM01_REC 0xC000
#define RFM01_D1 7
#define RFM01_D0 6
#define RFM01_G1 5
#define RFM01_G0 4
#define RFM01_REC_R2 3
#define RFM01_REC_R1 2
#define RFM01_REC_R0 1
#define RFM01_EN 0

// Wake-up timer command
#define RFM01_WUT 0xE000
#define RFM01_WUT_R4 12
#define RFM01_WUT_R3 11
#define RFM01_WUT_R2 10
#define RFM01_WUT_R1 9
#define RFM01_WUT_R0 8
#define RFM01_M7 7
#define RFM01_M6 6
#define RFM01_M5 5
#define RFM01_M4 4
#define RFM01_M3 3
#define RFM01_M2 2
#define RFM01_M1 1
#define RFM01_M0 0

// Low duty-cycle command
#define RFM01_LDC 0xCC00
#define RFM01_LDC_D6 7
#define RFM01_LDC_D5 6
#define RFM01_LDC_D4 5
#define RFM01_LDC_D3 4
#define RFM01_LDC_D2 3
#define RFM01_LDC_D1 2
#define RFM01_LDC_D0 1
#define RFM01_LDC_EN 0

// Low battery detector and clock divider command
#define RFM01_BCD 0xC200
#define RFM01_BCD_D2 7
#define RFM01_BCD_D1 6
#define RFM01_BCD_D0 5
#define RFM01_BCD_T4 4
#define RFM01_BCD_T3 3
#define RFM01_BCD_T2 2
#define RFM01_BCD_T1 1
#define RFM01_BCD_T0 0

// AFC control command
#define RFM01_AFC 0xC600
#define RFM01_A1 7
#define RFM01_A0 6
#define RFM01_RL1 5
#define RFM01_RL0 4
#define RFM01_ST 3
#define RFM01_FI 2
#define RFM01_OE 1
#define RFM01_AFC_EN 0

// Data filter command
#define RFM01_DF 0xC420
#define RFM01_AL 7
#define RFM01_ML 6
#define RFM01_S1 4
#define RFM01_S0 3
#define RFM01_DF_F2 2
#define RFM01_DF_F1 1
#define RFM01_DF_F0 0

// Data rate command
#define RFM01_DR 0xC800
#define RFM01_DR_CS 7
#define RFM01_DR_R6 6
#define RFM01_DR_R5 5
#define RFM01_DR_R4 4
#define RFM01_DR_R3 3
#define RFM01_DR_R2 2
#define RFM01_DR_R1 1
#define RFM01_DR_R0 0

// Output and fifo command
#define RFM01_OF 0xCE00
#define RFM01_OF_F3 7
#define RFM01_OF_F2 6
#define RFM01_OF_F1 5
#define RFM01_OF_F0 4
#define RFM01_OF_S1 3
#define RFM01_OF_S0 2
#define RFM01_FF 1
#define RFM01_FE 0

// Reset mode command
#define RFM01_RES 0xDA00
#define RFM01_DR 0

// Status read command
#define RFM01_RD 0x0000
