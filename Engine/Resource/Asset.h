/******************************************************************************
 * File: Asset.h
 * Author: AI Assistant
 * Created: 2026-02-02
 * Description: Base asset class for all loadable resources
 *              Based on asset_browser_architecture.md
 ******************************************************************************/

#pragma once

#include "AssetTypes.h"
#include <memory>
#include <atomic>
#include <functional>

namespace MyEngine {

/**
 * @brief Base class for all assets
 */
class Asset : public std::enable_shared_from_this<Asset> {
public:
    virtual ~Asset() = default;
    
    // Core interface - must be implemented by derived classes
    virtual AssetType GetType() const = 0;
    virtual bool Load() = 0;
    virtual bool Unload() = 0;
    
    // Optional overrides
    virtual bool Reload();
    
    // Metadata access
    const std::string& GetName() const { return m_Metadata.name; }
    const std::string& GetGUID() const { return m_Metadata.guid; }
    const std::string& GetPath() const { return m_Metadata.path; }
    const AssetMetadata& GetMetadata() const { return m_Metadata; }
    void SetMetadata(const AssetMetadata& metadata) { m_Metadata = metadata; }
    
    // Load state
    AssetLoadState GetLoadState() const { return m_LoadState; }
    bool IsLoaded() const { return m_LoadState == AssetLoadState::Loaded; }
    
    // Reference counting
    void AddReference() { ++m_ReferenceCount; }
    void RemoveReference() { --m_ReferenceCount; }
    int32_t GetReferenceCount() const { return m_ReferenceCount.load(); }
    
protected:
    Asset() : m_LoadState(AssetLoadState::Unloaded), m_ReferenceCount(0) {}
    
    void SetLoadState(AssetLoadState state) { m_LoadState = state; }
    
protected:
    AssetMetadata m_Metadata;
    AssetLoadState m_LoadState;
    std::atomic<int32_t> m_ReferenceCount;
};

} // namespace MyEngine
