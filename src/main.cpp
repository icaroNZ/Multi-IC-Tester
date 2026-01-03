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

// Global instances
UARTHandler uart;
CommandParser parser;
ModeManager modeManager;

// Function declarations
void handleModeCommand(const String& parameter);
void handleTestCommand();
void handleStatusCommand();
void handleResetCommand();
void handleHelpCommand();

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
                handleTestCommand();
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

            case INVALID:
                uart.sendError("Invalid command. Type HELP for command list.");
                break;
        }
    }
}

/**
 * Handle MODE command
 * Validates IC type parameter but doesn't switch mode (strategies not implemented yet)
 */
void handleModeCommand(const String& parameter) {
    // Check if parameter provided
    if (parameter.length() == 0) {
        uart.sendError("Missing IC type. Usage: MODE <IC>");
        uart.sendInfo("Valid IC types: Z80, 6502, 62256");
        return;
    }

    // Validate IC type
    if (parameter == "Z80") {
        uart.sendError("Z80 strategy not implemented yet");
        uart.sendInfo("Will be available in Phase 4");
    }
    else if (parameter == "6502") {
        uart.sendError("6502 strategy not implemented yet");
        uart.sendInfo("Will be available in Phase 5");
    }
    else if (parameter == "62256") {
        uart.sendError("HM62256 strategy not implemented yet");
        uart.sendInfo("Will be available in Phase 3");
    }
    else {
        uart.sendError("Invalid IC type");
        uart.sendInfo("Valid IC types: Z80, 6502, 62256");
    }
}

/**
 * Handle TEST command
 * Checks if mode is set and runs tests (not implemented yet)
 */
void handleTestCommand() {
    // Check if mode is set
    if (modeManager.getCurrentMode() == ModeManager::NONE) {
        uart.sendError("No IC mode selected");
        uart.sendInfo("Use MODE command first: MODE <Z80|6502|62256>");
        return;
    }

    // Get current strategy
    ICTestStrategy* strategy = modeManager.getCurrentStrategy();

    if (strategy == nullptr) {
        uart.sendError("No strategy configured");
        return;
    }

    // Run tests (strategy implementation coming in Phase 3+)
    uart.sendInfo("Starting tests...");
    strategy->runTests();

    // Result already sent by strategy->runTests()
    // No additional output needed here
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
    uart.sendInfo("    IC types: Z80, 6502, 62256");
    uart.sendInfo("    Example: MODE Z80");
    uart.sendInfo("");
    uart.sendInfo("  TEST");
    uart.sendInfo("    Run tests for selected IC");
    uart.sendInfo("    Must select MODE first");
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
    uart.sendInfo("========================================");
    uart.sendInfo("Notes:");
    uart.sendInfo("  - Commands are case-sensitive");
    uart.sendInfo("  - Only one IC tested at a time");
    uart.sendInfo("  - Strategies implemented in Phase 3+");
    uart.sendInfo("========================================");
}
