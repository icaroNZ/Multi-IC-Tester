# Phase 2 - Timer3 Clock System Testing Guide

## Overview

Phase 2 implements hardware PWM clock generation using Timer3:
- **Timer3Clock Class** - Configurable frequency generator (1 Hz to 8 MHz)
- **CTC Mode** - Clear Timer on Compare with toggle output
- **Hardware PWM** - PE3 (pin 5) toggles automatically, 50% duty cycle
- **Test Commands** - CLOCK and CLOCKSTOP for verification

**Note:** This is a test-only implementation. Clock commands allow verification before IC strategies integrate Timer3 in later phases.

## Prerequisites

- [x] **Hardware:** Arduino Mega 2560 connected via USB
- [x] **Software:** PlatformIO installed
- [x] **Tools:**
  - Serial terminal (PuTTY, Arduino Serial Monitor, or PlatformIO device monitor)
  - LED + resistor (220Ω - 1kΩ) for 1 Hz visual test
  - Oscilloscope (optional but recommended for frequency verification)
- [x] **Phase 1:** Completed successfully

## Testing Procedure

### Step 1: Compile the Firmware

Compile Phase 2 code:

```bash
python -m platformio run
```

**Expected Output:**
```
========================= [SUCCESS] Took X.XX seconds =========================
RAM:   [==        ]  23.8% (used 1946 bytes from 8192 bytes)
Flash: [          ]   3.3% (used 8328 bytes from 253952 bytes)
```

**Success Criteria:**
- ✅ Compilation succeeds with no errors
- ✅ RAM usage < 30% (should be ~23.8%)
- ✅ Flash usage < 5% (should be ~3.3%)
- ✅ RAM increased by ~376 bytes from Phase 1 (Timer3Clock instance)
- ✅ Flash increased by ~1364 bytes from Phase 1 (Timer3 code)

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
- ✅ Startup message displayed correctly
- ✅ System responsive

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

  CLOCK <frequency>
    Start Timer3 clock at frequency (Hz)
    Output on PE3 (pin 5)
    Example: CLOCK 1000000

  CLOCKSTOP
    Stop Timer3 clock output

========================================
Notes:
  - Commands are case-sensitive
  - Only one IC tested at a time
  - Strategies implemented in Phase 3+
  - CLOCK commands for Phase 2 testing
