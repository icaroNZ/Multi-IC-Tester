/**
 * UARTHandler.h
 *
 * UART communication wrapper for Multi-IC Tester
 * Provides formatted message output and line-based input
 *
 * Usage:
 *   UARTHandler uart;
 *   uart.begin(115200);
 *   if (uart.available()) {
 *       String line = uart.readLine();
 *       uart.sendOK("Command received");
 *   }
 */

#ifndef UART_HANDLER_H
#define UART_HANDLER_H

#include <Arduino.h>

class UARTHandler {
public:
    /**
     * Initialize UART communication
     * @param baud Baud rate (typically 115200)
     */
    void begin(uint32_t baud);

    /**
     * Check if data is available to read
     * @return true if data available
     */
    bool available();

    /**
     * Read a complete line (until \n or \r\n)
     * Blocks until newline received
     * @return String containing the line (trimmed, without line ending)
     */
    String readLine();

    /**
     * Send OK message
     * Format: "OK: <message>\n"
     * @param message Success message to send
     */
    void sendOK(const char* message);

    /**
     * Send error message
     * Format: "ERROR: <message>\n"
     * @param message Error description
     */
    void sendError(const char* message);

    /**
     * Send informational message
     * Format: "<message>\n"
     * @param message Information to send
     */
    void sendInfo(const char* message);

    /**
     * Send test result
     * Format: "RESULT: PASS\n" or "RESULT: FAIL - <message>\n"
     * @param passed true if test passed
     * @param message Additional message (for failures, or empty for pass)
     */
    void sendResult(bool passed, const char* message = "");
};

#endif // UART_HANDLER_H
