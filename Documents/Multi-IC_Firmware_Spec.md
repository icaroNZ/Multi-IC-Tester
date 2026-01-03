# Multi-IC Tester - Firmware Specification

## Document Purpose

This document provides a complete specification for an AI to implement firmware for a multi-IC tester that can test three different integrated circuits using a single Arduino Mega 2560:
- **Z80 CPU** (40-pin DIP)
- **6502 CPU** (40-pin DIP)
- **HM62256 SRAM** and compatible memory ICs (28-pin DIP)

The firmware enables UART-based control, automatic IC testing, and detailed progress reporting.

---

## Project Overview

### What This Project Does

The Multi-IC Tester is a hardware tester that validates the functionality of three different IC types using shared hardware resources (address bus, data bus, control signals). The Arduino Mega 2560 acts as:
- **Bus controller** - Drives address and data buses
- **Test orchestrator** - Runs automated test sequences
- **Interface** - Communicates with user via UART (115200 baud)
- **Signal generator** - Provides clock signals and control signals
- **Monitor** - Reads IC responses and validates behavior

### Key Design Principle

**ONE IC tested at a time.** Only one IC socket should have a chip installed during testing. Other sockets must be empty. The firmware reconfigures Arduino pins based on which IC is being tested.

### Hardware Platform

- **Microcontroller:** Arduino Mega 2560 (ATmega2560)
- **Communication:** UART at 115200 baud, 8N1
- **Power:** 5V logic levels for all ICs
- **Pin Configuration:** See `Multi-IC_Tester_Pinout.md` for complete pin mapping

### Testing Philosophy

**Simplified, automatic testing:**
- User selects IC type via MODE command
- Firmware automatically reconfigures pins
- User triggers TEST command
- Firmware runs comprehensive automatic tests
- Detailed progress reported via UART
- Final PASS/FAIL result with diagnostic details

---

## Hardware Architecture Summary

### Complete Pin Mapping Reference

**CRITICAL:** Refer to `Documents/Others/Multi-IC_Tester_Pinout.md` for:
- Complete pin-to-pin connections for all three ICs
- Shared bus mappings (address A0-A15, data D0-D7)
- Control signal assignments
- Signal logic inversions (very important!)
- Pull-up resistor locations

### Shared Buses

**Address Bus (A0-A15):**
- Arduino PORTA (pins 22-29) → A0-A7
- Arduino PORTC (pins 30-37) → A8-A15
- Direction: OUTPUT (Arduino drives all ICs)

**Data Bus (D0-D7):**
- Arduino PORTL (pins 42-49) → D0-D7
- Direction: BIDIRECTIONAL (controlled by DDRL register)
  - INPUT when reading from IC
  - OUTPUT when writing to IC

**Clock Signal:**
- Arduino PE3 (pin 5, Timer3 OC3A) → Z80 CLK, 6502 Φ0
- Hardware PWM output, configurable frequency

**Reset Signal:**
- Arduino PH6 (pin 9) → Z80 /RESET, 6502 RES
- Has 10kΩ pull-up to +5V
- Direction: OUTPUT (Arduino drives both CPUs)

### Critical Signal Inversions

**⚠️ IMPORTANT:** Some signals have inverted logic between ICs!

| Arduino Pin | Z80 Logic | 6502 Logic | Firmware Must |
|-------------|-----------|------------|---------------|
| **PG2 (39)** | /RD (LOW=read) | R/W (HIGH=read) | **Invert logic for 6502** |
| **PH3 (6)** | /M1 (LOW=fetch) | SYNC (HIGH=fetch) | **Invert logic for 6502** |
| **PB4 (10)** | /WAIT (LOW=wait) | RDY (HIGH=ready) | **Invert logic for 6502** |

### Pull-Up Resistors (10kΩ to +5V)

These pins have hardware pull-ups:
- PH6 (pin 9) - /RESET / RES
- PB4 (pin 10) - /WAIT / RDY
- PB5 (pin 11) - /INT / IRQ
- PB6 (pin 12) - /NMI
- PB7 (pin 13) - /BUSREQ (Z80 only)

All pull-ups are compatible across ICs (verified in pinout document).

### 6502-Specific Pins (New)

| Arduino Pin | Direction | 6502 Signal | Purpose |
|-------------|-----------|-------------|---------|
| **PD0 (21)** | INPUT | Φ1 (pin 3) | Monitor phase 1 clock output |
| **PD1 (20)** | INPUT | Φ2 (pin 39) | Monitor phase 2 clock output |
| **PD3 (18)** | OUTPUT | S.O. (pin 38) | Set Overflow flag control |

---

## UART Communication Protocol

### Serial Configuration

```
Baud Rate: 115200
Data Bits: 8
Parity: None
Stop Bits: 1
Flow Control: None
```

### Command Format

Commands are **line-based**, terminated by `\n` (newline) or `\r\n` (carriage return + newline).

**Format:**
```
COMMAND [PARAMETER1] [PARAMETER2] ...\n
```

**Rules:**
- Commands are **case-sensitive** (use UPPERCASE)
- Parameters separated by spaces
- Empty lines ignored
- Leading/trailing whitespace trimmed

### Response Format

All responses are human-readable ASCII text, terminated by newline.

**Success Response:**
```
OK: [message]\n
```

**Error Response:**
```
ERROR: [error description]\n
```

