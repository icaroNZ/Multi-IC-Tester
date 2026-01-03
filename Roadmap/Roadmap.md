# Multi-IC Tester - Development Roadmap

This roadmap guides the incremental development of the Multi-IC Tester firmware. Each phase builds upon the previous ones, and each item is designed to be completable in one work session (~30-60 minutes).

**Status Legend:**
- ⬜ Not Started
- 🔄 In Progress
- ✅ Completed
- ⚠️ Needs Review

---

## Phase 0: Setup & Architecture ⬜

**Goal:** Establish project structure, create strategy documents, and set up development environment.

**Prerequisites:** None

**Description:** This phase sets up the foundation for development. We'll create the folder structure, write architecture strategy documents, and ensure PlatformIO is properly configured. No code implementation yet - just planning and setup.

### Items:

#### 0.1 Verify PlatformIO Installation ⬜
**Description:** Ensure PlatformIO is installed and can compile the skeleton code.
**Details:**
- Run `pio --version` to check installation
- If not installed, install PlatformIO CLI or IDE
- Verify `pio run` compiles the skeleton main.cpp successfully
- Fix any platform or framework issues
**Output:** Successfully compiled firmware (even if it's just the skeleton)

#### 0.2 Create Folder Structure ⬜
**Description:** Create all necessary folders for organized development.
**Details:**
- Create `src/strategies/` folder for IC strategy classes
- Create `src/hardware/` folder for hardware abstraction (Timer3, BusController)
- Create `src/utils/` folder for utilities (UART, CommandParser)
- Create `include/` subfolders if needed for shared headers
- Verify folder structure matches CLAUDE.md guidelines
**Output:** Complete folder structure ready for code files

#### 0.3 Create Overall Architecture Strategy Document ⬜
**Description:** Document the high-level system architecture before coding.
**Details:**
- Create `Strategy/00-Overall-Architecture.md`
- Document the Strategy Pattern approach for IC handling
- Explain the command flow: UART → Parser → Strategy → Hardware
- Diagram the class relationships
- Document memory allocation strategy (static only)
- Explain bus contention prevention approach
- Document state management (current mode, current strategy)
**Output:** `Strategy/00-Overall-Architecture.md` with complete architecture design

---

## Phase 1: Foundation & Infrastructure ⬜

**Goal:** Implement core infrastructure - UART communication, command parser, and strategy pattern base classes.

**Prerequisites:** Phase 0 completed

**Description:** Build the foundational layer that all IC testing will use. This includes serial communication, command parsing, and the base abstract class for IC strategies. By the end of this phase, you should be able to receive commands via UART and route them to the appropriate (stub) handler.

**Strategy Document:** Create `Strategy/01-Phase1-Foundation.md` before starting this phase.

### Items:

#### 1.1 Implement UART Handler ⬜
**Description:** Create a UART communication wrapper for clean serial I/O.
**Details:**
- Create `src/utils/UARTHandler.h` and `.cpp`
- Class: `UARTHandler`
- Methods:
  - `void begin(uint32_t baud)` - Initialize serial at 115200
  - `bool available()` - Check if data available
  - `String readLine()` - Read until '\n', trim whitespace
  - `void sendOK(const char* message)` - Send "OK: message\n"
  - `void sendError(const char* message)` - Send "ERROR: message\n"
  - `void sendInfo(const char* message)` - Send "message\n"
  - `void sendResult(bool passed, const char* message)` - Send "RESULT: PASS/FAIL\n"
- Use Arduino Serial class internally
- Handle both '\n' and '\r\n' line endings
**Output:** Working UART handler, compile successfully

#### 1.2 Implement Command Parser ⬜
**Description:** Parse received command strings into structured data.
**Details:**
- Create `src/utils/CommandParser.h` and `.cpp`
- Struct: `ParsedCommand` with fields:
  - `enum CommandType { MODE, TEST, STATUS, RESET, HELP, INVALID }`
  - `CommandType type`
  - `String parameter` (for MODE command argument)
- Class: `CommandParser`
- Method: `ParsedCommand parse(String line)`
- Parse commands case-sensitively (MODE, TEST, STATUS, RESET, HELP)
- Extract parameter for MODE command (Z80, 6502, 62256)
- Return INVALID for unrecognized commands
**Output:** Working command parser, compile successfully

#### 1.3 Create ICTestStrategy Base Class ⬜
**Description:** Create abstract base class that all IC strategies will inherit from.
**Details:**
- Create `src/strategies/ICTestStrategy.h`
- Abstract class: `ICTestStrategy`
- Pure virtual methods:
  - `virtual void configurePins() = 0` - Set DDR and PORT registers
  - `virtual void reset() = 0` - Reset the IC
  - `virtual bool runTests() = 0` - Run all tests, return true if pass
  - `virtual const char* getName() const = 0` - Return IC name (e.g., "Z80")
- Add comment explaining this is the Strategy pattern interface
- NO implementation file needed (pure virtual)
**Output:** ICTestStrategy.h header file, compile successfully

#### 1.4 Create Pin Definitions Header ⬜
**Description:** Centralize all pin definitions in one header file.
**Details:**
- Create `src/hardware/PinConfig.h`
- Define constants for all pins using `constexpr`:
  - Address bus: `ADDR_PORT_LOW` (PORTA), `ADDR_PORT_HIGH` (PORTC)
  - Data bus: `DATA_PORT` (PORTL), `DATA_DDR` (DDRL), `DATA_PIN` (PINL)
  - Control pins: All PG, PH, PB, PE, PD pins as individual constants
  - Reset pin: `RESET_PIN`, `RESET_PORT`, `RESET_DDR`
- Add comments referencing pinout document
- Use meaningful names (e.g., `CONTROL_MREQ_RD_RW_OE_PIN` for PG2)
**Output:** PinConfig.h with all pin definitions, compile successfully

#### 1.5 Implement Mode Manager ⬜
**Description:** Manage current IC mode and strategy instance.
**Details:**
- Create `src/utils/ModeManager.h` and `.cpp`
- Class: `ModeManager`
- Members:
  - `ICTestStrategy* currentStrategy` (pointer to current strategy)
  - `enum ICMode { NONE, Z80, IC6502, SRAM62256 } currentMode`
- Methods:
  - `ICMode getCurrentMode() const`
  - `ICTestStrategy* getCurrentStrategy() const`
  - `void setStrategy(ICTestStrategy* strategy, ICMode mode)` - Switch strategy
  - `void clearStrategy()` - Set to NONE
- Use static allocation (strategies created in main.cpp, not here)
**Output:** ModeManager class, compile successfully

#### 1.6 Integrate in main.cpp ⬜
**Description:** Wire up UART, parser, and mode manager in main loop.
**Details:**
- Update `src/main.cpp`:
  - Include all created headers
  - Create global instances: `UARTHandler uart`, `CommandParser parser`, `ModeManager modeManager`
  - In `setup()`: Initialize UART at 115200, send startup message
  - In `loop()`:
    - Check if UART data available
    - Read line and parse command
    - Handle each command type (MODE, TEST, STATUS, RESET, HELP)
    - For MODE: validate parameter, send error (strategies not implemented yet)
    - For TEST: check if mode set, send error (not implemented yet)
    - For STATUS: report current mode (or NONE)
    - For RESET: send error (not implemented yet)
    - For HELP: send list of available commands
    - For INVALID: send error message
- Keep handlers as stubs for now (just respond with messages)
**Output:** Working command loop, compile successfully, test via serial monitor
**Testing:** Upload firmware, open serial monitor, send commands, verify responses

---

## Phase 2: Timer3 Clock System ⬜

**Goal:** Implement hardware PWM clock generation for CPU testing.

**Prerequisites:** Phase 1 completed

**Description:** Create the Timer3 clock management system that will generate the clock signal for Z80 and 6502 CPUs. This is a standalone module that can be tested independently. The clock will use CTC mode with toggle output on PE3.

**Strategy Document:** Create `Strategy/02-Phase2-Timer3.md` before starting this phase.

### Items:

#### 2.1 Implement Timer3 Class ⬜
**Description:** Create Timer3 clock generator with configurable frequency.
**Details:**
- Create `src/hardware/Timer3.h` and `.cpp`
- Class: `Timer3Clock`
- Members:
  - `uint32_t currentFrequency` - Track current frequency
  - `bool isRunning` - Track if clock is active
- Methods:
  - `void configure(uint32_t frequency)` - Set up Timer3 in CTC mode
    - Calculate prescaler (1, 8, or 64) based on frequency
    - Calculate OCR3A value using formula: `OCR3A = (F_CPU / (2 * prescaler * freq)) - 1`
    - Set WGM32 for CTC mode, COM3A0 for toggle output
    - Set CS3x bits for prescaler
    - Set PE3 as OUTPUT
  - `void start()` - Start the clock (set prescaler bits)
  - `void stop()` - Stop the clock (clear TCCR3B)
  - `uint32_t getFrequency() const` - Return current frequency
  - `bool running() const` - Return isRunning status
- Add safety: clamp OCR3A to uint16_t max (65535)
- Reference firmware spec for Timer3 configuration details
**Output:** Timer3Clock class, compile successfully

#### 2.2 Test Timer3 in main.cpp ⬜
**Description:** Add test commands to verify Timer3 operation.
**Details:**
- Add temporary global `Timer3Clock timer3` in main.cpp
- Add temporary command parsing for testing:
  - `CLOCK <frequency>` - Configure and start clock at frequency
  - `CLOCKSTOP` - Stop clock
- In command handler:
  - Parse CLOCK command with frequency parameter
  - Call `timer3.configure(freq)` and `timer3.start()`
  - Send OK message with actual frequency
  - For CLOCKSTOP, call `timer3.stop()`
- Use oscilloscope or LED on pin 5 to verify clock
**Output:** Working clock on PE3, verified with oscilloscope/LED
**Testing:** Test with 1 Hz (visible LED blink), 1 kHz, 100 kHz, 1 MHz
**Note:** Remove test commands after verification (or keep for debugging)

---

## Phase 3: HM62256 SRAM Testing ⬜

**Goal:** Implement complete SRAM testing capability (easiest IC - no CPU emulation needed).

**Prerequisites:** Phase 1 completed (Phase 2 not required for SRAM)

**Description:** Implement the SRAM62256 strategy with all 7 test patterns. This is the simplest IC to test since it requires no CPU emulation - just direct memory access. We'll implement all tests to validate the framework before moving to complex CPU testing.

**Strategy Document:** Create `Strategy/03-Phase3-SRAM.md` before starting this phase.

### Items:

#### 3.1 Implement SRAM62256Strategy Class Structure ⬜
**Description:** Create the SRAM strategy class skeleton inheriting from ICTestStrategy.
**Details:**
- Create `src/strategies/SRAM62256Strategy.h` and `.cpp`
- Class: `SRAM62256Strategy : public ICTestStrategy`
- Implement required virtual methods (stubs for now):
  - `void configurePins()` - Will configure for direct memory access
  - `void reset()` - SRAM has no reset, just deassert control signals
  - `bool runTests()` - Will run all 7 tests
  - `const char* getName() const` - Return "HM62256"
- Add private helper methods (to be implemented):
  - `void writeByte(uint16_t addr, uint8_t data)` - Write to SRAM
  - `uint8_t readByte(uint16_t addr)` - Read from SRAM
  - `void setAddress(uint16_t addr)` - Set address on bus
  - `bool testBasicReadWrite()` - Test 1
  - `bool testWalkingOnesAddress()` - Test 2
  - `bool testWalkingOnesData()` - Test 3
  - `bool testCheckerboard()` - Test 4
  - `bool testInverseCheckerboard()` - Test 5
  - `bool testAddressEqualsData()` - Test 6
  - `bool testRandomPattern()` - Test 7 (optional)
- Add constants:
  - `static constexpr uint16_t SRAM_SIZE = 32768` (32KB)
  - `static constexpr uint16_t MAX_ADDR = 0x7FFF`
**Output:** SRAM62256Strategy skeleton, compile successfully

#### 3.2 Implement SRAM Pin Configuration ⬜
**Description:** Implement configurePins() for SRAM mode.
**Details:**
- In `SRAM62256Strategy::configurePins()`:
  - Set address bus to OUTPUT: `DDRA = 0xFF; DDRC = 0xFF;`
  - Set data bus to INPUT (safe default): `DDRL = 0x00;`
  - Set control pins (PG0, PG2, PG3) to OUTPUT: `DDRG |= (1<<DDD41) | (1<<DDD39) | (1<<DDD38);`
  - Set all control signals HIGH (inactive): `PORTG |= (1<<41) | (1<<39) | (1<<38);`
  - Set unused pins to safe INPUT state
- Reference pinout document section "62256 Mode Configuration"
**Output:** Working pin configuration, compile successfully

#### 3.3 Implement SRAM Read/Write Operations ⬜
**Description:** Implement low-level memory access functions.
**Details:**
- `void setAddress(uint16_t addr)`:
  - `PORTA = (uint8_t)(addr & 0xFF)` - Low byte
  - `PORTC = (uint8_t)((addr >> 8) & 0xFF)` - High byte
- `void writeByte(uint16_t addr, uint8_t data)`:
  - Set address on bus
  - Set data bus to OUTPUT: `DDRL = 0xFF;`
  - Put data on bus: `PORTL = data;`
  - Assert /CS (PG0 LOW): `PORTG &= ~(1<<41);`
  - Assert /WE (PG3 LOW): `PORTG &= ~(1<<38);`
  - Wait ~1 µs (delayMicroseconds or NOP)
  - Deassert /WE (HIGH): `PORTG |= (1<<38);`
  - Deassert /CS (HIGH): `PORTG |= (1<<41);`
  - Set data bus to INPUT: `DDRL = 0x00;`
- `uint8_t readByte(uint16_t addr)`:
  - Set address on bus
  - Set data bus to INPUT: `DDRL = 0x00;`
  - Assert /CS (PG0 LOW)
  - Assert /OE (PG2 LOW)
  - Wait ~1 µs for access time
  - Read data: `uint8_t data = PINL;`
  - Deassert /OE (HIGH)
  - Deassert /CS (HIGH)
  - Return data
- Reference firmware spec "SRAM Control Signals" section
**Output:** Working read/write functions, compile successfully

#### 3.4 Implement SRAM Test 1: Basic Read/Write ⬜
**Description:** Implement the simplest test to verify basic SRAM operation.
**Details:**
- In `testBasicReadWrite()`:
  - Send info: "Test 1/7: Basic Read/Write Test"
  - Write 0x55 to address 0x0000
  - Read back, verify equals 0x55
  - If fail: send error, return false
  - Write 0xAA to address 0x0000
  - Read back, verify equals 0xAA
  - If fail: send error, return false
  - Send info: "Status: PASS"
  - Return true
- Use UARTHandler for all messages
**Output:** Working test, compile successfully
**Testing:** Can test manually after integrating MODE command

#### 3.5 Implement SRAM Test 2: Walking 1s Address Lines ⬜
**Description:** Test each address line individually to detect stuck/shorted lines.
**Details:**
- In `testWalkingOnesAddress()`:
  - Send info: "Test 2/7: Walking 1s - Address Lines"
  - For each address line A0-A14 (15 lines for 32KB):
    - Calculate address: `addr = (1 << bit)` (e.g., 0x0001, 0x0002, 0x0004...)
    - Write unique value (use bit number as data)
    - Send info for each line tested
  - Read back all addresses and verify
  - If any mismatch: send error with address, return false
  - Send info: "Status: PASS"
  - Return true
- Reference firmware spec "Test 2: Walking 1s Test - Address Lines"
**Output:** Working test, compile successfully

#### 3.6 Implement SRAM Test 3: Walking 1s Data Lines ⬜
**Description:** Test each data line individually at a fixed address.
**Details:**
- In `testWalkingOnesData()`:
  - Send info: "Test 3/7: Walking 1s - Data Lines"
  - Use address 0x0000
  - Test patterns: 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
  - For each pattern: write, read back, verify
  - Also test inverse patterns: 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F
  - If any fail: send error, return false
  - Send info: "Status: PASS"
  - Return true
**Output:** Working test, compile successfully

#### 3.7 Implement SRAM Tests 4-5: Checkerboard Patterns ⬜
**Description:** Test full memory range with checkerboard and inverse patterns.
**Details:**
- `testCheckerboard()`:
  - Send info: "Test 4/7: Checkerboard Pattern (32KB)"
  - Write pattern: even addresses = 0x55, odd addresses = 0xAA
  - Show progress every 1024 bytes (e.g., "Progress: 1024/32768")
  - Verify all 32KB
  - If fail: report first error address and values
  - Send "Status: PASS/FAIL"
- `testInverseCheckerboard()`:
  - Same as above but inverse: even = 0xAA, odd = 0x55
- Use direct port access, minimize Serial.print in loops
**Output:** Working tests, compile successfully
**Note:** These tests will take ~10-15 seconds each

#### 3.8 Implement SRAM Tests 6-7: Address=Data and Random ⬜
**Description:** Final two SRAM test patterns.
**Details:**
- `testAddressEqualsData()`:
  - Write low byte of address as data (e.g., addr 0x0123 → data 0x23)
  - Verify all 32KB
  - Show progress updates
- `testRandomPattern()` (optional but recommended):
  - Use simple PRNG (e.g., linear congruential generator)
  - Generate pseudo-random data for each address
  - Write, then regenerate sequence and verify
  - Seed PRNG with fixed value for repeatability
- Both send progress updates and PASS/FAIL status
**Output:** Working tests, compile successfully

#### 3.9 Implement SRAM runTests() Orchestrator ⬜
**Description:** Implement the main test runner that calls all tests.
**Details:**
- In `runTests()`:
  - Send info: "Starting HM62256 SRAM tests..."
  - Send info: "Testing address range: 0x0000 - 0x7FFF (32KB)"
  - Call each test in order (1-7)
  - Track pass/fail for each
  - If any test fails, continue but track failure
  - At end, send summary:
    - "==================================="
    - "RESULT: PASS" (if all passed) or "RESULT: FAIL - <first failed test>"
    - "All SRAM tests completed successfully" or failure details
    - "==================================="
  - Return true if all passed, false otherwise
- Reference firmware spec for exact output format
**Output:** Complete SRAM testing, compile successfully

#### 3.10 Integrate SRAM Strategy into main.cpp ⬜
**Description:** Wire up SRAM strategy to MODE and TEST commands.
**Details:**
- In main.cpp:
  - Create static instance: `SRAM62256Strategy sramStrategy;`
  - In MODE command handler:
    - If parameter == "62256":
      - Call `sramStrategy.configurePins()`
      - Call `modeManager.setStrategy(&sramStrategy, ModeManager::SRAM62256)`
      - Send OK message: "Switched to 62256 mode"
  - In TEST command handler:
    - Check if mode is set (not NONE)
    - Get current strategy from modeManager
    - Call `strategy->runTests()`
    - No additional response needed (strategy sends RESULT)
- Compile and upload
**Output:** Fully working SRAM testing via UART commands
**Testing:**
  - Send `MODE 62256`
  - Send `TEST`
  - Verify all 7 tests run and report PASS/FAIL
  - Test with actual HM62256 chip if available

---

## Phase 4: Z80 CPU Testing ⬜

**Goal:** Implement Z80 CPU testing with memory emulation and bus cycle handling.

**Prerequisites:** Phase 1 and Phase 2 completed, Phase 3 recommended (validates framework)

**Description:** Implement Z80 testing strategy. This is more complex than SRAM because it requires CPU emulation - the Arduino must respond to Z80 bus cycles in real-time, providing ROM and RAM emulation. The Z80 will execute actual machine code programs that we provide.

**Strategy Document:** Create `Strategy/04-Phase4-Z80.md` before starting this phase.

**Note:** Review `src/strategies/SRAM62256Strategy.h/.cpp` for reference on strategy implementation patterns.

### Items:

#### 4.1 Implement Z80Strategy Class Structure ⬜
**Description:** Create Z80 strategy class skeleton.
**Details:**
- Create `src/strategies/Z80Strategy.h` and `.cpp`
- Class: `Z80Strategy : public ICTestStrategy`
- Implement virtual methods (stubs initially)
- Add private members:
  - `uint8_t rom[4096]` - ROM emulation (4KB at 0x0000-0x0FFF)
  - `uint8_t ram[2048]` - RAM emulation (2KB at 0x1000-0x17FF)
- Add helper methods:
  - `void loadProgram(const uint8_t* program, uint16_t size, uint16_t startAddr)`
  - `void handleBusCycle()` - Respond to Z80 bus cycle
  - `uint16_t readAddress()` - Read address from bus
  - Test methods for each of 5 tests
- Include Timer3Clock reference (will need clock)
**Output:** Z80Strategy skeleton, compile successfully

#### 4.2 Implement Z80 Pin Configuration ⬜
**Description:** Configure pins for Z80 mode.
**Details:**
- In `configurePins()`:
  - Set address bus to INPUT (Z80 drives): `DDRA = 0x00; DDRC = 0x00;`
  - Set data bus to INPUT initially: `DDRL = 0x00;`
  - Set control input pins (PG0-3) to INPUT
  - Set control output pins to OUTPUT with safe defaults:
    - PH6 (RESET) = OUTPUT, HIGH
    - PB4-7 (WAIT, INT, NMI, BUSREQ) = OUTPUT, HIGH
  - Set status input pins to INPUT (PH3-5, PE4)
  - Set 6502-specific pins to safe INPUT state
- Reference pinout document "Z80 Mode Configuration"
**Output:** Working pin configuration, compile successfully

#### 4.3 Implement Z80 Bus Cycle Handler ⬜
**Description:** Handle Z80 memory requests in real-time.
**Details:**
- In `handleBusCycle()`:
  - Read address from bus: `uint16_t addr = PINA | (PINC << 8);`
  - Check /MREQ signal (PG0): if HIGH, return (not memory cycle)
  - Check /RD signal (PG2):
    - If LOW (read cycle):
      - Look up data from rom[] or ram[] based on address
      - Set data bus to OUTPUT: `DDRL = 0xFF;`
      - Put data on bus: `PORTL = data;`
      - Wait for Z80 to finish read
      - Set data bus back to INPUT: `DDRL = 0x00;`
  - Check /WR signal (PG3):
    - If LOW (write cycle):
      - Read data from bus: `uint8_t data = PINL;`
      - Store in ram[] if address in RAM range
- This is time-critical - use /WAIT signal if needed to slow Z80
- Reference firmware spec "ROM and RAM Emulation for Z80"
**Output:** Working bus cycle handler, compile successfully
**Note:** This may need optimization/interrupt handling later

#### 4.4 Implement Z80 Test 1: Control Signal Test ⬜
**Description:** Verify Z80 control signals at idle state.
**Details:**
- Hold Z80 in reset (PH6 LOW)
- Read control signals (PG0-3, PH3-5, PE4)
- Verify /MREQ, /IORQ, /RD, /WR are HIGH (idle)
- Verify /HALT is HIGH (not halted)
- Send detailed status for each signal
- Return true if all correct
**Output:** Working test, compile successfully

#### 4.5 Implement Z80 Test 2: Clock Generation ⬜
**Description:** Start Timer3 clock for Z80.
**Details:**
- Configure Timer3 for 100 kHz (safe low frequency)
- Start clock on PE3
- Verify clock started (check Timer3Clock status)
- Send info about clock frequency
- Return true if successful
- Keep clock running for subsequent tests
**Output:** Working test, compile successfully

#### 4.6 Implement Z80 Test 3: Simple Instruction Execution ⬜
**Description:** Run Z80 with simple JP loop program.
**Details:**
- Load program: `{0xC3, 0x00, 0x00}` (JP 0x0000) at address 0x0000
- Release Z80 from reset (PH6 HIGH)
- Monitor address bus for ~500 clock cycles
- Verify address sequence: 0x0000 → 0x0001 → 0x0002 → 0x0000 (repeating)
- Verify /M1 signal (PH3) goes LOW during opcode fetch
- Count cycles, report if stuck at one address
- If correct sequencing: return true
- If stuck or wrong sequence: return false with diagnostic
**Output:** Working test, compile successfully
**Challenge:** Need to respond to bus cycles while monitoring - may need interrupt or tight loop

#### 4.7 Implement Z80 Test 4: Memory Write Test ⬜
**Description:** Run program that writes to RAM.
**Details:**
- Load program from firmware spec (11 bytes):
  ```
  0x3E, 0x55,        // LD A, 0x55
  0x32, 0x00, 0x10,  // LD (0x1000), A
  0x3E, 0xAA,        // LD A, 0xAA
  0x32, 0x01, 0x10,  // LD (0x1001), A
  0x76               // HALT
  ```
- Run Z80, handle bus cycles
- Wait for HALT signal (PE4 goes LOW)
- Timeout after ~10000 cycles if no HALT
- Verify ram[0x1000] == 0x55 and ram[0x1001] == 0xAA
- Return true if values correct
**Output:** Working test, compile successfully

#### 4.8 Implement Z80 Test 5 and runTests() ⬜
**Description:** Memory read verification and test orchestrator.
**Details:**
- Test 5 just verifies Test 4 wrote correct values (already checked in Test 4)
- Implement `runTests()`:
  - Send "Starting Z80 CPU tests..."
  - Run tests 1-5 in order
  - Send detailed progress for each (reference firmware spec format)
  - Track pass/fail
  - Send final RESULT: PASS or FAIL
  - Return overall result
- Add `reset()` method: assert/deassert PH6
**Output:** Complete Z80 testing, compile successfully

#### 4.9 Integrate Z80 Strategy into main.cpp ⬜
**Description:** Add Z80 to MODE command handler.
**Details:**
- Create static instance: `Z80Strategy z80Strategy;`
- Pass Timer3Clock reference to z80Strategy
- In MODE handler, add case for "Z80"
- Test via serial: MODE Z80, TEST
**Output:** Working Z80 testing
**Testing:** Upload, test MODE Z80, TEST commands
**Note:** May need to iterate on bus cycle handling timing

---

## Phase 5: 6502 CPU Testing ⬜

**Goal:** Implement 6502 CPU testing with signal inversion handling.

**Prerequisites:** Phase 1, 2, and 4 completed (Z80 provides reference for CPU testing)

**Description:** Implement 6502 testing strategy. Similar to Z80 but with critical differences: inverted control signals (R/W, SYNC, RDY), no HALT instruction, and clock output monitoring (Φ1/Φ2). The bus cycle handling will be slightly different due to R/W signal inversion.

**Strategy Document:** Create `Strategy/05-Phase5-6502.md` before starting this phase.

**Note:** Review `src/strategies/Z80Strategy.h/.cpp` for CPU testing patterns, but remember signal inversions!

### Items:

#### 5.1 Implement IC6502Strategy Class Structure ⬜
**Description:** Create 6502 strategy class skeleton.
**Details:**
- Create `src/strategies/IC6502Strategy.h` and `.cpp`
- Class: `IC6502Strategy : public ICTestStrategy`
- Similar structure to Z80Strategy:
  - ROM/RAM buffers (same sizes)
  - Test methods for 5 tests
  - Bus cycle handler
- Add 6502-specific helpers:
  - `void setupResetVector()` - Write 0x0000 to addresses 0xFFFC-0xFFFD
  - `bool monitorPhiClocks()` - Monitor Φ1 and Φ2 outputs
- Remember: NO HALT instruction on 6502
**Output:** IC6502Strategy skeleton, compile successfully

#### 5.2 Implement 6502 Pin Configuration with Signal Inversions ⬜
**Description:** Configure pins for 6502 mode, handling inverted signals.
**Details:**
- In `configurePins()`:
  - Address and data bus: same as Z80 (INPUT)
  - Control inputs:
    - PG2 (R/W): INPUT - remember HIGH=read (inverted from Z80!)
    - PH3 (SYNC): INPUT - remember HIGH=fetch (inverted from Z80!)
  - Control outputs:
    - PH6 (RES): OUTPUT, HIGH (inactive)
    - PB4 (RDY): OUTPUT, HIGH (ready) - inverted from Z80!
    - PB5-6 (IRQ, NMI): OUTPUT, HIGH (inactive)
  - 6502-specific inputs:
    - PD0 (Φ1): INPUT
    - PD1 (Φ2): INPUT
  - 6502-specific outputs:
    - PD3 (S.O.): OUTPUT, HIGH (inactive)
  - Z80-specific pins: set to safe INPUT state
- Add comments explaining each inversion
- Reference pinout "6502 Mode Configuration" and "Signal Logic Inversions"
**Output:** Working pin configuration with proper inversions, compile successfully

#### 5.3 Implement 6502 Bus Cycle Handler with Inverted Logic ⬜
**Description:** Handle 6502 memory requests with R/W inversion.
**Details:**
- In `handleBusCycle()`:
  - Read address from bus (same as Z80)
  - Check R/W signal (PG2):
    - **If HIGH (read cycle):** - inverted!
      - Look up data from rom/ram
      - Set data bus OUTPUT
      - Put data on bus
      - Wait, then set data bus INPUT
    - **If LOW (write cycle):** - inverted!
      - Read data from bus
      - Store in ram if in RAM range
  - No /MREQ signal on 6502 - all cycles are valid
  - Handle reset vector (0xFFFC-0xFFFD) specially: return 0x00, 0x00
- Add comments explaining R/W inversion
**Output:** Working bus cycle handler with correct inversions, compile successfully

#### 5.4 Implement 6502 Test 1-2: Control Signals and Clock Monitoring ⬜
**Description:** Test control signals and Φ1/Φ2 clock outputs.
**Details:**
- Test 1: Control Signal Test
  - Hold in reset (PH6 LOW)
  - Verify R/W is HIGH (read state) - remember inversion!
  - Verify SYNC is LOW (not syncing) - remember inversion!
- Test 2: Clock Generation and Monitoring
  - Configure Timer3 for 100 kHz
  - Release reset
  - Monitor PD0 (Φ1) and PD1 (Φ2) for transitions
  - Count transitions over known time period (e.g., 10ms)
  - Calculate approximate frequency
  - Verify both clocks toggling
  - Send diagnostic info about clock frequencies
- Reference firmware spec "6502 Testing" section
**Output:** Working tests 1-2, compile successfully

#### 5.5 Implement 6502 Test 3: Simple Instruction Execution ⬜
**Description:** Run 6502 with JMP loop program.
**Details:**
- Load program: `{0x4C, 0x00, 0x00}` (JMP $0000) at address 0x0000
- Set reset vector to point to 0x0000
- Run 6502, monitor address bus
- Verify address sequence: 0xFFFC → 0xFFFD → 0x0000 → 0x0001 → 0x0002 → 0x0000
- Verify SYNC (PH3) goes HIGH during opcode fetch - inverted!
- Monitor for ~500 cycles
- Return true if correct sequencing
**Output:** Working test, compile successfully

#### 5.6 Implement 6502 Test 4-5: Memory Write and Verification ⬜
**Description:** Write test and memory verification.
**Details:**
- Test 4: Load program from firmware spec (14 bytes):
  ```
  0xA9, 0x55,        // LDA #$55
  0x8D, 0x00, 0x10,  // STA $1000
  0xA9, 0xAA,        // LDA #$AA
  0x8D, 0x01, 0x10,  // STA $1001
  0x4C, 0x00, 0x00   // JMP $0000 (loop - no HALT)
  ```
- Run for fixed number of cycles (~1000) - enough for writes to complete
- No HALT on 6502, so use cycle count or timeout
- Verify ram[0x1000] == 0x55 and ram[0x1001] == 0xAA
- Test 5: Just verification (already done in Test 4)
**Output:** Working tests 4-5, compile successfully

#### 5.7 Implement 6502 runTests() and Integration ⬜
**Description:** Complete 6502 testing and integrate into main.cpp.
**Details:**
- Implement `runTests()` orchestrator (similar to Z80)
- Implement `reset()` method
- Add to main.cpp:
  - Create static instance: `IC6502Strategy ic6502Strategy;`
  - Add to MODE handler for "6502"
- Test via serial: MODE 6502, TEST
**Output:** Complete 6502 testing, working via UART
**Testing:** Upload, test MODE 6502, TEST commands
**Note:** Watch for signal inversion bugs - double-check R/W, SYNC, RDY logic

---

## Phase 6: Integration & Polish ⬜

**Goal:** Complete all commands, add polish, and finalize firmware.

**Prerequisites:** Phases 1-5 completed

**Description:** Implement remaining commands (STATUS, RESET, HELP improvements), add error handling, validate all three IC modes work correctly, and prepare for deployment.

### Items:

#### 6.1 Implement STATUS Command ⬜
**Description:** Provide detailed status information.
**Details:**
- In STATUS command handler:
  - Send "===== Multi-IC Tester Status ====="
  - Send "Firmware Version: 1.0"
  - Send current mode (NONE, Z80, 6502, or HM62256)
  - If mode != NONE:
    - Send clock status (running/stopped, frequency if applicable)
    - Send pin configuration summary
    - Send "Ready for testing"
  - Send "Use TEST command to run tests"
  - Send "===================================="
- Reference firmware spec "STATUS Command" section
**Output:** Working STATUS command, compile successfully

#### 6.2 Implement RESET Command ⬜
**Description:** Reset the currently selected IC.
**Details:**
- In RESET command handler:
  - Check if mode is set (not NONE)
  - If NONE: send error "No IC mode selected"
  - Get current strategy
  - Call `strategy->reset()`
  - Send "OK: IC reset complete"
- Ensure each strategy's reset() method properly cycles reset signal
**Output:** Working RESET command, compile successfully

#### 6.3 Enhance HELP Command ⬜
**Description:** Provide comprehensive help information.
**Details:**
- In HELP command handler:
  - Send list of all commands with descriptions:
    - "Available Commands:"
    - "  MODE <IC>     - Select IC type (Z80, 6502, 62256)"
    - "  TEST          - Run tests for selected IC"
    - "  STATUS        - Show current configuration"
    - "  RESET         - Reset the selected IC"
    - "  HELP          - Show this help message"
  - Send usage examples
  - Send "Firmware Version: 1.0"
**Output:** Enhanced HELP command, compile successfully

#### 6.4 Add Comprehensive Error Handling ⬜
**Description:** Improve error messages and validation.
**Details:**
- Enhance MODE command:
  - Validate parameter (only Z80, 6502, 62256 accepted)
  - Send specific error for invalid IC type
  - Stop any running clocks before switching modes
  - Deassert all control signals to safe state
- Enhance TEST command:
  - Better error if no mode selected
  - Catch any exceptions/errors during testing
- Add validation in all command handlers
- Add watchdog timer or timeout for hung tests
**Output:** Robust error handling, compile successfully

#### 6.5 Final Integration Testing ⬜
**Description:** Test all three IC modes end-to-end.
**Details:**
- Test all commands with all three IC types
- Verify mode switching works cleanly
- Test error conditions (no mode set, invalid commands, etc.)
- Verify STATUS output for each mode
- Test RESET for each mode
- Document any issues found and fix
**Output:** Fully tested firmware across all modes
**Testing Checklist:**
  - [ ] MODE Z80 → TEST → verify all 5 tests
  - [ ] MODE 6502 → TEST → verify all 5 tests
  - [ ] MODE 62256 → TEST → verify all 7 tests
  - [ ] MODE switching (62256→Z80→6502→62256)
  - [ ] STATUS in each mode
  - [ ] RESET in each mode
  - [ ] HELP command
  - [ ] Error handling (TEST without MODE, invalid MODE, etc.)

#### 6.6 Code Cleanup and Documentation ⬜
**Description:** Clean up code, add comments, finalize documentation.
**Details:**
- Review all code for consistency
- Ensure all files have header comments
- Add inline comments where logic is complex
- Remove any debug/test code
- Verify all files compile with no warnings
- Update CLAUDE.md if any architectural changes were made
- Create final firmware spec if needed
**Output:** Clean, well-documented code

#### 6.7 Create User Guide ⬜
**Description:** Write end-user documentation.
**Details:**
- Create `Documents/User-Guide.md`:
  - Hardware setup instructions
  - How to connect to serial terminal
  - Command reference with examples
  - Troubleshooting section
  - Expected test outputs
- Keep it user-friendly, not developer-focused
**Output:** User guide document

---

## Notes for Future Phases

**If new ICs are added later:**
1. Create new strategy class inheriting from ICTestStrategy
2. Implement the 4 virtual methods
3. Add IC-specific pin configuration
4. Add to MODE command handler in main.cpp
5. Update HELP and STATUS commands
6. No changes needed to existing IC strategies (Open/Closed Principle)

**If new test patterns are added to existing ICs:**
1. Add new test method to appropriate strategy class
2. Call from runTests() method
3. Update test count in progress messages

---

## Current Status

**Last Updated:** 2026-01-03

**Current Phase:** Phase 0 - Setup & Architecture

**Completed Items:**
- ⬜ None yet

**In Progress:**
- ⬜ None yet

**Next Task:** Phase 0, Item 0.1 - Verify PlatformIO Installation

---

## Development Workflow

For each item:
1. **Read** the item description and details thoroughly
2. **Review** referenced documentation (firmware spec, pinout, etc.)
3. **Check** strategy document for that phase
4. **Implement** the code incrementally
5. **Compile** with `pio run` - fix any errors immediately
6. **Test** if applicable (upload and verify via serial monitor)
7. **Mark** item as completed in this roadmap
8. **Update** this roadmap if you discovered dependencies or issues
9. **Commit** working code before moving to next item

**Remember:** Ask clarifying questions if anything is unclear. Never assume!
