/**
 * SRAMStrategy.cpp
 *
 * Implementation of generic SRAM testing strategy
 */

#include "strategies/SRAMStrategy.h"
#include "hardware/PinConfig.h"
#include <Arduino.h>

SRAMStrategy::SRAMStrategy()
    : sramSize(0), maxAddress(0), addressBits(0), uart(nullptr) {
    // Initialize with no size configured
}

void SRAMStrategy::setSize(uint16_t sizeInBytes) {
    sramSize = sizeInBytes;
    maxAddress = sizeInBytes - 1;

    // Calculate number of address bits
    // 8192 (8KB) = 2^13 = 13 bits
    // 32768 (32KB) = 2^15 = 15 bits
    addressBits = 0;
    uint16_t temp = sizeInBytes - 1;
    while (temp > 0) {
        addressBits++;
        temp >>= 1;
    }
}

uint16_t SRAMStrategy::getSize() const {
    return sramSize;
}

void SRAMStrategy::setUARTHandler(UARTHandler* handler) {
    uart = handler;
}

const char* SRAMStrategy::getName() const {
    return "SRAM";
}

void SRAMStrategy::configurePins() {
    // Set address bus to OUTPUT (PORTA = A0-A7, PORTC = A8-A15)
    DDRA = 0xFF;  // A0-A7 OUTPUT
    DDRC = 0xFF;  // A8-A15 OUTPUT

    // Set data bus to INPUT (safe default)
    DDRL = 0x00;  // D0-D7 INPUT
    PORTL = 0x00; // Disable pull-ups

    // Set control pins to OUTPUT
    // PG0 = /CS, PG2 = /OE, PG3 = /WE
    DDRG |= (1 << 0) | (1 << 2) | (1 << 3);

    // Set all control signals HIGH (inactive)
    // /CS HIGH (deselected)
    // /OE HIGH (output disabled)
    // /WE HIGH (write disabled)
    PORTG |= (1 << 0) | (1 << 2) | (1 << 3);
}

void SRAMStrategy::reset() {
    // SRAM has no reset pin
    // Just deassert all control signals
    PORTG |= (1 << 0) | (1 << 2) | (1 << 3);

    // Set data bus to INPUT (safe state)
    DDRL = 0x00;
}

bool SRAMStrategy::runTests() {
    // Default: Run tests 1-6 (no random), QUICK mode
    return runAllTests(false, false);
}

void SRAMStrategy::setAddress(uint16_t addr) {
    // Set address on address bus
    // Low byte (A0-A7) on PORTA
    PORTA = (uint8_t)(addr & 0xFF);

    // High byte (A8-A15) on PORTC
    uint8_t highByte = (uint8_t)((addr >> 8) & 0xFF);

    // CRITICAL: Pin 26 differences between chips
    // - HM62256 (32KB): Pin 26 = A13 (address line, use normally)
    // - HM6265 (8KB):  Pin 26 = CS2 (must be HIGH to enable chip)
    // - D4168 (8KB):   Pin 26 = CS (must be HIGH to enable chip)
    //
    // For 8KB chips, force A13 (PORTC bit 5) HIGH to enable CS/CS2
    if (sramSize <= 8192) {
        highByte |= (1 << 5);  // Set A13 HIGH (PORTC bit 5 = pin 26)
    }

    PORTC = highByte;
}

void SRAMStrategy::writeByte(uint16_t addr, uint8_t data) {
    // CRITICAL: Prevent bus contention
    // Set data bus to OUTPUT before driving it

    // Set address
    setAddress(addr);

    // Set data bus to OUTPUT
    DDRL = 0xFF;

    // Put data on bus
    PORTL = data;

    // Assert /CS (LOW)
    PORTG &= ~(1 << 0);

    // Assert /WE (LOW) - initiate write
    PORTG &= ~(1 << 3);

    // Wait for write pulse width (tWP ~ 70ns, but use 1µs to be safe)
    delayMicroseconds(1);

    // Deassert /WE (HIGH) - latch data
    PORTG |= (1 << 3);

    // Deassert /CS (HIGH)
    PORTG |= (1 << 0);

    // Set data bus back to INPUT (safe state, prevent bus contention)
    DDRL = 0x00;
}