**Progress Updates:**
```
[test step description]\n
```

**Final Result:**
```
RESULT: PASS\n
or
RESULT: FAIL - [failure reason]\n
```

### Available Commands

| Command | Parameters | Description | Example |
|---------|------------|-------------|---------|
| `MODE` | `Z80` \| `6502` \| `62256` | Select IC type and reconfigure pins | `MODE Z80` |
| `TEST` | None | Run automatic test for selected IC | `TEST` |
| `STATUS` | None | Show current mode and system info | `STATUS` |
| `HELP` | None | Display command list | `HELP` |
| `RESET` | None | Reset the selected IC | `RESET` |

---

## MODE Command - IC Selection

### Command Syntax

```
MODE <IC_TYPE>
```

Where `<IC_TYPE>` is one of:
- `Z80` - Configure for Z80 CPU testing
- `6502` - Configure for 6502 CPU testing
- `62256` - Configure for HM62256 SRAM testing

### Example Usage

```
User: MODE Z80
Arduino: OK: Switched to Z80 mode
         Pins reconfigured for Z80 CPU
         Clock: 1 MHz (default)
         Ready for testing

User: MODE 6502
Arduino: OK: Switched to 6502 mode
         Pins reconfigured for 6502 CPU
         Clock: 1 MHz (default)
         Ready for testing

User: MODE 62256
Arduino: OK: Switched to 62256 mode
         Pins reconfigured for SRAM testing
         No clock (direct memory access)
         Ready for testing
```

### Pin Reconfiguration Requirements

When MODE command is executed, firmware must reconfigure Arduino pins based on the selected IC. See **Pin Reconfiguration Details** section below for specifics.

**High-level steps:**
1. Parse MODE command parameter
2. Validate parameter (Z80, 6502, or 62256)
3. Stop any running tests/clocks
4. Reconfigure pin directions (DDR registers)
5. Set default pin states
6. Configure clock (if needed)
7. Store current mode in global state
8. Send confirmation message

### Mode Persistence

The selected mode persists until:
- Different MODE command is issued
- Arduino is reset/powered off

---

## TEST Command - Automatic Testing

### Command Syntax

```
TEST
```

No parameters. Runs the automatic test suite for the currently selected IC mode.

### Behavior

1. **Check Prerequisites:**
   - Verify a mode has been selected (MODE command issued)
   - If no mode set, return `ERROR: No IC mode selected. Use MODE command first`

2. **Run Tests:**
   - Execute test sequence for selected IC type
   - Report detailed progress during testing
   - Track pass/fail for each test step

3. **Report Results:**
   - Show summary of all test results
   - Final PASS or FAIL status
   - Diagnostic information if any test failed

### Progress Reporting Format

During testing, firmware should output detailed progress:

```
Test 1/5: [Description]
  Step: [What is being tested]
  Result: [Intermediate values or status]
  Status: PASS / FAIL

Test 2/5: [Description]
  ...
```

### Example Output

```
User: TEST
Arduino: Starting Z80 CPU tests...

         Test 1/5: Control Signal Test
           Step: Checking /MREQ, /IORQ, /RD, /WR levels
           Result: All signals at expected idle state (HIGH)
           Status: PASS

         Test 2/5: Clock Generation
           Step: Starting 100 kHz clock
           Result: Clock running at 100.2 kHz
           Status: PASS

         Test 3/5: Simple Instruction Execution
           Step: Loading JP 0000 program
           Step: Starting Z80
           Step: Monitoring address bus
           Result: PC cycling 0000→0001→0002→0000 (correct)
           Status: PASS

         Test 4/5: Memory Write Test
           Step: Writing test pattern to RAM
           Result: Wrote 16 bytes at 0x1000
           Status: PASS

         Test 5/5: Memory Read Test
           Step: Reading back test pattern
           Result: All 16 bytes match expected values
           Status: PASS

         ===================================
         RESULT: PASS
         All Z80 tests completed successfully
         ===================================
```

---

## Z80 CPU Testing (Moderate Complexity)

### Test Objective

Verify that a Z80 CPU can:
1. Respond to control signals
2. Execute simple instructions
3. Access memory (read and write)
4. Cycle through addresses correctly

### Test Sequence

**Test 1: Control Signal Verification**
- Check that /MREQ, /IORQ, /RD, /WR are at idle levels (HIGH) when Z80 is held in reset
- Verify /HALT is HIGH (not halted)

**Test 2: Clock Generation**
- Configure Timer3 to generate 100 kHz clock (safe low frequency)
- Start clock output on PE3
- Verify clock is running (can check with counter or delay)

**Test 3: Simple Instruction Execution**
- Load a 3-byte program into ROM emulation: `C3 00 00` (JP 0x0000 - infinite loop)
- Release Z80 from reset
- Monitor address bus for ~100-1000 clock cycles
- Verify address cycles: 0x0000 → 0x0001 → 0x0002 → 0x0000 (repeating)
- Verify /M1 goes LOW during opcode fetch at 0x0000
- Verify /MREQ and /RD go LOW during memory reads

**Test 4: Memory Write Test**
- Load a program that writes test pattern to RAM:
  ```
  LD A, 0x55       ; 3E 55
  LD (0x1000), A   ; 32 00 10
  LD A, 0xAA       ; 3E AA
  LD (0x1001), A   ; 32 01 10
  HALT             ; 76
  ```
  Full program: `3E 55 32 00 10 3E AA 32 01 10 76` (11 bytes at 0x0000)
