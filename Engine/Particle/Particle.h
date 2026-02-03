/******************************************************************************
 * File: Particle.h
 * Description: Core particle data structures
 ******************************************************************************/

#pragma once

#include "Math/MathTypes.h"
#include <vector>
#include <cstdint>

namespace MyEngine {

// Single particle (AOS layout for simplicity)
struct Particle {
    Vec3 position;
    Vec3 velocity;
    Vec4 color;           // RGBA
    float size;
    float lifetime;       // Total lifetime
    float age;            // Current age
    float rotation;       // Z-axis rotation in radians
    bool alive;
    
    Particle()
        : position(0, 0, 0)
        , velocity(0, 0, 0)
        , color(1, 1, 1, 1)
        , size(1.0f)
        , lifetime(1.0f)
        , age(0.0f)
        , rotation(0.0f)
        , alive(false)
    {}
    
    float GetNormalizedAge() const {
        return lifetime > 0 ? (age / lifetime) : 1.0f;
    }
    
    bool IsDead() const {
        return !alive || age >= lifetime;
    }
};

// Particle pool (manages particle memory)
class ParticlePool {
public:
    ParticlePool(size_t maxParticles = 1000);
    
    void Reset();
    Particle* Spawn();
    void Kill(size_t index);
    
    size_t GetAliveCount() const { return m_AliveCount; }
    size_t GetMaxParticles() const { return m_Particles.size(); }
    
    std::vector<Particle>& GetParticles() { return m_Particles; }
    const std::vector<Particle>& GetParticles() const { return m_Particles; }
    
private:
    std::vector<Particle> m_Particles;
    size_t m_AliveCount;
    size_t m_NextIndex;
};

} // namespace MyEngine
