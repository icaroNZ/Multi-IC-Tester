/**
 * PinConfig.h
 *
 * Centralized pin definitions for Multi-IC Tester
 *
 * All pin assignments are defined here to avoid magic numbers throughout code.
 * Pin assignments match the hardware pinout documented in:
 * Documents/Multi-IC_Tester_Pinout.md
 *
 * Design: Uses constexpr for compile-time constants (type-safe, zero overhead)
 *
 * Usage:
 *   #include "hardware/PinConfig.h"
 *   pinMode(RESET_PIN, OUTPUT);
 *   digitalWrite(RESET_PIN, HIGH);
 */

#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include <Arduino.h>

//=============================================================================
// ADDRESS BUS (A0-A15) - Shared across all ICs
//=============================================================================

// Address bus low byte (A0-A7) - PORTA
constexpr uint8_t ADDR_A0_PIN  = 22;  // PA0
constexpr uint8_t ADDR_A1_PIN  = 23;  // PA1
constexpr uint8_t ADDR_A2_PIN  = 24;  // PA2
constexpr uint8_t ADDR_A3_PIN  = 25;  // PA3
constexpr uint8_t ADDR_A4_PIN  = 26;  // PA4
constexpr uint8_t ADDR_A5_PIN  = 27;  // PA5
constexpr uint8_t ADDR_A6_PIN  = 28;  // PA6
constexpr uint8_t ADDR_A7_PIN  = 29;  // PA7

// Address bus high byte (A8-A15) - PORTC
constexpr uint8_t ADDR_A8_PIN  = 37;  // PC0
constexpr uint8_t ADDR_A9_PIN  = 36;  // PC1
constexpr uint8_t ADDR_A10_PIN = 35;  // PC2
constexpr uint8_t ADDR_A11_PIN = 34;  // PC3
constexpr uint8_t ADDR_A12_PIN = 33;  // PC4
constexpr uint8_t ADDR_A13_PIN = 32;  // PC5
constexpr uint8_t ADDR_A14_PIN = 31;  // PC6
constexpr uint8_t ADDR_A15_PIN = 30;  // PC7

//=============================================================================
// DATA BUS (D0-D7) - Shared across all ICs, bidirectional
//=============================================================================

// Data bus (D0-D7) - PORTL
constexpr uint8_t DATA_D0_PIN = 49;  // PL0
constexpr uint8_t DATA_D1_PIN = 48;  // PL1
constexpr uint8_t DATA_D2_PIN = 47;  // PL2
constexpr uint8_t DATA_D3_PIN = 46;  // PL3
constexpr uint8_t DATA_D4_PIN = 45;  // PL4
constexpr uint8_t DATA_D5_PIN = 44;  // PL5
constexpr uint8_t DATA_D6_PIN = 43;  // PL6
constexpr uint8_t DATA_D7_PIN = 42;  // PL7

//=============================================================================
// CLOCK SIGNALS
//=============================================================================

constexpr uint8_t CLOCK_PIN = 5;     // PE3 (Timer3 OC3A) - CPU clock output
constexpr uint8_t PHI1_PIN  = 21;    // PD0 - 6502 Φ1 output (monitor only)
constexpr uint8_t PHI2_PIN  = 20;    // PD1 - 6502 Φ2 output (monitor only)

//=============================================================================
// RESET SIGNAL - Shared between Z80 and 6502
//=============================================================================

constexpr uint8_t RESET_PIN = 9;     // PH6 - /RESET (Z80) / RES (6502)

//=============================================================================
// CONTROL SIGNALS - Read/Write Operations
// NOTE: Different meanings for different ICs!
//=============================================================================

constexpr uint8_t CTRL_MREQ_CS_PIN = 41;   // PG0 - Z80 /MREQ, SRAM /CS
constexpr uint8_t CTRL_IORQ_PIN    = 40;   // PG1 - Z80 /IORQ only

