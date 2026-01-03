# Phase 1 - Foundation & Infrastructure Testing Guide

## Overview

Phase 1 implements the foundational infrastructure for the Multi-IC Tester:
- **UART Handler** - Serial communication at 115200 baud with formatted messaging
- **Command Parser** - Parses user commands (MODE, TEST, STATUS, RESET, HELP)
- **ICTestStrategy Base Class** - Abstract interface for IC testing strategies
- **Pin Definitions** - Centralized pin configuration constants
- **Mode Manager** - Tracks current IC selection and strategy
- **Main Integration** - Complete command loop with all commands implemented

**Note:** IC strategies (Z80, 6502, SRAM) are NOT implemented yet. Phase 1 validates the command infrastructure only.

## Prerequisites

- [x] **Hardware:** Arduino Mega 2560 connected via USB
- [x] **Software:** PlatformIO installed
- [x] **Tools:** Serial terminal (PuTTY, Arduino Serial Monitor, or PlatformIO device monitor)
- [x] **Phase 0:** Completed successfully

## Testing Procedure

### Step 1: Compile the Firmware

Compile Phase 1 code:

```bash
python -m platformio run
```

**Expected Output:**
```
========================= [SUCCESS] Took X.XX seconds =========================
RAM:   [==        ]  19.2% (used 1570 bytes from 8192 bytes)
Flash: [          ]   2.7% (used 6964 bytes from 253952 bytes)
```

**Success Criteria:**
- ✅ Compilation succeeds with no errors
- ✅ RAM usage < 25% (should be ~19%)
- ✅ Flash usage < 5% (should be ~2.7%)

---

### Step 2: Upload to Arduino

Upload firmware to Arduino Mega 2560:

```bash
python -m platformio run --target upload
```

**Expected:** Upload completes successfully.

---

### Step 3: Open Serial Monitor

Open serial monitor at 115200 baud:

```bash
python -m platformio device monitor --baud 115200
```

**Expected Startup Messages:**
```
========================================
  Multi-IC Tester v1.0
  Arduino Mega 2560
========================================

Supported ICs:
  - Z80 CPU (40-pin DIP)
  - 6502 CPU (40-pin DIP)
  - HM62256 SRAM (28-pin DIP)

Type HELP for command list

```

**Success Criteria:**
- ✅ Startup message displayed
- ✅ Version information shown
- ✅ Prompt for HELP command

---

### Step 4: Test HELP Command

Type in serial monitor:
```
HELP
```

**Expected Output:**
```
========================================
  Multi-IC Tester - Command Reference
========================================

Available Commands:

  MODE <IC>
    Select IC type for testing
    IC types: Z80, 6502, 62256
    Example: MODE Z80

  TEST
    Run tests for selected IC
    Must select MODE first

  STATUS
    Show current configuration
    and system information

  RESET
    Reset the selected IC
    Must select MODE first

  HELP
    Show this help message

========================================
Notes:
  - Commands are case-sensitive
  - Only one IC tested at a time
  - Strategies implemented in Phase 3+
========================================
```

**Success Criteria:**
- ✅ All 5 commands listed (MODE, TEST, STATUS, RESET, HELP)
- ✅ Usage information for each command
- ✅ Example shown for MODE command
- ✅ Notes section displayed

---

### Step 5: Test STATUS Command (No Mode Selected)

Type in serial monitor:
```
STATUS
```

**Expected Output:**
```
========================================
  Multi-IC Tester Status
========================================

Current Mode:
  NONE

Firmware:
  Version: 1.0 (Phase 1 Complete)
  Platform: Arduino Mega 2560
  UART: 115200 baud

Memory:

Ready for commands
Type HELP for command list
========================================
```

**Success Criteria:**
- ✅ Current mode shows "NONE"
- ✅ Firmware version displayed correctly
- ✅ Platform information shown

---

### Step 6: Test MODE Command - Valid IC Types

Test each valid IC type:

#### Test MODE Z80:
```
MODE Z80
```