- Run Z80
- Wait for HALT signal (PE4 goes LOW)
- Check that RAM emulation received writes at 0x1000 and 0x1001
- Verify /WR signal was activated during writes

**Test 5: Memory Read Test**
- Verify RAM[0x1000] = 0x55
- Verify RAM[0x1001] = 0xAA
- (These were written by Test 4)

### Expected Results

**PASS Criteria:**
- All control signals at correct idle levels
- Clock running at approximately 100 kHz
- Address bus cycles through JP loop correctly
- Memory write operations detected
- Correct values written to RAM and read back

**FAIL Criteria:**
- No response from Z80 (address bus stuck at 0x0000 or 0xFFFF)
- Incorrect address sequencing
- Control signals not toggling
- Memory write/read values incorrect

### ROM and RAM Emulation for Z80

**Memory Map:**
- ROM: 0x0000 - 0x0FFF (4KB) - Store loaded program here
- RAM: 0x1000 - 0x17FF (2KB) - Emulate read/write memory
- Unmapped: 0x1800 - 0xFFFF - Return 0xFF on reads

**Bus Cycle Handler:**
Firmware must monitor Z80 bus cycles and respond:
- When /MREQ LOW and /RD LOW: Output data from ROM/RAM to data bus
- When /MREQ LOW and /WR LOW: Read data from data bus, store in RAM
- Switch data bus direction appropriately (DDRL register)
- Use /WAIT signal if needed to give Arduino time to respond

### Test Output Example

```
Starting Z80 CPU tests...

Test 1/5: Control Signal Test
  Step: Holding Z80 in reset
  Step: Reading control signals
  Result: /MREQ=HIGH, /IORQ=HIGH, /RD=HIGH, /WR=HIGH, /HALT=HIGH
  Status: PASS

Test 2/5: Clock Generation
  Step: Configuring Timer3 for 100 kHz
  Step: Starting clock on pin 5 (PE3)
  Result: Clock started successfully
  Status: PASS

Test 3/5: Simple Instruction Execution (JP Loop)
  Step: Loading program: C3 00 00 (JP 0x0000)
  Step: Releasing reset
  Step: Monitoring address bus for 500 cycles
  Result: Address sequence: 0000→0001→0002→0000 (repeated 166 times)
  Result: /M1 toggled 166 times (opcode fetch detected)
  Status: PASS

Test 4/5: Memory Write Test
  Step: Loading program: 3E 55 32 00 10 3E AA 32 01 10 76
  Step: Running program until HALT
  Result: HALT detected after ~450 cycles
  Result: Write to 0x1000 detected, value=0x55
  Result: Write to 0x1001 detected, value=0xAA
  Status: PASS

Test 5/5: Memory Read Verification
  Step: Reading RAM[0x1000]
  Result: Value = 0x55 (expected 0x55) ✓
  Step: Reading RAM[0x1001]
  Result: Value = 0xAA (expected 0xAA) ✓
  Status: PASS

===================================
RESULT: PASS
All Z80 tests completed successfully
CPU is functioning correctly
===================================
```

---

## 6502 CPU Testing (Moderate Complexity)

### Test Objective

Verify that a 6502 CPU can:
1. Respond to control signals
2. Generate clock outputs (Φ1, Φ2)
3. Execute simple instructions
4. Access memory (read and write)

### Test Sequence

**Test 1: Control Signal Verification**
- Hold 6502 in reset (RES pin LOW)
- Verify R/W is HIGH (read state)
- Verify SYNC is LOW (not syncing)

**Test 2: Clock Generation and Monitoring**
- Configure Timer3 to generate 100 kHz clock on PE3 → 6502 Φ0 input
- Release 6502 from reset
- Monitor PD0 (Φ1 output) and PD1 (Φ2 output)
- Verify both Φ1 and Φ2 are toggling
- Count transitions over a known time period to estimate frequency
- Expected: Φ1 and Φ2 should toggle at the same frequency as Φ0 input

**Test 3: Simple Instruction Execution**
- Load a 3-byte program: `4C 00 00` (JMP $0000 - infinite loop)
- Run 6502
- Monitor address bus and SYNC signal
- Verify address cycles through: 0x0000 → 0x0001 → 0x0002 → 0x0000
- Verify SYNC goes HIGH during opcode fetch at 0x0000

**Test 4: Memory Write Test**
- Load a program that writes test pattern:
  ```
  LDA #$55         ; A9 55
  STA $1000        ; 8D 00 10
  LDA #$AA         ; A9 AA
  STA $1001        ; 8D 01 10
  JMP $0000        ; 4C 00 00 (loop, no HALT on 6502)
  ```
  Full program: `A9 55 8D 00 10 A9 AA 8D 01 10 4C 00 00` (14 bytes)
- Run for sufficient cycles to complete writes
- Detect writes by monitoring R/W signal (LOW during write)
- Verify writes to 0x1000 and 0x1001

**Test 5: Memory Read Test**
- Verify RAM[0x1000] = 0x55
- Verify RAM[0x1001] = 0xAA

### 6502-Specific Considerations

