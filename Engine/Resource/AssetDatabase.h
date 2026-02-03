/******************************************************************************
 * File: AssetDatabase.h
 * Author: AI Assistant
 * Created: 2026-02-02
 * Description: Asset database for indexing and caching assets
 *              Simplified version based on asset_browser_architecture.md
 ******************************************************************************/

#pragma once

#include "Asset.h"
#include "AssetTypes.h"
#include <unordered_map>
#include <vector>
#include <mutex>
#include <functional>

namespace MyEngine {

/**
 * @brief Asset database - manages asset indexing and caching
 */
class AssetDatabase {
public:
    static AssetDatabase& GetInstance();
    
    // Asset loading
    std::shared_ptr<Asset> LoadAsset(const std::string& guid);
    std::shared_ptr<Asset> LoadAssetAtPath(const std::string& path);
    
    // Asset registration
    bool RegisterAsset(const std::string& path, const AssetMetadata& metadata);
    bool UnregisterAsset(const std::string& guid);
    bool RefreshAsset(const std::string& guid);
    
    // Asset search
    std::vector<AssetMetadata> FindAssets(const std::string& searchPattern);
    std::vector<AssetMetadata> FindAssetsByType(AssetType type);
    std::vector<AssetMetadata> FindAssetsInFolder(const std::string& folderPath);
    std::vector<AssetMetadata> GetAllAssets() const;
    
    // Metadata access
    const AssetMetadata* GetMetadata(const std::string& guid) const;
    std::string GetGUIDFromPath(const std::string& path) const;
    
    // Cache management
    void UnloadUnusedAssets();
    void ClearCache();
    size_t GetCachedAssetCount() const;
    
    // Directory scanning
    void ScanDirectory(const std::string& directoryPath);
    void Refresh();
    
private:
    AssetDatabase() = default;
    ~AssetDatabase() = default;
    
    // Delete copy/move constructors
    AssetDatabase(const AssetDatabase&) = delete;
    AssetDatabase& operator=(const AssetDatabase&) = delete;
    
    // Internal helpers
    std::shared_ptr<Asset> LoadAssetInternal(const std::string& guid);
    void CacheAsset(const std::string& guid, std::shared_ptr<Asset> asset);
    std::string GenerateGUID() const;
    AssetType DetermineAssetType(const std::string& extension) const;
    
private:
    std::unordered_map<std::string, AssetMetadata> m_AssetRegistry;  // GUID -> Metadata
    std::unordered_map<std::string, std::string> m_PathToGUID;       // Path -> GUID
    std::unordered_map<std::string, std::weak_ptr<Asset>> m_AssetCache;  // GUID -> Asset
    
    mutable std::mutex m_Mutex;
};

} // namespace MyEngine