**Expected Output:**
```
ERROR: Z80 strategy not implemented yet
Will be available in Phase 4
```

#### Test MODE 6502:
```
MODE 6502
```

**Expected Output:**
```
ERROR: 6502 strategy not implemented yet
Will be available in Phase 5
```

#### Test MODE 62256:
```
MODE 62256
```

**Expected Output:**
```
ERROR: HM62256 strategy not implemented yet
Will be available in Phase 3
```

**Success Criteria:**
- ✅ Each IC type recognized
- ✅ Appropriate error message shown
- ✅ Phase number mentioned for each IC

---

### Step 7: Test MODE Command - Invalid IC Type

Type in serial monitor:
```
MODE INVALID
```

**Expected Output:**
```
ERROR: Invalid IC type
Valid IC types: Z80, 6502, 62256
```

**Success Criteria:**
- ✅ Error message displayed
- ✅ Valid IC types listed

---

### Step 8: Test MODE Command - Missing Parameter

Type in serial monitor:
```
MODE
```

**Expected Output:**
```
ERROR: Missing IC type. Usage: MODE <IC>
Valid IC types: Z80, 6502, 62256
```

**Success Criteria:**
- ✅ Error for missing parameter
- ✅ Usage help shown

---

### Step 9: Test TEST Command (No Mode Selected)

Type in serial monitor:
```
TEST
```

**Expected Output:**
```
ERROR: No IC mode selected
Use MODE command first: MODE <Z80|6502|62256>
```

**Success Criteria:**
- ✅ Error message appropriate
- ✅ Guidance to use MODE first

---

### Step 10: Test RESET Command (No Mode Selected)

Type in serial monitor:
```
RESET
```

**Expected Output:**
```
ERROR: No IC mode selected
Use MODE command first
```

**Success Criteria:**
- ✅ Error message appropriate
- ✅ Guidance provided

---

### Step 11: Test Invalid Command

Type in serial monitor:
```
INVALID_COMMAND
```

**Expected Output:**
```
ERROR: Invalid command. Type HELP for command list.
```

**Success Criteria:**
- ✅ Error message displayed
- ✅ Suggestion to use HELP

---

### Step 12: Test Case Sensitivity

Type in serial monitor (lowercase):
```
help
```

**Expected Output:**
```
ERROR: Invalid command. Type HELP for command list.
```

Type (uppercase):
```
HELP
```

**Expected:** Help message displayed successfully.

**Success Criteria:**
- ✅ Commands are case-sensitive (lowercase rejected)
- ✅ Uppercase commands work correctly

---

### Step 13: Test Empty Lines

Press Enter several times without typing anything.

**Expected:** No output, no errors.

**Success Criteria:**
- ✅ Empty lines ignored gracefully
- ✅ No error messages
- ✅ System remains responsive

---

### Step 14: Test Long Commands

Type a very long invalid command:
```
THIS_IS_A_VERY_LONG_COMMAND_THAT_SHOULD_BE_HANDLED_GRACEFULLY
```

**Expected Output:**
```
ERROR: Invalid command. Type HELP for command list.
```

**Success Criteria:**
- ✅ Long commands handled correctly
- ✅ No crashes or hangs

---

## Expected Results Summary

After completing all steps, you should have verified:

### ✅ UART Communication
- [x] Serial communication works at 115200 baud
- [x] Startup message displays correctly
- [x] Messages formatted properly (OK:, ERROR:, plain text)

### ✅ Command Parsing
- [x] All 5 commands recognized (MODE, TEST, STATUS, RESET, HELP)
- [x] Command parameters parsed correctly
- [x] Invalid commands handled gracefully
- [x] Case sensitivity enforced
- [x] Empty lines ignored

### ✅ Mode Management
- [x] Initial mode is NONE
- [x] MODE command validates IC types
- [x] TEST and RESET check for mode selection
- [x] Appropriate error messages when no mode set

### ✅ Help System
- [x] HELP command shows all commands
- [x] STATUS command shows system information
- [x] Error messages provide guidance