**⚠️ Signal Inversions:**
- **R/W (PG2):** HIGH = read, LOW = write (OPPOSITE of Z80 /RD, /WR)
- **SYNC (PH3):** HIGH = opcode fetch (OPPOSITE of Z80 /M1)
- **RDY (PB4):** HIGH = ready, LOW = pause (OPPOSITE of Z80 /WAIT)

**No HALT Instruction:**
- 6502 does not have a HALT instruction
- Test programs should loop indefinitely
- Stop testing after a fixed number of cycles or time period

**Clock Outputs:**
- 6502 generates Φ1 and Φ2 outputs based on Φ0 input
- These should be monitored to verify internal clock circuitry works
- Φ1 and Φ2 are non-overlapping phases

### ROM and RAM Emulation for 6502

**Memory Map:**
- ROM: 0x0000 - 0x0FFF (4KB) - Store loaded program
- RAM: 0x1000 - 0x17FF (2KB) - Emulate read/write memory
- Reset Vector: 0xFFFC-0xFFFD should point to 0x0000 (start address)
  - Return 0x00 for 0xFFFC, 0x00 for 0xFFFD

**Bus Cycle Handler:**
- When R/W HIGH and address valid: Output data from ROM/RAM
- When R/W LOW and address valid: Read data from data bus, store in RAM
- Note inverted logic compared to Z80!

### Test Output Example

```
Starting 6502 CPU tests...

Test 1/5: Control Signal Test
  Step: Holding 6502 in reset (RES=LOW)
  Step: Reading control signals
  Result: R/W=HIGH (read mode), SYNC=LOW (not syncing)
  Status: PASS

Test 2/5: Clock Generation and Output Monitoring
  Step: Configuring Timer3 for 100 kHz clock
  Step: Starting Φ0 clock on pin 5 (PE3)
  Step: Releasing reset
  Step: Monitoring Φ1 (pin 21, PD0) and Φ2 (pin 20, PD1)
  Result: Φ1 toggling detected: 1024 transitions in 10.24ms (~100kHz)
  Result: Φ2 toggling detected: 1024 transitions in 10.24ms (~100kHz)
  Status: PASS

Test 3/5: Simple Instruction Execution (JMP Loop)
  Step: Loading program: 4C 00 00 (JMP $0000)
  Step: Running 6502
  Step: Monitoring address bus and SYNC for 500 cycles
  Result: Address sequence: 0000→0001→0002→0000 (repeated 166 times)
  Result: SYNC went HIGH 166 times (opcode fetch detected)
  Status: PASS

Test 4/5: Memory Write Test
  Step: Loading program: A9 55 8D 00 10 A9 AA 8D 01 10 4C 00 00
  Step: Running for 1000 cycles
  Result: Write to 0x1000 detected (R/W=LOW), value=0x55
  Result: Write to 0x1001 detected (R/W=LOW), value=0xAA
  Status: PASS

Test 5/5: Memory Read Verification
  Step: Reading RAM[0x1000]
  Result: Value = 0x55 (expected 0x55) ✓
  Step: Reading RAM[0x1001]
  Result: Value = 0xAA (expected 0xAA) ✓
  Status: PASS

===================================
RESULT: PASS
All 6502 tests completed successfully
CPU is functioning correctly
Clock outputs verified
===================================
```

---

## HM62256 SRAM Testing (Comprehensive)

### Test Objective

Thoroughly test SRAM functionality:
1. Verify all address lines working
2. Verify all data lines working
3. Test read/write operations
4. Detect address line failures (stuck bits, shorts)
4. Support different memory sizes (32KB, 64KB, 128KB)

### Memory IC Compatibility

**Supported ICs with same 28-pin pinout:**
- **HM62256** - 32KB SRAM (32K × 8)
- **62256** - 32KB SRAM (generic)
- **27C256** - 32KB EPROM (read-only, can test reads)
- **27C512** - 64KB EPROM (uses A15)
- **28C256** - 32KB EEPROM
- Other pin-compatible memory ICs

**Address Range Detection:**
Firmware should support configurable memory size. For automatic detection:
- Default: Test 32KB (0x0000 - 0x7FFF, 15 address bits A0-A14)
- Extended: If A15 available, can test up to 64KB

For this spec, focus on **HM62256 (32KB)** but design to be extensible.

### Test Sequence

**Test 1: Basic Read/Write Test**
- Write 0x55 to address 0x0000
- Read back from 0x0000
- Verify value = 0x55
- Write 0xAA to address 0x0000
- Read back from 0x0000
- Verify value = 0xAA
- This tests basic chip functionality

**Test 2: Walking 1s Test - Address Lines**
Test each address line individually:
- Write unique values to addresses with single bit set:
  - 0x0001 (A0) → write 0x01
  - 0x0002 (A1) → write 0x02
  - 0x0004 (A2) → write 0x04
  - 0x0008 (A3) → write 0x08
  - ... up to 0x4000 (A14)
- Read back all addresses
- Verify each value is correct
- Detects stuck address lines or address line shorts

**Test 3: Walking 1s Test - Data Lines**
Test each data line individually at address 0x0000:
- Write and verify: 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
- Write and verify: 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F
- Detects stuck data lines

**Test 4: Checkerboard Pattern (Full Range)**
- Write checkerboard pattern across entire 32KB:
  - Even addresses: 0x55
  - Odd addresses: 0xAA
- Read back entire range and verify
- Tests for pattern sensitivity and address decoding

