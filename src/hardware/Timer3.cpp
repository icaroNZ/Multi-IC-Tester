/**
 * Timer3.cpp
 *
 * Implementation of Timer3 hardware PWM clock generator
 */

#include "hardware/Timer3.h"

Timer3Clock::Timer3Clock()
    : currentFrequency(0), isRunning(false), prescalerBits(0) {
    // Initialize in stopped state
}

void Timer3Clock::configure(uint32_t frequency) {
    // Stop timer first for safe reconfiguration
    stop();

    // Calculate optimal prescaler and OCR3A value
    uint16_t ocr3a = 0;
    prescalerBits = selectPrescaler(frequency, ocr3a);

    // Configure Timer3 for CTC mode with toggle output
    // TCCR3A: COM3A0 = 1 (toggle OC3A on compare), WGM3[1:0] = 00
    TCCR3A = (1 << COM3A0);

    // TCCR3B: WGM32 = 1 (CTC mode), CS3x = 0 (timer stopped)
    // Prescaler bits will be set by start()
    TCCR3B = (1 << WGM32);

    // Set compare value
    OCR3A = ocr3a;

    // Clear timer counter (clean start)
    TCNT3 = 0;

    // Set PE3 (pin 5) as OUTPUT for hardware PWM
    DDRE |= (1 << DDE3);

    // Store configured frequency
    currentFrequency = frequency;

    // Timer configured but not running yet
    isRunning = false;
}

void Timer3Clock::start() {
    // Set prescaler bits to start timer
    // Keep WGM32 bit, add prescaler selection
    TCCR3B = (1 << WGM32) | prescalerBits;

    // Mark as running
    isRunning = true;
}

void Timer3Clock::stop() {
    // Clear TCCR3B to stop timer (clears prescaler bits)
    TCCR3B = 0;

    // Clear TCCR3A (disable compare output)
    TCCR3A = 0;

    // Set PE3 LOW (clean state)
    PORTE &= ~(1 << PORTE3);

    // Mark as stopped
    isRunning = false;
}

uint32_t Timer3Clock::getFrequency() const {
    return currentFrequency;
}

bool Timer3Clock::running() const {
    return isRunning;
}

uint8_t Timer3Clock::selectPrescaler(uint32_t frequency, uint16_t& ocr3a) {
    // Prescaler values and corresponding CS3x bits
    const uint16_t prescalers[] = {1, 8, 64, 256, 1024};
    const uint8_t csBits[] = {
        (1 << CS30),                    // clk/1   (CS3[2:0] = 001)
        (1 << CS31),                    // clk/8   (CS3[2:0] = 010)
        (1 << CS31) | (1 << CS30),      // clk/64  (CS3[2:0] = 011)
        (1 << CS32),                    // clk/256 (CS3[2:0] = 100)
        (1 << CS32) | (1 << CS30)       // clk/1024(CS3[2:0] = 101)
    };

    // Try each prescaler in order (smallest to largest)
    for (int i = 0; i < 5; i++) {
        // Calculate OCR3A: OCR3A = (F_CPU / (2 * prescaler * freq)) - 1
        // Use 32-bit arithmetic to avoid overflow
        uint32_t calc = (F_CPU / (2UL * prescalers[i] * frequency));

        // Check if result is valid (must be 1-65536, stored as 0-65535)
        if (calc > 0 && calc <= 65536) {
            ocr3a = (uint16_t)(calc - 1);
            return csBits[i];
        }
    }

    // If no prescaler works (frequency too low), use largest prescaler
    // and clamp OCR3A to maximum
    ocr3a = 65535;
    return csBits[4];  // clk/1024
}
