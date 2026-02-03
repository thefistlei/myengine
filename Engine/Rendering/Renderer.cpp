/******************************************************************************
 * File: Renderer.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: High-level rendering system implementation
 ******************************************************************************/

#include "Renderer.h"
#include "Resource/Mesh.h"
#include "ECS/Components.h"

namespace MyEngine {

Renderer::SceneData* Renderer::s_SceneData = nullptr;
std::unique_ptr<RenderBackend> Renderer::s_RenderBackend = nullptr;

void Renderer::Init() {
    if (!s_SceneData) s_SceneData = new Renderer::SceneData();
    if (!s_RenderBackend) s_RenderBackend = RenderBackend::Create();
    
    if (s_RenderBackend) {
        s_RenderBackend->Init();
    }
}

void Renderer::Shutdown() {
    if (s_RenderBackend) {
        s_RenderBackend->Shutdown();
        s_RenderBackend.reset();
    }
    
    if (s_SceneData) {
        delete s_SceneData;
        s_SceneData = nullptr;
    }
}

void Renderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    if (s_RenderBackend) s_RenderBackend->SetViewport(x, y, width, height);
}

void Renderer::SetClearColor(const Vec4& color) {
    if (s_RenderBackend) s_RenderBackend->SetClearColor(color);
}

void Renderer::Clear() {
    if (s_RenderBackend) s_RenderBackend->Clear();
}

void Renderer::BeginScene(const Camera& camera, const Mat4& viewMatrix) {
    if (!s_SceneData) return;
    s_SceneData->ViewProjectionMatrix = camera.GetProjection() * viewMatrix;
}

void Renderer::BeginScene(const Mat4& viewMatrix, const Mat4& projectionMatrix) {
    if (!s_SceneData) return;
    s_SceneData->ViewProjectionMatrix = projectionMatrix * viewMatrix;
}

void Renderer::EndScene() {
}

void Renderer::Submit(Shader* shader, VertexArray* vertexArray, const Mat4& transform) {
    if (!shader || !vertexArray || !s_RenderBackend) return;
    
    shader->Bind();
    shader->SetMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
    shader->SetMat4("u_Transform", transform);
    
    vertexArray->Bind();
    auto ib = vertexArray->GetIndexBuffer();
    if (ib) {
        s_RenderBackend->DrawIndexed(ib->GetCount());
    }
}

void Renderer::DrawMesh(const Mesh& mesh, const Mat4& transform) {
    if (!s_RenderBackend) return;
    
    auto va = mesh.GetVertexArray();
    if (!va) return;
    
    va->Bind();
    auto ib = va->GetIndexBuffer();
    if (ib) {
        s_RenderBackend->DrawIndexed(ib->GetCount());
    }
}

void RenderSystem::Render(Registry& registry, const Camera& camera, const Mat4& viewMatrix) {
    Renderer::BeginScene(camera, viewMatrix);
    
    // Iterate over all entities with MeshFilter and Transform
    auto view = registry.GetEntitiesWith<MeshFilterComponent, TransformComponent>();
    
    for (auto entityID : view) {
        // auto& meshFilter = registry.GetComponent<MeshFilterComponent>(entityID);
        auto& transformComp = registry.GetComponent<TransformComponent>(entityID);
        
        // In a real system we would use ResourceRegistry to get the mesh from the handle
        // and Material to get the shader.
        // For now, this is a placeholder for the logic.
        (void)transformComp; 
    }
    
    Renderer::EndScene();
}

} // namespace MyEngine
