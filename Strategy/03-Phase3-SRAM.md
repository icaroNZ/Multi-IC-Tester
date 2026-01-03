# Phase 3 Strategy: Generic SRAM Testing

## 1. Overview

Phase 3 implements comprehensive SRAM testing with support for multiple chip sizes. Unlike CPU testing (Phases 4-5), SRAM testing requires no clock signal and no CPU emulation—just direct memory access with control signals.

**Supported Chips:**
- **HM62256**: 32KB SRAM (32768 bytes), 15 address lines (A0-A14)
- **HM6265**: 8KB SRAM (8192 bytes), 13 address lines (A0-A12)
- **D4168**: 8KB SRAM (8192 bytes), 13 address lines (A0-A12), pin-compatible with HM6265

**Generic Approach:** User specifies memory size in bytes, strategy adapts test patterns accordingly.

**Mode Command:** `MODE SRAM <size_in_bytes>`
- Example: `MODE SRAM 32768` for HM62256
- Example: `MODE SRAM 8192` for HM6265 or D4168

## 2. SRAM Characteristics

### Common Features (All Chips):
- **Organization**: 8-bit data bus (D0-D7)
- **Package**: 28-pin DIP
- **Technology**: Static RAM (no refresh needed)
- **Access**: Asynchronous (no clock required)
- **Voltage**: 5V TTL-compatible
- **Pin-Compatible**: HM6265 and D4168 are fully compatible

### Chip-Specific Details:

**HM62256 (32KB):**
- Address Range: 0x0000 - 0x7FFF (15 address lines)
- Pins Used: A0-A14 (all 15 address lines)
- Control: /CS, /OE, /WE

**HM6265 (8KB):**
- Address Range: 0x0000 - 0x1FFF (13 address lines)
- Pins Used: A0-A12 (address lines), A13-A14 unused
- Control: /CS, /OE, /WE

**D4168 (8KB):**
- Address Range: 0x0000 - 0x1FFF (13 address lines)
- Pin 1: NC (No Connect)
- Pin 20: /CS (active low chip select)
- Pin 26: CS (active high chip select - opposite of pin 20)
- Compatible with HM6265 (can use /CS on pin 20, ignore CS on pin 26)

## 3. Pin Configuration

### 28-Pin DIP Pinout (Generic):

```
         ┌──────┐
    A14 │1   28│ VCC
    A12 │2   27│ /WE
    A7  │3   26│ CS  (D4168 only, NC for HM62256/HM6265)
    A6  │4   25│ A8
    A5  │5   24│ A9
    A4  │6   23│ A11
    A3  │7   22│ /OE
    A2  │8   21│ A10
    A1  │9   20│ /CS
    A0  │10  19│ D7
    D0  │11  18│ D6
    D1  │12  17│ D5
    D2  │13  16│ D4
    GND │14  15│ D3
         └──────┘
```

### Arduino Mega 2560 Pin Mapping (from PinConfig.h):

**Address Bus (16 bits available):**
- **Low Byte (A0-A7)**: PORTA (pins 22-29)
- **High Byte (A8-A15)**: PORTC (pins 37-30)

**Data Bus (8 bits):**
- **D0-D7**: PORTL (pins 49-42)

**Control Signals:**
- **PG0 (pin 41)**: /CS (Chip Select, active LOW)
- **PG2 (pin 39)**: /OE (Output Enable, active LOW)
- **PG3 (pin 38)**: /WE (Write Enable, active LOW)

### Control Signal Timing:

**WRITE Cycle:**
1. Set address on address bus
2. Set /CS LOW (select chip)
3. Set data on data bus (DDR = OUTPUT)
4. Set /WE LOW (initiate write)
5. Wait ~1µs (tWP - write pulse width)
6. Set /WE HIGH (latch data)
7. Set /CS HIGH (deselect chip)
8. Set data bus to INPUT (safe state)

**READ Cycle:**
1. Set address on address bus
2. Set data bus to INPUT (prepare to read)
3. Set /CS LOW (select chip)
4. Set /OE LOW (enable output)
5. Wait ~1µs (tACC - access time)
6. Read data from data bus (PINL)
7. Set /OE HIGH (disable output)
8. Set /CS HIGH (deselect chip)

## 4. Test Patterns (All 7)

### Test 1: Basic Read/Write
**Purpose:** Verify basic functionality

**Algorithm:**
1. Write 0xAA to address 0x0000
2. Read back and verify 0xAA
3. Write 0x55 to address 0x0000
4. Read back and verify 0x55
5. Repeat for strategic addresses (first/last, random samples)