**Test 5: Inverse Checkerboard Pattern**
- Write inverse checkerboard:
  - Even addresses: 0xAA
  - Odd addresses: 0x55
- Read back and verify
- Tests for data retention and pattern interference

**Test 6: Address = Data Test**
- For each address, write the address value itself
  - Address 0x0000 → write 0x00
  - Address 0x0001 → write 0x01
  - Address 0x0123 → write 0x23 (low byte)
  - Address 0x7FFF → write 0xFF
- Read back all 32KB and verify pattern
- Detects complex address/data interactions

**Test 7: Random Pattern (Optional but Recommended)**
- Generate pseudo-random data using simple PRNG
- Write to random addresses
- Read back and verify
- Good for catching subtle timing issues

### SRAM Control Signals

**Direct Arduino Control** (no CPU involved):
- **/CS (PG0):** Chip Select - Drive LOW to enable chip
- **/OE (PG2):** Output Enable - Drive LOW to read from chip
- **/WE (PG3):** Write Enable - Drive LOW to write to chip

**Read Cycle:**
1. Set address on bus (PORTA/PORTC)
2. Set data bus to INPUT (DDRL = 0x00)
3. Assert /CS (LOW)
4. Assert /OE (LOW)
5. Wait for access time (~70-100ns, add safety margin)
6. Read data from PINL
7. Deassert /OE (HIGH)
8. Deassert /CS (HIGH)

**Write Cycle:**
1. Set address on bus (PORTA/PORTC)
2. Set data bus to OUTPUT (DDRL = 0xFF)
3. Put data on bus (PORTL = data)
4. Assert /CS (LOW)
5. Assert /WE (LOW)
6. Wait for write pulse width (~50ns minimum, add margin)
7. Deassert /WE (HIGH)
8. Deassert /CS (HIGH)
9. Set data bus back to INPUT (safe state)

### Test Output Example

```
Starting HM62256 SRAM tests...
Detected IC: 28-pin SRAM
Testing address range: 0x0000 - 0x7FFF (32KB)

Test 1/7: Basic Read/Write Test
  Step: Writing 0x55 to address 0x0000
  Step: Reading back from 0x0000
  Result: Read 0x55 (expected 0x55) ✓
  Step: Writing 0xAA to address 0x0000
  Step: Reading back from 0x0000
  Result: Read 0xAA (expected 0xAA) ✓
  Status: PASS

Test 2/7: Walking 1s - Address Lines
  Step: Testing 15 address lines (A0-A14)
  Step: Writing unique values to single-bit addresses
  Result: A0  (0x0001) → wrote 0x01, read 0x01 ✓
  Result: A1  (0x0002) → wrote 0x02, read 0x02 ✓
  Result: A2  (0x0004) → wrote 0x04, read 0x04 ✓
  Result: A3  (0x0008) → wrote 0x08, read 0x08 ✓
  Result: A4  (0x0010) → wrote 0x10, read 0x10 ✓
  Result: A5  (0x0020) → wrote 0x20, read 0x20 ✓
  Result: A6  (0x0040) → wrote 0x40, read 0x40 ✓
  Result: A7  (0x0080) → wrote 0x80, read 0x80 ✓
  Result: A8  (0x0100) → wrote 0x01, read 0x01 ✓
  Result: A9  (0x0200) → wrote 0x02, read 0x02 ✓
  Result: A10 (0x0400) → wrote 0x04, read 0x04 ✓
  Result: A11 (0x0800) → wrote 0x08, read 0x08 ✓
  Result: A12 (0x1000) → wrote 0x10, read 0x10 ✓
  Result: A13 (0x2000) → wrote 0x20, read 0x20 ✓
  Result: A14 (0x4000) → wrote 0x40, read 0x40 ✓
  Status: PASS

Test 3/7: Walking 1s - Data Lines
  Step: Testing 8 data lines at address 0x0000
  Result: D0 (0x01) → wrote 0x01, read 0x01 ✓
  Result: D1 (0x02) → wrote 0x02, read 0x02 ✓
  Result: D2 (0x04) → wrote 0x04, read 0x04 ✓
  Result: D3 (0x08) → wrote 0x08, read 0x08 ✓
  Result: D4 (0x10) → wrote 0x10, read 0x10 ✓
  Result: D5 (0x20) → wrote 0x20, read 0x20 ✓
  Result: D6 (0x40) → wrote 0x40, read 0x40 ✓
  Result: D7 (0x80) → wrote 0x80, read 0x80 ✓
  Result: Inverse patterns all passed ✓
  Status: PASS

Test 4/7: Checkerboard Pattern (32KB)
  Step: Writing 0x55/0xAA pattern to 32768 bytes
  Progress: [######################] 100% (32768/32768)
  Step: Verifying pattern
  Progress: [######################] 100% (32768/32768)
  Result: All 32768 bytes verified correctly
  Status: PASS

Test 5/7: Inverse Checkerboard Pattern (32KB)
  Step: Writing 0xAA/0x55 pattern to 32768 bytes
  Progress: [######################] 100% (32768/32768)
  Step: Verifying pattern
  Progress: [######################] 100% (32768/32768)
  Result: All 32768 bytes verified correctly
  Status: PASS

Test 6/7: Address=Data Pattern (32KB)
  Step: Writing address low byte as data
  Progress: [######################] 100% (32768/32768)
  Step: Verifying pattern
  Progress: [######################] 100% (32768/32768)
  Result: All 32768 bytes verified correctly
  Status: PASS

Test 7/7: Random Pattern (32KB)
  Step: Generating pseudo-random data
  Step: Writing random pattern
  Progress: [######################] 100% (32768/32768)
  Step: Verifying pattern
  Progress: [######################] 100% (32768/32768)
  Result: All 32768 bytes verified correctly
  Status: PASS

===================================
RESULT: PASS
All SRAM tests completed successfully
Total bytes tested: 32768 (32KB)
Total test time: 14.2 seconds
Memory is functioning correctly
===================================
```

