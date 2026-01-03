/**
 * CommandParser.h
 *
 * Parses UART command strings into structured data
 *
 * Supported commands:
 * - MODE <IC>    Select IC type (Z80, 6502, 62256)
 * - TEST         Run tests for selected IC
 * - STATUS       Show current configuration
 * - RESET        Reset the selected IC
 * - HELP         Show help message
 *
 * Usage:
 *   CommandParser parser;
 *   ParsedCommand cmd = parser.parse("MODE Z80");
 *   if (cmd.type == MODE) {
 *       // Handle MODE command with cmd.parameter
 *   }
 */

#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <Arduino.h>

/**
 * Enumeration of all supported command types
 */
enum CommandType {
    MODE,       // Select IC type
    TEST,       // Run tests
    STATUS,     // Show status
    RESET,      // Reset IC
    HELP,       // Show help
    INVALID     // Unknown command
};

/**
 * Parsed command structure
 */
struct ParsedCommand {
    CommandType type;     // Command type
    String parameter;     // Command parameter (for MODE command)
};

/**
 * Command parser class
 */
class CommandParser {
public:
    /**
     * Parse a command string
     * @param line Command line to parse (e.g., "MODE Z80")
     * @return ParsedCommand structure with type and parameter
     */
    ParsedCommand parse(const String& line);

private:
    /**
     * Parse command type from string
     * @param cmd Command string (e.g., "MODE")
     * @return CommandType enum value
     */
    CommandType parseCommandType(const String& cmd);
};

#endif // COMMAND_PARSER_H
