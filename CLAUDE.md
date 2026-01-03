# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Multi-IC Tester for Arduino Mega 2560 - A hardware testing platform that validates three different IC types using shared hardware resources:
- **Z80 CPU** (40-pin DIP)
- **6502 CPU** (40-pin DIP)
- **HM62256 SRAM** and compatible memory ICs (28-pin DIP)

**Key Principle:** Only ONE IC tested at a time. Other sockets must be empty. Firmware reconfigures Arduino pins based on which IC is being tested.

## Development Guidelines

### Mandatory Development Practices

**ALWAYS follow these rules when working on this codebase:**

1. **Incremental Development**
   - Work in small, incremental steps
   - Complete one task at a time
   - Each commit should represent a working state

2. **Compilation Verification**
   - **ALWAYS compile code before finishing any task**: `pio run`
   - Never leave non-compiling code
   - Fix compilation errors immediately
   - If PlatformIO is not installed, install it first

3. **Code Quality Principles**
   - Follow **SOLID** principles (Single Responsibility, Open/Closed, Liskov Substitution, Interface Segregation, Dependency Inversion)
   - Follow **DRY** principle (Don't Repeat Yourself)
   - Use the **Strategy Pattern** for IC-specific implementations (allows adding new ICs without modifying existing code)
   - Use clear, descriptive names for variables and functions
   - Add comments only where logic is non-obvious

4. **Arduino Best Practices**
   - Use C++ classes with virtual functions for polymorphism
   - **Avoid dynamic memory allocation** (no `new`/`delete`) - use static allocation
   - Use `const` and `constexpr` for constants
   - Use `PROGMEM` for large constant data (lookup tables, strings)
   - Use direct port manipulation for performance-critical code (not `digitalWrite`)
   - Minimize Arduino library functions in time-critical sections

5. **Strategy-Driven Development**
   - **Before coding**, create a strategy document in `Strategy/` folder
   - Document architectural decisions and approach
   - Reference strategy documents when implementing
   - Update strategies if approach changes

6. **Never Assume - Always Ask**
   - If unclear about requirements, **ask clarifying questions**
   - If uncertain about implementation approach, **ask first**
   - Never assume what the user wants
   - Better to ask than to implement incorrectly

7. **Roadmap Management**
   - Consult `Roadmap/Roadmap.md` for current phase and tasks
   - **Update Roadmap.md as needed** during development:
     - Add notes to future phases about new files created
     - Add dependencies between phases
     - Flag items that need review
   - Mark items as completed in roadmap
   - Add new items if scope expands

8. **Testing Documentation (MANDATORY)**
   - **After completing each phase**, create a testing guide in `Tests/` folder
   - **File naming:** `Phase<X>-<Description>.md` (e.g., `Phase1-Foundation-Testing.md`)
   - **ALWAYS create** this documentation before moving to the next phase
   - Each test guide must include:
     - **Overview:** What was implemented in this phase
     - **Prerequisites:** What must be ready before testing (hardware, software, tools)
     - **Step-by-step testing procedure:** Numbered steps, easy to follow
     - **Command line commands:** In copy-paste code blocks
     - **Expected results:** What should happen for each step
     - **Troubleshooting:** Common issues and solutions
     - **Success criteria:** How to know testing passed
   - Make instructions **simple and beginner-friendly**
   - Use **actual commands** that can be copy-pasted directly
   - Include **screenshots or ASCII diagrams** where helpful
   - Update test documentation if implementation changes

**Testing Documentation Template:**

```markdown
# Phase X - [Description] Testing Guide

## Overview
Brief description of what was implemented in this phase.

## Prerequisites
- [ ] Hardware: Arduino Mega 2560 connected via USB
- [ ] Software: PlatformIO installed
- [ ] Tools: Serial terminal (PuTTY, Arduino Serial Monitor, etc.)

## Testing Procedure

### Step 1: Compile the Firmware
\```bash
python -m platformio run
\```
**Expected:** Compilation succeeds with no errors.

### Step 2: Upload to Arduino
\```bash
python -m platformio run --target upload
\```
**Expected:** Upload completes successfully.

### Step 3: Open Serial Monitor
\```bash
python -m platformio device monitor --baud 115200
\```
**Expected:** Serial monitor opens at 115200 baud.

### Step 4: Test [Feature Name]
Type the following command in serial monitor:
\```
COMMAND PARAMETER
\```
**Expected:** Response should be:
\```
OK: [success message]
\```

## Expected Results Summary
- ✅ All compilation successful
- ✅ Upload successful
- ✅ Serial communication working
- ✅ Commands respond correctly

## Troubleshooting

### Issue: "pio: command not found"
**Solution:** Use `python -m platformio` instead of `pio`

### Issue: "No serial port found"
**Solution:** Check USB cable and drivers for Arduino Mega 2560

## Success Criteria
- [ ] All steps completed without errors
- [ ] All expected results matched actual results
- [ ] Ready to proceed to next phase
```

### Code Organization Pattern

**Strategy Pattern Implementation:**

Each IC type implements a common interface:

```cpp
class ICTestStrategy {
public:
    virtual void configurePins() = 0;
    virtual void reset() = 0;
    virtual bool runTests() = 0;
    virtual const char* getName() = 0;
};

class Z80Strategy : public ICTestStrategy { /* ... */ };
class IC6502Strategy : public ICTestStrategy { /* ... */ };
class SRAM62256Strategy : public ICTestStrategy { /* ... */ };
```

**Benefits:**
- Adding new ICs requires NO changes to existing code
- Each IC's logic is encapsulated in its own class
- Easy to test and maintain
- Follows Open/Closed Principle

### File Organization

```
src/
├── main.cpp                          # Main entry point, command parser
├── strategies/
│   ├── ICTestStrategy.h              # Abstract base class
│   ├── Z80Strategy.h/.cpp            # Z80 implementation
│   ├── IC6502Strategy.h/.cpp         # 6502 implementation
│   └── SRAM62256Strategy.h/.cpp      # SRAM implementation
├── hardware/
│   ├── Timer3.h/.cpp                 # Timer3 clock management
│   ├── PinConfig.h                   # Pin definitions
│   └── BusController.h/.cpp          # Bus management
└── utils/
    ├── UARTHandler.h/.cpp            # Serial communication
    └── CommandParser.h/.cpp          # Command parsing

Strategy/                              # Strategy documents
├── 00-Overall-Architecture.md        # Overall system design
├── 01-Phase1-Foundation.md           # Phase-specific strategies
└── ...

Roadmap/
└── Roadmap.md                        # Development roadmap

Tests/                                 # Testing documentation
├── Phase0-Setup-Testing.md           # Phase 0 testing guide
├── Phase1-Foundation-Testing.md      # Phase 1 testing guide
├── Phase2-Timer3-Testing.md          # Phase 2 testing guide
├── Phase3-SRAM-Testing.md            # Phase 3 testing guide
├── Phase4-Z80-Testing.md             # Phase 4 testing guide
├── Phase5-6502-Testing.md            # Phase 5 testing guide
└── Phase6-Integration-Testing.md     # Phase 6 testing guide
```

## Build and Development Commands

This project uses PlatformIO, not Arduino IDE.

```bash
# Build the project
pio run

# Upload to Arduino Mega 2560
pio run --target upload

# Clean build files
pio run --target clean

# Open serial monitor (115200 baud)
pio device monitor --baud 115200

# Build and upload in one command
pio run --target upload && pio device monitor --baud 115200
```

**Target Board:** `megaatmega2560` (defined in platformio.ini)

## Architecture

### Hardware Architecture

**Shared Bus System:**
- **Address Bus (A0-A15):** PORTA (pins 22-29) + PORTC (pins 30-37) - shared across all ICs
- **Data Bus (D0-D7):** PORTL (pins 42-49) - bidirectional, DDR controlled by firmware
- **Clock:** PE3 (pin 5) using Timer3 hardware PWM - generates clock for both CPUs

**Critical Design Aspects:**

1. **Signal Inversions Between ICs** - Some signals have OPPOSITE logic:
   - PG2 (pin 39): Z80 /RD (LOW=read) vs 6502 R/W (HIGH=read)
   - PH3 (pin 6): Z80 /M1 (LOW=fetch) vs 6502 SYNC (HIGH=fetch)
   - PB4 (pin 10): Z80 /WAIT (LOW=wait) vs 6502 RDY (HIGH=ready)

2. **Pin Direction Changes** - Same Arduino pins have different directions for different ICs:
   - PG0-PG3: INPUT for Z80/6502 (monitor), OUTPUT for 62256 (control)
   - PORTA/PORTC: INPUT for CPUs (monitor addresses), OUTPUT for SRAM (drive addresses)

3. **Mode-Specific Pins:**
   - 6502-only: PD0 (Φ1 monitor), PD1 (Φ2 monitor), PD3 (S.O. control)
   - Z80-only: PH4 (/RFSH), PH5 (/BUSACK), PB7 (/BUSREQ)

### Software Architecture

**Command-Based UART Interface:**
- UART at 115200 baud, 8N1
- Line-based commands terminated by `\n`
- Response format: `OK:` or `ERROR:` or `RESULT: PASS/FAIL`

**Main Commands:**
- `MODE <Z80|6502|62256>` - Select IC and reconfigure pins
- `TEST` - Run automatic test for selected IC
- `STATUS` - Show current mode and configuration
- `RESET` - Reset the selected IC
- `HELP` - Display command list

**Firmware Module Organization (Recommended):**
- `main.cpp` - Main loop, UART handling, command parsing
- `pin_config.cpp/h` - Pin reconfiguration for each mode
- `test_z80.cpp/h` - Z80 testing (opcode execution, memory emulation)
- `test_6502.cpp/h` - 6502 testing (handles inverted signals, Φ1/Φ2 monitoring)
- `test_sram.cpp/h` - SRAM testing (walking 1s, checkerboard patterns)
- `timer3.cpp/h` - Hardware PWM clock generation
- `uart.cpp/h` - Serial communication utilities

### Memory Emulation for CPU Testing

**Z80/6502 Memory Map:**
- ROM: 0x0000 - 0x0FFF (4KB) - Store test programs
- RAM: 0x1000 - 0x17FF (2KB) - Emulate read/write memory
- 6502 Reset Vector: 0xFFFC-0xFFFD returns 0x0000

**Bus Cycle Handling:**
- Firmware monitors CPU bus cycles and responds in real-time
- Must switch data bus direction (DDRL) appropriately
- Use /WAIT (Z80) or RDY (6502) signals if Arduino response time needed

## Critical Implementation Notes

### Timer3 Clock Configuration

Use CTC mode with toggle output on OC3A (PE3):

```cpp
// Formula: f_out = F_CPU / (2 * prescaler * (1 + OCR3A))
// Common frequencies:
// - 1 MHz for normal operation
// - 100 kHz for initial testing (safer)
```

Choose prescaler based on target frequency:
- No prescaling (1) for >= 31.25 kHz
- Prescaler 8 for >= 3.9 kHz
- Prescaler 64 for lower frequencies

### Data Bus Direction Safety

**CRITICAL:** Prevent bus contention
- Always set data bus to INPUT when uncertain: `DDRL = 0x00`
- Never drive OUTPUT while IC is also driving
- Default state should be INPUT

### Signal Logic When Mode Switching

When `MODE` command executes:
1. Stop any running clocks (Timer3)
2. Reset the IC (assert reset signal)
3. Reconfigure all pin directions (DDR registers)
4. Set default pin states
5. Handle signal inversions for 6502 mode
6. Configure Timer3 if needed
7. Store current mode in global state

### SRAM Testing Performance

Use direct port access, not digitalWrite():
```cpp
// Fast write:
PORTA = addr_low;    // A0-A7
PORTC = addr_high;   // A8-A15
PORTL = data;        // D0-D7
// Control /CS, /WE timing
```

Show progress updates periodically (e.g., every 1024 bytes) - Serial.print in tight loops slows testing significantly.

## Pin Mapping Reference

Complete mappings in `Documents/Multi-IC_Tester_Pinout.md`

**Most Critical Pins:**
- PE3 (5): Timer3 clock output → Z80 CLK / 6502 Φ0
- PH6 (9): /RESET / RES (shared reset, active LOW)
- PG0-PG3 (41-38): Control signals (different meanings per IC)
- PORTA (22-29): Address bus low byte
- PORTC (30-37): Address bus high byte
- PORTL (42-49): Data bus (bidirectional)

## Test Strategy

**Z80 Testing:**
1. Control signal verification (idle states)
2. Clock generation (100 kHz)
3. Simple instruction execution (JP 0x0000 loop)
4. Memory write test (LD instructions)
5. Memory read verification

**6502 Testing:**
1. Control signal verification (R/W, SYNC)
2. Clock generation + Φ1/Φ2 output monitoring
3. Simple instruction execution (JMP $0000 loop)
4. Memory write test (STA instructions)
5. Memory read verification

**HM62256 SRAM Testing (Comprehensive):**
1. Basic read/write test
2. Walking 1s - address lines (detects stuck/shorted address lines)
3. Walking 1s - data lines (detects stuck data lines)
4. Checkerboard pattern (full 32KB)
5. Inverse checkerboard pattern
6. Address=Data pattern
7. Random pattern (optional)

## Documentation Structure

- `Documents/Multi-IC_Firmware_Spec.md` - Complete firmware specification (1200+ lines)
- `Documents/Multi-IC_Tester_Pinout.md` - Comprehensive pin mapping for all ICs
- `Documents/Pinouts/` - Individual IC pinout references (Z80, 6502, HM62256)
- `platformio.ini` - Build configuration
- `src/main.cpp` - Main firmware source (currently skeleton)

## Common Pitfalls to Avoid

1. **Signal Inversion:** Forgetting to invert R/W, SYNC, RDY logic for 6502
2. **Bus Contention:** Setting data bus OUTPUT while IC drives it
3. **Pin Direction:** Not reconfiguring DDR registers when switching modes
4. **Serial Performance:** Using Serial.print inside tight loops (use sparingly)
5. **Reset Vector:** 6502 requires reset vector at 0xFFFC-0xFFFD to point to program start
6. **6502 No HALT:** Unlike Z80, 6502 has no HALT instruction - use infinite loops
7. **Pull-ups:** Hardware has 10kΩ pull-ups on /RESET, /WAIT, /INT, /NMI, /BUSREQ - firmware should not fight these

## Reference Documents

Always consult these documents when working on IC-specific code:
- Firmware spec: `Documents/Multi-IC_Firmware_Spec.md`
- Pin mapping: `Documents/Multi-IC_Tester_Pinout.md`
- Individual IC pinouts: `Documents/Pinouts/*.md`
