//
//  Timer.h
//  Changing Seasons
//
//  Created by JALAL on 6/14/25.
//

#pragma once

// Timer.h
// A high-precision timer for calculating delta time. Essential for frame-rate
// independent animation.

#include <chrono> // C++11 standard library for time utilities.

class Timer {
private:
    // We store the time point of the last frame.
    std::chrono::high_resolution_clock::time_point lastFrameTime;

public:
    Timer();

    // Calculates the time elapsed since the last call to tick()
    // and resets the timer.
    // Returns the elapsed time in seconds.
    float tick();
};
