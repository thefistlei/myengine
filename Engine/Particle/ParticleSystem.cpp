/******************************************************************************
 * File: ParticleSystem.cpp
 * Description: Particle system implementation with presets
 ******************************************************************************/

#include "ParticleSystem.h"
#include "Rendering/OpenGL/OpenGLShader.h"
#include "Rendering/OpenGL/OpenGLVertexArray.h"
#include "Rendering/OpenGL/OpenGLBuffer.h"
#include "Core/Log.h"
#include <glad/gl.h>
#include <cmath>
#include <random>

namespace MyEngine {

// Random number generator
static std::random_device rd;
static std::mt19937 gen(rd());

static float RandomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen);
}

static Vec3 RandomVec3(const Vec3& min, const Vec3& max) {
    return Vec3(
        RandomFloat(min.x, max.x),
        RandomFloat(min.y, max.y),
        RandomFloat(min.z, max.z)
    );
}

ParticleSystem::ParticleSystem(const ParticleSystemConfig& config)
    : m_Config(config)
    , m_Pool(config.maxParticles)
    , m_Time(0.0f)
    , m_EmissionAccumulator(0.0f)
    , m_Playing(true)
    , m_BurstTriggered(false)
    , m_RenderingInitialized(false)
{
    // Reserve vertex data
    m_VertexData.reserve(config.maxParticles * 6 * 9);  // 6 verts per particle, 9 floats per vert
}

ParticleSystem::~ParticleSystem() = default;

void ParticleSystem::Update(float deltaTime) {
    if (!m_Playing) return;
    
    m_Time += deltaTime;
    
    // Loop check
    if (m_Config.loop && m_Time >= m_Config.duration) {
        m_Time = 0.0f;
        m_BurstTriggered = false;
    }
    
    // Emission
    EmitParticles(deltaTime);
    
    // Update particles
    UpdateParticles(deltaTime);
}

void ParticleSystem::EmitParticles(float deltaTime) {
    if (m_Config.burst && !m_BurstTriggered) {
        // Burst emission
        for (int i = 0; i < m_Config.burstCount; i++) {
            Particle* p = m_Pool.Spawn();
            if (!p) break;
            
            // Initialize particle
            p->position = m_Config.emitterPosition + RandomVec3(
                Vec3(-m_Config.emitterRadius, -m_Config.emitterRadius, -m_Config.emitterRadius),
                Vec3(m_Config.emitterRadius, m_Config.emitterRadius, m_Config.emitterRadius)
            );
            p->velocity = m_Config.initialVelocity + RandomVec3(
                -m_Config.velocityRandom,
                m_Config.velocityRandom
            );
            p->lifetime = RandomFloat(m_Config.lifetimeMin, m_Config.lifetimeMax);
            p->size = RandomFloat(m_Config.sizeMin, m_Config.sizeMax);
            p->color = m_Config.colorStart;
            p->rotation = RandomFloat(0, 6.28318f);
            p->age = 0.0f;
        }
        m_BurstTriggered = true;
    } else if (!m_Config.burst) {
        // Continuous emission
        m_EmissionAccumulator += m_Config.emissionRate * deltaTime;
        
        int spawnCount = static_cast<int>(m_EmissionAccumulator);
        m_EmissionAccumulator -= spawnCount;
        
        for (int i = 0; i < spawnCount; i++) {
            Particle* p = m_Pool.Spawn();
            if (!p) break;
            
            // Initialize particle
            float angle = RandomFloat(0, 6.28318f);
            float radius = RandomFloat(0, m_Config.emitterRadius);
            p->position = m_Config.emitterPosition + Vec3(
                cos(angle) * radius,
                0,
                sin(angle) * radius
            );
            p->velocity = m_Config.initialVelocity + RandomVec3(
                -m_Config.velocityRandom,
                m_Config.velocityRandom
            );
            p->lifetime = RandomFloat(m_Config.lifetimeMin, m_Config.lifetimeMax);
            p->size = RandomFloat(m_Config.sizeMin, m_Config.sizeMax);
            p->color = m_Config.colorStart;
            p->rotation = RandomFloat(0, 6.28318f);
            p->age = 0.0f;
        }
    }
}

