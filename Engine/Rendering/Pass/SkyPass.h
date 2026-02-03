/******************************************************************************
 * File: SkyPass.h
 * Author: AI Assistant
 * Created: 2026-01-31
 * Description: Sky rendering pass - atmospheric scattering skybox
 ******************************************************************************/

#pragma once

#include "RenderPass.h"
#include "Rendering/Shader.h"
#include "Resource/Mesh.h"
#include <memory>

namespace MyEngine {

class SkyPass : public GraphicsPass {
public:
    const char* GetName() const override { return "SkyPass"; }
    PassCategory GetCategory() const override { return PassCategory::Sky; }
    
    void OnCreate(RenderBackend* backend) override;
    void OnDestroy() override;
    
    void Execute(const SceneView& view, Registry* registry) override;
    
    void OnGUI() override;
    
private:
    Shader* m_Shader = nullptr;
    std::unique_ptr<Mesh> m_SkyMesh;
    
    // Sky parameters
    Vec3 m_SunDirection = Vec3(0.0f, 0.5f, 0.5f).Normalized();
    float m_SunIntensity = 10.0f;
    Vec3 m_SkyColor = Vec3(0.5f, 0.7f, 1.0f);
    Vec3 m_HorizonColor = Vec3(0.8f, 0.9f, 1.0f);
    Vec3 m_GroundColor = Vec3(0.3f, 0.25f, 0.2f);
    
    void CreateSkySphere();
    void CreateSkyShader();
};

} // namespace MyEngine
