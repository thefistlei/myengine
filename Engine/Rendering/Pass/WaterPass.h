/******************************************************************************
 * File: WaterPass.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Modified: 2026-02-01
 * Description: Water surface rendering pass with reflections and waves
 ******************************************************************************/

#pragma once

#include "RenderPass.h"
#include "Resource/Mesh.h"
#include "Resource/Texture.h"
#include "Rendering/Shader.h"
#include "Math/MathTypes.h"
#include <memory>

namespace MyEngine {

/**
 * @brief Renders water surfaces with reflection, refraction, and wave animation
 * 
 * Features:
 * - Procedural wave animation using sine waves
 * - Fresnel effect for water surface
 * - Normal mapping for realistic wave details
 * - Optional reflection cubemap
 * - Adjustable water color and transparency
 */
class WaterPass : public GraphicsPass {
public:
    WaterPass();
    virtual ~WaterPass() = default;
    
    const char* GetName() const override { return "WaterPass"; }
    PassCategory GetCategory() const override { return PassCategory::Water; }
    
    void OnCreate(RenderBackend* backend) override;
    void OnDestroy() override;
    void Execute(const SceneView& view, Registry* registry) override;
    void OnGUI() override;
    
    // Water configuration
    void SetWaterLevel(float level) { m_WaterLevel = level; }
    void SetWaterColor(const Vec3& color) { m_WaterColor = color; }
    void SetWaveSpeed(float speed) { m_WaveSpeed = speed; }
    void SetWaveAmplitude(float amplitude) { m_WaveAmplitude = amplitude; }
    void SetSubdivisions(int subdivisions);
    
    // Texture management
    void SetNormalMap(const std::string& path);
    void SetReflectionMap(const std::string& path);
    
private:
    void CreateWaterPlane();
    void CreateWaterShader();
    void LoadTextures();
    void CreateDefaultNormalMap();
    
private:
    // Rendering resources
    std::unique_ptr<Mesh> m_WaterMesh;
    std::unique_ptr<Shader> m_WaterShader;
    std::shared_ptr<Texture> m_NormalMap;
    std::shared_ptr<Texture> m_ReflectionMap;
    
    // Water properties
    float m_WaterLevel = 0.0f;
    Vec3 m_WaterColor = Vec3(0.1f, 0.4f, 0.7f);     // Bright ocean blue (更鲜艳的蓝色)
    float m_WaveSpeed = 1.2f;                        // Slightly faster for ocean waves
    float m_WaveAmplitude = 0.08f;                   // Bigger waves for ocean effect
    float m_WaveFrequency = 1.5f;                    // Larger, rolling ocean waves
    float m_Transparency = 0.75f;                    // Less transparent for deeper ocean look
    float m_Time = 0.0f;
    
    // Normal map settings
    float m_NormalStrength = 1.0f;
    float m_NormalScale = 10.0f;
    
    // Water plane geometry
    float m_PlaneSize = 100.0f;
    int m_Subdivisions = 64;  // Higher subdivision for smoother waves
    
    // Feature toggles
    bool m_UseNormalMap = true;
    bool m_UseReflectionMap = false;
};

} // namespace MyEngine