========================================
```

**Success Criteria:**
- ✅ CLOCK command listed with example
- ✅ CLOCKSTOP command listed
- ✅ Output pin documented (PE3, pin 5)

---

### Step 5: Test CLOCK Command - Missing Parameter

Type in serial monitor:
```
CLOCK
```

**Expected Output:**
```
ERROR: Missing frequency. Usage: CLOCK <frequency>
Example: CLOCK 1000000 (for 1 MHz)
```

**Success Criteria:**
- ✅ Error message for missing parameter
- ✅ Usage example provided

---

### Step 6: Test CLOCK Command - Invalid Frequency

Test frequency too low:
```
CLOCK 0
```

**Expected Output:**
```
ERROR: Frequency out of range (1 Hz to 8 MHz)
```

Test frequency too high:
```
CLOCK 10000000
```

**Expected Output:**
```
ERROR: Frequency out of range (1 Hz to 8 MHz)
```

**Success Criteria:**
- ✅ Validates minimum frequency (1 Hz)
- ✅ Validates maximum frequency (8 MHz)

---

### Step 7: Test 1 Hz Clock (Visual Verification)

**Hardware Setup:**
1. Connect LED anode to PE3 (pin 5 on Arduino Mega)
2. Connect LED cathode to resistor (220Ω - 1kΩ)
3. Connect resistor to GND

Type in serial monitor:
```
CLOCK 1
```

**Expected Output:**
```
OK: Clock started at 1 Hz
Output on PE3 (pin 5)
```

**Visual Verification:**
- ✅ LED blinks ON and OFF once per second
- ✅ Equal ON and OFF times (50% duty cycle)
- ✅ Stable, consistent blinking

**Success Criteria:**
- ✅ Command accepted
- ✅ LED blinks visibly at 1 Hz rate
- ✅ No flickering or instability

---

### Step 8: Test Clock Frequency Changes

Test changing frequency while running:

```
CLOCK 2
```

**Expected:** LED blinks twice per second

```
CLOCK 5
```

**Expected:** LED blinks 5 times per second (visible but fast)

```
CLOCK 10
```

**Expected:** LED appears dimly lit (too fast to see individual blinks)

**Success Criteria:**
- ✅ Frequency changes without stopping clock first
- ✅ LED responds to different frequencies
- ✅ Smooth transitions

---

### Step 9: Test CLOCKSTOP Command

Type in serial monitor:
```
CLOCKSTOP
```

**Expected Output:**
```
OK: Clock stopped
```

**Visual Verification:**
- ✅ LED stops blinking
- ✅ LED goes dark (PE3 set to LOW)

**Success Criteria:**
- ✅ Command accepted
- ✅ Clock output stopped
- ✅ Pin set to clean LOW state

---

### Step 10: Test 100 Hz Clock

Type in serial monitor:
```
CLOCK 100
```

**Expected Output:**
```
OK: Clock started at 100 Hz
Output on PE3 (pin 5)
```

**Visual Verification (LED):**
- LED appears dimly lit (flicker fusion threshold ~50-60 Hz)
- No visible blinking

**Oscilloscope Verification (if available):**
- Frequency: 100 Hz (10 ms period)
- Duty cycle: 50% (5 ms high, 5 ms low)
- Voltage: 0V to 5V
- Clean square wave

**Success Criteria:**
- ✅ Command accepted
- ✅ LED appears steady (not blinking)
- ✅ Oscilloscope shows correct frequency (if available)

---

### Step 11: Test 1 kHz Clock

Type in serial monitor:
```
CLOCK 1000
```

**Expected Output:**
```
OK: Clock started at 1000 Hz
Output on PE3 (pin 5)
```

**Oscilloscope Verification (if available):**
- Frequency: 1 kHz (1 ms period)
- Duty cycle: 50% (500 µs high, 500 µs low)
- Voltage: 0V to 5V
- Clean square wave

**Success Criteria:**
- ✅ Command accepted
- ✅ Oscilloscope shows correct frequency

---

### Step 12: Test 100 kHz Clock

Type in serial monitor:
```
CLOCK 100000
```

**Expected Output:**
```
OK: Clock started at 100000 Hz
Output on PE3 (pin 5)
```

**Oscilloscope Verification:**
- Frequency: 100 kHz (10 µs period)
- Duty cycle: 50% (5 µs high, 5 µs low)
- Voltage: 0V to 5V

**Success Criteria:**
- ✅ Command accepted
- ✅ Oscilloscope shows correct frequency
- ✅ Clean transitions (minimal ringing)

---

### Step 13: Test 1 MHz Clock (Target CPU Speed)

Type in serial monitor:
```
CLOCK 1000000
```

**Expected Output:**
```
OK: Clock started at 1000000 Hz
Output on PE3 (pin 5)
```

**Oscilloscope Verification:**
- Frequency: 1 MHz (1 µs period)
- Duty cycle: 50% (500 ns high, 500 ns low)
- Voltage: 0V to 5V

**Success Criteria:**
- ✅ Command accepted
- ✅ Oscilloscope shows correct frequency
- ✅ This is the target speed for Z80/6502 testing

---

### Step 14: Test 4 MHz Clock (Maximum Practical)

Type in serial monitor:
```
CLOCK 4000000
```

**Expected Output:**
```
OK: Clock started at 4000000 Hz
Output on PE3 (pin 5)
```

**Oscilloscope Verification:**
- Frequency: 4 MHz (250 ns period)
- Duty cycle: 50% (125 ns high, 125 ns low)
- Voltage: 0V to 5V

**Success Criteria:**
- ✅ Command accepted
- ✅ Oscilloscope shows correct frequency
- ✅ Tests upper limit of timer capability

---

### Step 15: Test Multiple Start/Stop Cycles

Test reliability:

```
CLOCK 1000
CLOCKSTOP
CLOCK 1000
CLOCKSTOP
CLOCK 1000
CLOCKSTOP
```

**Success Criteria:**
- ✅ Clock starts and stops reliably
- ✅ No crashes or hangs
- ✅ Each cycle works correctly

---

### Step 16: Stress Test - Rapid Frequency Changes

Type rapidly:
```
CLOCK 1
CLOCK 100
CLOCK 1000
CLOCK 10000
CLOCK 100000
CLOCK 1000000
CLOCKSTOP
```

**Success Criteria:**
- ✅ All commands accepted
- ✅ No crashes or errors
- ✅ System remains responsive

---

## Expected Results Summary

After completing all steps, you should have verified:

### ✅ Timer3Clock Class Implementation
- [x] Class compiles successfully
- [x] Memory usage acceptable (RAM: 23.8%, Flash: 3.3%)
- [x] No compilation warnings

### ✅ CLOCK Command
- [x] Accepts frequency parameter
- [x] Validates frequency range (1 Hz to 8 MHz)
- [x] Configures and starts Timer3
- [x] Outputs confirmation message
- [x] Clock signal appears on PE3 (pin 5)

### ✅ CLOCKSTOP Command
- [x] Stops clock output
- [x] Sets PE3 to LOW (clean state)
- [x] Outputs confirmation message

### ✅ Frequency Accuracy
- [x] 1 Hz: LED blinks once per second (visual)
- [x] 100 Hz: LED appears steady (visual)
- [x] 1 kHz: Correct on oscilloscope
- [x] 100 kHz: Correct on oscilloscope
- [x] 1 MHz: Correct on oscilloscope (target CPU speed)
- [x] 4 MHz: Correct on oscilloscope (maximum speed)

### ✅ Reliability
- [x] Clock starts and stops cleanly
- [x] Frequency changes work correctly
- [x] No crashes during rapid changes
- [x] System remains responsive

### ✅ Hardware PWM
- [x] 50% duty cycle confirmed
- [x] Clean square wave output
- [x] Stable frequency (no jitter)

---

## Troubleshooting

### Issue: No LED blinking at 1 Hz

**Solution:**
1. Check LED polarity (anode to pin 5, cathode to resistor)
2. Verify resistor value (220Ω - 1kΩ)
3. Check connections to pin 5 (PE3)
4. Try different LED (may be faulty)
5. Send CLOCKSTOP and try again

---

### Issue: LED always ON or always OFF

**Solution:**
1. Check if clock is actually running (send STATUS command - not implemented yet)
2. Try different frequency (CLOCK 1)
3. Verify LED circuit is correct
4. Check for short circuits

---

### Issue: Oscilloscope shows wrong frequency

**Possible Causes:**
1. **Prescaler calculation error** - Check Timer3.cpp selectPrescaler()
2. **F_CPU mismatch** - Verify F_CPU = 16000000 in platformio.ini
3. **Oscilloscope probe calibration** - Recalibrate probe
4. **Integer rounding** - Some frequencies may be slightly off due to integer math

**Debugging:**
- Try exact frequencies that divide evenly (1 MHz, 100 kHz, 10 kHz)
- Check OCR3A value calculation
- Verify prescaler selection

---

### Issue: "Frequency out of range" for valid frequency

**Solution:**
1. Check for typos in frequency value
2. Verify no extra spaces
3. Try different frequency within range
4. Check if toInt() parsing correctly

---

### Issue: Clock doesn't stop with CLOCKSTOP

**Solution:**
1. Check TCCR3B is being cleared in stop()
2. Verify PORTE bit is being cleared
3. Try power cycling Arduino
4. Re-upload firmware

---

## Oscilloscope Testing Tips

### Probe Setup:
- **Probe:** 1x or 10x probe
- **Coupling:** DC coupling
- **Impedance:** 1 MΩ (standard)
- **Ground:** Connect scope ground to Arduino GND

### Measurement Verification:
1. **Frequency:** Measure period, calculate f = 1/T
2. **Duty Cycle:** Measure high time / period * 100%
3. **Rise/Fall Time:** Should be < 100 ns at 1 MHz
4. **Amplitude:** Should be 0V to 5V

### Expected Waveform:
```
    5V ┌──┐  ┌──┐  ┌──┐
       │  │  │  │  │  │
    0V └──┘  └──┘  └──┘
       ├──┤
       T/2  (50% duty cycle)
