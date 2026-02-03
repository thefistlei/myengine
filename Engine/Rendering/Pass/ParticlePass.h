/******************************************************************************
 * File: ParticlePass.h
 * Description: Particle rendering pass for the pass-based render system
 ******************************************************************************/

#pragma once

#include "Rendering/Pass/RenderPass.h"
#include "Particle/ParticleSystem.h"
#include <memory>
#include <chrono>

namespace MyEngine {

class ParticlePass : public RenderPass {
public:
    ParticlePass(ParticlePreset preset);
    virtual ~ParticlePass() = default;
    
    // RenderPass interface
    const char* GetName() const override;
    PassType GetType() const override { return PassType::Graphics; }
    PassCategory GetCategory() const override { return PassCategory::VFX; }
    
    void OnCreate(RenderBackend* backend) override;
    void Execute(const SceneView& view, Registry* registry) override;
    void OnDestroy() override;
    void OnGUI() override;  // Editor UI
    
    // Access
    ParticleSystem* GetParticleSystem() { return m_ParticleSystem.get(); }
    
private:
    ParticlePreset m_Preset;
    std::unique_ptr<ParticleSystem> m_ParticleSystem;
    std::string m_PassName;
    
    // Time tracking
    std::chrono::steady_clock::time_point m_LastUpdateTime;
    bool m_FirstFrame;
};

} // namespace MyEngine
