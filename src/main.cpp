/**
 * Multi-IC Tester - Main Firmware
 *
 * Tests Z80 CPU, 6502 CPU, and HM62256 SRAM using Arduino Mega 2560
 *
 * Architecture: Strategy Pattern for IC-specific testing
 * Communication: UART at 115200 baud
 *
 * Phase 1: Foundation & Infrastructure
 * - UART communication implemented
 * - Command parsing implemented
 * - Mode management implemented
 * - Ready for IC strategy implementation (Phase 3+)
 *
 * See CLAUDE.md for development guidelines
 * See Roadmap/Roadmap.md for implementation plan
 */

#include <Arduino.h>
#include "utils/UARTHandler.h"
#include "utils/CommandParser.h"
#include "utils/ModeManager.h"
#include "hardware/Timer3.h"
#include "strategies/SRAMStrategy.h"

// Global instances
UARTHandler uart;
CommandParser parser;
ModeManager modeManager;
Timer3Clock timer3;  // Phase 2: Clock generator for testing
SRAMStrategy sramStrategy;  // Phase 3: SRAM testing strategy

// Function declarations
void handleModeCommand(const String& parameter);
void handleTestCommand(const String& parameter);
void handleStatusCommand();
void handleResetCommand();
void handleHelpCommand();
void handleClockCommand(const String& parameter);
void handleClockStopCommand();

void setup() {
    // Initialize UART communication
    uart.begin(115200);

    // Send startup message
    uart.sendInfo("========================================");
    uart.sendInfo("  Multi-IC Tester v1.0");
    uart.sendInfo("  Arduino Mega 2560");
    uart.sendInfo("========================================");
    uart.sendInfo("");
    uart.sendInfo("Supported ICs:");
    uart.sendInfo("  - Z80 CPU (40-pin DIP)");
    uart.sendInfo("  - 6502 CPU (40-pin DIP)");
    uart.sendInfo("  - HM62256 SRAM (28-pin DIP)");
    uart.sendInfo("");
    uart.sendInfo("Type HELP for command list");
    uart.sendInfo("");
}

void loop() {
    // Check if command received
    if (uart.available()) {
        // Read and parse command
        String line = uart.readLine();

        // Skip empty lines
        if (line.length() == 0) {
            return;
        }

        ParsedCommand cmd = parser.parse(line);

        // Handle command
        switch (cmd.type) {
            case MODE:
                handleModeCommand(cmd.parameter);
                break;

            case TEST:
                handleTestCommand(cmd.parameter);
                break;

            case STATUS:
                handleStatusCommand();
                break;

            case RESET:
                handleResetCommand();
                break;

            case HELP:
                handleHelpCommand();
                break;

            case CLOCK:
                handleClockCommand(cmd.parameter);
                break;

            case CLOCKSTOP:
                handleClockStopCommand();
                break;

            case INVALID:
                uart.sendError("Invalid command. Type HELP for command list.");
                break;
        }
    }
}

/**
 * Handle MODE command
 * Supports: MODE Z80, MODE 6502, MODE SRAM <size>
 */
void handleModeCommand(const String& parameter) {
    // Check if parameter provided
    if (parameter.length() == 0) {
        uart.sendError("Missing IC type. Usage: MODE <IC>");
        uart.sendInfo("IC types: Z80, 6502, SRAM <size>");
        uart.sendInfo("Example: MODE SRAM 32768");
        return;
    }

    // Check for SRAM mode
    if (parameter.startsWith("SRAM ")) {
        // Extract size parameter
        String sizeStr = parameter.substring(5);
        sizeStr.trim();

        if (sizeStr.length() == 0) {
            uart.sendError("Missing SRAM size. Usage: MODE SRAM <size>");
            uart.sendInfo("Valid sizes: 8192 (8KB), 32768 (32KB)");
            return;
        }

        uint32_t size = sizeStr.toInt();

        // Validate size (must be power of 2 and <= 64KB)
        if (size == 0 || size > 65536) {
            uart.sendError("Invalid SRAM size");
            uart.sendInfo("Valid sizes: 8192 (8KB), 32768 (32KB)");
            return;
        }

        // Configure SRAM strategy
        sramStrategy.setSize((uint16_t)size);
        sramStrategy.setUARTHandler(&uart);
        sramStrategy.configurePins();
        modeManager.setStrategy(&sramStrategy, ModeManager::SRAM62256);

        String msg = "SRAM mode set: " + String((uint16_t)size) + " bytes";
        uart.sendOK(msg.c_str());

        if (size == 32768) {
            uart.sendInfo("Configured for HM62256 (32KB)");
        } else if (size == 8192) {
            uart.sendInfo("Configured for HM6265/D4168 (8KB)");
        }

        return;
    }

    // Check other IC types
    if (parameter == "Z80") {
        uart.sendError("Z80 strategy not implemented yet");
        uart.sendInfo("Will be available in Phase 4");
    }
    else if (parameter == "6502") {
        uart.sendError("6502 strategy not implemented yet");
        uart.sendInfo("Will be available in Phase 5");
    }
    else {
        uart.sendError("Invalid IC type");
        uart.sendInfo("IC types: Z80, 6502, SRAM <size>");
        uart.sendInfo("Example: MODE SRAM 32768");
    }
}

