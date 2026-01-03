/**
 * SRAMStrategy.h
 *
 * Generic SRAM testing strategy for multiple chip sizes
 *
 * Supported Chips:
 * - HM62256: 32KB SRAM (32768 bytes)
 * - HM6265:  8KB SRAM (8192 bytes)
 * - D4168:   8KB SRAM (8192 bytes, pin-compatible with HM6265)
 *
 * User specifies memory size via MODE command:
 *   MODE SRAM 32768  → HM62256
 *   MODE SRAM 8192   → HM6265 or D4168
 *
 * Test Patterns (7 total):
 * 1. Basic Read/Write
 * 2. Walking Ones Address
 * 3. Walking Ones Data
 * 4. Checkerboard (0x55/0xAA)
 * 5. Inverse Checkerboard (0xAA/0x55)
 * 6. Address Equals Data
 * 7. Random Pattern
 *
 * Test Modes:
 * - QUICK: Fast sampling (~1-2 seconds per test)
 * - FULL:  Complete memory test (~5-20 seconds per test)
 *
 * Usage:
 *   SRAMStrategy sram;
 *   sram.setSize(32768);      // Configure for HM62256
 *   sram.configurePins();     // Set up hardware
 *   sram.runAllTests(false, false);  // Run tests 1-6, QUICK mode
 *
 * See Strategy/03-Phase3-SRAM.md for implementation details
 */

#ifndef SRAM_STRATEGY_H
#define SRAM_STRATEGY_H

#include "strategies/ICTestStrategy.h"
#include "utils/UARTHandler.h"

class SRAMStrategy : public ICTestStrategy {
public:
    /**
     * Constructor
     * Initializes with size = 0 (must call setSize() before use)
     */
    SRAMStrategy();

    /**
     * Set SRAM size
     *
     * @param sizeInBytes Memory size in bytes (8192 or 32768)
     *
     * Calculates maxAddress and addressBits based on size.
     * Must call before configurePins() or runTests().
     *
     * Example:
     *   sram.setSize(32768);  // HM62256 (32KB)
     *   sram.setSize(8192);   // HM6265 or D4168 (8KB)
     */
    void setSize(uint16_t sizeInBytes);

    /**
     * Get current configured size
     *
     * @return Memory size in bytes, or 0 if not configured
     */
    uint16_t getSize() const;

    // ICTestStrategy interface implementation
    void configurePins() override;
    void reset() override;
    bool runTests() override;  // Default: runs tests 1-6, QUICK mode
    const char* getName() const override;

    /**
     * Run specific test by number
     *
     * @param testNumber Test to run (1-7)
     * @param fullTest true for FULL mode, false for QUICK mode
     * @return true if test passed, false if failed
     *
     * Test Numbers:
     *   1 = Basic Read/Write
     *   2 = Walking Ones Address
     *   3 = Walking Ones Data
     *   4 = Checkerboard
     *   5 = Inverse Checkerboard
     *   6 = Address Equals Data
     *   7 = Random Pattern
     *
     * Example:
     *   sram.runTest(2, false);  // Run test 2, QUICK mode
     *   sram.runTest(4, true);   // Run test 4, FULL mode
     */
    bool runTest(uint8_t testNumber, bool fullTest);

    /**
     * Run all tests
     *
     * @param includeRandom true to include test 7 (random), false for tests 1-6 only
     * @param fullTest true for FULL mode, false for QUICK mode
     * @return true if all tests passed, false if any failed
     *
     * Example:
     *   sram.runAllTests(false, false);  // Tests 1-6, QUICK (default)
     *   sram.runAllTests(false, true);   // Tests 1-6, FULL
     *   sram.runAllTests(true, false);   // Tests 1-7, QUICK
     *   sram.runAllTests(true, true);    // Tests 1-7, FULL
     */
    bool runAllTests(bool includeRandom, bool fullTest);

    /**
     * Set UART handler for progress updates
     *
     * @param handler Pointer to UARTHandler instance
     *
     * If set, progress updates will be sent during long tests.
     * Optional - tests work without it but no progress shown.
     */
    void setUARTHandler(UARTHandler* handler);

private:
    uint16_t sramSize;      // Memory size in bytes (0 = not configured)
    uint16_t maxAddress;    // Maximum valid address (sramSize - 1)
    uint8_t addressBits;    // Number of address lines (13 for 8KB, 15 for 32KB)
    UARTHandler* uart;      // Optional UART handler for progress updates

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

    // Helper functions
    bool shouldTestAddress(uint16_t addr, bool fullTest);
    const char* getTestName(uint8_t testNumber);
    void sendProgress(const char* message);
    void sendProgress(const char* message, uint16_t current, uint16_t total);
    void sendTestStart(uint8_t testNumber, bool fullTest);
    void sendTestResult(uint8_t testNumber, bool passed);
    void sendTestError(uint8_t testNumber, uint16_t addr, uint8_t expected, uint8_t actual);
};

#endif // SRAM_STRATEGY_H
