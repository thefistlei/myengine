/******************************************************************************
 * File: GeometryPass.h
 * Author: AI Assistant
 * Created: 2026-01-31
 * Description: Geometry rendering pass - renders all mesh entities
 ******************************************************************************/

#pragma once

#include "RenderPass.h"
#include "Rendering/Shader.h"

namespace MyEngine {

class GeometryPass : public GraphicsPass {
public:
    const char* GetName() const override { return "GeometryPass"; }
    PassCategory GetCategory() const override { return PassCategory::GBuffer; }
    
    void OnCreate(RenderBackend* backend) override;
    void OnDestroy() override;
    
    void Execute(const SceneView& view, Registry* registry) override;
    
    void OnGUI() override;
    
    void SetShader(Shader* shader) { m_Shader = shader; }
    Shader* GetShader() const { return m_Shader; }
    
private:
    Shader* m_Shader = nullptr;
    bool m_ShowWireframe = false;
    bool m_EnableBackfaceCulling = false;
};

} // namespace MyEngine
