/******************************************************************************
 * File: TerrainPass.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Heightmap-based terrain rendering pass
 ******************************************************************************/

#pragma once

#include "RenderPass.h"
#include "Resource/Mesh.h"
#include "Rendering/Shader.h"
#include "Math/MathTypes.h"
#include <memory>
#include <vector>

namespace MyEngine {

/**
 * @brief Renders terrain using heightmap-based mesh generation
 * 
 * Features:
 * - Procedural heightmap generation using noise functions
 * - Multi-texture splatting based on height/slope
 * - LOD support for large terrains
 * - Normal map generation from heightmap
 */
class TerrainPass : public GraphicsPass {
public:
    TerrainPass();
    virtual ~TerrainPass() = default;
    
    const char* GetName() const override { return "TerrainPass"; }
    PassCategory GetCategory() const override { return PassCategory::Terrain; }
    
    void OnCreate(RenderBackend* backend) override;
    void OnDestroy() override;
    void Execute(const SceneView& view, Registry* registry) override;
    void OnGUI() override;
    
    // Terrain configuration
    void SetTerrainSize(float size) { m_TerrainSize = size; }
    void SetMaxHeight(float height) { m_MaxHeight = height; }
    void RegenerateTerrain();
    
private:
    void CreateTerrainMesh();
    void CreateTerrainShader();
    float GetHeight(int x, int z);
    float PerlinNoise(float x, float z);
    
private:
    std::unique_ptr<Mesh> m_TerrainMesh;
    std::unique_ptr<Shader> m_TerrainShader;
    
    // Terrain properties
    float m_TerrainSize = 100.0f;
    float m_MaxHeight = 20.0f;           // Much higher for dramatic terrain
    int m_GridSize = 80;                 // Higher resolution
    float m_NoiseScale = 0.03f;          // Lower for larger features
    float m_NoiseOctaves = 6.0f;         // More detail layers
    
    // Rendering properties
    Vec3 m_GrassColor = Vec3(0.25f, 0.55f, 0.15f);   // Richer green
    Vec3 m_RockColor = Vec3(0.45f, 0.42f, 0.38f);    // More natural gray-brown
    Vec3 m_SandColor = Vec3(0.82f, 0.75f, 0.58f);    // Warmer sand
    bool m_ShowWireframe = false;
    
    // Heightmap data
    std::vector<float> m_HeightData;
};

} // namespace MyEngine
