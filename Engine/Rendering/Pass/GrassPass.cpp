/******************************************************************************
 * File: GrassPass.cpp
 * Author: AI Assistant
 * Created: 2026-01-31
 * Description: Grass pass implementation with wind animation
 ******************************************************************************/

#include "GrassPass.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderBackend.h"
#include "Rendering/OpenGL/OpenGLShader.h"
#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "Core/Log.h"
#include <imgui.h>
#include <glad/gl.h>
#include <cmath>
#include <random>

namespace MyEngine {

void GrassPass::OnCreate(RenderBackend* backend) {
    m_Backend = backend;
    
    // Do NOT create resources if pass is disabled
    // Resources will be created on first Execute() call if needed
    
    ENGINE_INFO("[GrassPass] Created (deferred resource initialization)");
}

void GrassPass::OnDestroy() {
    m_GrassMesh.reset();
    if (m_Shader) {
        delete m_Shader;
        m_Shader = nullptr;
    }
    ENGINE_INFO("[GrassPass] Destroyed");
}

void GrassPass::Execute(const SceneView& view, Registry* registry) {
    // Lazy initialization on first execute
    if (!m_Shader || !m_GrassMesh) {
        // Save OpenGL state before creating resources
        GLint oldVAO, oldArrayBuffer, oldElementBuffer;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldVAO);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &oldArrayBuffer);
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &oldElementBuffer);
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        
        CreateGrassField();
        CreateGrassShader();
        
        // Restore OpenGL state after creating resources
        glBindVertexArray(oldVAO);
        glBindBuffer(GL_ARRAY_BUFFER, oldArrayBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oldElementBuffer);
        glUseProgram(oldProgram);
        
        ENGINE_INFO("[GrassPass] Resources initialized on first execute");
    }
    
    if (!m_Shader || !m_GrassMesh) return;
    
    // Save current OpenGL state
    GLboolean wasBlendEnabled = glIsEnabled(GL_BLEND);
    GLboolean wasDepthMaskEnabled;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &wasDepthMaskEnabled);
    GLboolean wasCullFaceEnabled = glIsEnabled(GL_CULL_FACE);
    
    // Update time for wind animation
    m_Time += 0.016f;  // Approximate 60fps
    
    // Enable alpha blending for grass transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Enable depth test but disable depth writing for correct alpha blending
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    
    // Disable backface culling to see grass from both sides
    glDisable(GL_CULL_FACE);
    
    // Bind shader
    m_Shader->Bind();
    
    // Find the GrassPass entity and get its transform
    Mat4 modelMatrix = Mat4(); // Identity matrix by default
    if (registry) {
        auto entities = registry->GetEntitiesWith<PassComponent, TransformComponent>();
        for (auto entityID : entities) {
            Entity entity(entityID, registry);
            auto& passComp = entity.GetComponent<PassComponent>();
            if (passComp.pass == this) {
                auto& transform = entity.GetComponent<TransformComponent>();
                modelMatrix = transform.localMatrix;
                break;
            }
        }
    }
    
    // Set uniforms
    m_Shader->SetMat4("u_ViewProjection", view.GetViewProjectionMatrix());
    m_Shader->SetMat4("u_Model", modelMatrix);
    
    // Grass properties
    m_Shader->SetFloat3("u_GrassColor", m_GrassColor);
    m_Shader->SetFloat3("u_GrassTipColor", m_GrassTipColor);
    m_Shader->SetFloat("u_GrassHeight", m_GrassHeight);
    
    // Wind animation
    m_Shader->SetFloat3("u_WindDirection", m_WindDirection.Normalized());
    m_Shader->SetFloat("u_WindStrength", m_WindStrength);
    m_Shader->SetFloat("u_WindSpeed", m_WindSpeed);
    m_Shader->SetFloat("u_Time", m_Time);
    
    // Lighting
    m_Shader->SetFloat3("u_LightDir", Vec3(0.3f, -0.7f, 0.5f).Normalized());
    m_Shader->SetFloat3("u_LightColor", Vec3(1.0f, 0.98f, 0.9f));
    
    // Render grass
    Renderer::DrawMesh(*m_GrassMesh, modelMatrix);
    
    // Restore OpenGL state to previous values
    if (wasDepthMaskEnabled) {
        glDepthMask(GL_TRUE);
    } else {
        glDepthMask(GL_FALSE);
    }
    
    if (!wasBlendEnabled) {
        glDisable(GL_BLEND);
    }
    
    if (wasCullFaceEnabled) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

