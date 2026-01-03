/**
 * ModeManager.h
 *
 * Manages current IC mode and testing strategy selection
 *
 * Tracks which IC is currently selected and provides access to the
 * appropriate testing strategy. Ensures only one IC mode is active at a time.
 *
 * Usage:
 *   ModeManager manager;
 *   manager.setStrategy(&z80Strategy, ModeManager::Z80);
 *   ICTestStrategy* current = manager.getCurrentStrategy();
 *   if (current != nullptr) {
 *       current->runTests();
 *   }
 */

#ifndef MODE_MANAGER_H
#define MODE_MANAGER_H

#include "strategies/ICTestStrategy.h"

class ModeManager {
public:
    /**
     * IC mode enumeration
     * Represents the currently selected IC type
     */
    enum ICMode {
        NONE,        // No IC selected (default state)
        Z80,         // Z80 CPU testing mode
        IC6502,      // 6502 CPU testing mode
        SRAM62256    // HM62256 SRAM testing mode
    };

    /**
     * Constructor
     * Initializes with no mode selected
     */
    ModeManager();

    /**
     * Set the current strategy and mode
     *
     * @param strategy Pointer to the IC test strategy to use
     * @param mode The IC mode being set
     *
     * Example:
     *   manager.setStrategy(&z80Strategy, ModeManager::Z80);
     */
    void setStrategy(ICTestStrategy* strategy, ICMode mode);

    /**
     * Get the current testing strategy
     *
     * @return Pointer to current strategy, or nullptr if no mode set
     *
     * IMPORTANT: Always check for nullptr before using!
     * Example:
     *   ICTestStrategy* strategy = manager.getCurrentStrategy();
     *   if (strategy != nullptr) {
     *       strategy->runTests();
     *   }
     */
    ICTestStrategy* getCurrentStrategy() const;

    /**
     * Get the current IC mode
     *
     * @return Current ICMode enum value
     */
    ICMode getCurrentMode() const;

    /**
     * Clear the current strategy (set to NONE)
     *
     * Sets mode to NONE and strategy to nullptr.
     * Call this before switching modes to ensure clean state.
     */
    void clearStrategy();

    /**
     * Get mode name as string
     *
     * @param mode The mode to get name for
     * @return Null-terminated string with mode name
     *
     * Example:
     *   const char* name = ModeManager::getModeName(manager.getCurrentMode());
     *   // Returns: "NONE", "Z80", "6502", or "HM62256"
     */
    static const char* getModeName(ICMode mode);

private:
    ICTestStrategy* currentStrategy;  // Pointer to current strategy (nullptr if none)
    ICMode currentMode;                // Current IC mode
};

#endif // MODE_MANAGER_H