**QUICK Mode:** Test ~100 addresses
**FULL Mode:** Test all addresses

**Detects:** Gross failures, stuck bits

---

### Test 2: Walking Ones Address
**Purpose:** Validate all address lines are working and not shorted

**Algorithm:**
```cpp
for (bit = 0; bit < address_bits; bit++) {
    addr = (1 << bit);  // 0x0001, 0x0002, 0x0004, 0x0008, ...
    write(addr, test_pattern);
    verify(addr, test_pattern);
}
```

**Addresses Tested (for 32KB):**
- 0x0001, 0x0002, 0x0004, 0x0008
- 0x0010, 0x0020, 0x0040, 0x0080
- 0x0100, 0x0200, 0x0400, 0x0800
- 0x1000, 0x2000, 0x4000

**Detects:** Shorted address lines, stuck address lines, missing address lines

---

### Test 3: Walking Ones Data
**Purpose:** Validate all data lines are working and not shorted

**Algorithm:**
```cpp
for (bit = 0; bit < 8; bit++) {
    data = (1 << bit);  // 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
    write(test_addr, data);
    verify(test_addr, data);
}
```

**Data Tested:** 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80

**Detects:** Shorted data lines, stuck data lines, bus contention

---

### Test 4: Checkerboard Pattern
**Purpose:** Detect bit coupling and pattern sensitivity

**Algorithm:**
1. Write 0x55 (01010101) to all tested addresses
2. Read back and verify all addresses
3. Write 0xAA (10101010) to all tested addresses
4. Read back and verify all addresses

**QUICK Mode:** Test first/last 1KB + samples
**FULL Mode:** Test entire memory

**Detects:** Bit coupling, pattern-sensitive failures, cell interference

---

### Test 5: Inverse Checkerboard Pattern
**Purpose:** Detect bit coupling with opposite pattern

**Algorithm:**
1. Write 0xAA (10101010) to all tested addresses
2. Read back and verify all addresses
3. Write 0x55 (01010101) to all tested addresses
4. Read back and verify all addresses

**QUICK Mode:** Test first/last 1KB + samples
**FULL Mode:** Test entire memory

**Detects:** Complementary bit coupling, inverse pattern sensitivity

---

### Test 6: Address Equals Data
**Purpose:** Detect address/data bus crosstalk and miswiring

**Algorithm:**
```cpp
// Write phase
for (addr = 0; addr < max; addr++) {
    data = (uint8_t)(addr & 0xFF);  // Low byte of address
    write(addr, data);
}

// Verify phase
for (addr = 0; addr < max; addr++) {
    expected = (uint8_t)(addr & 0xFF);
    actual = read(addr);
    verify(actual == expected);
}
```

**Example:**
- Address 0x0000 → Data 0x00
- Address 0x0055 → Data 0x55
- Address 0x00AA → Data 0xAA
- Address 0x01FF → Data 0xFF
- Address 0x0200 → Data 0x00 (wraps)

**QUICK Mode:** Test strategic ranges
**FULL Mode:** Test entire memory

**Detects:** Address/data crosstalk, swapped address/data lines, miswiring

---

### Test 7: Random Pattern
**Purpose:** Detect random pattern sensitivity and pseudo-random failures

**Algorithm:**
```cpp
// Seed with known value
srand(12345);

// Write phase
for (addr = 0; addr < max; addr++) {
    data = (uint8_t)(rand() & 0xFF);
    write(addr, data);
    save_pattern[addr] = data;  // Save for verification
}

// Verify phase
srand(12345);  // Reset seed
for (addr = 0; addr < max; addr++) {
    expected = (uint8_t)(rand() & 0xFF);
    actual = read(addr);
    verify(actual == expected);
}
```

**Note:** For 32KB, storing pattern requires 32KB RAM (not available). Instead:
- Regenerate random sequence using same seed
- Or use QUICK mode with limited address range

**QUICK Mode:** Test 1KB with saved patterns
**FULL Mode:** Use deterministic random (re-seed for verify)

**Detects:** Pattern-sensitive failures not caught by deterministic tests

## 5. QUICK vs FULL Testing Strategy

### QUICK Mode (Fast - 2-5 seconds):

**Sampling Strategy:**
1. **First 512 bytes** (0x0000 - 0x01FF): Verify low address decoding
2. **Last 512 bytes** (max-512 to max): Verify high address decoding
3. **Strategic samples**: Power-of-2 addresses (walking ones address test)
4. **Random samples**: ~100 addresses throughout memory

