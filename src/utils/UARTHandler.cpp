/**
 * UARTHandler.cpp
 *
 * Implementation of UART communication wrapper
 */

#include "utils/UARTHandler.h"

void UARTHandler::begin(uint32_t baud) {
    Serial.begin(baud);
    // Wait for serial port to initialize
    delay(100);
}

bool UARTHandler::available() {
    return Serial.available() > 0;
}

String UARTHandler::readLine() {
    String line = "";

    // Wait for data if not available
    while (!Serial.available()) {
        delay(1);
    }

    // Read until newline
    while (true) {
        if (Serial.available()) {
            char c = Serial.read();

            // Handle line endings (\n or \r\n)
            if (c == '\n') {
                break;
            }
            else if (c == '\r') {
                // Skip \r, wait for \n
                continue;
            }
            else {
                line += c;
            }
        }
    }

    // Trim whitespace
    line.trim();

    return line;
}

void UARTHandler::sendOK(const char* message) {
    Serial.print("OK: ");
    Serial.println(message);
}

void UARTHandler::sendError(const char* message) {
    Serial.print("ERROR: ");
    Serial.println(message);
}

void UARTHandler::sendInfo(const char* message) {
    Serial.println(message);
}

void UARTHandler::sendResult(bool passed, const char* message) {
    if (passed) {
        Serial.println("RESULT: PASS");
    } else {
        Serial.print("RESULT: FAIL");
        if (message && strlen(message) > 0) {
            Serial.print(" - ");
            Serial.println(message);
        } else {
            Serial.println();
        }
    }
}
