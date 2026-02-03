/******************************************************************************
 * File: GeometryPass.cpp
 * Author: AI Assistant
 * Created: 2026-01-31
 * Description: Geometry pass implementation
 ******************************************************************************/

#include "GeometryPass.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderBackend.h"
#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "Resource/Mesh.h"
#include "Core/Log.h"
#include <imgui.h>
#include <glad/gl.h>

namespace MyEngine {

void GeometryPass::OnCreate(RenderBackend* backend) {
    m_Backend = backend;
    ENGINE_INFO("[GeometryPass] Created");
}

void GeometryPass::OnDestroy() {
    ENGINE_INFO("[GeometryPass] Destroyed");
}

void GeometryPass::Execute(const SceneView& view, Registry* registry) {
    if (!m_Shader || !registry) return;
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Backface culling
    if (m_EnableBackfaceCulling) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    } else {
        glDisable(GL_CULL_FACE);
    }
    
    // Wireframe mode
    if (m_ShowWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    // Bind shader
    m_Shader->Bind();
    
    // Set view-projection matrix
    m_Shader->SetMat4("u_ViewProjection", view.GetViewProjectionMatrix());
    
    // Set camera position for lighting
    m_Shader->SetFloat3("u_CameraPos", view.GetPosition());
    
    // Set lighting uniforms (CRITICAL: must override shader defaults)
    Vec3 lightPos(10.0f, 10.0f, 10.0f);
    Vec3 lightColor(1.0f, 1.0f, 1.0f);
    Vec3 objectColor(0.7f, 0.75f, 0.8f);
    
    m_Shader->SetFloat3("u_LightPos", lightPos);
    m_Shader->SetFloat3("u_LightColor", lightColor);
    m_Shader->SetFloat3("u_ObjectColor", objectColor);
    
    // Only log once at startup
    static bool logged = false;
    if (!logged) {
        ENGINE_INFO("[GeometryPass] Lighting configured: LightPos({}, {}, {}), ObjectColor({}, {}, {})",
                    lightPos.x, lightPos.y, lightPos.z, objectColor.x, objectColor.y, objectColor.z);
        logged = true;
    }
    
    // Render all entities with MeshFilterComponent
    auto entities = registry->GetEntitiesWith<TransformComponent, MeshFilterComponent>();
    
    for (EntityID entityID : entities) {
        Entity entity(entityID, registry);
        
        auto& transform = entity.GetComponent<TransformComponent>();
        auto& meshFilter = entity.GetComponent<MeshFilterComponent>();
        
        // Build model matrix from transform
        Mat4 translationMat = Mat4::Translation(transform.localPosition);
        Mat4 rotationMat = Mat4(); // TODO: Implement Quat::ToMatrix()
        Mat4 scaleMat = Mat4::Scale(transform.localScale);
        Mat4 modelMatrix = translationMat * rotationMat * scaleMat;
        
        // Set model matrix uniform
        m_Shader->SetMat4("u_Transform", modelMatrix);
        
        // Render the mesh
        if (meshFilter.mesh) {
            Renderer::DrawMesh(*meshFilter.mesh, modelMatrix);
        }
    }
}

void GeometryPass::OnGUI() {
    ImGui::Checkbox("Wireframe Mode", &m_ShowWireframe);
    ImGui::Checkbox("Backface Culling", &m_EnableBackfaceCulling);
}

// Auto-register this pass
REGISTER_PASS(GeometryPass);

} // namespace MyEngine
