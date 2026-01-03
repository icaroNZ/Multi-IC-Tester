# Overall Architecture Strategy

**Document:** Multi-IC Tester - System Architecture
**Version:** 1.0
**Date:** 2026-01-03
**Status:** Architecture Design Document

---

## 1. System Overview

### 1.1 Purpose

This document defines the software architecture for the Multi-IC Tester firmware. The system must test three different IC types (Z80, 6502, HM62256) using shared hardware resources on an Arduino Mega 2560, with the ability to add new IC types in the future without modifying existing code.

### 1.2 Core Requirements

1. **Extensibility:** New ICs can be added without changing existing code
2. **Safety:** Prevent bus contention and hardware damage
3. **Reliability:** Static memory allocation only (no heap fragmentation)
4. **Clarity:** Clean separation of concerns
5. **Testability:** Each IC's logic is independent and encapsulated

### 1.3 Key Constraints

- **Platform:** Arduino Mega 2560 (ATmega2560, 16 MHz, 8KB SRAM)
- **Memory:** Static allocation only - no `new`/`delete`
- **Real-time:** Bus cycle handling must be fast (microsecond response)
- **Shared Resources:** One physical bus, multiple IC modes
- **UART:** 115200 baud, command-response protocol

---

## 2. Architectural Patterns

### 2.1 Strategy Pattern (Primary Pattern)

**Problem:** Different ICs require completely different testing logic, pin configurations, and bus handling.

**Solution:** Each IC type implements the `ICTestStrategy` interface. The system selects the appropriate strategy at runtime based on user commands.

**Benefits:**
- **Open/Closed Principle:** Open for extension (new ICs), closed for modification (existing code unchanged)
- **Single Responsibility:** Each strategy handles exactly one IC type
- **Polymorphism:** Runtime selection of appropriate behavior

**Implementation:**

```cpp
// Abstract base class (interface)
class ICTestStrategy {
public:
    virtual void configurePins() = 0;      // Configure DDR/PORT registers for this IC
    virtual void reset() = 0;              // Reset the IC
    virtual bool runTests() = 0;           // Run all tests, return true if pass
    virtual const char* getName() const = 0; // Get IC name for display
    virtual ~ICTestStrategy() = default;   // Virtual destructor
};

// Concrete strategies
class Z80Strategy : public ICTestStrategy {
    // Z80-specific implementation
    uint8_t rom[4096];  // ROM emulation buffer
    uint8_t ram[2048];  // RAM emulation buffer
    Timer3Clock* clock; // Reference to shared clock

    void configurePins() override { /* Z80 pin config */ }
    void reset() override { /* Toggle PH6 for Z80 */ }
    bool runTests() override { /* Run 5 Z80 tests */ }
    const char* getName() const override { return "Z80"; }

private:
    void handleBusCycle(); // Z80 bus cycle handler
    bool testControlSignals();
    bool testSimpleExecution();
    // ... other test methods
};

class IC6502Strategy : public ICTestStrategy {
    // 6502-specific implementation with signal inversions
    // Similar structure to Z80Strategy but different logic
};

class SRAM62256Strategy : public ICTestStrategy {
    // SRAM testing - no CPU emulation needed
    // Direct memory access, 7 test patterns
};
```

**Runtime Selection:**

```cpp
// In main.cpp
ICTestStrategy* currentStrategy = nullptr;

// User sends: MODE Z80
if (parameter == "Z80") {
    currentStrategy = &z80Strategy;  // Select Z80 strategy
    currentStrategy->configurePins(); // Configure for Z80
}

// User sends: TEST
if (currentStrategy != nullptr) {
    bool passed = currentStrategy->runTests(); // Polymorphic call
}
```

### 2.2 State Pattern (Mode Management)

**Problem:** System has multiple modes (NONE, Z80, 6502, SRAM62256) with different behaviors.

**Solution:** `ModeManager` class tracks current mode and current strategy.

```cpp
class ModeManager {
public:
    enum ICMode { NONE, Z80, IC6502, SRAM62256 };

    void setStrategy(ICTestStrategy* strategy, ICMode mode);
    void clearStrategy();
    ICTestStrategy* getCurrentStrategy() const;
    ICMode getCurrentMode() const;

private:
    ICTestStrategy* currentStrategy;  // Current strategy pointer
    ICMode currentMode;                // Current mode enum
};
```

