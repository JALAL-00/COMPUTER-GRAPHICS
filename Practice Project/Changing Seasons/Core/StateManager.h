//
//  StateManager.h
//  Changing Seasons
//
//  Created by JALAL on 6/14/25.
//

#pragma once

enum class Season { WINTER, SPRING, SUMMER, AUTUMN };

class StateManager {
private:
    Season currentSeason;
    float timeOfDay; // 0.0 (midnight) to 1.0 (next midnight)
    bool isDayNightCycleActive;

public:
    StateManager();

    void setSeason(Season newSeason);
    Season getCurrentSeason() const;

    // --- New Day/Night Cycle Methods ---
    void toggleDayNightCycle();
    void update(float deltaTime); // Will advance the time
    float getTimeOfDay() const;
};
