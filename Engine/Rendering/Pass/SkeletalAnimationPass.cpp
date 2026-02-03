/******************************************************************************
 * File: SkeletalAnimationPass.cpp
 * Description: Skeletal animation pass implementation (LearnOpenGL example)
 ******************************************************************************/

#include "SkeletalAnimationPass.h"
#include "Rendering/OpenGL/OpenGLShader.h"
#include "Core/Log.h"
#include <fstream>
#include <sstream>
#include <vector>

// Use glad for OpenGL functions
#include <glad/gl.h>

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui.h>

namespace MyEngine {

void SkeletalAnimationPass::OnCreate(RenderBackend* backend) {
    m_Backend = backend;
    
    CreateAnimatedModel();
    CreateAnimationShader();
    LoadTextures();
    
    ENGINE_INFO("[SkeletalAnimationPass] Created");
}

void SkeletalAnimationPass::CreateAnimatedModel() {
    // Load animated model
    m_ModelLoader = std::make_unique<AnimatedModelLoader>();
    
    std::string modelPath = "E:/lei/ai-project/qcoder/t2/docs/refer/vampire/dancing_vampire.dae";
    ENGINE_INFO("[SkeletalAnimationPass] Loading model: {}", modelPath);
    
    m_AnimatedMesh = m_ModelLoader->LoadMesh(modelPath);
    if (!m_AnimatedMesh) {
        ENGINE_ERROR("[SkeletalAnimationPass] Failed to load animated mesh!");
        return;
    }
    
    // Load animation
    m_Animation = m_ModelLoader->LoadAnimation(modelPath);
    if (!m_Animation) {
        ENGINE_ERROR("[SkeletalAnimationPass] Failed to load animation!");
        return;
    }
    
    // Create animator
    m_Animator = std::make_unique<Animator>(m_Animation.get());
    
    ENGINE_INFO("[SkeletalAnimationPass] Model loaded successfully");
}

void SkeletalAnimationPass::CreateAnimationShader() {
    // Read shader files
    std::string vsPath = "E:/lei/ai-project/qcoder/t2/docs/refer/anim_model.vs";
    std::string fsPath = "E:/lei/ai-project/qcoder/t2/docs/refer/anim_model.fs";
    
    // Read vertex shader
    std::string vsSource;
    {
        std::ifstream file(vsPath);
        if (file) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            vsSource = buffer.str();
        } else {
            ENGINE_ERROR("[SkeletalAnimationPass] Failed to read vertex shader: {}", vsPath);
            return;
        }
    }
    
    // Read fragment shader
    std::string fsSource;
    {
        std::ifstream file(fsPath);
        if (file) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            fsSource = buffer.str();
        } else {
            ENGINE_ERROR("[SkeletalAnimationPass] Failed to read fragment shader: {}", fsPath);
            return;
        }
    }
    
    m_Shader = std::make_shared<OpenGLShader>("SkeletalAnimation", vsSource, fsSource);
    
    ENGINE_INFO("[SkeletalAnimationPass] Shader created");
}

