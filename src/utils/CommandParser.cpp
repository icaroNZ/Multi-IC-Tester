/**
 * CommandParser.cpp
 *
 * Implementation of command parsing logic
 */

#include "CommandParser.h"

ParsedCommand CommandParser::parse(const String& line) {
    ParsedCommand result;
    result.type = INVALID;
    result.parameter = "";

    // Handle empty lines
    if (line.length() == 0) {
        return result;
    }

    // Find first space to separate command from parameter
    int spaceIndex = line.indexOf(' ');

    String command;
    if (spaceIndex == -1) {
        // No space - entire line is the command
        command = line;
    } else {
        // Split into command and parameter
        command = line.substring(0, spaceIndex);
        result.parameter = line.substring(spaceIndex + 1);
        result.parameter.trim();  // Remove leading/trailing whitespace
    }

    // Parse command type
    result.type = parseCommandType(command);

    return result;
}

CommandType CommandParser::parseCommandType(const String& cmd) {
    // Case-sensitive command matching
    if (cmd == "MODE") {
        return MODE;
    }
    else if (cmd == "TEST") {
        return TEST;
    }
    else if (cmd == "STATUS") {
        return STATUS;
    }
    else if (cmd == "RESET") {
        return RESET;
    }
    else if (cmd == "HELP") {
        return HELP;
    }
    else {
        return INVALID;
    }
}
