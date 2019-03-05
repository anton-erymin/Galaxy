#pragma once

#include "Image.h"

#include <chrono>

/* Time utility class for measurements. */
template <typename Period = std::ratio<1>>
class Timer {
public:
    Timer() = default;
    explicit Timer(float* value) : value(value) { }

    ~Timer()
    {
        if (value)
        {
            *value = GetPassedTime();
        }
    }

    /* Returns passed time since timer was created or reseted. */
    float GetPassedTime() const
    {
        return std::chrono::duration_cast<std::chrono::duration<float, Period>>(std::chrono::high_resolution_clock::now() - lastTime).count();
    }

    /* Resets the timer. */
    void Reset() { lastTime = std::chrono::high_resolution_clock::now(); }

private:
    std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();
    float* value = nullptr;
};
