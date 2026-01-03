# Phase 2 Strategy: Timer3 Clock System

## 1. Overview

Phase 2 implements hardware PWM clock generation using the ATmega2560's Timer3 peripheral. This clock will drive Z80 and 6502 CPUs during testing, allowing precise control of CPU execution speed.

**Purpose:** Generate stable, configurable clock signals for CPU testing
**Hardware:** Timer3 with Output Compare A on PE3 (pin 5)
**Frequencies:** 1 Hz to ~4 MHz (limited by prescaler and 16-bit counter)
**Mode:** CTC (Clear Timer on Compare) with toggle output

## 2. Why Timer3?

**Timer3 Characteristics:**
- 16-bit timer with hardware PWM output
- Output Compare pin PE3 (pin 5) is available on Arduino Mega
- CTC mode allows precise frequency generation
- Toggle mode generates 50% duty cycle automatically
- No CPU intervention needed once configured

**Alternatives Considered:**
- **Timer0/Timer2:** Used by Arduino millis()/micros() functions - avoid
- **Timer1:** Could work, but reserved for potential future features
- **Timer4/Timer5:** Available but Timer3 is sufficient and well-documented
- **Software PWM:** Too unstable and CPU-intensive

## 3. Timer3 Register Configuration

### Key Registers:

**TCCR3A (Timer/Counter Control Register A):**
- `COM3A[1:0]`: Compare Output Mode for Channel A
  - `COM3A1 = 0, COM3A0 = 1`: Toggle OC3A (PE3) on Compare Match
- `WGM3[1:0]`: Lower bits of Waveform Generation Mode
  - Set to `00` for CTC mode (upper bits in TCCR3B)

**TCCR3B (Timer/Counter Control Register B):**
- `WGM3[3:2]`: Upper bits of Waveform Generation Mode
  - `WGM32 = 1, WGM33 = 0`: CTC mode (WGM3 = 0100 overall)
- `CS3[2:0]`: Clock Select (Prescaler)
  - `001`: No prescaling (clk/1)
  - `010`: clk/8
  - `011`: clk/64
  - `100`: clk/256
  - `101`: clk/1024

**OCR3A (Output Compare Register A):**
- 16-bit register that determines when timer resets
- Timer counts from 0 to OCR3A, then resets and toggles PE3
- Formula: `OCR3A = (F_CPU / (2 * prescaler * desired_freq)) - 1`

**TCNT3 (Timer/Counter Register):**
- Current timer value (auto-increments)
- Auto-resets to 0 when matches OCR3A in CTC mode

**DDRE (Data Direction Register E):**
- Must set PE3 as OUTPUT for hardware PWM output
- `DDRE |= (1 << DDE3);`

## 4. Frequency Calculation

### Formula:
```
f_output = F_CPU / (2 * prescaler * (OCR3A + 1))
```

**Solving for OCR3A:**
```
OCR3A = (F_CPU / (2 * prescaler * f_desired)) - 1
```

**Given:**
- `F_CPU = 16,000,000 Hz` (Arduino Mega 2560)
- `OCR3A` is 16-bit (0 to 65535)

### Prescaler Selection Logic:

Try prescalers in order until OCR3A fits in 16 bits:

1. **Prescaler = 1** (no prescaling):
   - Max frequency: ~8 MHz (limited by toggle, actually ~4 MHz usable)
   - Min frequency: ~122 Hz (`16MHz / (2 * 1 * 65535)`)
   - Best for: CPU clocks > 122 Hz

2. **Prescaler = 8:**
   - Max frequency: ~1 MHz
   - Min frequency: ~15.3 Hz
   - Best for: Mid-range frequencies

3. **Prescaler = 64:**
   - Max frequency: ~125 kHz
   - Min frequency: ~1.9 Hz
   - Best for: Slow clocks and visible LED testing

4. **Prescaler = 256:**
   - Max frequency: ~31.25 kHz
   - Min frequency: ~0.48 Hz
   - Best for: Very slow clocks

5. **Prescaler = 1024:**
   - Max frequency: ~7.8 kHz
   - Min frequency: ~0.12 Hz
   - Best for: Ultra-slow debugging

### Example Calculations:

**1 MHz Clock (Z80/6502 testing):**
```
OCR3A = (16,000,000 / (2 * 1 * 1,000,000)) - 1
      = (16,000,000 / 2,000,000) - 1
      = 8 - 1
      = 7
Prescaler: 1
Actual frequency: 16,000,000 / (2 * 1 * 8) = 1,000,000 Hz ✓
```

**1 Hz LED Test:**
```
OCR3A = (16,000,000 / (2 * 64 * 1)) - 1
      = (16,000,000 / 128) - 1
      = 125,000 - 1
      = 124,999
Prescaler: 64
Actual frequency: 16,000,000 / (2 * 64 * 125,000) = 1.0 Hz ✓
```

**100 kHz Clock:**
```
OCR3A = (16,000,000 / (2 * 1 * 100,000)) - 1
      = (16,000,000 / 200,000) - 1
      = 80 - 1
      = 79
Prescaler: 1
Actual frequency: 16,000,000 / (2 * 1 * 80) = 100,000 Hz ✓
```

## 5. Implementation Approach

### Class Design: `Timer3Clock`

**Public Interface:**
```cpp
class Timer3Clock {
public:
    void configure(uint32_t frequency);  // Set frequency and configure registers
    void start();                         // Start clock output
    void stop();                          // Stop clock output
    uint32_t getFrequency() const;       // Get current configured frequency
    bool running() const;                 // Check if clock is running

private:
    uint32_t currentFrequency;
    bool isRunning;
    uint8_t prescalerBits;               // CS3x bits to set

    uint8_t selectPrescaler(uint32_t frequency, uint16_t& ocr3a);
};
```

### Method Implementation Strategy:

**1. configure(uint32_t frequency):**
- Stop timer first (clear TCCR3B)
- Calculate best prescaler and OCR3A value
- Clamp OCR3A to 65535 maximum
- Set TCCR3A: `COM3A0 = 1` (toggle), `WGM3[1:0] = 00`
- Set TCCR3B: `WGM32 = 1` (CTC mode), prescaler bits cleared
- Set OCR3A register
- Set PE3 as OUTPUT
- Store currentFrequency
- Set isRunning = false (not started yet)

**2. start():**
- Set prescaler bits in TCCR3B (starts timer)
- Set isRunning = true

**3. stop():**
- Clear TCCR3B (stops timer)
- Set PE3 LOW (clean state)
- Set isRunning = false

**4. selectPrescaler():**
```cpp
uint8_t selectPrescaler(uint32_t freq, uint16_t& ocr3a) {
    // Try prescalers: 1, 8, 64, 256, 1024
    const uint16_t prescalers[] = {1, 8, 64, 256, 1024};
    const uint8_t csBits[] = {
        (1<<CS30),                    // clk/1
        (1<<CS31),                    // clk/8
        (1<<CS31)|(1<<CS30),          // clk/64
        (1<<CS32),                    // clk/256
        (1<<CS32)|(1<<CS30)           // clk/1024
    };

    for (int i = 0; i < 5; i++) {
        uint32_t calc = (F_CPU / (2UL * prescalers[i] * freq)) - 1;
        if (calc <= 65535) {
            ocr3a = (uint16_t)calc;
            return csBits[i];
        }
    }

    // If no prescaler works, use largest and clamp
    ocr3a = 65535;
    return csBits[4];  // clk/1024
}
```

## 6. Testing Strategy

### Test Frequencies:

1. **1 Hz** - Visible LED blink (PE3 connected to LED)
   - Verify: LED blinks once per second
   - Easy visual confirmation

2. **100 Hz** - Just beyond human vision
   - Verify: LED appears dimly lit (not blinking)
   - Tests mid-range prescaler

3. **1 kHz** - Low audio frequency
   - Verify: Oscilloscope shows 1 ms period
   - Tests precision at kHz range

4. **100 kHz** - SRAM testing speed
   - Verify: Oscilloscope shows 10 µs period
   - Practical speed for initial CPU tests

5. **1 MHz** - Z80/6502 operating speed
   - Verify: Oscilloscope shows 1 µs period
   - Target speed for CPU testing

6. **4 MHz** - Maximum practical frequency
   - Verify: Oscilloscope shows 250 ns period
   - Tests upper limit

### Test Commands (Temporary):