/**
 * Handle TEST command
 * Supports: TEST, TEST FULL, TEST RANDOM, TEST RANDOM FULL, TEST <N>, TEST <N> FULL
 */
void handleTestCommand(const String& parameter) {
    // Check if mode is set
    if (modeManager.getCurrentMode() == ModeManager::NONE) {
        uart.sendError("No IC mode selected");
        uart.sendInfo("Use MODE command first: MODE SRAM <size>");
        return;
    }

    // Get current strategy
    ICTestStrategy* strategy = modeManager.getCurrentStrategy();

    if (strategy == nullptr) {
        uart.sendError("No strategy configured");
        return;
    }

    // For SRAM, parse test options
    if (modeManager.getCurrentMode() == ModeManager::SRAM62256) {
        SRAMStrategy* sram = static_cast<SRAMStrategy*>(strategy);

        String param = parameter;
        param.trim();

        // Check for FULL mode flag
        bool fullTest = param.endsWith("FULL");
        if (fullTest) {
            param = param.substring(0, param.length() - 4);
            param.trim();
        }

        if (param.length() == 0) {
            // No parameter: Default (tests 1-6, QUICK or FULL)
            uart.sendInfo(fullTest ? "Running tests 1-6 (FULL mode)..." : "Running tests 1-6 (QUICK mode)...");
            sram->runAllTests(false, fullTest);
            return;
        }

        if (param == "RANDOM") {
            // Run all tests including random
            uart.sendInfo(fullTest ? "Running tests 1-7 (FULL mode)..." : "Running tests 1-7 (QUICK mode)...");
            sram->runAllTests(true, fullTest);
            return;
        }

        // Check if it's a test number
        uint8_t testNum = param.toInt();
        if (testNum >= 1 && testNum <= 7) {
            uart.sendInfo(fullTest ? "Running single test (FULL mode)..." : "Running single test (QUICK mode)...");
            sram->runTest(testNum, fullTest);
            return;
        }

        uart.sendError("Invalid TEST parameter");
        uart.sendInfo("Usage: TEST [FULL|RANDOM|RANDOM FULL|<1-7>|<1-7> FULL]");
        return;
    }

    // For other ICs, use default runTests()
    uart.sendInfo("Starting tests...");
    strategy->runTests();
}

/**
 * Handle STATUS command
 * Shows current mode and system information
 */
void handleStatusCommand() {
    uart.sendInfo("========================================");
    uart.sendInfo("  Multi-IC Tester Status");
    uart.sendInfo("========================================");

    // Current mode
    uart.sendInfo("");
    uart.sendInfo("Current Mode:");
    const char* modeName = ModeManager::getModeName(modeManager.getCurrentMode());
    String modeStr = String("  ") + modeName;
    uart.sendInfo(modeStr.c_str());

    // Firmware version
    uart.sendInfo("");
    uart.sendInfo("Firmware:");
    uart.sendInfo("  Version: 1.0 (Phase 1 Complete)");
    uart.sendInfo("  Platform: Arduino Mega 2560");
    uart.sendInfo("  UART: 115200 baud");

    // Memory usage
    uart.sendInfo("");
    uart.sendInfo("Memory:");
    // Note: Free RAM calculation could be added here in future

    // Available commands
    uart.sendInfo("");
    uart.sendInfo("Ready for commands");
    uart.sendInfo("Type HELP for command list");
    uart.sendInfo("========================================");
}