void GrassPass::OnGUI() {
    ImGui::Text("Grass Parameters");
    
    if (ImGui::ColorEdit3("Grass Color", &m_GrassColor.x)) {
        // Color changed
    }
    if (ImGui::ColorEdit3("Tip Color", &m_GrassTipColor.x)) {
        // Tip color changed
    }
    
    if (ImGui::SliderFloat("Height", &m_GrassHeight, 0.1f, 3.0f)) {
        // Grass height changed
    }
    
    if (ImGui::SliderInt("Density", &m_GrassDensity, 10, 100)) {
        CreateGrassField();  // Regenerate grass
    }
    
    if (ImGui::SliderFloat("Spread", &m_GrassSpread, 10.0f, 200.0f)) {
        CreateGrassField();  // Regenerate grass
    }
    
    ImGui::Separator();
    ImGui::Text("Wind Parameters");
    
    ImGui::SliderFloat3("Wind Direction", &m_WindDirection.x, -1.0f, 1.0f);
    ImGui::SliderFloat("Wind Strength", &m_WindStrength, 0.0f, 2.0f);
    ImGui::SliderFloat("Wind Speed", &m_WindSpeed, 0.1f, 5.0f);
    
    if (ImGui::Button("Reset Wind")) {
        m_WindDirection = Vec3(1.0f, 0.0f, 0.3f);
        m_WindStrength = 0.5f;
        m_WindSpeed = 2.0f;
    }
}

void GrassPass::CreateGrassField() {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    
    // Random number generator for variation
    std::random_device rd;
    std::mt19937 gen(42);  // Fixed seed for consistency
    std::uniform_real_distribution<float> heightVar(0.8f, 1.2f);
    std::uniform_real_distribution<float> angleVar(0.0f, 6.28318f);  // 0 to 2*PI
    std::uniform_real_distribution<float> offsetVar(-0.3f, 0.3f);
    
    uint32_t indexOffset = 0;
    
    // Generate grass blades in a grid pattern
    for (int x = 0; x < m_GrassDensity; ++x) {
        for (int z = 0; z < m_GrassDensity; ++z) {
            // Calculate blade position
            float xPos = (x / static_cast<float>(m_GrassDensity - 1) - 0.5f) * m_GrassSpread;
            float zPos = (z / static_cast<float>(m_GrassDensity - 1) - 0.5f) * m_GrassSpread;
            
            // Add random offset for natural look
            xPos += offsetVar(gen);
            zPos += offsetVar(gen);
            
            // Random height variation
            float height = m_GrassHeight * heightVar(gen);
            float width = m_GrassWidth * 0.5f;
            
            // Random rotation
            float angle = angleVar(gen);
            float cosAngle = std::cos(angle);
            float sinAngle = std::sin(angle);
            
            // Create a quad for grass blade (two triangles)
            // Bottom vertices (at ground level)
            Vertex v0, v1, v2, v3;
            
            // Bottom left
            v0.Position = Vec3(xPos - width * cosAngle, 0.0f, zPos - width * sinAngle);
            v0.Normal = Vec3(sinAngle, 0.0f, -cosAngle);  // Perpendicular to blade
            v0.TexCoord = Vec2(0.0f, 0.0f);
            
            // Bottom right
            v1.Position = Vec3(xPos + width * cosAngle, 0.0f, zPos + width * sinAngle);
            v1.Normal = Vec3(sinAngle, 0.0f, -cosAngle);
            v1.TexCoord = Vec2(1.0f, 0.0f);
            
            // Top left
            v2.Position = Vec3(xPos - width * cosAngle, height, zPos - width * sinAngle);
            v2.Normal = Vec3(sinAngle, 0.0f, -cosAngle);
            v2.TexCoord = Vec2(0.0f, 1.0f);
            
            // Top right
            v3.Position = Vec3(xPos + width * cosAngle, height, zPos + width * sinAngle);
            v3.Normal = Vec3(sinAngle, 0.0f, -cosAngle);
            v3.TexCoord = Vec2(1.0f, 1.0f);
            
            vertices.push_back(v0);
            vertices.push_back(v1);
            vertices.push_back(v2);
            vertices.push_back(v3);
            
            // Two triangles for the blade
            indices.push_back(indexOffset + 0);
            indices.push_back(indexOffset + 1);
            indices.push_back(indexOffset + 2);
            
            indices.push_back(indexOffset + 1);
            indices.push_back(indexOffset + 3);
            indices.push_back(indexOffset + 2);
            
            indexOffset += 4;
        }
    }
    
    m_GrassMesh = std::make_unique<Mesh>(vertices, indices);
    ENGINE_INFO("[GrassPass] Created grass field: {} blades, {} vertices, {} triangles",
                m_GrassDensity * m_GrassDensity, vertices.size(), indices.size() / 3);
}