**State Transitions:**
- NONE → Z80/6502/SRAM62256 (via MODE command)
- Z80 → 6502 (via MODE command, reconfigure pins)
- Any → NONE (clear mode)

**Safety:** Always clear previous mode before setting new one to prevent mixed pin configurations.

### 2.3 Command Pattern (Implicit)

**Problem:** UART commands need to be parsed and dispatched.

**Solution:** `CommandParser` parses strings into structured commands, main.cpp dispatches to handlers.

```cpp
struct ParsedCommand {
    enum CommandType { MODE, TEST, STATUS, RESET, HELP, INVALID };
    CommandType type;
    String parameter; // For MODE command
};

class CommandParser {
public:
    ParsedCommand parse(String line);
};
```

**Command Flow:**
```
UART → UARTHandler.readLine() → CommandParser.parse() → main.cpp switch/case → Handler
```

---

## 3. System Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                         Arduino Mega 2560                        │
│                                                                  │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │                      main.cpp (Main Loop)                   │ │
│  │                                                             │ │
│  │  ┌──────────────┐     ┌─────────────┐    ┌──────────────┐ │ │
│  │  │ UARTHandler  │────▶│CommandParser│───▶│ MODE Handler │ │ │
│  │  └──────────────┘     └─────────────┘    └──────┬───────┘ │ │
│  │        ▲                                         │         │ │
│  │        │                                         ▼         │ │
│  │        │                                  ┌──────────────┐ │ │
│  │   Response                                │ ModeManager  │ │ │
│  │        │                                  └──────┬───────┘ │ │
│  │        │                                         │         │ │
│  │        │                                         ▼         │ │
│  │  ┌─────┴──────┐                    ┌────────────────────┐ │ │
│  │  │TEST Handler│                    │ ICTestStrategy*    │ │ │
│  │  └─────┬──────┘                    │  (current)         │ │ │
│  │        │                           └──────┬─────────────┘ │ │
│  │        └──────────────────────────────────┘               │ │
│  │                                            │               │ │
│  └────────────────────────────────────────────┼───────────────┘ │
│                                               │                 │
│  ┌────────────────────────────────────────────┼───────────────┐ │
│  │              Strategy Layer (Polymorphic)  ▼               │ │
│  │  ┌─────────────────┐  ┌──────────────────┐ ┌────────────┐ │ │
│  │  │  Z80Strategy    │  │ IC6502Strategy   │ │SRAM62256   │ │ │
│  │  │                 │  │                  │ │Strategy    │ │ │
│  │  │ - rom[4K]       │  │ - rom[4K]        │ │            │ │ │
│  │  │ - ram[2K]       │  │ - ram[2K]        │ │ - tests    │ │ │
│  │  │ - busCycle()    │  │ - busCycle()     │ │ - direct   │ │ │
│  │  │ - 5 tests       │  │ - 5 tests        │ │   memory   │ │ │
│  │  └────────┬────────┘  └─────────┬────────┘ └──────┬─────┘ │ │
│  │           │                     │                  │       │ │
│  └───────────┼─────────────────────┼──────────────────┼───────┘ │
│              │                     │                  │         │
│  ┌───────────┼─────────────────────┼──────────────────┼───────┐ │
│  │        Hardware Abstraction Layer                  │       │ │
│  │  ┌───────▼────────┐    ┌────────▼──────┐  ┌───────▼─────┐ │ │
│  │  │  Timer3Clock   │    │ BusController  │  │  PinConfig  │ │ │
│  │  │  (PE3 PWM)     │    │  (PORTA/C/L)   │  │(constexpr)  │ │ │
│  │  └────────────────┘    └────────────────┘  └─────────────┘ │ │
│  └────────────────────────────────────────────────────────────┘ │
│                                                                  │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │                   Hardware (Registers)                      │ │
│  │    PORTA/DDRA  PORTC/DDRC  PORTL/DDRL/PINL  PORTG/H/B/E/D  │ │
│  └────────────────────────────────────────────────────────────┘ │
│                                   │                              │
└───────────────────────────────────┼──────────────────────────────┘
                                    │
                    ┌───────────────┴───────────────┐
                    │     Physical Bus & ICs        │
                    │  (Z80 or 6502 or HM62256)     │
                    └───────────────────────────────┘
