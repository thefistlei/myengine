/******************************************************************************
 * File: ParticleSystem.h
 * Description: Unified particle system with emitter, modules, and rendering
 ******************************************************************************/

#pragma once

#include "Particle.h"
#include "Math/MathTypes.h"
#include <string>
#include <memory>
#include <functional>

namespace MyEngine {

// Forward declarations
class Shader;
class VertexArray;

// Particle system preset types
enum class ParticlePreset {
    Fire,
    Smoke,
    Explosion
};

// Simple animation curve (linear interpolation between keys)
struct AnimationCurve {
    struct Key {
        float time;   // [0, 1] normalized lifetime
        float value;
        
        Key(float t, float v) : time(t), value(v) {}
    };
    
    std::vector<Key> keys;
    
    void AddKey(float time, float value) {
        keys.push_back(Key(time, value));
    }
    
    float Evaluate(float t) const {
        if (keys.empty()) return 1.0f;
        if (keys.size() == 1) return keys[0].value;
        
        // Find surrounding keys
        for (size_t i = 0; i < keys.size() - 1; i++) {
            if (t >= keys[i].time && t <= keys[i + 1].time) {
                float alpha = (t - keys[i].time) / (keys[i + 1].time - keys[i].time);
                return keys[i].value + alpha * (keys[i + 1].value - keys[i].value);
            }
        }
        
        return keys.back().value;
    }
};

// Color gradient
struct ColorGradient {
    struct ColorKey {
        float time;
        Vec4 color;
        
        ColorKey(float t, const Vec4& c) : time(t), color(c) {}
    };
    
    std::vector<ColorKey> keys;
    
    void AddKey(float time, const Vec4& color) {
        keys.push_back(ColorKey(time, color));
    }
    
    Vec4 Evaluate(float t) const {
        if (keys.empty()) return Vec4(1, 1, 1, 1);
        if (keys.size() == 1) return keys[0].color;
        
        for (size_t i = 0; i < keys.size() - 1; i++) {
            if (t >= keys[i].time && t <= keys[i + 1].time) {
                float alpha = (t - keys[i].time) / (keys[i + 1].time - keys[i].time);
                Vec4 c1 = keys[i].color;
                Vec4 c2 = keys[i + 1].color;
                return Vec4(
                    c1.x + alpha * (c2.x - c1.x),
                    c1.y + alpha * (c2.y - c1.y),
                    c1.z + alpha * (c2.z - c1.z),
                    c1.w + alpha * (c2.w - c1.w)
                );
            }
        }
        
        return keys.back().color;
    }
};

// Particle system configuration
struct ParticleSystemConfig {
    std::string name;
    bool loop = true;
    float duration = 5.0f;
    size_t maxParticles = 1000;
    
    // Emission
    float emissionRate = 50.0f;    // Particles per second
    bool burst = false;             // One-time burst
    int burstCount = 100;           // Particles in burst
    
    // Emitter shape
    Vec3 emitterPosition = Vec3(0, 0, 0);
    float emitterRadius = 0.5f;
    
    // Initial properties
    float lifetimeMin = 1.0f;
    float lifetimeMax = 2.0f;
    Vec3 initialVelocity = Vec3(0, 2, 0);
    Vec3 velocityRandom = Vec3(0.5f, 0.5f, 0.5f);
    float sizeMin = 0.3f;
    float sizeMax = 0.5f;
    Vec4 colorStart = Vec4(1, 1, 1, 1);
    Vec4 colorEnd = Vec4(1, 1, 1, 0);
    
    // Modules
    bool sizeOverLifetimeEnabled = true;
    AnimationCurve sizeOverLifetime;
    
    bool colorOverLifetimeEnabled = true;
    ColorGradient colorOverLifetime;
    
    bool velocityOverLifetimeEnabled = false;
    float velocityDamping = 0.0f;
    
    // Physics
    Vec3 gravity = Vec3(0, -1, 0);
    
    // Rendering
    bool additiveBlending = true;
};

// Main particle system class
class ParticleSystem {
public:
    ParticleSystem(const ParticleSystemConfig& config);
    ~ParticleSystem();
    
    void Update(float deltaTime);
    void Render(const Mat4& view, const Mat4& projection);
    
    void Reset();
    void Play();
    void Stop();
    
    // Configuration access
    ParticleSystemConfig& GetConfig() { return m_Config; }
    const ParticleSystemConfig& GetConfig() const { return m_Config; }
    
    // Stats
    size_t GetAliveCount() const { return m_Pool.GetAliveCount(); }
    size_t GetMaxParticles() const { return m_Pool.GetMaxParticles(); }
    
    // Transform
    void SetPosition(const Vec3& pos) { m_Config.emitterPosition = pos; }
    Vec3 GetPosition() const { return m_Config.emitterPosition; }
    
private:
    void EmitParticles(float deltaTime);
    void UpdateParticles(float deltaTime);
    void ApplyModules(Particle& p);
    void BuildVertexData();
    void InitializeRendering();
    
    ParticleSystemConfig m_Config;
    ParticlePool m_Pool;
    
    float m_Time;
    float m_EmissionAccumulator;
    bool m_Playing;
    bool m_BurstTriggered;
    
    // Rendering
    std::shared_ptr<Shader> m_Shader;
    std::shared_ptr<VertexArray> m_VAO;
    std::vector<float> m_VertexData;  // Interleaved vertex data
    bool m_RenderingInitialized;
};

// Preset factory
class ParticlePresetFactory {
public:
    static ParticleSystemConfig CreateFirePreset();
    static ParticleSystemConfig CreateSmokePreset();
    static ParticleSystemConfig CreateExplosionPreset();
    
    static ParticleSystemConfig CreatePreset(ParticlePreset preset) {
        switch (preset) {
            case ParticlePreset::Fire: return CreateFirePreset();
            case ParticlePreset::Smoke: return CreateSmokePreset();
            case ParticlePreset::Explosion: return CreateExplosionPreset();
            default: return CreateFirePreset();
        }
    }
};

} // namespace MyEngine