```

---

## Success Criteria

Phase 2 is successfully tested when ALL of the following are true:

- [ ] ✅ Firmware compiles with no errors
- [ ] ✅ RAM usage < 30% (currently 23.8%)
- [ ] ✅ Flash usage < 5% (currently 3.3%)
- [ ] ✅ Upload succeeds
- [ ] ✅ HELP command shows CLOCK and CLOCKSTOP
- [ ] ✅ CLOCK validates frequency parameter
- [ ] ✅ CLOCK rejects invalid frequencies
- [ ] ✅ CLOCK requires frequency parameter
- [ ] ✅ 1 Hz test: LED blinks once per second
- [ ] ✅ 100 Hz test: LED appears steady
- [ ] ✅ 1 kHz test: Oscilloscope shows 1 kHz (if available)
- [ ] ✅ 100 kHz test: Oscilloscope shows 100 kHz (if available)
- [ ] ✅ 1 MHz test: Oscilloscope shows 1 MHz (if available)
- [ ] ✅ CLOCKSTOP stops clock output
- [ ] ✅ Clock can be started and stopped multiple times
- [ ] ✅ No crashes or hangs during testing
- [ ] ✅ Serial communication stable

---

## Performance Metrics

**Compilation:**
- Build time: ~1.5 seconds
- No warnings

**Memory Usage:**
- RAM: 1946 bytes (23.8%) - Increased 376 bytes from Phase 1
- Flash: 8328 bytes (3.3%) - Increased 1364 bytes from Phase 1
- Still excellent headroom for IC strategies

**Timer3Clock Instance:**
- Size: ~6 bytes (currentFrequency, isRunning, prescalerBits)
- Methods: ~300 bytes of code
- Total overhead: < 400 bytes

**Frequency Accuracy:**
- 1 Hz: ±0.001 Hz (limited by 16-bit counter)
- 1 MHz: Exact (OCR3A = 7, prescaler = 1)
- 100 kHz: Exact (OCR3A = 79, prescaler = 1)

---

## What's Next?

✅ **Phase 2 Complete!**

**Phase 3: HM62256 SRAM Testing**
- Implement SRAM62256Strategy class
- Add 7 comprehensive test patterns
- Enable MODE 62256 command
- Test complete SRAM functionality

**Note:** SRAM testing does NOT use Timer3 clock (SRAM is asynchronous)

**Phase 4: Z80 CPU Testing**
- Implement Z80Strategy class
- Integrate Timer3 clock for CPU operation
- Add Z80-specific test sequences

**To proceed to Phase 3:**
```
See Roadmap/Roadmap.md - Phase 3
```

---

## Verification Checklist

Before moving to Phase 3, verify each test:

```bash
# Compile
python -m platformio run

# Upload
python -m platformio run --target upload

# Monitor
python -m platformio device monitor --baud 115200

# In serial monitor, test these commands:
HELP            # ✅ Shows CLOCK and CLOCKSTOP
CLOCK           # ✅ Shows missing parameter error
CLOCK 0         # ✅ Shows out of range error
CLOCK 10000000  # ✅ Shows out of range error
CLOCK 1         # ✅ LED blinks 1 Hz
CLOCK 100       # ✅ LED steady
CLOCK 1000000   # ✅ 1 MHz on scope
CLOCKSTOP       # ✅ Clock stops, LED off
```

**All tests passing? Phase 2 complete! 🎉**

---

**Phase 2 Testing Complete! ✅**

**GitHub Repository:** https://github.com/icaroNZ/Multi-IC-Tester