```

---

## 4. Memory Allocation Strategy

### 4.1 Static Allocation Only

**Rule:** NO dynamic allocation (`new`, `delete`, `malloc`, `free`) anywhere in code.

**Rationale:**
- Avoid heap fragmentation on small 8KB SRAM
- Deterministic memory usage
- No risk of allocation failures during testing
- Faster execution (no heap management overhead)

### 4.2 Memory Layout

**Global/Static Objects (in main.cpp):**
```cpp
// Infrastructure (small)
UARTHandler uart;                    // ~50 bytes
CommandParser parser;                // ~20 bytes
ModeManager modeManager;             // ~10 bytes

// Strategy instances (large)
Z80Strategy z80Strategy;             // ~6KB (4KB ROM + 2KB RAM + code)
IC6502Strategy ic6502Strategy;       // ~6KB (4KB ROM + 2KB RAM + code)
SRAM62256Strategy sramStrategy;      // ~100 bytes (no buffers, direct access)

// Hardware abstractions
Timer3Clock timer3;                  // ~20 bytes
```

**Total Estimated SRAM Usage:** ~12.5 KB

**Concern:** This exceeds 8KB SRAM!

**Solution:**
1. **Lazy ROM/RAM buffers:** Use `PROGMEM` for test programs (stored in Flash, not SRAM)
2. **Shared buffers:** Z80 and 6502 never run simultaneously - share ROM/RAM buffers
3. **Reduce buffer sizes:** Use 1KB ROM instead of 4KB for testing

**Revised Shared Buffer Approach:**

```cpp
// Shared between Z80 and 6502 (only one active at a time)
uint8_t sharedROM[1024];  // 1KB ROM buffer
uint8_t sharedRAM[512];   // 512 bytes RAM buffer

// Pass to strategies
Z80Strategy z80Strategy(sharedROM, sharedRAM);
IC6502Strategy ic6502Strategy(sharedROM, sharedRAM);
```

**New Estimated SRAM:** ~2KB - well within limits!

### 4.3 Stack Usage

- Avoid deep recursion
- Avoid large local arrays
- Use references instead of copies
- Keep stack usage < 1KB

---

## 5. Bus Contention Prevention

### 5.1 The Problem

**Critical Safety Issue:** If Arduino drives data bus OUTPUT while IC also drives it OUTPUT, both chips can be damaged (bus contention).

### 5.2 Data Bus State Machine

**Safe State:** Data bus = INPUT (`DDRL = 0x00`)

**State Transitions:**

```
                  ┌─────────┐
                  │  INPUT  │  (Safe default)
                  └────┬────┘
                       │
              ┌────────┴────────┐
              │                 │
          Write to IC       Read from IC
              │                 │
              ▼                 ▼
        ┌──────────┐      ┌──────────┐
        │  OUTPUT  │      │  INPUT   │
        │ (write   │      │ (already │
        │  data)   │      │  INPUT)  │
        └─────┬────┘      └────┬─────┘
              │                 │
        Finish write      Finish read
              │                 │
              ▼                 ▼
          ┌─────────┐      ┌─────────┐
          │  INPUT  │      │  INPUT  │
          │ (safe)  │      │ (safe)  │
          └─────────┘      └─────────┘
```

**Rules:**
1. **Default state:** Always INPUT
2. **Before OUTPUT:** Verify IC is not driving bus (check control signals)
3. **After OUTPUT:** Return to INPUT immediately
4. **On mode switch:** Force INPUT before reconfiguring pins
5. **On error:** Emergency return to INPUT

### 5.3 Implementation Pattern

```cpp
// Reading from IC (Z80 example)
void handleBusCycleRead(uint16_t addr) {
    // Data bus already INPUT - safe
    uint8_t data = lookupMemory(addr);

    // Switch to OUTPUT
    DDRL = 0xFF;
    PORTL = data;

    // Wait for IC to read
    delayMicroseconds(1);

    // CRITICAL: Return to INPUT
    DDRL = 0x00;
}