// ⚠️ CRITICAL: PG2 has INVERTED logic between ICs!
// Z80:   /RD (LOW = read)
// 6502:  R/W (HIGH = read)  ← OPPOSITE!
// SRAM:  /OE (LOW = output enable)
constexpr uint8_t CTRL_RD_RW_OE_PIN = 39;  // PG2

// 6502 does not use /WR (R/W handles both)
constexpr uint8_t CTRL_WR_WE_PIN = 38;     // PG3 - Z80 /WR, SRAM /WE

//=============================================================================
// CONTROL SIGNALS - Wait/Ready
//=============================================================================

// ⚠️ CRITICAL: PB4 has INVERTED logic between ICs!
// Z80:  /WAIT (LOW = wait)
// 6502: RDY (HIGH = ready)  ← OPPOSITE!
constexpr uint8_t CTRL_WAIT_RDY_PIN = 10;  // PB4

//=============================================================================
// CONTROL SIGNALS - Interrupts
//=============================================================================

constexpr uint8_t CTRL_INT_IRQ_PIN = 11;   // PB5 - /INT (Z80) / IRQ (6502)
constexpr uint8_t CTRL_NMI_PIN     = 12;   // PB6 - /NMI (Z80 and 6502)

//=============================================================================
// CONTROL SIGNALS - Status/Sync
//=============================================================================

// ⚠️ CRITICAL: PH3 has INVERTED logic between ICs!
// Z80:  /M1 (LOW = opcode fetch)
// 6502: SYNC (HIGH = opcode fetch)  ← OPPOSITE!
constexpr uint8_t CTRL_M1_SYNC_PIN = 6;    // PH3

constexpr uint8_t CTRL_HALT_PIN = 2;       // PE4 - Z80 /HALT only

//=============================================================================
// CONTROL SIGNALS - Z80 Specific (not used for 6502/SRAM)
//=============================================================================

constexpr uint8_t CTRL_RFSH_PIN   = 7;     // PH4 - Z80 /RFSH (DRAM refresh)
constexpr uint8_t CTRL_BUSACK_PIN = 8;     // PH5 - Z80 /BUSACK
constexpr uint8_t CTRL_BUSREQ_PIN = 13;    // PB7 - Z80 /BUSREQ (DMA)

//=============================================================================
// CONTROL SIGNALS - 6502 Specific (not used for Z80/SRAM)
//=============================================================================

constexpr uint8_t CTRL_SO_PIN = 18;        // PD3 - 6502 S.O. (Set Overflow)

//=============================================================================
// PORT REGISTER ALIASES
// For performance-critical code, use direct port manipulation
//=============================================================================

// These are already defined by Arduino, but listed here for reference:
// PORTA - Address A0-A7 (pins 22-29)
// PORTC - Address A8-A15 (pins 30-37)
// PORTL - Data D0-D7 (pins 42-49)
// PORTG - Control signals (pins 38-41)
// PORTH - Control signals (pins 6-9)
// PORTB - Control signals (pins 10-13)
// PORTE - Clock and /HALT (pins 2, 5)
// PORTD - 6502 specific (pins 18, 20-21)

//=============================================================================
// IMPORTANT NOTES
//=============================================================================

/*
 * SIGNAL INVERSIONS - MUST handle in firmware!
 *
 * PG2 (pin 39):
 *   - Z80:   /RD (LOW = read)
 *   - 6502:  R/W (HIGH = read)  ← INVERTED!
 *   - SRAM:  /OE (LOW = enable)
 *
 * PH3 (pin 6):
 *   - Z80:   /M1 (LOW = fetch)
 *   - 6502:  SYNC (HIGH = fetch)  ← INVERTED!
 *
 * PB4 (pin 10):
 *   - Z80:   /WAIT (LOW = wait)
 *   - 6502:  RDY (HIGH = ready)  ← INVERTED!
 *
 * HARDWARE PULL-UPS (10kΩ to +5V):
 *   - PH6 (RESET)
 *   - PB4 (WAIT/RDY)
 *   - PB5 (INT/IRQ)
 *   - PB6 (NMI)
 *   - PB7 (BUSREQ)
 */

#endif // PIN_CONFIG_H
