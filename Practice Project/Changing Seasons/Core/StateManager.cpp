//
//  StateManager.cpp
//  Changing Seasons
//
//  Created by JALAL on 6/14/25.
//

#include "StateManager.h"

StateManager::StateManager() {
    currentSeason = Season::SUMMER;
    timeOfDay = 0.5f; // Start at noon
    isDayNightCycleActive = false; // Cycle is off by default
}

void StateManager::setSeason(Season newSeason) {
    currentSeason = newSeason;
}

Season StateManager::getCurrentSeason() const {
    return currentSeason;
}

// --- New Day/Night Cycle Implementations ---

void StateManager::toggleDayNightCycle() {
    isDayNightCycleActive = !isDayNightCycleActive;
}

void StateManager::update(float deltaTime) {
    if (isDayNightCycleActive) {
        // A full day-night cycle will take 60 seconds.
        // You can change 60.0f to a smaller number for a faster cycle.
        timeOfDay += deltaTime / 60.0f;
        if (timeOfDay >= 1.0f) {
            timeOfDay -= 1.0f;
        }
    }
}

float StateManager::getTimeOfDay() const {
    return timeOfDay;
}