void ParticleSystem::UpdateParticles(float deltaTime) {
    auto& particles = m_Pool.GetParticles();
    
    for (size_t i = 0; i < particles.size(); i++) {
        Particle& p = particles[i];
        if (!p.alive) continue;
        
        // Age
        p.age += deltaTime;
        
        // Kill dead particles
        if (p.IsDead()) {
            m_Pool.Kill(i);
            continue;
        }
        
        // Physics
        p.velocity = p.velocity + m_Config.gravity * deltaTime;
        
        // Velocity damping
        if (m_Config.velocityOverLifetimeEnabled) {
            p.velocity = p.velocity * (1.0f - m_Config.velocityDamping * deltaTime);
        }
        
        p.position = p.position + p.velocity * deltaTime;
        
        // Apply modules
        ApplyModules(p);
    }
}

void ParticleSystem::ApplyModules(Particle& p) {
    float t = p.GetNormalizedAge();
    
    // Size over lifetime
    if (m_Config.sizeOverLifetimeEnabled) {
        float sizeMult = m_Config.sizeOverLifetime.Evaluate(t);
        p.size = RandomFloat(m_Config.sizeMin, m_Config.sizeMax) * sizeMult;
    }
    
    // Color over lifetime
    if (m_Config.colorOverLifetimeEnabled) {
        p.color = m_Config.colorOverLifetime.Evaluate(t);
    }
}

void ParticleSystem::Render(const Mat4& view, const Mat4& projection) {
    if (!m_RenderingInitialized) {
        InitializeRendering();
        m_RenderingInitialized = true;
    }
    
    if (!m_Shader || m_Pool.GetAliveCount() == 0) return;
    
    BuildVertexData();
    
    // Update vertex buffer
    if (m_VAO && !m_VertexData.empty()) {
        auto vb = std::static_pointer_cast<OpenGLVertexBuffer>(m_VAO->GetVertexBuffers()[0]);
        vb->Bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, m_VertexData.size() * sizeof(float), m_VertexData.data());
    }
    
    // Render
    glEnable(GL_BLEND);
    if (m_Config.additiveBlending) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // Additive blending
    } else {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Alpha blending
    }
    glDepthMask(GL_FALSE);  // Disable depth writes for transparent particles
    
    m_Shader->Bind();
    m_Shader->SetMat4("u_ViewProjection", projection * view);
    
    m_VAO->Bind();
    glDrawArrays(GL_TRIANGLES, 0, m_Pool.GetAliveCount() * 6);
    
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void ParticleSystem::BuildVertexData() {
    m_VertexData.clear();
    
    auto& particles = m_Pool.GetParticles();
    
    for (const auto& p : particles) {
        if (!p.alive) continue;
        
        // Billboard quad (2 triangles, 6 vertices)
        float halfSize = p.size * 0.5f;
        
        // Vertex format: Position(3) + TexCoord(2) + Color(4)
        // Triangle 1: 0-1-2
        // Triangle 2: 2-1-3
        
        // v0: bottom-left
        m_VertexData.push_back(p.position.x - halfSize);
        m_VertexData.push_back(p.position.y - halfSize);
        m_VertexData.push_back(p.position.z);
        m_VertexData.push_back(0.0f);  // u
        m_VertexData.push_back(0.0f);  // v
        m_VertexData.push_back(p.color.x);
        m_VertexData.push_back(p.color.y);
        m_VertexData.push_back(p.color.z);
        m_VertexData.push_back(p.color.w);
        
        // v1: bottom-right
        m_VertexData.push_back(p.position.x + halfSize);
        m_VertexData.push_back(p.position.y - halfSize);
        m_VertexData.push_back(p.position.z);
        m_VertexData.push_back(1.0f);
        m_VertexData.push_back(0.0f);
        m_VertexData.push_back(p.color.x);
        m_VertexData.push_back(p.color.y);
        m_VertexData.push_back(p.color.z);
        m_VertexData.push_back(p.color.w);
        
        // v2: top-left
        m_VertexData.push_back(p.position.x - halfSize);
        m_VertexData.push_back(p.position.y + halfSize);
        m_VertexData.push_back(p.position.z);
        m_VertexData.push_back(0.0f);
        m_VertexData.push_back(1.0f);
        m_VertexData.push_back(p.color.x);
        m_VertexData.push_back(p.color.y);
        m_VertexData.push_back(p.color.z);
        m_VertexData.push_back(p.color.w);
        
        // Triangle 2
        // v2: top-left (repeat)
        m_VertexData.push_back(p.position.x - halfSize);
        m_VertexData.push_back(p.position.y + halfSize);
        m_VertexData.push_back(p.position.z);
        m_VertexData.push_back(0.0f);
        m_VertexData.push_back(1.0f);
        m_VertexData.push_back(p.color.x);
        m_VertexData.push_back(p.color.y);
        m_VertexData.push_back(p.color.z);
        m_VertexData.push_back(p.color.w);
        
        // v1: bottom-right (repeat)
        m_VertexData.push_back(p.position.x + halfSize);
        m_VertexData.push_back(p.position.y - halfSize);
        m_VertexData.push_back(p.position.z);
        m_VertexData.push_back(1.0f);
        m_VertexData.push_back(0.0f);
        m_VertexData.push_back(p.color.x);
        m_VertexData.push_back(p.color.y);
        m_VertexData.push_back(p.color.z);
        m_VertexData.push_back(p.color.w);
        
        // v3: top-right
        m_VertexData.push_back(p.position.x + halfSize);
        m_VertexData.push_back(p.position.y + halfSize);
        m_VertexData.push_back(p.position.z);
        m_VertexData.push_back(1.0f);
        m_VertexData.push_back(1.0f);
        m_VertexData.push_back(p.color.x);
        m_VertexData.push_back(p.color.y);
        m_VertexData.push_back(p.color.z);
        m_VertexData.push_back(p.color.w);
    }
}