uint8_t SRAMStrategy::readByte(uint16_t addr) {
    // Set address
    setAddress(addr);

    // Ensure data bus is INPUT (safe to read)
    DDRL = 0x00;

    // Assert /CS (LOW)
    PORTG &= ~(1 << 0);

    // Assert /OE (LOW) - enable output
    PORTG &= ~(1 << 2);

    // Wait for access time (tACC ~ 70ns, but use 1µs to be safe)
    delayMicroseconds(1);

    // Read data from data bus
    uint8_t data = PINL;

    // Deassert /OE (HIGH)
    PORTG |= (1 << 2);

    // Deassert /CS (HIGH)
    PORTG |= (1 << 0);

    return data;
}

bool SRAMStrategy::shouldTestAddress(uint16_t addr, bool fullTest) {
    if (fullTest) {
        // FULL mode: test every address
        return true;
    }

    // QUICK mode: Strategic sampling

    // Always test first 512 bytes
    if (addr < 512) {
        return true;
    }

    // Always test last 512 bytes
    if (addr > maxAddress - 512) {
        return true;
    }

    // Test power-of-2 addresses (walking ones)
    if ((addr & (addr - 1)) == 0) {
        return true;
    }

    // Test every 128th address for sampling
    if ((addr & 0x7F) == 0) {
        return true;
    }

    return false;
}

const char* SRAMStrategy::getTestName(uint8_t testNumber) {
    switch (testNumber) {
        case 1: return "Basic Read/Write";
        case 2: return "Walking Ones Address";
        case 3: return "Walking Ones Data";
        case 4: return "Checkerboard";
        case 5: return "Inverse Checkerboard";
        case 6: return "Address Equals Data";
        case 7: return "Random Pattern";
        default: return "Unknown";
    }
}

void SRAMStrategy::sendProgress(const char* message) {
    if (uart != nullptr) {
        uart->sendInfo(message);
    }
}

void SRAMStrategy::sendProgress(const char* message, uint16_t current, uint16_t total) {
    if (uart != nullptr) {
        uint8_t percent = (uint32_t)current * 100 / total;
        String msg = String(message) + ": " + String(percent) + "%";
        uart->sendInfo(msg.c_str());
    }
}

void SRAMStrategy::sendTestStart(uint8_t testNumber, bool fullTest) {
    if (uart != nullptr) {
        String msg = "Test " + String(testNumber) + " (" + getTestName(testNumber) + ") - ";
        msg += fullTest ? "FULL mode" : "QUICK mode";
        uart->sendInfo(msg.c_str());
    }
}

void SRAMStrategy::sendTestResult(uint8_t testNumber, bool passed) {
    if (uart != nullptr) {
        String msg = "Test " + String(testNumber) + " (" + getTestName(testNumber) + ") - ";
        msg += passed ? "PASSED" : "FAILED";
        if (passed) {
            uart->sendOK(msg.c_str());
        } else {
            uart->sendError(msg.c_str());
        }
    }
}

void SRAMStrategy::sendTestError(uint8_t testNumber, uint16_t addr, uint8_t expected, uint8_t actual) {
    if (uart != nullptr) {
        char buf[80];
        sprintf(buf, "Test %d FAIL - Addr: 0x%04X Expected: 0x%02X Got: 0x%02X",
                testNumber, addr, expected, actual);
        uart->sendError(buf);
    }
}

bool SRAMStrategy::runTest(uint8_t testNumber, bool fullTest) {
    switch (testNumber) {
        case 1: return testBasicReadWrite(fullTest);
        case 2: return testWalkingOnesAddress(fullTest);
        case 3: return testWalkingOnesData(fullTest);
        case 4: return testCheckerboard(fullTest);
        case 5: return testInverseCheckerboard(fullTest);
        case 6: return testAddressEqualsData(fullTest);
        case 7: return testRandomPattern(fullTest);
        default:
            if (uart != nullptr) {
                uart->sendError("Invalid test number (1-7)");
            }
            return false;
    }
}

bool SRAMStrategy::runAllTests(bool includeRandom, bool fullTest) {
    if (sramSize == 0) {
        if (uart != nullptr) {
            uart->sendError("SRAM size not configured");
        }
        return false;
    }

    uint8_t maxTest = includeRandom ? 7 : 6;
    bool allPassed = true;

    for (uint8_t test = 1; test <= maxTest; test++) {
        bool passed = runTest(test, fullTest);
        if (!passed) {
            allPassed = false;
        }
    }

    if (uart != nullptr) {
        if (allPassed) {
            uart->sendOK("All tests PASSED");
        } else {
            uart->sendError("Some tests FAILED");
        }
    }

    return allPassed;
}

