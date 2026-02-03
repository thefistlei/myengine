/******************************************************************************
 * File: Timer.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Timer implementation
 ******************************************************************************/

#include "Timer.h"
#include <iostream>

namespace MyEngine {

ScopedTimer::~ScopedTimer() {
    float time = m_Timer.ElapsedMillis();
    std::cout << "[TIMER] " << m_Name << ": " << time << "ms" << std::endl;
}

} // namespace MyEngine
