/******************************************************************************
 * File: ParticlePass.cpp
 * Description: Particle pass implementation with ImGui properties
 ******************************************************************************/

#include "ParticlePass.h"
#include "Core/Log.h"
#include <imgui.h>

namespace MyEngine {

ParticlePass::ParticlePass(ParticlePreset preset)
    : m_Preset(preset)
    , m_FirstFrame(true)
{
    // Set pass name based on preset
    switch (preset) {
        case ParticlePreset::Fire:
            m_PassName = "ParticlePass_Fire";
            break;
        case ParticlePreset::Smoke:
            m_PassName = "ParticlePass_Smoke";
            break;
        case ParticlePreset::Explosion:
            m_PassName = "ParticlePass_Explosion";
            break;
    }
}

const char* ParticlePass::GetName() const {
    return m_PassName.c_str();
}

void ParticlePass::OnCreate(RenderBackend* backend) {
    m_Backend = backend;
    
    // Create particle system with preset
    ParticleSystemConfig config = ParticlePresetFactory::CreatePreset(m_Preset);
    m_ParticleSystem = std::make_unique<ParticleSystem>(config);
    
    ENGINE_INFO("[{}] Created", m_PassName);
}

void ParticlePass::Execute(const SceneView& view, Registry* registry) {
    if (!m_ParticleSystem) return;
    
    // Calculate delta time
    float deltaTime = 0.016f;  // Default 60fps
    
    if (m_FirstFrame) {
        m_LastUpdateTime = std::chrono::steady_clock::now();
        m_FirstFrame = false;
    } else {
        auto currentTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - m_LastUpdateTime);
        deltaTime = duration.count() / 1000000.0f;  // Convert to seconds
        m_LastUpdateTime = currentTime;
        
        // Clamp deltaTime to avoid huge jumps
        if (deltaTime > 0.1f) deltaTime = 0.016f;
    }
    
    // Update particle system
    m_ParticleSystem->Update(deltaTime);
    
    // Render particles
    m_ParticleSystem->Render(view.GetViewMatrix(), view.GetProjectionMatrix());
}

void ParticlePass::OnDestroy() {
    m_ParticleSystem.reset();
    ENGINE_INFO("[{}] Destroyed", m_PassName);
}

void ParticlePass::OnGUI() {
    if (!m_ParticleSystem) return;
    
    ImGui::Text("Particle System: %s", m_ParticleSystem->GetConfig().name.c_str());
    ImGui::Separator();
    
    auto& config = m_ParticleSystem->GetConfig();
    
    // Position
    float pos[3] = { config.emitterPosition.x, config.emitterPosition.y, config.emitterPosition.z };
    if (ImGui::DragFloat3("Position", pos, 0.1f)) {
        config.emitterPosition = Vec3(pos[0], pos[1], pos[2]);
        ENGINE_INFO("[{}] Position changed to: ({}, {}, {})", m_PassName, pos[0], pos[1], pos[2]);
    }
    
    // Emission rate
    if (ImGui::DragFloat("Emission Rate", &config.emissionRate, 1.0f, 0.0f, 200.0f)) {
    }
    
    // Lifetime
    ImGui::DragFloat("Lifetime Min", &config.lifetimeMin, 0.1f, 0.1f, 10.0f);
    ImGui::DragFloat("Lifetime Max", &config.lifetimeMax, 0.1f, 0.1f, 10.0f);
    
    // Size
    ImGui::DragFloat("Size Min", &config.sizeMin, 0.01f, 0.01f, 5.0f);
    ImGui::DragFloat("Size Max", &config.sizeMax, 0.01f, 0.01f, 5.0f);
    
    // Control buttons
    ImGui::Separator();
    if (ImGui::Button("Reset")) {
        m_ParticleSystem->Reset();
    }
    ImGui::SameLine();
    if (ImGui::Button("Play")) {
        m_ParticleSystem->Play();
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        m_ParticleSystem->Stop();
    }
    
    // Stats
    ImGui::Separator();
    ImGui::Text("Alive: %zu / %zu", 
                m_ParticleSystem->GetAliveCount(),
                m_ParticleSystem->GetMaxParticles());
    
    // Current config display
    ImGui::Text("Current Position: (%.2f, %.2f, %.2f)",
                config.emitterPosition.x,
                config.emitterPosition.y,
                config.emitterPosition.z);
}

} // namespace MyEngine