// Test 1: Basic Read/Write
bool SRAMStrategy::testBasicReadWrite(bool fullTest) {
    sendTestStart(1, fullTest);

    // Test pattern 1: 0xAA
    for (uint16_t addr = 0; addr <= maxAddress; addr++) {
        if (!shouldTestAddress(addr, fullTest)) continue;

        writeByte(addr, 0xAA);
        uint8_t read = readByte(addr);
        if (read != 0xAA) {
            sendTestError(1, addr, 0xAA, read);
            sendTestResult(1, false);
            return false;
        }

        // Progress update every 4096 addresses
        if (fullTest && (addr & 0x0FFF) == 0 && addr > 0) {
            sendProgress("Test 1", addr, maxAddress);
        }
    }

    // Test pattern 2: 0x55
    for (uint16_t addr = 0; addr <= maxAddress; addr++) {
        if (!shouldTestAddress(addr, fullTest)) continue;

        writeByte(addr, 0x55);
        uint8_t read = readByte(addr);
        if (read != 0x55) {
            sendTestError(1, addr, 0x55, read);
            sendTestResult(1, false);
            return false;
        }
    }

    sendTestResult(1, true);
    return true;
}

// Test 2: Walking Ones Address
bool SRAMStrategy::testWalkingOnesAddress(bool fullTest) {
    sendTestStart(2, fullTest);

    uint8_t testPattern = 0xAA;

    // Test each address bit
    for (uint8_t bit = 0; bit < addressBits; bit++) {
        uint16_t addr = (1 << bit);

        writeByte(addr, testPattern);
        uint8_t read = readByte(addr);

        if (read != testPattern) {
            sendTestError(2, addr, testPattern, read);
            if (uart != nullptr) {
                char buf[50];
                sprintf(buf, "Possible issue with address line A%d", bit);
                uart->sendInfo(buf);
            }
            sendTestResult(2, false);
            return false;
        }
    }

    sendTestResult(2, true);
    return true;
}

// Test 3: Walking Ones Data
bool SRAMStrategy::testWalkingOnesData(bool fullTest) {
    sendTestStart(3, fullTest);

    uint16_t testAddr = 0x0000;  // Use address 0 for data line test

    // Test each data bit
    for (uint8_t bit = 0; bit < 8; bit++) {
        uint8_t testPattern = (1 << bit);

        writeByte(testAddr, testPattern);
        uint8_t read = readByte(testAddr);

        if (read != testPattern) {
            sendTestError(3, testAddr, testPattern, read);
            if (uart != nullptr) {
                char buf[50];
                sprintf(buf, "Possible issue with data line D%d", bit);
                uart->sendInfo(buf);
            }
            sendTestResult(3, false);
            return false;
        }
    }

    sendTestResult(3, true);
    return true;
}

// Test 4: Checkerboard Pattern
bool SRAMStrategy::testCheckerboard(bool fullTest) {
    sendTestStart(4, fullTest);

    // Write phase: 0x55
    for (uint16_t addr = 0; addr <= maxAddress; addr++) {
        if (!shouldTestAddress(addr, fullTest)) continue;
        writeByte(addr, 0x55);

        if (fullTest && (addr & 0x0FFF) == 0 && addr > 0) {
            sendProgress("Test 4 (write 0x55)", addr, maxAddress);
        }
    }

    // Verify phase: 0x55
    for (uint16_t addr = 0; addr <= maxAddress; addr++) {
        if (!shouldTestAddress(addr, fullTest)) continue;

        uint8_t read = readByte(addr);
        if (read != 0x55) {
            sendTestError(4, addr, 0x55, read);
            sendTestResult(4, false);
            return false;
        }

        if (fullTest && (addr & 0x0FFF) == 0 && addr > 0) {
            sendProgress("Test 4 (verify 0x55)", addr, maxAddress);
        }
    }

    // Write phase: 0xAA
    for (uint16_t addr = 0; addr <= maxAddress; addr++) {
        if (!shouldTestAddress(addr, fullTest)) continue;
        writeByte(addr, 0xAA);
    }

    // Verify phase: 0xAA
    for (uint16_t addr = 0; addr <= maxAddress; addr++) {
        if (!shouldTestAddress(addr, fullTest)) continue;

        uint8_t read = readByte(addr);
        if (read != 0xAA) {
            sendTestError(4, addr, 0xAA, read);
            sendTestResult(4, false);
            return false;
        }
    }

    sendTestResult(4, true);
    return true;
}

