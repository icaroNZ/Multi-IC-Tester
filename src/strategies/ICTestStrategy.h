/**
 * ICTestStrategy.h
 *
 * Abstract base class for all IC testing strategies
 *
 * This interface defines the contract that all IC-specific test strategies
 * must implement. Uses the Strategy Pattern to allow runtime selection of
 * IC testing behavior without modifying existing code.
 *
 * Design Pattern: Strategy Pattern
 * - Allows adding new ICs without changing existing code (Open/Closed Principle)
 * - Each IC's testing logic is encapsulated in its own class (Single Responsibility)
 * - Client code (main.cpp) depends on this abstraction, not concrete implementations
 *
 * To add a new IC:
 * 1. Create new class inheriting from ICTestStrategy
 * 2. Implement all 4 pure virtual methods
 * 3. Add instance to main.cpp
 * 4. Add to MODE command handler
 * No changes needed to existing IC strategies!
 *
 * Example:
 *   class Z80Strategy : public ICTestStrategy {
 *   public:
 *       void configurePins() override { ... }
 *       void reset() override { ... }
 *       bool runTests() override { ... }
 *       const char* getName() const override { return "Z80"; }
 *   };
 */

#ifndef IC_TEST_STRATEGY_H
#define IC_TEST_STRATEGY_H

/**
 * Abstract base class for IC testing strategies
 *
 * All concrete IC strategies (Z80Strategy, IC6502Strategy, SRAM62256Strategy)
 * must inherit from this class and implement all pure virtual methods.
 */
class ICTestStrategy {
public:
    /**
     * Configure Arduino pins for this specific IC
     *
     * This method must:
     * - Set DDR registers for correct pin directions (INPUT/OUTPUT)
     * - Set PORT registers for correct initial pin states
     * - Handle any IC-specific pin configurations
     * - Account for signal inversions (e.g., 6502 R/W vs Z80 /RD)
     *
     * Called when: User switches to this IC via MODE command
     */
    virtual void configurePins() = 0;

    /**
     * Reset the IC
     *
     * This method must:
     * - Assert the IC's reset signal (if applicable)
     * - Hold for appropriate duration
     * - Deassert reset signal
     * - Wait for IC to stabilize if needed
     *
     * For SRAM (no reset): Just ensure control signals are in safe state
     *
     * Called when: User issues RESET command, or before running tests
     */
    virtual void reset() = 0;

    /**
     * Run all tests for this IC
     *
     * This method must:
     * - Run comprehensive test suite for this IC
     * - Send progress updates via UART during testing
     * - Send RESULT: PASS or RESULT: FAIL at end
     * - Return true if all tests passed, false if any failed
     *
     * Test complexity varies by IC:
     * - SRAM: 7 test patterns (walking 1s, checkerboard, etc.)
     * - Z80: 5 tests (control signals, clock, instruction execution, memory)
     * - 6502: 5 tests (similar to Z80 but with signal inversions)
     *
     * Called when: User issues TEST command
     *
     * @return true if all tests passed, false if any test failed
     */
    virtual bool runTests() = 0;

    /**
     * Get IC name for display
     *
     * @return Null-terminated string with IC name (e.g., "Z80", "6502", "HM62256")
     */
    virtual const char* getName() const = 0;

    /**
     * Virtual destructor
     *
     * Ensures proper cleanup of derived classes when deleted through base pointer.
     * Even though we use static allocation (no delete), this is good practice.
     */
    virtual ~ICTestStrategy() = default;
};

#endif // IC_TEST_STRATEGY_H
