/******************************************************************************
 * File: GrassPass.h
 * Author: AI Assistant
 * Created: 2026-01-31
 * Description: Grass rendering pass with wind animation
 ******************************************************************************/

#pragma once

#include "RenderPass.h"
#include "Math/MathTypes.h"
#include "Resource/Mesh.h"
#include <memory>

namespace MyEngine {

class Shader;

class GrassPass : public RenderPass {
public:
    GrassPass() = default;
    ~GrassPass() override = default;
    
    const char* GetName() const override { return "GrassPass"; }
    PassType GetType() const override { return PassType::Graphics; }
    PassCategory GetCategory() const override { return PassCategory::Vegetation; }

    void OnCreate(RenderBackend* backend) override;
    void OnDestroy() override;
    void Execute(const SceneView& view, Registry* registry) override;
    void OnGUI() override;

private:
    void CreateGrassField();
    void CreateGrassShader();

    Shader* m_Shader = nullptr;
    std::unique_ptr<Mesh> m_GrassMesh;
    
    // Grass parameters
    Vec3 m_GrassColor = Vec3(0.3f, 0.6f, 0.2f);  // Green grass
    Vec3 m_GrassTipColor = Vec3(0.5f, 0.8f, 0.3f);  // Lighter at tips
    float m_GrassHeight = 0.8f;
    float m_GrassWidth = 0.1f;
    int m_GrassDensity = 50;  // Grass blades per axis (50x50 = 2500 blades)
    float m_GrassSpread = 80.0f;  // Area coverage
    
    // Wind parameters
    Vec3 m_WindDirection = Vec3(1.0f, 0.0f, 0.3f);
    float m_WindStrength = 0.5f;
    float m_WindSpeed = 2.0f;
    
    float m_Time = 0.0f;
};

} // namespace MyEngine
