/******************************************************************************
 * File: Components.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Common ECS components
 ******************************************************************************/

#pragma once

#include <string>
#include "Math/Transform.h"
#include "Math/MathTypes.h"
#include "Resource/ResourceHandle.h"
#include "Registry.h"

// Forward declarations
namespace MyEngine {
class RenderPass;
}

namespace MyEngine {

class Mesh;
class Material;

/**
 * @brief Tag component for naming entities
 */
struct TagComponent {
    std::string Tag;

    TagComponent() = default;
    TagComponent(const std::string& tag) : Tag(tag) {}
};

/**
 * @brief Hierarchy component for parent-child relationships
 * This is a flattened tree structure representation for cache-friendly traversal
 */
struct HierarchyComponent {
    EntityID parent = 0;           // Parent entity (0 = root)
    EntityID firstChild = 0;       // First child in linked list
    EntityID nextSibling = 0;      // Next sibling in linked list
    uint32_t depth = 0;            // Depth in hierarchy (for sorting)
    
    HierarchyComponent() = default;
    HierarchyComponent(EntityID parentID) : parent(parentID) {}
    
    bool IsRoot() const { return parent == 0; }
    bool HasChildren() const { return firstChild != 0; }
    bool HasSiblings() const { return nextSibling != 0; }
};

/**
 * @brief Enhanced Transform component with version-based update tracking
 * Implements industrial-grade lazy evaluation system
 */
struct TransformComponent {
    // Local space transform (relative to parent)
    Vec3 localPosition = Vec3(0, 0, 0);
    Quat localRotation = Quat::Identity();
    Vec3 localScale = Vec3(1, 1, 1);
    
    // Cached matrices
    Mat4 localMatrix;              // Local transformation matrix
    Mat4 worldMatrix;              // World transformation matrix
    
    // Version tracking for lazy updates
    uint32_t localVersion = 0;     // Incremented when local transform changes
    uint32_t worldVersion = 0;     // Incremented when world matrix updates
    uint32_t parentVersion = 0;    // Last parent worldVersion we saw
    
    TransformComponent() = default;
    TransformComponent(const Vec3& position) : localPosition(position) {
        UpdateLocalMatrix();
    }
    
    // Mark local transform as dirty
    void MarkDirty() {
        localVersion++;
    }
    
    // Update local matrix from position/rotation/scale
    void UpdateLocalMatrix() {
        localMatrix = Mat4::Translation(localPosition) * Mat4::Scale(localScale);
        // TODO: Add rotation when Mat4::FromQuaternion is implemented
        MarkDirty();
    }
    
    // Check if world matrix needs update
    bool NeedsUpdate(uint32_t parentWorldVersion) const {
        return (parentVersion != parentWorldVersion) || (worldVersion != localVersion);
    }
};

/**
 * @brief Mesh renderer component
 */
struct MeshRendererComponent {
    Handle<Material> MaterialHandle;
    bool CastShadows = true;
    bool ReceiveShadows = true;
};

/**
 * @brief Mesh filter component (stores the mesh geometry)
 */
struct MeshFilterComponent {
    Handle<Mesh> MeshHandle;
    
    // Temporary: direct mesh pointer for editor (until Handle system is fully integrated)
    std::shared_ptr<Mesh> mesh;
};

/**
 * @brief Pass component linking an entity to a rendering pass
 * Allows editing pass properties through the entity's properties panel
 */
struct PassComponent {
    RenderPass* pass = nullptr;  // Non-owning pointer to the pass
    std::string passType;        // Type name for identification (e.g., "WaterPass")
    
    PassComponent() = default;
    PassComponent(RenderPass* p, const std::string& type) 
        : pass(p), passType(type) {}
    
    bool IsValid() const { return pass != nullptr; }
};

// Forward declaration for ScriptComponent (defined in Scripting module)
struct ScriptComponent;

} // namespace MyEngine
