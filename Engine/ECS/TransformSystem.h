/******************************************************************************
 * File: TransformSystem.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Transform system with hierarchy-ordered batch processing
 *              Implements industrial-grade linear traversal instead of recursion
 ******************************************************************************/

#pragma once

#include "Registry.h"
#include "Components.h"
#include "Core/Log.h"
#include <vector>
#include <algorithm>

namespace MyEngine {

/**
 * @brief High-performance transform system with linear hierarchy traversal
 * 
 * Key features:
 * - Flattened tree structure (no recursion)
 * - Topological sorting for correct parent-before-child update order
 * - Version-based lazy evaluation (only update when needed)
 * - Cache-friendly linear iteration
 * - SIMD-friendly (future optimization)
 */
class TransformSystem {
public:
    TransformSystem(Registry* registry) : m_Registry(registry) {}
    
    /**
     * @brief Rebuild hierarchy-ordered entity list
     * Call this when hierarchy changes (parent/child relationships modified)
     */
    void RebuildHierarchyOrder() {
        m_HierarchyOrdered.clear();
        
        // Get all entities with HierarchyComponent
        auto hierarchyEntities = m_Registry->GetEntitiesWith<HierarchyComponent>();
        
        // Sort by depth (topological order: parents before children)
        std::sort(hierarchyEntities.begin(), hierarchyEntities.end(),
            [this](EntityID a, EntityID b) {
                auto& ha = m_Registry->GetComponent<HierarchyComponent>(a);
                auto& hb = m_Registry->GetComponent<HierarchyComponent>(b);
                return ha.depth < hb.depth;
            });
        
        m_HierarchyOrdered = std::move(hierarchyEntities);
        
        ENGINE_INFO("TransformSystem: Rebuilt hierarchy order ({} entities)", m_HierarchyOrdered.size());
    }
    
    /**
     * @brief Update all transforms in hierarchy order
     * Uses version-based lazy evaluation - only updates when needed
     */
    void Update() {
        if (m_HierarchyOrdered.empty()) {
            return;
        }
        
        uint32_t updatedCount = 0;
        
        // Linear traversal (no recursion!)
        for (EntityID entityID : m_HierarchyOrdered) {
            auto& hierarchy = m_Registry->GetComponent<HierarchyComponent>(entityID);
            auto& transform = m_Registry->GetComponent<TransformComponent>(entityID);
            
            // Root entity (no parent)
            if (hierarchy.IsRoot()) {
                // Check if local transform changed
                if (transform.worldVersion != transform.localVersion) {
                    transform.worldMatrix = transform.localMatrix;
                    transform.worldVersion++;
                    updatedCount++;
                }
            }
            // Child entity
            else {
                auto& parentTransform = m_Registry->GetComponent<TransformComponent>(hierarchy.parent);
                
                // Only update if needed (version-based check)
                if (transform.NeedsUpdate(parentTransform.worldVersion)) {
                    transform.worldMatrix = parentTransform.worldMatrix * transform.localMatrix;
                    transform.parentVersion = parentTransform.worldVersion;
                    transform.worldVersion++;
                    updatedCount++;
                }
            }
        }
        
        // Debug logging (can be removed in production)
        if (updatedCount > 0) {
            ENGINE_TRACE("TransformSystem: Updated {} / {} transforms", updatedCount, m_HierarchyOrdered.size());
        }
    }
    
    /**
     * @brief Add entity to hierarchy with optional parent
     */
    void AddToHierarchy(EntityID entityID, EntityID parentID = 0) {
        auto& hierarchy = m_Registry->GetComponent<HierarchyComponent>(entityID);
        hierarchy.parent = parentID;
        
        if (parentID != 0) {
            // Update parent's child list
            auto& parentHierarchy = m_Registry->GetComponent<HierarchyComponent>(parentID);
            
            // Add as first child (or sibling if parent already has children)
            if (parentHierarchy.firstChild == 0) {
                parentHierarchy.firstChild = entityID;
            } else {
                // Find last sibling
                EntityID lastSibling = parentHierarchy.firstChild;
                while (true) {
                    auto& siblingHierarchy = m_Registry->GetComponent<HierarchyComponent>(lastSibling);
                    if (siblingHierarchy.nextSibling == 0) {
                        siblingHierarchy.nextSibling = entityID;
                        break;
                    }
                    lastSibling = siblingHierarchy.nextSibling;
                }
            }
            
            // Calculate depth
            auto& parentHierarchyRef = m_Registry->GetComponent<HierarchyComponent>(parentID);
            hierarchy.depth = parentHierarchyRef.depth + 1;
        } else {
            hierarchy.depth = 0; // Root entity
        }
        
        // Rebuild hierarchy order
        RebuildHierarchyOrder();
    }
    
    /**
     * @brief Get world matrix for entity
     */
    const Mat4& GetWorldMatrix(EntityID entityID) const {
        return m_Registry->GetComponent<TransformComponent>(entityID).worldMatrix;
    }
    
    /**
     * @brief Set local position (marks transform as dirty)
     */
    void SetLocalPosition(EntityID entityID, const Vec3& position) {
        auto& transform = m_Registry->GetComponent<TransformComponent>(entityID);
        transform.localPosition = position;
        transform.UpdateLocalMatrix();
    }
    
    /**
     * @brief Set local scale (marks transform as dirty)
     */
    void SetLocalScale(EntityID entityID, const Vec3& scale) {
        auto& transform = m_Registry->GetComponent<TransformComponent>(entityID);
        transform.localScale = scale;
        transform.UpdateLocalMatrix();
    }
    
    /**
     * @brief Get hierarchy-ordered entity list (for debugging)
     */
    const std::vector<EntityID>& GetHierarchyOrdered() const {
        return m_HierarchyOrdered;
    }
    
private:
    Registry* m_Registry;
    
    // Hierarchy-ordered entity list (topologically sorted)
    // Parents always appear before their children
    std::vector<EntityID> m_HierarchyOrdered;
};

} // namespace MyEngine