void ParticleSystem::InitializeRendering() {
    // Create shader
    std::string vsSource = R"(
        #version 330 core
        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec2 a_TexCoord;
        layout(location = 2) in vec4 a_Color;
        
        uniform mat4 u_ViewProjection;
        
        out vec2 v_TexCoord;
        out vec4 v_Color;
        
        void main() {
            gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
            v_TexCoord = a_TexCoord;
            v_Color = a_Color;
        }
    )";
    
    std::string fsSource = R"(
        #version 330 core
        in vec2 v_TexCoord;
        in vec4 v_Color;
        
        out vec4 FragColor;
        
        void main() {
            // Simple circular particle
            vec2 coord = v_TexCoord * 2.0 - 1.0;
            float dist = length(coord);
            float alpha = 1.0 - smoothstep(0.0, 1.0, dist);
            
            FragColor = v_Color * alpha;
        }
    )";
    
    m_Shader = std::make_shared<OpenGLShader>("ParticleShader", vsSource, fsSource);
    
    // Create VAO
    size_t maxVerts = m_Config.maxParticles * 6;
    size_t bufferSize = maxVerts * 9 * sizeof(float);
    
    auto vb = std::make_shared<OpenGLVertexBuffer>(nullptr, bufferSize);
    vb->SetLayout({
        { ShaderDataType::Float3, "a_Position" },
        { ShaderDataType::Float2, "a_TexCoord" },
        { ShaderDataType::Float4, "a_Color" }
    });
    
    m_VAO = std::make_shared<OpenGLVertexArray>();
    m_VAO->AddVertexBuffer(vb);
    
    ENGINE_INFO("[ParticleSystem] Rendering initialized: {} max particles", m_Config.maxParticles);
}

void ParticleSystem::Reset() {
    m_Pool.Reset();
    m_Time = 0.0f;
    m_EmissionAccumulator = 0.0f;
    m_BurstTriggered = false;
}

void ParticleSystem::Play() {
    m_Playing = true;
}

void ParticleSystem::Stop() {
    m_Playing = false;
}

// ============================================================================
// Preset Factory
// ============================================================================