### Failure Example

```
Test 4/7: Checkerboard Pattern (32KB)
  Step: Writing 0x55/0xAA pattern to 32768 bytes
  Progress: [######################] 100% (32768/32768)
  Step: Verifying pattern
  Progress: [##########-----------] 45% (14893/32768)
  ERROR: Mismatch at address 0x3A2D
         Expected: 0xAA
         Read:     0xAB
  ERROR: Mismatch at address 0x3A2E
         Expected: 0x55
         Read:     0x57
  Result: 2 errors found
  Status: FAIL

===================================
RESULT: FAIL - SRAM test failed
Failed test: Checkerboard Pattern
First error at address: 0x3A2D
Possible causes:
  - Bad SRAM chip
  - Data line D0 or D1 issue
  - Contact problem in socket
===================================
```

---

## Pin Reconfiguration Details

### Z80 Mode Configuration

**When MODE Z80 is executed:**

```cpp
// Address bus: OUTPUT (Arduino drives for emulation, but reads for monitoring)
// Actually INPUT for Z80 mode - Z80 drives addresses
DDRA = 0x00;  // A0-A7: INPUT
DDRC = 0x00;  // A8-A15: INPUT
PORTA = 0x00; // No pull-ups
PORTC = 0x00;

// Data bus: Start as INPUT, switch dynamically
DDRL = 0x00;  // D0-D7: INPUT initially
PORTL = 0x00; // No pull-ups

// Control inputs (monitor Z80 outputs)
DDRG = 0x00;  // PG0-PG3: INPUT (/MREQ, /IORQ, /RD, /WR)
PORTG = 0x00; // No pull-ups (Z80 drives these)

// Control outputs (drive Z80 inputs)
DDRH |= (1 << 6);  // PH6: OUTPUT (/RESET)
PORTH |= (1 << 6); // /RESET HIGH (inactive)

DDRH &= ~((1 << 3) | (1 << 4) | (1 << 5)); // PH3-5: INPUT (/M1, /RFSH, /BUSACK)
PORTH &= ~((1 << 3) | (1 << 4) | (1 << 5)); // No pull-ups

DDRB |= 0xF0;  // PB4-7: OUTPUT (/WAIT, /INT, /NMI, /BUSREQ)
PORTB |= 0xF0; // All HIGH (inactive, pull-ups already present)

DDRE &= ~(1 << 4); // PE4: INPUT (/HALT)
PORTE &= ~(1 << 4); // No pull-up

// Clock output
DDRE |= (1 << 3);  // PE3: OUTPUT (Timer3 CLK)

// Configure Timer3 for Z80 clock (e.g., 1 MHz default)
// See Timer3 configuration section

// 6502-specific pins: Set to safe state (INPUT, not used)
DDRD &= ~((1 << 0) | (1 << 1) | (1 << 3)); // PD0,1,3: INPUT
PORTD &= ~((1 << 0) | (1 << 1) | (1 << 3)); // No pull-ups
```

### 6502 Mode Configuration

**When MODE 6502 is executed:**

```cpp
// Address bus: INPUT (6502 drives addresses)
DDRA = 0x00;  // A0-A7: INPUT
DDRC = 0x00;  // A8-A15: INPUT
PORTA = 0x00;
PORTC = 0x00;

// Data bus: Start as INPUT, switch dynamically
DDRL = 0x00;  // D0-D7: INPUT initially
PORTL = 0x00;

// Control inputs (monitor 6502 outputs)
// PG2 monitors R/W (input from 6502)
DDRG &= ~(1 << 2); // PG2: INPUT (R/W)
PORTG &= ~(1 << 2);

// PG0, PG1, PG3: Not used for 6502, set to INPUT
DDRG &= ~((1 << 0) | (1 << 1) | (1 << 3));
PORTG &= ~((1 << 0) | (1 << 1) | (1 << 3));

// PH3: INPUT (SYNC from 6502)
DDRH &= ~(1 << 3);
PORTH &= ~(1 << 3);

// Control outputs (drive 6502 inputs)
DDRH |= (1 << 6);  // PH6: OUTPUT (RES)
PORTH |= (1 << 6); // RES HIGH (inactive)

DDRB |= ((1 << 4) | (1 << 5) | (1 << 6)); // PB4-6: OUTPUT (RDY, IRQ, NMI)
PORTB |= (1 << 4); // RDY HIGH (ready) - note: inverted from Z80!
PORTB |= (1 << 5); // IRQ HIGH (inactive)
PORTB |= (1 << 6); // NMI HIGH (inactive)

// PB7: Not used for 6502
DDRB |= (1 << 7);
PORTB |= (1 << 7); // Keep HIGH

// Clock output
DDRE |= (1 << 3);  // PE3: OUTPUT (Timer3 Φ0)

// 6502-specific pins
DDRD &= ~((1 << 0) | (1 << 1)); // PD0,1: INPUT (monitor Φ1, Φ2)
PORTD &= ~((1 << 0) | (1 << 1));

DDRD |= (1 << 3);  // PD3: OUTPUT (S.O. control)
PORTD |= (1 << 3); // S.O. HIGH (inactive)

// Configure Timer3 for 6502 clock (e.g., 1 MHz default)
```