**Total addresses tested:** ~1000-1500 (regardless of chip size)

**Advantages:**
- Fast execution
- Catches 95%+ of failures
- Good for production testing

**Trade-offs:**
- May miss localized failures
- Less thorough coverage

---

### FULL Mode (Thorough - 10-30 seconds):

**Strategy:**
- Test **every single byte** from 0x0000 to maxAddress
- For 32KB: 32768 addresses tested
- For 8KB: 8192 addresses tested

**Timing Estimates:**
- Read cycle: ~10µs (with overhead)
- Write cycle: ~10µs (with overhead)
- 32KB full test: ~20 seconds per pattern
- 8KB full test: ~5 seconds per pattern

**Advantages:**
- Complete coverage
- Catches all stuck bits
- Best for qualification testing

**Trade-offs:**
- Slower execution
- May be overkill for known-good chips

## 6. Implementation Strategy

### Class Design:

```cpp
class SRAMStrategy : public ICTestStrategy {
public:
    SRAMStrategy();
    void setSize(uint16_t sizeInBytes);  // Configure for chip size

    // ICTestStrategy interface
    void configurePins() override;
    void reset() override;
    bool runTests() override;  // Default: tests 1-6, QUICK
    const char* getName() const override;

    // Extended interface for test selection
    bool runTest(uint8_t testNumber, bool fullTest);
    bool runAllTests(bool includeRandom, bool fullTest);

private:
    uint16_t sramSize;      // Memory size in bytes (8192 or 32768)
    uint16_t maxAddress;    // Maximum valid address (sramSize - 1)
    uint8_t addressBits;    // Number of address lines needed

    // Low-level hardware access
    void setAddress(uint16_t addr);
    void writeByte(uint16_t addr, uint8_t data);
    uint8_t readByte(uint16_t addr);

    // Test implementations
    bool testBasicReadWrite(bool fullTest);
    bool testWalkingOnesAddress(bool fullTest);
    bool testWalkingOnesData(bool fullTest);
    bool testCheckerboard(bool fullTest);
    bool testInverseCheckerboard(bool fullTest);
    bool testAddressEqualsData(bool fullTest);
    bool testRandomPattern(bool fullTest);

    // Sampling helper
    bool shouldTestAddress(uint16_t addr, bool fullTest);
};
```

### Memory Allocation:

**Static Allocation Only:**
- No dynamic memory (new/delete)
- Pattern buffer for random test: Limited to 1KB max
- All arrays declared as static or stack-based

**RAM Budget:**
- SRAMStrategy instance: ~10 bytes
- Pattern buffer (random test): 1024 bytes (only during test 7)
- Local variables: ~50 bytes
- Total: ~1100 bytes additional

**Current RAM Usage:** 1946 bytes (23.8%)
**After Phase 3:** ~3046 bytes (37.2%) - Still excellent

## 7. Error Reporting

### Test Results Format:

**Success:**
```
OK: Test 1 (Basic Read/Write) - PASSED
```

**Failure:**
```
ERROR: Test 2 (Walking Ones Address) - FAILED
Address: 0x0080 Expected: 0xAA Got: 0x00
Possible cause: Address line A7 stuck LOW
```

**Progress Updates:**
```
Test 4 (Checkerboard): Testing 32768 bytes...
Test 4 (Checkerboard): Progress 25%...
Test 4 (Checkerboard): Progress 50%...
Test 4 (Checkerboard): Progress 75%...
OK: Test 4 (Checkerboard) - PASSED
```

### Diagnostic Information:

For failures, provide actionable guidance:
- **Address line failures**: "Check address line A7 connection"
- **Data line failures**: "Check data line D3 connection"
- **Chip select failures**: "Check /CS, /OE, /WE signals"
- **Pattern failures**: "Possible defective memory cell at address 0x1234"

## 8. Integration with Main Loop

### MODE Command:

```
MODE SRAM <size>
```

**Example:**
```
> MODE SRAM 32768
OK: SRAM mode set: 32768 bytes (HM62256)
Pin configuration complete

> MODE SRAM 8192
OK: SRAM mode set: 8192 bytes (HM6265/D4168)
Pin configuration complete
```

### TEST Command Variants:

```
> TEST
Running tests 1-6 (QUICK mode)...
OK: Test 1 (Basic Read/Write) - PASSED
OK: Test 2 (Walking Ones Address) - PASSED
OK: Test 3 (Walking Ones Data) - PASSED
OK: Test 4 (Checkerboard) - PASSED
OK: Test 5 (Inverse Checkerboard) - PASSED
OK: Test 6 (Address Equals Data) - PASSED
All tests PASSED

> TEST FULL
Running tests 1-6 (FULL mode)...
[Tests run on full memory]

> TEST RANDOM
Running tests 1-7 (QUICK mode)...
[Includes random test]

> TEST 2
Running test 2 (Walking Ones Address) - QUICK mode...
OK: Test 2 - PASSED

> TEST 4 FULL
Running test 4 (Checkerboard) - FULL mode...
OK: Test 4 - PASSED
```

## 9. Safety Considerations

### Bus Contention Prevention:

**Critical Rule:** Never drive data bus OUTPUT while SRAM also drives it!

**Safe Sequence:**
1. Always set data bus to INPUT before reading
2. Only set data bus to OUTPUT during write cycles
3. Return data bus to INPUT after write complete

**Implementation:**
```cpp
void writeByte(uint16_t addr, uint8_t data) {
    setAddress(addr);
    DDRL = 0xFF;           // Data bus OUTPUT
    PORTL = data;          // Put data on bus
    PORTG &= ~(1 << 0);    // /CS LOW
    PORTG &= ~(1 << 3);    // /WE LOW
    delayMicroseconds(1);  // Write pulse
    PORTG |= (1 << 3);     // /WE HIGH
    PORTG |= (1 << 0);     // /CS HIGH
    DDRL = 0x00;           // Data bus INPUT (safe)
}
```

### Electrical Safety:

- VCC = 5V (Arduino Mega provides this)
- All signals are 5V TTL-compatible
- Current limits: < 40 mA per pin
- Use protection: SRAM chips are ESD-sensitive

## 10. Performance Metrics

### Expected Test Times:

**QUICK Mode (1KB sampling):**
- Test 1: < 1 second
- Test 2: < 1 second (15 addresses for 32KB)
- Test 3: < 1 second (8 data patterns)
- Test 4: 2-3 seconds
- Test 5: 2-3 seconds
- Test 6: 2-3 seconds
- Test 7: 2-3 seconds
- **Total (tests 1-6):** ~10-15 seconds
- **Total (tests 1-7):** ~12-18 seconds

**FULL Mode (complete memory):**
- 32KB chip:
  - Test 1: 2 seconds
  - Test 2: 1 second
  - Test 3: 1 second
  - Tests 4-6: 15-20 seconds each
  - Test 7: 20 seconds
  - **Total (tests 1-6):** ~60-80 seconds
  - **Total (tests 1-7):** ~80-100 seconds

- 8KB chip:
  - **Total (tests 1-6):** ~15-20 seconds
  - **Total (tests 1-7):** ~20-25 seconds

### Memory Access Speed:

- **Theoretical:** HM62256 access time = 70ns (14 MHz)
- **Practical (Arduino):** ~10µs per access (including overhead)
- **Bottleneck:** digitalWrite() and Serial communication
- **Optimization:** Direct port manipulation used (PORTA, PORTC, PORTL)

## 11. Future Enhancements (Not Phase 3)

**Stress Testing:**
- Temperature cycling
- Voltage margining (4.5V - 5.5V)
- Extended pattern testing

**Advanced Diagnostics:**
- Bit error rate calculation
- Retention testing (write, wait, verify)
- Speed characterization

**Additional Patterns:**
- Sliding diagonal
- March algorithms (March C-, March A, MATS+)
- Hammer testing (row disturb)

---

## Summary

Phase 3 implements a robust, generic SRAM testing framework supporting chips from 8KB to 32KB. The strategy uses direct memory access with careful control signal timing, comprehensive test patterns to catch various failure modes, and user-selectable test coverage (QUICK vs FULL).

The implementation uses static memory allocation, direct port manipulation for speed, and provides clear diagnostic information for failures. With 7 test patterns and 2 coverage modes, the framework can adapt from quick production testing to thorough qualification testing.

**Phase 3 Success Criteria:**
- [ ] MODE SRAM <size> accepts 8192 and 32768
- [ ] All 7 test patterns implemented
- [ ] QUICK and FULL modes working
- [ ] Test selection working (individual tests 1-7)
- [ ] All tests pass on known-good SRAM chips
- [ ] Clear error messages for failures
- [ ] Memory usage < 40% RAM
- [ ] Flash usage < 10%
- [ ] No bus contention or electrical issues
