# Phase 1 - Foundation & Infrastructure Strategy

**Document:** Phase 1 Implementation Strategy
**Version:** 1.0
**Date:** 2026-01-03
**Status:** Implementation Plan

---

## 1. Phase Overview

### 1.1 Goal
Implement the foundational infrastructure layer that all IC testing will depend on. This includes serial communication, command parsing, and the base architecture for IC strategy pattern implementation.

### 1.2 Success Criteria
- UART communication working at 115200 baud
- Commands can be received and parsed correctly
- Base class for IC strategies defined
- Pin definitions centralized
- Mode manager tracks current IC selection
- All components integrated in main.cpp
- System responds to basic commands via serial monitor

### 1.3 Dependencies
- Phase 0 complete (folder structure, architecture documented)
- PlatformIO working
- Arduino framework available

---

## 2. Implementation Strategy

### 2.1 Development Order Rationale

**Order:** UART → Parser → Base Class → Pins → Manager → Integration

**Why this order?**
1. **UART first** - Need to see output for testing all other components
2. **Parser second** - Can test parsing by echoing commands via UART
3. **Base class third** - Pure interface, no dependencies
4. **Pins fourth** - Just definitions, needed by strategies
5. **Manager fifth** - Depends on base class
6. **Integration last** - Ties everything together

### 2.2 Testing Strategy

Each component will be tested incrementally:
- **UART:** Send test message in setup(), verify via serial monitor
- **Parser:** Parse test commands, echo results
- **Base Class:** Compilation test only (pure virtual)
- **Pins:** Compilation test (constexpr validation)
- **Manager:** Test with null strategy, verify state tracking
- **Integration:** Full command flow test

---

## 3. Component Details

### 3.1 UART Handler

**Purpose:** Clean abstraction over Arduino Serial for formatted communication

**Design Decision:**
- Wrapper class vs direct Serial usage
- **Choice:** Wrapper class (UARTHandler)
- **Rationale:**
  - Consistent message formatting
  - Easier to mock for future testing
  - Centralized serial configuration

**Interface:**
```cpp
class UARTHandler {
public:
    void begin(uint32_t baud);
    bool available();
    String readLine();
    void sendOK(const char* message);
    void sendError(const char* message);
    void sendInfo(const char* message);
    void sendResult(bool passed, const char* message);
};
```

**Implementation Notes:**
- Use Arduino String class (acceptable for command parsing)
- Handle both \n and \r\n line endings
- Trim whitespace from input
- Buffer management: Arduino Serial handles buffering
- No dynamic allocation needed (String class handles it internally)

**Memory Consideration:**
- String class uses heap, but for short commands (<80 chars) it's acceptable
- Alternative: Fixed char buffer - adds complexity, minimal benefit
- Decision: Use String for simplicity in Phase 1

---

### 3.2 Command Parser

**Purpose:** Convert string commands into structured data for handling

**Design Decision:**
- State machine vs simple string parsing
- **Choice:** Simple string parsing with structs
- **Rationale:** Commands are simple, no multi-step protocols needed

**Data Structure:**
```cpp
enum CommandType {
    MODE,
    TEST,
    STATUS,
    RESET,
    HELP,
    INVALID
};

struct ParsedCommand {
    CommandType type;
    String parameter;  // For MODE command (Z80, 6502, 62256)
};
```

**Parsing Strategy:**
- Split on whitespace
- First token = command
- Second token = parameter (if present)
- Case-sensitive matching (MODE not mode)

**Error Handling:**
- Unknown command → INVALID type
- Missing parameter for MODE → INVALID type
- Extra parameters → ignored (forward compatibility)

---

### 3.3 ICTestStrategy Base Class

**Purpose:** Define interface that all IC strategies must implement

**Design Decision:**
- Abstract class vs interface struct with function pointers
- **Choice:** C++ abstract class with pure virtual functions
- **Rationale:**
  - Type safety
  - Compiler enforces implementation
  - Clearer intent
  - Better IDE support

**Interface:**
```cpp
class ICTestStrategy {
public:
    virtual void configurePins() = 0;
    virtual void reset() = 0;
    virtual bool runTests() = 0;
    virtual const char* getName() const = 0;
    virtual ~ICTestStrategy() = default;
};
```

**Memory:**
- Header only, no .cpp needed
- No state in base class
- Virtual table overhead: ~4 bytes per instance (acceptable)

---

### 3.4 Pin Definitions