void GrassPass::CreateGrassShader() {
    const char* vertexSrc = R"(
        #version 330 core
        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec3 a_Normal;
        layout(location = 2) in vec2 a_TexCoord;
        
        uniform mat4 u_ViewProjection;
        uniform mat4 u_Model;
        uniform float u_GrassHeight;
        
        // Wind parameters
        uniform vec3 u_WindDirection;
        uniform float u_WindStrength;
        uniform float u_WindSpeed;
        uniform float u_Time;
        
        out vec3 v_Normal;
        out vec2 v_TexCoord;
        out float v_Height;
        
        void main() {
            vec4 worldPos = u_Model * vec4(a_Position, 1.0);
            
            // Apply wind animation to top vertices (based on Y position)
            float heightFactor = a_Position.y / u_GrassHeight;  // 0 at bottom, 1 at top
            heightFactor = heightFactor * heightFactor;  // Square for more natural bend
            
            // Wave effect with multiple frequencies for natural look
            float wave1 = sin(worldPos.x * 0.3 + u_Time * u_WindSpeed) * 0.5;
            float wave2 = sin(worldPos.z * 0.4 + u_Time * u_WindSpeed * 1.3) * 0.3;
            float wave = (wave1 + wave2) * heightFactor * u_WindStrength;
            
            // Apply wind displacement
            worldPos.xyz += u_WindDirection * wave;
            worldPos.y -= wave * 0.2;  // Slight droop when bending
            
            v_Normal = mat3(u_Model) * a_Normal;
            v_TexCoord = a_TexCoord;
            v_Height = heightFactor;
            
            gl_Position = u_ViewProjection * worldPos;
        }
    )";
    
    const char* fragmentSrc = R"(
        #version 330 core
        
        in vec3 v_Normal;
        in vec2 v_TexCoord;
        in float v_Height;
        
        uniform vec3 u_GrassColor;
        uniform vec3 u_GrassTipColor;
        uniform vec3 u_LightDir;
        uniform vec3 u_LightColor;
        
        out vec4 FragColor;
        
        void main() {
            // Interpolate color from base to tip
            vec3 grassColor = mix(u_GrassColor, u_GrassTipColor, v_Height);
            
            // Simple lighting
            vec3 norm = normalize(v_Normal);
            float diff = max(dot(norm, -u_LightDir), 0.0);
            float ambient = 0.5;
            float lighting = ambient + diff * (1.0 - ambient);
            
            vec3 finalColor = grassColor * u_LightColor * lighting;
            
            // Alpha fade at edges for softer look
            float alpha = 1.0 - abs(v_TexCoord.x * 2.0 - 1.0);  // Fade at X edges
            alpha *= 0.9;  // Overall slight transparency
            
            FragColor = vec4(finalColor, alpha);
        }
    )";
    
    m_Shader = new OpenGLShader("GrassShader", vertexSrc, fragmentSrc);
    ENGINE_INFO("[GrassPass] Grass shader created");
}

// Auto-register this pass
REGISTER_PASS(GrassPass);

} // namespace MyEngine
