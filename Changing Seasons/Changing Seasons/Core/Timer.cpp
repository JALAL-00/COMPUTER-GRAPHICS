//
//  Timer.cpp
//  Changing Seasons
//
//  Created by JALAL on 6/14/25.
//

#include "Timer.h"

Timer::Timer() {
    // When the timer is created, we initialize the last frame time to the current time.
    lastFrameTime = std::chrono::high_resolution_clock::now();
}

float Timer::tick() {
    // 1. Get the current time.
    auto currentTime = std::chrono::high_resolution_clock::now();

    // 2. Calculate the duration between the current frame and the last frame.
    std::chrono::duration<float> deltaTime = currentTime - lastFrameTime;

    // 3. Update the last frame time to the current time for the next tick.
    lastFrameTime = currentTime;

    // 4. Return the delta time as a float (in seconds).
    return deltaTime.count();
}