### 62256 Mode Configuration

**When MODE 62256 is executed:**

```cpp
// Address bus: OUTPUT (Arduino drives addresses)
DDRA = 0xFF;  // A0-A7: OUTPUT
DDRC = 0xFF;  // A8-A15: OUTPUT (only A8-A14 used for 32KB)
PORTA = 0x00; // Start at 0x0000
PORTC = 0x00;

// Data bus: Start as INPUT, switch to OUTPUT when writing
DDRL = 0x00;  // D0-D7: INPUT initially
PORTL = 0x00;

// Control outputs (Arduino controls SRAM)
DDRG |= ((1 << 0) | (1 << 2) | (1 << 3)); // PG0,2,3: OUTPUT (/CS, /OE, /WE)
PORTG |= ((1 << 0) | (1 << 2) | (1 << 3)); // All HIGH (inactive)

// Other pins: Set to safe INPUT state
DDRG &= ~(1 << 1); // PG1: INPUT
DDRH = 0x00;       // All PORTH inputs
DDRB = 0x00;       // All PORTB inputs
DDRE &= ~((1 << 3) | (1 << 4)); // PE3,4: INPUT (no clock needed)

// 6502-specific pins: INPUT
DDRD &= ~((1 << 0) | (1 << 1) | (1 << 3));
PORTD &= ~((1 << 0) | (1 << 1) | (1 << 3));

// Stop any running Timer3 clock
// Disable Timer3 output
```

---

## Timer3 Clock Configuration

### Purpose

Generate stable, configurable-frequency clock for Z80 and 6502 CPUs using hardware PWM.

### Configuration (CTC Mode, Toggle Output)

```cpp
void configureTimer3Clock(uint32_t frequency) {
    // Disable interrupts during configuration
    cli();

    // Stop Timer3
    TCCR3B = 0;

    // Set CTC mode (Clear Timer on Compare Match)
    // WGM33:0 = 0100 (CTC mode, TOP = OCR3A)
    TCCR3A = 0;
    TCCR3B = (1 << WGM32);

    // Toggle OC3A (PE3) on compare match
    TCCR3A |= (1 << COM3A0);

    // Calculate prescaler and OCR3A value
    // Formula: f_out = F_CPU / (2 * prescaler * (1 + OCR3A))
    // Rearranged: OCR3A = (F_CPU / (2 * prescaler * f_out)) - 1

    uint16_t prescaler;
    uint32_t ocr_value;

    // Choose prescaler based on desired frequency
    if (frequency >= 31250) {
        // No prescaling (prescaler = 1)
        prescaler = 1;
        TCCR3B |= (1 << CS30);
    } else if (frequency >= 3906) {
        // Prescaler = 8
        prescaler = 8;
        TCCR3B |= (1 << CS31);
    } else {
        // Prescaler = 64
        prescaler = 64;
        TCCR3B |= (1 << CS31) | (1 << CS30);
    }

    // Calculate OCR3A
    ocr_value = (F_CPU / (2UL * prescaler * frequency)) - 1;

    // Clamp to 16-bit range
    if (ocr_value > 65535) ocr_value = 65535;

    OCR3A = (uint16_t)ocr_value;

    // Enable output on PE3
    DDRE |= (1 << PE3);

    // Re-enable interrupts
    sei();
}

// Example usage:
// configureTimer3Clock(1000000);  // 1 MHz clock
// configureTimer3Clock(100000);   // 100 kHz clock
```

### Stopping Clock

```cpp
void stopTimer3Clock() {
    // Stop timer
    TCCR3B = 0;
    TCCR3A = 0;

    // PE3 to INPUT (safe state)
    DDRE &= ~(1 << PE3);
}
```

---

## STATUS Command

### Purpose

Display current mode, configuration, and system state.

### Example Output

```
User: STATUS
Arduino: ===== Multi-IC Tester Status =====
         Firmware Version: 1.0
         Current Mode: Z80
         Clock Status: Stopped
         Default Frequency: 1000000 Hz (1.00 MHz)

         Pin Configuration:
           Address Bus: INPUT (monitoring Z80)
           Data Bus: INPUT (idle)
           Control Signals: Configured for Z80

         Ready for testing
         Use TEST command to run tests
         ==================================
```

---

## Error Handling

### No Mode Selected

```
User: TEST
Arduino: ERROR: No IC mode selected
         Use MODE command first: MODE Z80, MODE 6502, or MODE 62256
```

### Invalid Mode Parameter

```
User: MODE Z8080
Arduino: ERROR: Invalid mode 'Z8080'
         Valid modes: Z80, 6502, 62256
```

### Test Failure - No IC Response