ParticleSystemConfig ParticlePresetFactory::CreateFirePreset() {
    ParticleSystemConfig config;
    config.name = "Fire";
    config.loop = true;
    config.duration = 5.0f;
    config.maxParticles = 500;
    config.emissionRate = 50.0f;
    config.burst = false;
    
    config.emitterRadius = 0.3f;
    config.lifetimeMin = 0.8f;
    config.lifetimeMax = 1.5f;
    config.initialVelocity = Vec3(0, 2.5f, 0);
    config.velocityRandom = Vec3(0.3f, 0.5f, 0.3f);
    config.sizeMin = 0.3f;
    config.sizeMax = 0.5f;
    
    config.gravity = Vec3(0, 0.5f, 0);  // Slight upward drift
    
    // Size over lifetime: small -> big -> small
    config.sizeOverLifetimeEnabled = true;
    config.sizeOverLifetime.AddKey(0.0f, 0.3f);
    config.sizeOverLifetime.AddKey(0.3f, 1.0f);
    config.sizeOverLifetime.AddKey(1.0f, 0.1f);
    
    // Color over lifetime: bright yellow -> orange -> dark
    config.colorOverLifetimeEnabled = true;
    config.colorOverLifetime.AddKey(0.0f, Vec4(1.0f, 0.9f, 0.3f, 1.0f));   // Bright yellow
    config.colorOverLifetime.AddKey(0.5f, Vec4(1.0f, 0.3f, 0.1f, 0.8f));   // Orange
    config.colorOverLifetime.AddKey(1.0f, Vec4(0.2f, 0.1f, 0.1f, 0.0f));   // Dark fade
    
    config.additiveBlending = true;
    
    return config;
}

ParticleSystemConfig ParticlePresetFactory::CreateSmokePreset() {
    ParticleSystemConfig config;
    config.name = "Smoke";
    config.loop = true;
    config.duration = 5.0f;
    config.maxParticles = 300;
    config.emissionRate = 30.0f;
    config.burst = false;
    
    config.emitterRadius = 0.2f;
    config.lifetimeMin = 3.0f;
    config.lifetimeMax = 5.0f;
    config.initialVelocity = Vec3(0, 1.5f, 0);
    config.velocityRandom = Vec3(0.5f, 0.5f, 0.5f);
    config.sizeMin = 0.5f;
    config.sizeMax = 0.8f;
    
    config.gravity = Vec3(0, 0.2f, 0);  // Slow rise
    config.velocityOverLifetimeEnabled = true;
    config.velocityDamping = 0.3f;
    
    // Size over lifetime: expand
    config.sizeOverLifetimeEnabled = true;
    config.sizeOverLifetime.AddKey(0.0f, 0.5f);
    config.sizeOverLifetime.AddKey(0.5f, 2.0f);
    config.sizeOverLifetime.AddKey(1.0f, 3.0f);
    
    // Color over lifetime: fade out
    config.colorOverLifetimeEnabled = true;
    config.colorOverLifetime.AddKey(0.0f, Vec4(0.3f, 0.3f, 0.3f, 0.8f));   // Dark gray
    config.colorOverLifetime.AddKey(0.7f, Vec4(0.5f, 0.5f, 0.5f, 0.4f));   // Light gray
    config.colorOverLifetime.AddKey(1.0f, Vec4(0.6f, 0.6f, 0.6f, 0.0f));   // Fade
    
    config.additiveBlending = false;  // Alpha blending
    
    return config;
}

ParticleSystemConfig ParticlePresetFactory::CreateExplosionPreset() {
    ParticleSystemConfig config;
    config.name = "Explosion";
    config.loop = false;  // One-shot
    config.duration = 2.0f;
    config.maxParticles = 1000;
    config.burst = true;
    config.burstCount = 500;
    
    config.emitterRadius = 0.1f;
    config.lifetimeMin = 0.5f;
    config.lifetimeMax = 1.5f;
    config.initialVelocity = Vec3(0, 0, 0);
    config.velocityRandom = Vec3(10.0f, 10.0f, 10.0f);  // Explode in all directions
    config.sizeMin = 0.1f;
    config.sizeMax = 0.3f;
    
    config.gravity = Vec3(0, -5.0f, 0);  // Strong gravity
    
    // Size over lifetime: flash then shrink
    config.sizeOverLifetimeEnabled = true;
    config.sizeOverLifetime.AddKey(0.0f, 0.2f);
    config.sizeOverLifetime.AddKey(0.1f, 1.5f);
    config.sizeOverLifetime.AddKey(1.0f, 0.1f);
    
    // Color over lifetime: bright flash -> dark
    config.colorOverLifetimeEnabled = true;
    config.colorOverLifetime.AddKey(0.0f, Vec4(1.0f, 1.0f, 0.8f, 1.0f));   // White flash
    config.colorOverLifetime.AddKey(0.3f, Vec4(1.0f, 0.5f, 0.2f, 0.8f));   // Orange
    config.colorOverLifetime.AddKey(1.0f, Vec4(0.1f, 0.1f, 0.1f, 0.0f));   // Fade
    
    config.additiveBlending = true;
    
    return config;
}

} // namespace MyEngine
