/******************************************************************************
 * File: Particle.cpp
 * Description: Particle pool implementation
 ******************************************************************************/

#include "Particle.h"

namespace MyEngine {

ParticlePool::ParticlePool(size_t maxParticles)
    : m_AliveCount(0)
    , m_NextIndex(0)
{
    m_Particles.resize(maxParticles);
}

void ParticlePool::Reset() {
    for (auto& p : m_Particles) {
        p.alive = false;
    }
    m_AliveCount = 0;
    m_NextIndex = 0;
}

Particle* ParticlePool::Spawn() {
    if (m_AliveCount >= m_Particles.size()) {
        return nullptr;  // Pool full
    }
    
    // Find next dead particle
    size_t searchCount = 0;
    while (searchCount < m_Particles.size()) {
        if (!m_Particles[m_NextIndex].alive) {
            Particle* p = &m_Particles[m_NextIndex];
            p->alive = true;
            p->age = 0.0f;
            m_AliveCount++;
            m_NextIndex = (m_NextIndex + 1) % m_Particles.size();
            return p;
        }
        m_NextIndex = (m_NextIndex + 1) % m_Particles.size();
        searchCount++;
    }
    
    return nullptr;
}

void ParticlePool::Kill(size_t index) {
    if (index < m_Particles.size() && m_Particles[index].alive) {
        m_Particles[index].alive = false;
        m_AliveCount--;
    }
}

} // namespace MyEngine
