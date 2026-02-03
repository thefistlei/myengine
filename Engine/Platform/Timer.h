/******************************************************************************
 * File: Timer.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: High-precision timer for frame timing and profiling
 * Dependencies: <chrono>
 ******************************************************************************/

#pragma once

#include <chrono>

namespace MyEngine {

/**
 * @brief High-precision timer using std::chrono
 * 
 * Provides microsecond-precision timing for frame rate calculation,
 * delta time, and performance profiling.
 */
class Timer {
public:
    Timer() {
        Reset();
    }
    
    /**
     * @brief Reset timer to current time
     */
    void Reset() {
        m_Start = std::chrono::high_resolution_clock::now();
    }
    
    /**
     * @brief Get elapsed time since last reset in seconds
     */
    float Elapsed() const {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<float>(now - m_Start);
        return duration.count();
    }
    
    /**
     * @brief Get elapsed time in milliseconds
     */
    float ElapsedMillis() const {
        return Elapsed() * 1000.0f;
    }
    
    /**
     * @brief Get elapsed time in microseconds
     */
    float ElapsedMicros() const {
        return Elapsed() * 1000000.0f;
    }
    
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
};

/**
 * @brief Scoped timer for automatic timing (RAII)
 * 
 * Usage:
 *   {
 *       ScopedTimer timer("MyFunction");
 *       // Code to time...
 *   } // Automatically prints elapsed time on destruction
 */
class ScopedTimer {
public:
    ScopedTimer(const char* name)
        : m_Name(name), m_Timer() {}
    
    ~ScopedTimer();
    
private:
    const char* m_Name;
    Timer m_Timer;
};

} // namespace MyEngine