/**
 * Handle RESET command
 * Resets the currently selected IC (not implemented yet)
 */
void handleResetCommand() {
    // Check if mode is set
    if (modeManager.getCurrentMode() == ModeManager::NONE) {
        uart.sendError("No IC mode selected");
        uart.sendInfo("Use MODE command first");
        return;
    }

    // Get current strategy
    ICTestStrategy* strategy = modeManager.getCurrentStrategy();

    if (strategy == nullptr) {
        uart.sendError("No strategy configured");
        return;
    }

    // Reset IC (strategy implementation coming in Phase 3+)
    uart.sendInfo("Resetting IC...");
    strategy->reset();
    uart.sendOK("IC reset complete");
}

/**
 * Handle HELP command
 * Displays available commands and usage
 */
void handleHelpCommand() {
    uart.sendInfo("========================================");
    uart.sendInfo("  Multi-IC Tester - Command Reference");
    uart.sendInfo("========================================");
    uart.sendInfo("");
    uart.sendInfo("Available Commands:");
    uart.sendInfo("");
    uart.sendInfo("  MODE <IC>");
    uart.sendInfo("    Select IC type for testing");
    uart.sendInfo("    IC types: Z80, 6502, SRAM <size>");
    uart.sendInfo("    Example: MODE SRAM 32768 (HM62256)");
    uart.sendInfo("");
    uart.sendInfo("  TEST [options]");
    uart.sendInfo("    Run tests for selected IC");
    uart.sendInfo("    Must select MODE first");
    uart.sendInfo("    For SRAM:");
    uart.sendInfo("      TEST          - Tests 1-6, QUICK");
    uart.sendInfo("      TEST FULL     - Tests 1-6, FULL");
    uart.sendInfo("      TEST RANDOM   - Tests 1-7, QUICK");
    uart.sendInfo("      TEST <1-7>    - Run single test");
    uart.sendInfo("");
    uart.sendInfo("  STATUS");
    uart.sendInfo("    Show current configuration");
    uart.sendInfo("    and system information");
    uart.sendInfo("");
    uart.sendInfo("  RESET");
    uart.sendInfo("    Reset the selected IC");
    uart.sendInfo("    Must select MODE first");
    uart.sendInfo("");
    uart.sendInfo("  HELP");
    uart.sendInfo("    Show this help message");
    uart.sendInfo("");
    uart.sendInfo("  CLOCK <frequency>");
    uart.sendInfo("    Start Timer3 clock at frequency (Hz)");
    uart.sendInfo("    Output on PE3 (pin 5)");
    uart.sendInfo("    Example: CLOCK 1000000");
    uart.sendInfo("");
    uart.sendInfo("  CLOCKSTOP");
    uart.sendInfo("    Stop Timer3 clock output");
    uart.sendInfo("");
    uart.sendInfo("========================================");
    uart.sendInfo("Notes:");
    uart.sendInfo("  - Commands are case-sensitive");
    uart.sendInfo("  - Only one IC tested at a time");
    uart.sendInfo("  - Strategies implemented in Phase 3+");
    uart.sendInfo("  - CLOCK commands for Phase 2 testing");
    uart.sendInfo("========================================");
}

/**
 * Handle CLOCK command (Phase 2 testing)
 * Configure and start Timer3 clock at specified frequency
 */
void handleClockCommand(const String& parameter) {
    // Check if parameter provided
    if (parameter.length() == 0) {
        uart.sendError("Missing frequency. Usage: CLOCK <frequency>");
        uart.sendInfo("Example: CLOCK 1000000 (for 1 MHz)");
        return;
    }

    // Parse frequency
    uint32_t frequency = parameter.toInt();

    // Validate frequency (1 Hz to 8 MHz)
    if (frequency < 1 || frequency > 8000000) {
        uart.sendError("Frequency out of range (1 Hz to 8 MHz)");
        return;
    }

    // Configure and start clock
    timer3.configure(frequency);
    timer3.start();

    // Send confirmation
    String msg = "Clock started at " + String(frequency) + " Hz";
    uart.sendOK(msg.c_str());
    uart.sendInfo("Output on PE3 (pin 5)");
}

/**
 * Handle CLOCKSTOP command (Phase 2 testing)
 * Stop Timer3 clock output
 */
void handleClockStopCommand() {
    // Stop clock
    timer3.stop();

    // Send confirmation
    uart.sendOK("Clock stopped");
}