// Test 5: Inverse Checkerboard Pattern
bool SRAMStrategy::testInverseCheckerboard(bool fullTest) {
    sendTestStart(5, fullTest);

    // Write phase: 0xAA
    for (uint16_t addr = 0; addr <= maxAddress; addr++) {
        if (!shouldTestAddress(addr, fullTest)) continue;
        writeByte(addr, 0xAA);

        if (fullTest && (addr & 0x0FFF) == 0 && addr > 0) {
            sendProgress("Test 5 (write 0xAA)", addr, maxAddress);
        }
    }

    // Verify phase: 0xAA
    for (uint16_t addr = 0; addr <= maxAddress; addr++) {
        if (!shouldTestAddress(addr, fullTest)) continue;

        uint8_t read = readByte(addr);
        if (read != 0xAA) {
            sendTestError(5, addr, 0xAA, read);
            sendTestResult(5, false);
            return false;
        }

        if (fullTest && (addr & 0x0FFF) == 0 && addr > 0) {
            sendProgress("Test 5 (verify 0xAA)", addr, maxAddress);
        }
    }

    // Write phase: 0x55
    for (uint16_t addr = 0; addr <= maxAddress; addr++) {
        if (!shouldTestAddress(addr, fullTest)) continue;
        writeByte(addr, 0x55);
    }

    // Verify phase: 0x55
    for (uint16_t addr = 0; addr <= maxAddress; addr++) {
        if (!shouldTestAddress(addr, fullTest)) continue;

        uint8_t read = readByte(addr);
        if (read != 0x55) {
            sendTestError(5, addr, 0x55, read);
            sendTestResult(5, false);
            return false;
        }
    }

    sendTestResult(5, true);
    return true;
}

// Test 6: Address Equals Data
bool SRAMStrategy::testAddressEqualsData(bool fullTest) {
    sendTestStart(6, fullTest);

    // Write phase
    for (uint16_t addr = 0; addr <= maxAddress; addr++) {
        if (!shouldTestAddress(addr, fullTest)) continue;

        uint8_t data = (uint8_t)(addr & 0xFF);  // Low byte of address
        writeByte(addr, data);

        if (fullTest && (addr & 0x0FFF) == 0 && addr > 0) {
            sendProgress("Test 6 (write)", addr, maxAddress);
        }
    }

    // Verify phase
    for (uint16_t addr = 0; addr <= maxAddress; addr++) {
        if (!shouldTestAddress(addr, fullTest)) continue;

        uint8_t expected = (uint8_t)(addr & 0xFF);
        uint8_t actual = readByte(addr);

        if (actual != expected) {
            sendTestError(6, addr, expected, actual);
            sendTestResult(6, false);
            return false;
        }

        if (fullTest && (addr & 0x0FFF) == 0 && addr > 0) {
            sendProgress("Test 6 (verify)", addr, maxAddress);
        }
    }

    sendTestResult(6, true);
    return true;
}

// Test 7: Random Pattern
bool SRAMStrategy::testRandomPattern(bool fullTest) {
    sendTestStart(7, fullTest);

    // Seed random number generator with known value
    randomSeed(12345);

    // Write phase
    for (uint16_t addr = 0; addr <= maxAddress; addr++) {
        if (!shouldTestAddress(addr, fullTest)) continue;

        uint8_t data = (uint8_t)(random(256));
        writeByte(addr, data);

        if (fullTest && (addr & 0x0FFF) == 0 && addr > 0) {
            sendProgress("Test 7 (write)", addr, maxAddress);
        }
    }

    // Reset random seed for verification
    randomSeed(12345);

    // Verify phase
    for (uint16_t addr = 0; addr <= maxAddress; addr++) {
        if (!shouldTestAddress(addr, fullTest)) continue;

        uint8_t expected = (uint8_t)(random(256));
        uint8_t actual = readByte(addr);

        if (actual != expected) {
            sendTestError(7, addr, expected, actual);
            sendTestResult(7, false);
            return false;
        }

        if (fullTest && (addr & 0x0FFF) == 0 && addr > 0) {
            sendProgress("Test 7 (verify)", addr, maxAddress);
        }
    }

    sendTestResult(7, true);
    return true;
}
