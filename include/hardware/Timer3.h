/**
 * Timer3.h
 *
 * Hardware PWM clock generation using Timer3 in CTC mode
 *
 * Generates stable clock signals for Z80 and 6502 CPU testing.
 * Uses Timer3 with Output Compare A on PE3 (pin 5).
 * Frequency range: 1 Hz to ~4 MHz (auto-selects prescaler).
 *
 * Usage:
 *   Timer3Clock clock;
 *   clock.configure(1000000);  // 1 MHz
 *   clock.start();
 *   // ... clock running on PE3 ...
 *   clock.stop();
 *
 * Technical Details:
 * - Mode: CTC (Clear Timer on Compare) with toggle output
 * - Output: PE3 (pin 5) toggles on compare match
 * - Duty Cycle: 50% (hardware toggle)
 * - Prescalers: Auto-selected from 1, 8, 64, 256, 1024
 * - Formula: f_out = F_CPU / (2 * prescaler * (OCR3A + 1))
 *
 * See Strategy/02-Phase2-Timer3.md for implementation details
 */

#ifndef TIMER3_H
#define TIMER3_H

#include <Arduino.h>

class Timer3Clock {
public:
    /**
     * Constructor
     * Initializes timer in stopped state
     */
    Timer3Clock();

    /**
     * Configure timer for specified frequency
     *
     * Calculates optimal prescaler and OCR3A value.
     * Does NOT start the timer - call start() to begin output.
     *
     * @param frequency Desired frequency in Hz (1 Hz to ~4 MHz)
     *
     * Example:
     *   clock.configure(1000000);  // 1 MHz for Z80/6502
     */
    void configure(uint32_t frequency);

    /**
     * Start clock output on PE3 (pin 5)
     *
     * Must call configure() first.
     * Sets prescaler bits to start timer.
     *
     * Example:
     *   clock.configure(1000000);
     *   clock.start();  // Clock now running
     */
    void start();

    /**
     * Stop clock output
     *
     * Clears timer control register and sets PE3 LOW.
     * Safe to call multiple times.
     *
     * Example:
     *   clock.stop();  // Clock stopped, PE3 = LOW
     */
    void stop();

    /**
     * Get currently configured frequency
     *
     * @return Frequency in Hz, or 0 if not configured
     *
     * Note: Returns configured frequency, not actual output
     *       (actual may differ slightly due to integer rounding)
     */
    uint32_t getFrequency() const;

    /**
     * Check if clock is running
     *
     * @return true if clock is running, false if stopped
     */
    bool running() const;

private:
    uint32_t currentFrequency;  // Configured frequency in Hz
    bool isRunning;             // true if clock is active
    uint8_t prescalerBits;      // CS3x bits for TCCR3B

    /**
     * Select optimal prescaler for given frequency
     *
     * Tries prescalers 1, 8, 64, 256, 1024 in order.
     * Selects first prescaler where OCR3A fits in 16 bits.
     *
     * @param frequency Desired frequency in Hz
     * @param ocr3a Output: calculated OCR3A value
     * @return CS3x bits for TCCR3B
     */
    uint8_t selectPrescaler(uint32_t frequency, uint16_t& ocr3a);
};

#endif // TIMER3_H
