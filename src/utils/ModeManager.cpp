/**
 * ModeManager.cpp
 *
 * Implementation of Mode Manager
 */

#include "ModeManager.h"

ModeManager::ModeManager()
    : currentStrategy(nullptr), currentMode(NONE) {
    // Initialize with no mode selected
}

void ModeManager::setStrategy(ICTestStrategy* strategy, ICMode mode) {
    currentStrategy = strategy;
    currentMode = mode;
}

ICTestStrategy* ModeManager::getCurrentStrategy() const {
    return currentStrategy;
}

ModeManager::ICMode ModeManager::getCurrentMode() const {
    return currentMode;
}

void ModeManager::clearStrategy() {
    currentStrategy = nullptr;
    currentMode = NONE;
}

const char* ModeManager::getModeName(ICMode mode) {
    switch (mode) {
        case NONE:
            return "NONE";
        case Z80:
            return "Z80";
        case IC6502:
            return "6502";
        case SRAM62256:
            return "HM62256";
        default:
            return "UNKNOWN";
    }
}