### ✅ Infrastructure
- [x] Code compiles successfully
- [x] No runtime errors or crashes
- [x] System responsive to all commands
- [x] Memory usage reasonable (RAM: 19%, Flash: 2.7%)

---

## Troubleshooting

### Issue: No serial output after upload

**Solution:**
1. Wait 2 seconds after upload for Arduino to reset
2. Close and reopen serial monitor
3. Press reset button on Arduino

---

### Issue: Garbled characters in serial monitor

**Solution:**
Ensure baud rate is set to 115200:
```bash
python -m platformio device monitor --baud 115200
```

---

### Issue: "No serial port found"

**Solution:**
1. Check USB cable is connected
2. Verify Arduino Mega 2560 drivers installed
3. Try different USB port
4. On Linux: Check permissions (`sudo chmod 666 /dev/ttyUSB0`)

---

### Issue: Commands not recognized

**Solution:**
1. Ensure commands are UPPERCASE (HELP not help)
2. Check for extra spaces
3. Verify line ending is \n or \r\n (most terminals handle both)

---

### Issue: Arduino freezes or doesn't respond

**Solution:**
1. Press reset button on Arduino
2. Re-upload firmware
3. Check for short circuits or hardware issues

---

## Success Criteria

Phase 1 is successfully tested when ALL of the following are true:

- [ ] ✅ Firmware compiles with no errors
- [ ] ✅ RAM usage < 25% (currently 19.2%)
- [ ] ✅ Flash usage < 5% (currently 2.7%)
- [ ] ✅ Upload succeeds
- [ ] ✅ Startup message displays correctly
- [ ] ✅ HELP command shows all 5 commands
- [ ] ✅ STATUS command shows current mode (NONE)
- [ ] ✅ MODE command validates all IC types (Z80, 6502, 62256)
- [ ] ✅ MODE command rejects invalid IC types
- [ ] ✅ MODE command requires parameter
- [ ] ✅ TEST command checks for mode selection
- [ ] ✅ RESET command checks for mode selection
- [ ] ✅ Invalid commands show error with guidance
- [ ] ✅ Commands are case-sensitive
- [ ] ✅ Empty lines handled gracefully
- [ ] ✅ No crashes or hangs during testing
- [ ] ✅ Serial communication stable

---

## Performance Metrics

**Compilation:**
- Build time: ~2 seconds
- No warnings (except unused variable, which was fixed)

**Memory Usage:**
- RAM: 1570 bytes (19.2%) - Excellent
- Flash: 6964 bytes (2.7%) - Excellent
- Plenty of room for IC strategies (Phases 3-5)

**Response Time:**
- Command response: Instantaneous
- No noticeable delays

---

## What's Next?

✅ **Phase 1 Complete!**

**Phase 2: Timer3 Clock System**
- Implement hardware PWM clock generation
- Configure Timer3 for Z80/6502 CPUs
- Test clock output on oscilloscope

**Phase 3: HM62256 SRAM Testing**
- Implement SRAM62256Strategy class
- Add 7 comprehensive test patterns
- Enable MODE 62256 command

**To proceed to Phase 2:**
```
See Roadmap/Roadmap.md - Phase 2
```

---

## Verification Checklist

Before moving to Phase 2, verify each test:

```bash
# Compile
python -m platformio run

# Upload
python -m platformio run --target upload

# Monitor
python -m platformio device monitor --baud 115200

# In serial monitor, test these commands:
HELP         # ✅ Shows help
STATUS       # ✅ Shows NONE mode
MODE Z80     # ✅ Shows not implemented
MODE 6502    # ✅ Shows not implemented
MODE 62256   # ✅ Shows not implemented
MODE INVALID # ✅ Shows error
MODE         # ✅ Shows missing parameter error
TEST         # ✅ Shows no mode error
RESET        # ✅ Shows no mode error
help         # ✅ Shows case sensitivity error
```

**All tests passing? Phase 1 complete! 🎉**

---

**Phase 1 Testing Complete! ✅**

**GitHub Repository:** https://github.com/icaroNZ/Multi-IC-Tester
