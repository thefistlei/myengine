/******************************************************************************
 * File: PostProcessPass.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Post-processing effects pass for screen-space effects
 ******************************************************************************/

#pragma once

#include "RenderPass.h"
#include "Rendering/Shader.h"
#include "Math/MathTypes.h"
#include <memory>

namespace MyEngine {

/**
 * @brief Post-processing pass for screen-space effects
 * 
 * Features:
 * - Tone mapping (HDR to LDR)
 * - Color grading
 * - Bloom effect
 * - Vignette effect
 * - Chromatic aberration
 * - Film grain
 */
class PostProcessPass : public GraphicsPass {
public:
    PostProcessPass();
    virtual ~PostProcessPass() = default;
    
    const char* GetName() const override { return "PostProcessPass"; }
    PassCategory GetCategory() const override { return PassCategory::PostProcess; }
    
    void OnCreate(RenderBackend* backend) override;
    void OnDestroy() override;
    void Execute(const SceneView& view, Registry* registry) override;
    void OnGUI() override;
    
    // Effect toggles
    void SetBloomEnabled(bool enabled) { m_EnableBloom = enabled; }
    void SetVignetteEnabled(bool enabled) { m_EnableVignette = enabled; }
    void SetChromaticAberrationEnabled(bool enabled) { m_EnableChromaticAberration = enabled; }
    void SetFilmGrainEnabled(bool enabled) { m_EnableFilmGrain = enabled; }
    
private:
    void CreateFullscreenQuad();
    void CreatePostProcessShader();
    
private:
    std::unique_ptr<Shader> m_PostProcessShader;
    uint32_t m_QuadVAO = 0;
    uint32_t m_QuadVBO = 0;
    uint32_t m_FallbackWhiteTex = 0;
    uint32_t m_ScreenColorTexture = 0;
    
    // Effect parameters
    bool m_EnableBloom = true;
    bool m_EnableVignette = true;
    bool m_EnableChromaticAberration = false;
    bool m_EnableFilmGrain = false;
    
    float m_Exposure = 1.0f;
    float m_Contrast = 1.0f;
    float m_Saturation = 1.0f;
    float m_Brightness = 0.0f;
    
    float m_BloomIntensity = 0.3f;
    float m_BloomThreshold = 1.0f;
    
    float m_VignetteIntensity = 0.3f;
    float m_VignetteRadius = 0.8f;
    
    float m_ChromaticAberrationStrength = 0.002f;
    float m_FilmGrainIntensity = 0.05f;
    
    float m_Time = 0.0f;
};

} // namespace MyEngine
