/******************************************************************************
 * File: UUID.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: UUID implementation
 ******************************************************************************/

#include "UUID.h"
#include <random>
#include <sstream>
#include <iomanip>

namespace MyEngine {

// Static random number generator
static std::random_device s_RandomDevice;
static std::mt19937_64 s_RandomEngine(s_RandomDevice());
static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

UUID::UUID()
    : m_UUID(s_UniformDistribution(s_RandomEngine))
{
}

UUID::UUID(uint64_t id)
    : m_UUID(id)
{
}

std::string UUID::ToString() const {
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << m_UUID;
    return ss.str();
}

} // namespace MyEngine