// Writing to IC
void handleBusCycleWrite(uint16_t addr) {
    // Data bus must be INPUT
    // IC is driving data
    uint8_t data = PINL;
    storeMemory(addr, data);
    // Data bus stays INPUT - safe
}
```

---

## 6. Command Flow Architecture

### 6.1 Command Processing Pipeline

```
User Input (Serial Terminal)
    │
    ▼
"MODE Z80\n"
    │
    ▼
┌─────────────────┐
│ UARTHandler     │ readLine() → "MODE Z80"
└────────┬────────┘
         ▼
┌─────────────────┐
│ CommandParser   │ parse() → {type: MODE, param: "Z80"}
└────────┬────────┘
         ▼
┌─────────────────┐
│ main.cpp        │ switch(command.type)
│ Command Handler │
└────────┬────────┘
         ▼
    case MODE:
         ▼
┌─────────────────────────┐
│ MODE Handler Logic:     │
│ 1. Validate param       │
│ 2. Stop clocks          │
│ 3. Clear current mode   │
│ 4. Configure pins       │
│ 5. Set new strategy     │
│ 6. Send OK response     │
└────────┬────────────────┘
         ▼
┌─────────────────┐
│ UARTHandler     │ sendOK("Switched to Z80 mode")
└────────┬────────┘
         ▼
Serial Terminal
"OK: Switched to Z80 mode"
```

### 6.2 Error Handling Flow

```
Error Condition Detected
    │
    ▼
┌─────────────────────────┐
│ Set data bus to INPUT   │ (Safety first!)
└────────┬────────────────┘
         ▼
┌─────────────────────────┐
│ Stop any running clocks │
└────────┬────────────────┘
         ▼
┌─────────────────────────┐
│ UARTHandler.sendError() │
└────────┬────────────────┘
         ▼
"ERROR: <description>"
```

---

## 7. Key Design Decisions

### 7.1 Why Strategy Pattern?

**Alternatives Considered:**
1. **Giant switch statement** - Violates Open/Closed, hard to maintain
2. **Separate firmwares** - User must flash different firmware for each IC
3. **Function pointers** - Less type-safe, harder to understand

**Why Strategy Wins:**
- Clean separation of IC-specific logic
- Easy to add new ICs (just add new strategy class)
- Type-safe polymorphism
- Follows SOLID principles

### 7.2 Why Static Allocation?

**Alternatives Considered:**
1. **Dynamic allocation** - Risk of fragmentation, allocation failures

**Why Static Wins:**
- Predictable memory usage
- No runtime allocation failures
- Faster (no heap management)
- Safer for embedded systems

### 7.3 Why Shared ROM/RAM Buffers?

**Alternatives Considered:**
1. **Separate buffers for each CPU** - Exceeds SRAM budget
2. **No buffers, use PROGMEM** - Too slow for bus cycle handling

**Why Shared Wins:**
- Only one CPU tested at a time (architectural constraint)
- Saves ~6KB SRAM
- Simple to implement (just pass references)

### 7.4 Why Direct Port Manipulation?

**Alternatives Considered:**
1. **Arduino digitalWrite/digitalRead** - Too slow for bus cycles

**Why Direct Ports Win:**
- 50x faster (critical for bus cycle handling)
- Full control over DDR registers
- Industry standard for performance-critical Arduino code

---

## 8. Class Relationships

### 8.1 Inheritance Hierarchy

```
ICTestStrategy (abstract)
    ├── Z80Strategy
    ├── IC6502Strategy
    └── SRAM62256Strategy
```

### 8.2 Composition Relationships

```
main.cpp
    ├── owns: UARTHandler
    ├── owns: CommandParser
    ├── owns: ModeManager
    │       └── references: ICTestStrategy* (current)
    ├── owns: Z80Strategy
    │       └── references: Timer3Clock*
    ├── owns: IC6502Strategy
    │       └── references: Timer3Clock*
    ├── owns: SRAM62256Strategy
    └── owns: Timer3Clock
```

### 8.3 Dependency Rules

**Dependencies flow one direction:**
```
main.cpp
    ↓ depends on
Strategies (Z80Strategy, etc.)
    ↓ depends on
Hardware Abstractions (Timer3Clock, BusController)
    ↓ depends on