Add to main.cpp for testing:
```
CLOCK <frequency>  - Configure and start clock
CLOCKSTOP          - Stop clock
```

### Verification Methods:

1. **LED Test (1 Hz):**
   - Connect LED between PE3 (pin 5) and GND (with resistor)
   - Should blink once per second

2. **Oscilloscope (all frequencies):**
   - Probe PE3 (pin 5)
   - Verify frequency and 50% duty cycle
   - Check rise/fall times

3. **Logic Analyzer (optional):**
   - More precise frequency measurement
   - Useful for verifying jitter

## 7. Safety Considerations

### Electrical Safety:
- PE3 is 5V tolerant - safe for Z80/6502 clock inputs
- Use current-limiting resistor for LED testing (220Ω - 1kΩ)
- Never exceed 40 mA per pin (Arduino Mega limit)

### Software Safety:
- Always stop timer before reconfiguring
- Set PE3 as OUTPUT before starting timer (prevent floating input)
- Clear TCNT3 when changing frequency (clean start)
- Validate frequency range (1 Hz to 8 MHz)

### CPU Safety:
- Don't start clock without CPU properly inserted
- Clock must be stable before releasing CPU reset
- Stop clock before removing CPU

## 8. Memory Considerations

### RAM Usage:
```cpp
class Timer3Clock {
    uint32_t currentFrequency;  // 4 bytes
    bool isRunning;              // 1 byte
    uint8_t prescalerBits;      // 1 byte (optional, can recalculate)
    // Total: ~6 bytes
};
```

**Global Instance:** 6 bytes
**Stack Usage:** Minimal (~10 bytes for local variables in configure())
**Total Impact:** < 20 bytes (negligible)

### Flash Usage:
- Estimated: ~200-300 bytes for all methods
- Minimal impact on 253 KB available

### Register Usage:
Timer3 uses dedicated hardware registers (no RAM impact):
- TCCR3A, TCCR3B, TCNT3, OCR3A, ICR3
- These are memory-mapped I/O, not RAM

## 9. Integration with IC Strategies

### Z80 Strategy:
```cpp
void Z80Strategy::configurePins() {
    // ... pin configuration ...
    timer3.configure(1000000);  // 1 MHz clock
    timer3.start();
}

void Z80Strategy::reset() {
    timer3.stop();
    // ... reset sequence ...
    timer3.start();
}
```

### 6502 Strategy:
```cpp
void IC6502Strategy::configurePins() {
    // ... pin configuration ...
    timer3.configure(1000000);  // 1 MHz clock
    timer3.start();
}
```

### SRAM Strategy:
```cpp
// SRAM does NOT need clock - don't start Timer3
void SRAM62256Strategy::configurePins() {
    // No clock needed
}
```

## 10. Future Enhancements (Not Phase 2)

**Variable Speed Testing:**
- Test CPUs at different speeds (100 kHz to 4 MHz)
- Characterize speed limits
- Find marginal chips

**Frequency Measurement:**
- Read back actual frequency using Input Capture
- Verify stability

**Phase-Locked Loop:**
- Lock to external reference
- Ultra-precise timing

**Multiple Timers:**
- Use Timer4/Timer5 for additional clocks
- Independent clock domains

---

## Summary

Phase 2 implements a clean, hardware-based clock generation system using Timer3 in CTC mode with toggle output. The implementation is simple (~100 lines of code), uses minimal resources (< 20 bytes RAM, ~300 bytes flash), and provides precise, stable clock signals from 1 Hz to 4 MHz.

The Timer3Clock class is self-contained and can be easily integrated into IC testing strategies. Testing with LED (1 Hz) and oscilloscope (higher frequencies) ensures the clock works correctly before proceeding to CPU testing in later phases.

**Phase 2 Success Criteria:**
- [ ] Timer3Clock class compiles with no errors
- [ ] 1 Hz test: LED blinks visibly once per second
- [ ] 1 kHz test: Oscilloscope shows 1 ms period, 50% duty cycle
- [ ] 100 kHz test: Oscilloscope shows 10 µs period
- [ ] 1 MHz test: Oscilloscope shows 1 µs period
- [ ] Clock can be stopped and restarted cleanly
- [ ] No crashes or instability
- [ ] Memory usage remains low (< 25% RAM total)