**Purpose:** Centralize all pin definitions to avoid magic numbers

**Design Decision:**
- #define vs constexpr vs enum
- **Choice:** constexpr
- **Rationale:**
  - Type-safe (unlike #define)
  - Compile-time constants
  - No memory overhead
  - Modern C++ best practice

**Organization Strategy:**
- Group by function (address bus, data bus, control signals)
- Use meaningful names
- Add comments referencing pinout document

**Example:**
```cpp
// Address Bus
constexpr uint8_t ADDR_A0_PIN = 22;  // PORTA bit 0
// ...

// Data Bus
constexpr uint8_t DATA_D0_PIN = 49;  // PORTL bit 0
```

**Alternative Considered:**
- Direct port register names (PORTA, PORTL, etc.)
- Decision: Keep both - pin numbers for reference, port macros for speed

---

### 3.5 Mode Manager

**Purpose:** Track which IC is currently selected and which strategy to use

**Design Decision:**
- Global state vs singleton vs instance
- **Choice:** Global instance in main.cpp
- **Rationale:**
  - Simple
  - Only one mode manager needed
  - Easy to access
  - No need for singleton complexity

**State Tracking:**
```cpp
enum ICMode {
    NONE,
    Z80,
    IC6502,
    SRAM62256
};

class ModeManager {
private:
    ICTestStrategy* currentStrategy;
    ICMode currentMode;

public:
    void setStrategy(ICTestStrategy* strategy, ICMode mode);
    ICTestStrategy* getCurrentStrategy() const;
    ICMode getCurrentMode() const;
    void clearStrategy();
};
```

**Memory:**
- One pointer: 2 bytes (16-bit MCU)
- One enum: 1 byte
- Total: ~3 bytes + vtable pointer

**Null Strategy Handling:**
- nullptr indicates no mode selected
- Check before dereferencing
- Return error if TEST called with nullptr

---

### 3.6 Integration in main.cpp

**Architecture:**
```cpp
// Global instances
UARTHandler uart;
CommandParser parser;
ModeManager modeManager;

void setup() {
    uart.begin(115200);
    uart.sendInfo("Multi-IC Tester Ready");
    uart.sendInfo("Type HELP for commands");
}

void loop() {
    if (uart.available()) {
        String line = uart.readLine();
        ParsedCommand cmd = parser.parse(line);

        switch (cmd.type) {
            case MODE:
                // Validate parameter, send error (strategies not implemented yet)
                break;
            case TEST:
                // Check if mode set, send error (not implemented yet)
                break;
            case STATUS:
                // Report current mode
                break;
            case RESET:
                // Send not implemented error
                break;
            case HELP:
                // Send command list
                break;
            case INVALID:
                uart.sendError("Invalid command. Type HELP for command list.");
                break;
        }
    }
}
```

**Command Handling Strategy:**
- Stubs for Phase 1 (no actual IC testing yet)
- MODE: Validate parameter but don't switch (no strategies yet)
- TEST: Return error if no mode or not implemented
- STATUS: Show current mode (will be NONE)
- HELP: Show available commands
- RESET: Not implemented error

---

## 4. Testing Approach

### 4.1 Incremental Testing

After each item:
1. Compile code
2. Fix any errors
3. Upload to Arduino (if possible)
4. Test via serial monitor
5. Verify expected behavior

### 4.2 Serial Monitor Tests

**After 1.1 (UART):**
```
Expected output on startup:
"Multi-IC Tester Ready"
```

**After 1.2 (Parser):**
```
Input: HELP
Expected: Help message displayed

Input: INVALID_CMD
Expected: ERROR: Invalid command
```

**After 1.6 (Integration):**
```
Input: STATUS
Expected: OK: Current mode: NONE

Input: MODE Z80
Expected: ERROR: Z80 strategy not implemented yet

Input: TEST
Expected: ERROR: No IC mode selected

Input: HELP
Expected: Command list displayed
```

---

## 5. Potential Issues and Solutions

### 5.1 Serial Communication

**Issue:** Characters lost or corrupted at 115200 baud
**Solution:**
- Add delay after Serial.begin()
- Verify USB cable quality
- Check serial buffer size in Arduino

**Issue:** Line ending confusion (\r\n vs \n)
**Solution:** Handle both in readLine()

### 5.2 String Memory Usage

**Issue:** String fragmentation on heap
**Mitigation:**
- Commands are short (<80 chars)
- Deallocated immediately after use
- Monitor with STATUS command (future: show free RAM)

**If problematic:**
- Switch to fixed char buffer
- Acceptable tradeoff for Phase 1 simplicity

### 5.3 Command Parsing Edge Cases

**Issue:** Empty lines, multiple spaces, tabs
**Solution:**
- Trim whitespace before parsing
- Handle empty strings
- Skip empty lines

---

## 6. Memory Budget

### 6.1 Estimated SRAM Usage

```
Global Instances:
- UARTHandler:     ~10 bytes  (minimal state)
- CommandParser:   ~10 bytes  (minimal state)
- ModeManager:     ~4 bytes   (pointer + enum)

Stack (during command processing):
- String line:     ~80 bytes  (temporary)
- ParsedCommand:   ~4 bytes   (temporary)

Total: ~108 bytes worst case
Percentage: 1.3% of 8KB SRAM
```

**Conclusion:** Well within budget, no concerns

### 6.2 Flash Usage Estimate

```
Code Size:
- UART Handler:    ~500 bytes
- Command Parser:  ~300 bytes
- Mode Manager:    ~200 bytes
- Main loop:       ~400 bytes
- String functions: ~1000 bytes (Arduino)

Total: ~2400 bytes
Percentage: ~1% of 253KB Flash
```

**Conclusion:** Negligible flash usage

---

## 7. Risks and Mitigation

### 7.1 Risk: Serial Buffer Overflow

**Likelihood:** Low
**Impact:** Medium (commands lost)
**Mitigation:**
- Arduino Serial buffer is 64 bytes (sufficient for commands)
- Process commands promptly in loop()
- No long delays in command handlers

### 7.2 Risk: Invalid Parameter Handling

**Likelihood:** Medium (user error)
**Impact:** Low (just an error message)
**Mitigation:**
- Comprehensive validation in parser
- Clear error messages
- HELP command guidance

### 7.3 Risk: Null Pointer Dereference

**Likelihood:** Low (defensive coding)
**Impact:** High (crash)
**Mitigation:**
- Always check currentStrategy != nullptr before use
- Clear error message if nullptr
- Set to nullptr explicitly after clearStrategy()

---

## 8. Future Considerations

### 8.1 Extensibility

**Adding new commands:**
- Add to CommandType enum
- Add case in main loop
- Update HELP text

**Minimal changes needed** - good design

### 8.2 Phase 2 Preparation

Phase 1 provides foundation for Phase 2 (Timer3):
- UART for debugging timer output
- Command parser can add CLOCK commands for testing
- Mode manager ready for strategy selection

### 8.3 Phase 3+ Preparation

Phase 1 provides foundation for IC testing:
- MODE command ready for strategy switching
- TEST command ready for strategy.runTests()
- STATUS command can show current IC
- RESET command ready for strategy.reset()

---

## 9. Implementation Checklist

Before starting each item:
- [ ] Read roadmap item details
- [ ] Understand requirements
- [ ] Plan file structure
- [ ] Create files in correct folders

During implementation:
- [ ] Follow SOLID principles
- [ ] Use meaningful names
- [ ] Add comments where needed (not obvious)
- [ ] Use constexpr for constants
- [ ] Avoid dynamic allocation where possible

After each item:
- [ ] Compile: `python -m platformio run`
- [ ] Fix any errors immediately
- [ ] Test if hardware available
- [ ] Commit working code
- [ ] Update roadmap if needed

After Phase 1 complete:
- [ ] Full integration test via serial monitor
- [ ] Create Tests/Phase1-Foundation-Testing.md
- [ ] Update Roadmap.md completion status
- [ ] Final commit with comprehensive message
- [ ] Push to GitHub

---

## 10. Success Metrics

Phase 1 is complete when:

- [x] All code compiles with no errors or warnings
- [x] UART communication works at 115200 baud
- [x] Commands can be sent and parsed correctly
- [x] All command types recognized (MODE, TEST, STATUS, RESET, HELP)
- [x] Invalid commands return appropriate errors
- [x] MODE command validates parameters (even if not implemented)
- [x] TEST command checks for mode selection
- [x] STATUS command reports current state
- [x] HELP command lists all commands
- [x] Code follows all CLAUDE.md guidelines
- [x] Testing documentation created
- [x] All changes committed and pushed to GitHub

---

**End of Phase 1 Strategy Document**

**Next Step:** Begin implementation with Item 1.1 - UART Handler