void SkeletalAnimationPass::LoadTextures() {
    ENGINE_INFO("[SkeletalAnimationPass] LoadTextures() called");
    
    // Create fallback white texture
    glGenTextures(1, &m_WhiteTexture);
    glBindTexture(GL_TEXTURE_2D, m_WhiteTexture);
    unsigned char whitePixel[4] = { 255, 255, 255, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    ENGINE_INFO("[SkeletalAnimationPass] White texture created");
    
    // TODO: Texture loading is currently disabled due to STB_IMAGE_IMPLEMENTATION conflicts
    // The stb_image library is already defined in OpenGLTexture.cpp, causing symbol conflicts
    // when used here. Future solutions:
    // 1. Use a separate PNG loader library (lodepng, libpng)
    // 2. Refactor OpenGLTexture to be reusable without conflicts
    // 3. Load textures through AnimatedModelLoader using Assimp's embedded texture support
    
    ENGINE_WARN("[SkeletalAnimationPass] Texture loading temporarily disabled (stb_image conflict)");
    ENGINE_WARN("[SkeletalAnimationPass] Using white texture as fallback - skeletal animation will render without color");
    
    m_DiffuseTextureID = m_WhiteTexture;
}

void SkeletalAnimationPass::Execute(const SceneView& view, Registry* registry) {
    if (!m_Shader || !m_AnimatedMesh || !m_Animator) return;
    
    // Update animation (assuming 60fps)
    m_Animator->UpdateAnimation(0.016f);
    
    // Save OpenGL state
    GLboolean wasDepthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean wasCullFaceEnabled = glIsEnabled(GL_CULL_FACE);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Bind shader
    m_Shader->Bind();
    
    // Bind diffuse texture (or white fallback)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_DiffuseTextureID ? m_DiffuseTextureID : m_WhiteTexture);
    m_Shader->SetInt("texture_diffuse1", 0);
    
    // Setup matrices
    Mat4 projection = view.GetProjectionMatrix();
    Mat4 viewMat = view.GetViewMatrix();
    Mat4 model = Mat4::Translation(m_Position) * Mat4::Scale(m_Scale);
    
    // Log every 60 frames (about 1 second)
    static int frameCounter = 0;
    if (frameCounter++ % 60 == 0) {
        ENGINE_INFO("[SkeletalAnimationPass::Execute] Using Position: ({}, {}, {}), Scale: ({}, {}, {})",
                    m_Position.x, m_Position.y, m_Position.z, m_Scale.x, m_Scale.y, m_Scale.z);
    }
    
    m_Shader->SetMat4("projection", projection);
    m_Shader->SetMat4("view", viewMat);
    m_Shader->SetMat4("model", model);
    
    // Upload bone matrices
    auto transforms = m_Animator->GetFinalBoneMatrices();
    for (size_t i = 0; i < transforms.size(); ++i) {
        std::string uniformName = "finalBonesMatrices[" + std::to_string(i) + "]";
        m_Shader->SetMat4(uniformName, transforms[i]);
    }
    
    // Draw mesh
    auto vao = m_AnimatedMesh->GetVertexArray();
    vao->Bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_AnimatedMesh->GetIndices().size()), 
                  GL_UNSIGNED_INT, nullptr);
    vao->Unbind();
    
    m_Shader->Unbind();
    
    // Restore OpenGL state
    if (!wasDepthTestEnabled) glDisable(GL_DEPTH_TEST);
    if (!wasCullFaceEnabled) glDisable(GL_CULL_FACE);
}

void SkeletalAnimationPass::OnDestroy() {
    if (m_DiffuseTextureID) {
        glDeleteTextures(1, &m_DiffuseTextureID);
        m_DiffuseTextureID = 0;
    }
    
    if (m_WhiteTexture) {
        glDeleteTextures(1, &m_WhiteTexture);
        m_WhiteTexture = 0;
    }
    
    ENGINE_INFO("[SkeletalAnimationPass] Destroyed");
}

void SkeletalAnimationPass::OnGUI() {
    ImGui::Text("Skeletal Animation Settings");
    ImGui::Separator();
    
    // Position controls
    ImGui::Text("Position");
    float pos[3] = { m_Position.x, m_Position.y, m_Position.z };
    if (ImGui::DragFloat3("##Position", pos, 0.01f, -10.0f, 10.0f)) {
        m_Position = Vec3(pos[0], pos[1], pos[2]);
        ENGINE_INFO("[SkeletalAnimationPass] Position changed to: ({}, {}, {})", pos[0], pos[1], pos[2]);
    }
    
    // Scale controls
    ImGui::Text("Scale");
    float scale[3] = { m_Scale.x, m_Scale.y, m_Scale.z };
    if (ImGui::DragFloat3("##Scale", scale, 0.01f, 0.01f, 5.0f)) {
        m_Scale = Vec3(scale[0], scale[1], scale[2]);
        ENGINE_INFO("[SkeletalAnimationPass] Scale changed to: ({}, {}, {})", scale[0], scale[1], scale[2]);
    }
    
    // Info
    ImGui::Separator();
    ImGui::Text("Model: dancing_vampire.dae");
    if (m_AnimatedMesh) {
        ImGui::Text("Vertices: %zu", m_AnimatedMesh->GetVertices().size());
        ImGui::Text("Indices: %zu", m_AnimatedMesh->GetIndices().size());
    }
    if (m_Animation) {
        ImGui::Text("Duration: %.2f", m_Animation->GetDuration());
        ImGui::Text("Ticks/Sec: %.2f", m_Animation->GetTicksPerSecond());
    }
}

} // namespace MyEngine