AVR Registers (PORTA, DDRL, etc.)
```

**No circular dependencies!**

---

## 9. Testing Strategy (Development, Not Runtime Tests)

### 9.1 Incremental Testing

Each phase in roadmap builds and tests incrementally:

1. **Phase 1:** Test UART echo, command parsing separately
2. **Phase 2:** Test Timer3 with LED on PE3
3. **Phase 3:** Test SRAM completely before CPU testing
4. **Phase 4-5:** Test Z80, then 6502
5. **Phase 6:** Integration testing

### 9.2 Serial Monitor Testing

All testing via serial monitor:
- Send commands, verify responses
- Check PASS/FAIL results
- Verify error handling

### 9.3 Hardware Testing

- Use actual ICs to verify tests work
- Use oscilloscope to verify clock output
- Use LED on data bus to verify bus activity

---

## 10. Future Extensibility

### 10.1 Adding a New IC (e.g., 8085 CPU)

**Steps:**
1. Create `IC8085Strategy.h` and `.cpp`
2. Inherit from `ICTestStrategy`
3. Implement 4 virtual methods
4. Add to main.cpp: `IC8085Strategy ic8085Strategy;`
5. Add to MODE handler: `if (param == "8085") ...`

**Files Changed:**
- `src/strategies/IC8085Strategy.h` (new)
- `src/strategies/IC8085Strategy.cpp` (new)
- `src/main.cpp` (MODE handler only)
- `Roadmap/Roadmap.md` (update with new phase)

**Files NOT Changed:**
- All other strategies (Z80, 6502, SRAM62256)
- Infrastructure (UART, Parser, ModeManager)
- Hardware abstractions (Timer3, PinConfig)

**Effort:** ~1-2 days for simple IC

### 10.2 Adding New Test Patterns

**To SRAM (e.g., Test 8: March C-):**
1. Add method to `SRAM62256Strategy`: `bool testMarchC()`
2. Call from `runTests()`
3. Update progress messages

**Files Changed:**
- `src/strategies/SRAM62256Strategy.cpp` only

---

## 11. Critical Invariants (Must Always Be True)

1. **Data bus direction:** Always INPUT except during verified write cycle
2. **Mode consistency:** `modeManager.currentMode` matches `modeManager.currentStrategy`
3. **Single IC:** Only one IC in socket at a time (enforced by hardware, trusted by firmware)
4. **Command atomicity:** Each command completes before next is processed
5. **Clock safety:** Clock stopped before mode switch
6. **Memory bounds:** All array accesses checked against buffer size
7. **Static allocation:** Zero calls to `new`, `delete`, `malloc`, `free`

---

## 12. Performance Considerations

### 12.1 Bus Cycle Timing

**Z80 at 100 kHz:**
- Clock period: 10 µs
- Bus cycle: ~3-4 clock periods = 30-40 µs
- Arduino must respond in < 30 µs

**Arduino @ 16 MHz:**
- Single instruction: 62.5 ns
- Available cycles per bus cycle: ~480 cycles
- **Conclusion:** Plenty of time if using direct port access

### 12.2 Optimization Targets

**Critical (must be fast):**
- Bus cycle handling (Z80/6502)
- Data bus direction switching
- Address decoding

**Non-critical (can be slow):**
- UART communication
- Command parsing
- Status reporting

**Optimization Techniques:**
- Use direct port access (PORTA, DDRL, etc.)
- Use lookup tables instead of calculations
- Inline small functions
- Avoid Serial.print in tight loops

---

## 13. Revision History

| Version | Date       | Changes                          |
|---------|------------|----------------------------------|
| 1.0     | 2026-01-03 | Initial architecture design      |

---

## 14. References

- `Documents/Multi-IC_Firmware_Spec.md` - Detailed firmware specification
- `Documents/Multi-IC_Tester_Pinout.md` - Hardware pin mapping
- `Roadmap/Roadmap.md` - Development roadmap
- `CLAUDE.md` - Development guidelines

---

**Next Steps:**

1. Review this architecture with team/user
2. Proceed to Phase 0 of roadmap (verify PlatformIO)
3. Create phase-specific strategy documents as needed
4. Begin incremental implementation

---

**End of Architecture Strategy Document**
