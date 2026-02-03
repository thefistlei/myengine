/******************************************************************************
 * File: TestSkeletalAnimation.cpp
 * Description: Test skeletal animation with vampire model
 ******************************************************************************/

#include "Core/Log.h"
#include "Animation/AnimatedModelLoader.h"
#include "Animation/Animator.h"
#include "Rendering/OpenGL/OpenGLShader.h"
#include "Platform/Window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <cmath>
#include <fstream>
#include <sstream>

using namespace MyEngine;

int main() {
    // Initialize logging
    Log::Init();
    ENGINE_INFO("=== Skeletal Animation Test ===");
    
    // Create window
    WindowProps props;
    props.Title = "Skeletal Animation Test";
    props.Width = 800;
    props.Height = 600;
    
    auto window = Window::Create(props);
    if (!window) {
        ENGINE_ERROR("Failed to create window!");
        return -1;
    }
    
    // Load animated model
    auto modelLoader = std::make_unique<AnimatedModelLoader>();
    
    std::string modelPath = "E:/lei/ai-project/qcoder/t2/docs/refer/vampire/dancing_vampire.dae";
    ENGINE_INFO("Loading model: {}", modelPath);
    
    auto animatedMesh = modelLoader->LoadMesh(modelPath);
    if (!animatedMesh) {
        ENGINE_ERROR("Failed to load animated mesh!");
        return -1;
    }
    
    // Load animation
    auto animation = modelLoader->LoadAnimation(modelPath);
    if (!animation) {
        ENGINE_ERROR("Failed to load animation!");
        return -1;
    }
    
    // Create animator
    auto animator = std::make_unique<Animator>(animation.get());
    
    // Load shader (read shader source files)
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
            ENGINE_ERROR("Failed to read vertex shader: {}", vsPath);
            return -1;
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
            ENGINE_ERROR("Failed to read fragment shader: {}", fsPath);
            return -1;
        }
    }
    
    auto shader = std::make_shared<OpenGLShader>("SkeletalAnimation", vsSource, fsSource);
    
    // Create a default white texture (1x1 pixel)
    GLuint whiteTexture;
    glGenTextures(1, &whiteTexture);
    glBindTexture(GL_TEXTURE_2D, whiteTexture);
    unsigned char whitePixel[4] = { 255, 255, 255, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    ENGINE_INFO("Initialization complete!");
    
    // Main loop
    float lastTime = 0.0f;
    
    while (!window->ShouldClose()) {
        // Calculate delta time
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        // Update animation
        animator->UpdateAnimation(deltaTime);
        
        // Clear screen
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        
        // Render animated model
        shader->Bind();
        
        // Bind white texture to texture unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, whiteTexture);
        shader->SetInt("texture_diffuse1", 0);
        
        // Setup matrices
        Mat4 projection = Mat4::Perspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);
        
        // Simple view matrix (camera at (0,0,3) looking at origin)
        Vec3 eye(0, 0, 3);
        Vec3 center(0, 0, 0);
        Vec3 up(0, 1, 0);
        
        Vec3 f = (center - eye).Normalized();
        Vec3 s = Vec3::Cross(f, up).Normalized();
        Vec3 u = Vec3::Cross(s, f);
        
        Mat4 view;
        view.m[0] = s.x; view.m[4] = s.y; view.m[8] = s.z;   view.m[12] = -Vec3::Dot(s, eye);
        view.m[1] = u.x; view.m[5] = u.y; view.m[9] = u.z;   view.m[13] = -Vec3::Dot(u, eye);
        view.m[2] = -f.x; view.m[6] = -f.y; view.m[10] = -f.z; view.m[14] = Vec3::Dot(f, eye);
        view.m[3] = 0;   view.m[7] = 0;   view.m[11] = 0;    view.m[15] = 1;
        
        Mat4 model = Mat4::Translation(Vec3(0, -0.4f, 0)) * Mat4::Scale(Vec3(0.5f, 0.5f, 0.5f));
        
        shader->SetMat4("projection", projection);
        shader->SetMat4("view", view);
        shader->SetMat4("model", model);
        
        // Upload bone matrices
        auto transforms = animator->GetFinalBoneMatrices();
        for (size_t i = 0; i < transforms.size(); ++i) {
            std::string uniformName = "finalBonesMatrices[" + std::to_string(i) + "]";
            shader->SetMat4(uniformName, transforms[i]);
        }
        
        // Draw mesh
        auto vao = animatedMesh->GetVertexArray();
        vao->Bind();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(animatedMesh->GetIndices().size()), 
                      GL_UNSIGNED_INT, nullptr);
        vao->Unbind();
        
        shader->Unbind();
        
        // Swap buffers and poll events
        window->OnUpdate();
    }
    
    ENGINE_INFO("Shutting down Skeletal Animation Test");
    return 0;
}