```
Starting Z80 CPU tests...

Test 1/5: Control Signal Test
  Status: PASS

Test 2/5: Clock Generation
  Status: PASS

Test 3/5: Simple Instruction Execution (JP Loop)
  Step: Loading program: C3 00 00
  Step: Releasing reset
  Step: Monitoring address bus for 500 cycles
  Result: Address stuck at 0x0000 (no changes detected)
  Status: FAIL

===================================
RESULT: FAIL - Z80 not responding
Failed test: Simple Instruction Execution
Possible causes:
  - No Z80 chip in socket
  - Z80 chip is defective
  - Power supply issue
  - Clock not reaching Z80
  - Wiring problem
===================================
```

### Test Failure - Wrong IC Detected

If behavior doesn't match expected IC type, report warning:

```
WARNING: Unexpected behavior detected
         Selected mode: 6502
         Observed: R/W signal not toggling (expected for 6502)
         Possible causes:
           - Wrong IC type in socket
           - Defective IC
           - Wiring issue
```

---

## Implementation Guidelines

### Code Organization Suggestions

**Modular Structure:**
- `main.cpp` - Main loop, UART handling, command parsing
- `pin_config.cpp/h` - Pin configuration functions for each mode
- `test_z80.cpp/h` - Z80 testing functions
- `test_6502.cpp/h` - 6502 testing functions
- `test_sram.cpp/h` - SRAM testing functions
- `timer3.cpp/h` - Timer3 clock configuration
- `uart.cpp/h` - UART communication helpers

**Global State:**
```cpp
enum ICMode {
    MODE_NONE,
    MODE_Z80,
    MODE_6502,
    MODE_62256
};

ICMode currentMode = MODE_NONE;
uint32_t defaultFrequency = 1000000; // 1 MHz
```

### Performance Considerations

**SRAM Testing Speed:**
- Full 32KB test takes time (write + verify = ~65,536 operations)
- Use direct port access (PORTA, PORTC, PORTL) not digitalWrite
- Minimize Serial.print during loops (slows down significantly)
- Show progress updates periodically (e.g., every 1024 bytes)

**Bus Cycle Handling for CPUs:**
- Must be fast enough to respond to CPU bus cycles
- Use interrupts or very tight polling loops
- Minimize processing in cycle handlers
- Pre-compute ROM/RAM contents

### Safety Considerations

**Bus Contention Prevention:**
- Always set data bus to INPUT when uncertain
- Never drive data bus OUTPUT while IC is also driving it
- Use /WAIT or RDY to pause CPU when switching data direction

**Power-On Defaults:**
- All control outputs should default to safe inactive states (HIGH for active-low signals)
- Data bus defaults to INPUT
- Address bus defaults to INPUT (for CPU modes) or all zeros (for SRAM mode)

**Mode Switching:**
- Stop any running clocks before reconfiguring
- Reset IC before starting new test
- Clear any internal state/buffers

---

## Testing Checklist for AI Implementation

When implementing this firmware, verify:

- [ ] UART communication works at 115200 baud
- [ ] MODE command correctly reconfigures pins for each IC type
- [ ] Timer3 generates stable clock at requested frequency
- [ ] Z80 tests can execute simple program and verify results
- [ ] 6502 tests can execute simple program and monitor Φ1/Φ2
- [ ] SRAM tests can write and read full 32KB range
- [ ] Signal inversions handled correctly for 6502 (R/W, SYNC, RDY)
- [ ] Progress reporting is detailed and helpful
- [ ] Error messages are clear and actionable
- [ ] All tests report PASS/FAIL correctly
- [ ] Data bus direction switching is safe (no contention)
- [ ] Pull-up resistors don't interfere with operation

---

## Appendix: Quick Reference Tables

### Command Summary

| Command | Parameters | Function |
|---------|------------|----------|
| MODE | Z80 \| 6502 \| 62256 | Select IC and reconfigure pins |
| TEST | (none) | Run automatic tests for selected IC |
| STATUS | (none) | Show current mode and configuration |
| HELP | (none) | Display command list |
| RESET | (none) | Reset the selected IC |

### Pin Direction Summary by Mode

| Pin Group | Z80 Mode | 6502 Mode | 62256 Mode |
|-----------|----------|-----------|------------|
| Address (PA, PC) | INPUT | INPUT | OUTPUT |
| Data (PL) | Bi (DDR) | Bi (DDR) | Bi (DDR) |
| PG0 (/MREQ or /CS) | INPUT | INPUT | OUTPUT |
| PG2 (/RD, R/W, /OE) | INPUT | INPUT | OUTPUT |
| PG3 (/WR or /WE) | INPUT | INPUT | OUTPUT |
| PH6 (/RESET, RES) | OUTPUT | OUTPUT | INPUT |
| PB4-6 (wait/int/nmi) | OUTPUT | OUTPUT | INPUT |
| PE3 (CLK) | OUTPUT | OUTPUT | INPUT |
| PD0,1 (Φ1,Φ2) | INPUT | INPUT | INPUT |
| PD3 (S.O.) | INPUT | OUTPUT | INPUT |

### Test Duration Estimates

| IC Type | Test Complexity | Estimated Duration |
|---------|----------------|-------------------|
| Z80 | Moderate | 2-5 seconds |
| 6502 | Moderate | 2-5 seconds |
| HM62256 | Comprehensive | 10-20 seconds |

---

**Document Version:** 1.0
**Last Updated:** 2026-01-03
**Target Firmware Platform:** Arduino Mega 2560 (ATmega2560)
**Author:** Multi-IC Tester Project
**Status:** Complete Specification for AI Implementation
