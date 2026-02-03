/******************************************************************************
 * File: RenderPass.h
 * Author: AI Assistant
 * Created: 2026-01-31
 * Description: Lightweight Pass base classes for modular rendering
 ******************************************************************************/

#pragma once

#include "Math/MathTypes.h"
#include "Rendering/Camera.h"
#include "ECS/Registry.h"
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

namespace MyEngine {

// Forward declarations
class RenderBackend;
class Shader;

// ============================================================================
// Pass Type and Category
// ============================================================================

enum class PassType {
    Graphics,       // Graphics rendering pass
    Compute,        // Compute pass
    Transfer,       // Data transfer pass
    Present         // Present pass
};

enum class PassCategory {
    // Geometry stage
    Shadow,         // Shadow mapping
    Depth,          // Depth pre-pass
    GBuffer,        // Geometry buffer
    
    // Lighting stage
    Lighting,       // Lighting
    Reflection,     // Reflection
    GI,             // Global illumination
    
    // Effects stage
    Sky,            // Sky rendering
    Terrain,        // Terrain
    Water,          // Water
    Vegetation,     // Vegetation
    Particles,      // Particles
    VFX,            // Visual effects
    
    // Post-processing stage
    PostProcess,    // Post-processing
    UI,             // UI rendering
    Debug           // Debug visualization
};

// ============================================================================
// Scene View - Camera and viewport info
// ============================================================================

struct SceneView {
    Mat4 viewMatrix;
    Mat4 projectionMatrix;
    Mat4 viewProjectionMatrix;
    Vec3 cameraPosition;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    
    SceneView() = default;
    
    SceneView(const Mat4& view, const Mat4& proj, const Vec3& camPos)
        : viewMatrix(view)
        , projectionMatrix(proj)
        , viewProjectionMatrix(proj * view)
        , cameraPosition(camPos)
    {}
    
    Mat4 GetViewMatrix() const { return viewMatrix; }
    Mat4 GetProjectionMatrix() const { return projectionMatrix; }
    Mat4 GetViewProjectionMatrix() const { return viewProjectionMatrix; }
    Vec3 GetPosition() const { return cameraPosition; }
    float GetNearPlane() const { return nearPlane; }
    float GetFarPlane() const { return farPlane; }
};

// ============================================================================
// RenderPass Base Class
// ============================================================================

class RenderPass {
public:
    virtual ~RenderPass() = default;
    
    // ========== Meta information ==========
    virtual const char* GetName() const = 0;
    virtual PassType GetType() const = 0;
    virtual PassCategory GetCategory() const = 0;
    
    // ========== Lifecycle ==========
    virtual void OnCreate(RenderBackend* backend) {}
    virtual void OnDestroy() {}
    virtual void OnResize(uint32_t width, uint32_t height) {}
    
    // ========== Execution ==========
    virtual void Execute(const SceneView& view, Registry* registry) = 0;
    
    // ========== GUI configuration (Editor) ==========
    virtual void OnGUI() {}
    
    // ========== Enable/Disable ==========
    void SetEnabled(bool enabled) { m_Enabled = enabled; }
    bool IsEnabled() const { return m_Enabled; }
    
    void SetPriority(int priority) { m_Priority = priority; }
    int GetPriority() const { return m_Priority; }
    
protected:
    RenderBackend* m_Backend = nullptr;
    bool m_Enabled = true;
    int m_Priority = 0;  // Execution priority (lower executes first)
};

// ============================================================================
// GraphicsPass - For rendering passes
// ============================================================================

class GraphicsPass : public RenderPass {
public:
    PassType GetType() const override { return PassType::Graphics; }
    
protected:
    // Helper for clearing
    void ClearColor(const Vec4& color);
    void ClearDepth(float depth = 1.0f);
};

// ============================================================================
// ComputePass - For compute passes
// ============================================================================

class ComputePass : public RenderPass {
public:
    PassType GetType() const override { return PassType::Compute; }
    
protected:
    struct WorkgroupSize {
        uint32_t x = 8;
        uint32_t y = 8;
        uint32_t z = 1;
    };
    
    WorkgroupSize m_WorkgroupSize;
    
    // Calculate dispatch size for given data dimensions
    struct DispatchSize {
        uint32_t x, y, z;
    };
    
    DispatchSize CalculateDispatchSize(uint32_t dataX, uint32_t dataY, uint32_t dataZ = 1) const {
        return {
            (dataX + m_WorkgroupSize.x - 1) / m_WorkgroupSize.x,
            (dataY + m_WorkgroupSize.y - 1) / m_WorkgroupSize.y,
            (dataZ + m_WorkgroupSize.z - 1) / m_WorkgroupSize.z
        };
    }
};

// ============================================================================
// Pass Registry - Factory for creating passes
// ============================================================================

class PassRegistry {
public:
    using PassFactory = std::function<std::unique_ptr<RenderPass>()>;
    
    static PassRegistry& Get() {
        static PassRegistry instance;
        return instance;
    }
    
    void Register(const char* name, PassFactory factory) {
        m_Factories[name] = std::move(factory);
    }
    
    std::unique_ptr<RenderPass> Create(const char* name) {
        auto it = m_Factories.find(name);
        if (it != m_Factories.end()) {
            return it->second();
        }
        return nullptr;
    }
    
    std::vector<std::string> GetRegisteredPasses() const {
        std::vector<std::string> names;
        for (const auto& [name, _] : m_Factories) {
            names.push_back(name);
        }
        return names;
    }
    
private:
    PassRegistry() = default;
    std::unordered_map<std::string, PassFactory> m_Factories;
};

// ============================================================================
// Auto-registration macro
// ============================================================================

#define REGISTER_PASS(ClassName) \
    namespace { \
        struct ClassName##Registrar { \
            ClassName##Registrar() { \
                PassRegistry::Get().Register(#ClassName, []() { \
                    return std::make_unique<ClassName>(); \
                }); \
            } \
        }; \
        static ClassName##Registrar g_##ClassName##Registrar; \
    }

// ============================================================================
// PassManager - Manages and executes passes
// ============================================================================

class PassManager {
public:
    void AddPass(std::unique_ptr<RenderPass> pass);
    void RemovePass(const char* name);
    
    RenderPass* GetPass(const char* name);
    
    void OnCreate(RenderBackend* backend);
    void OnDestroy();
    void OnResize(uint32_t width, uint32_t height);
    
    void Execute(const SceneView& view, Registry* registry);
    
    void SetPassEnabled(const char* name, bool enabled);
    
    std::vector<RenderPass*> GetAllPasses();
    
private:
    std::vector<std::unique_ptr<RenderPass>> m_Passes;
    
    void SortPassesByPriority();
};

} // namespace MyEngine
