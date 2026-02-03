/******************************************************************************
 * File: Renderer.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: High-level rendering system
 ******************************************************************************/

#pragma once

#include "Math/MathTypes.h"
#include "Rendering/VertexArray.h"
#include "Rendering/Shader.h"
#include "Rendering/Camera.h"
#include "Rendering/RenderBackend.h"
#include "ECS/Registry.h"

namespace MyEngine {

class Mesh;
class Material;

/**
 * @brief Static renderer class for submitting draw calls
 */
class Renderer {
public:
    static void Init();
    static void Shutdown();
    
    static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    static void SetClearColor(const Vec4& color);
    static void Clear();
    
    static void BeginScene(const Camera& camera, const Mat4& viewMatrix);
    static void BeginScene(const Mat4& viewMatrix, const Mat4& projectionMatrix);
    static void EndScene();
    
    static void Submit(Shader* shader, 
                      VertexArray* vertexArray, 
                      const Mat4& transform = Mat4());
    
    static void DrawMesh(const Mesh& mesh, 
                        const Mat4& transform);
    
    static RenderBackend* GetBackend() { return s_RenderBackend.get(); }
    
private:
    struct SceneData {
        Mat4 ViewProjectionMatrix;
    };
    
    static SceneData* s_SceneData;
    static std::unique_ptr<RenderBackend> s_RenderBackend;
};

/**
 * @brief System that iterates over ECS and submits to Renderer
 */
class RenderSystem {
public:
    static void Render(Registry& registry, const Camera& camera, const Mat4& viewMatrix);
};

} // namespace MyEngine
