/******************************************************************************
 * File: AssetDatabase.cpp
 * Author: AI Assistant
 * Created: 2026-02-02
 * Description: AssetDatabase implementation
 ******************************************************************************/

#include "AssetDatabase.h"
#include "Core/Log.h"
#include "Core/UUID.h"
#include "Platform/FileSystem.h"
#include <algorithm>
#include <filesystem>
#include <sstream>

namespace MyEngine {

AssetDatabase& AssetDatabase::GetInstance() {
    static AssetDatabase instance;
    return instance;
}

std::shared_ptr<Asset> AssetDatabase::LoadAsset(const std::string& guid) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    // Check cache first
    auto cacheIt = m_AssetCache.find(guid);
    if (cacheIt != m_AssetCache.end()) {
        if (auto asset = cacheIt->second.lock()) {
            return asset;
        }
    }
    
    // Load from disk
    return LoadAssetInternal(guid);
}

std::shared_ptr<Asset> AssetDatabase::LoadAssetAtPath(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_PathToGUID.find(path);
    if (it != m_PathToGUID.end()) {
        return LoadAsset(it->second);
    }
    
    ENGINE_WARN("Asset not found at path: {}", path);
    return nullptr;
}

bool AssetDatabase::RegisterAsset(const std::string& path, const AssetMetadata& metadata) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    m_AssetRegistry[metadata.guid] = metadata;
    m_PathToGUID[path] = metadata.guid;
    
    ENGINE_INFO("Registered asset: {} ({})", metadata.name, metadata.guid);
    return true;
}

bool AssetDatabase::UnregisterAsset(const std::string& guid) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_AssetRegistry.find(guid);
    if (it != m_AssetRegistry.end()) {
        m_PathToGUID.erase(it->second.path);
        m_AssetCache.erase(guid);
        m_AssetRegistry.erase(it);
        return true;
    }
    
    return false;
}

bool AssetDatabase::RefreshAsset(const std::string& guid) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto cacheIt = m_AssetCache.find(guid);
    if (cacheIt != m_AssetCache.end()) {
        if (auto asset = cacheIt->second.lock()) {
            return asset->Reload();
        }
    }
    
    return false;
}

std::vector<AssetMetadata> AssetDatabase::FindAssets(const std::string& searchPattern) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    std::vector<AssetMetadata> results;
    std::string lowerPattern = searchPattern;
    std::transform(lowerPattern.begin(), lowerPattern.end(), lowerPattern.begin(), ::tolower);
    
    for (const auto& [guid, metadata] : m_AssetRegistry) {
        std::string lowerName = metadata.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        
        if (lowerName.find(lowerPattern) != std::string::npos) {
            results.push_back(metadata);
        }
    }
    
    return results;
}

std::vector<AssetMetadata> AssetDatabase::FindAssetsByType(AssetType type) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    std::vector<AssetMetadata> results;
    for (const auto& [guid, metadata] : m_AssetRegistry) {
        if (metadata.type == type) {
            results.push_back(metadata);
        }
    }
    
    return results;
}

std::vector<AssetMetadata> AssetDatabase::FindAssetsInFolder(const std::string& folderPath) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    std::vector<AssetMetadata> results;
    for (const auto& [guid, metadata] : m_AssetRegistry) {
        if (metadata.path.find(folderPath) == 0) {
            results.push_back(metadata);
        }
    }
    
    return results;
}

std::vector<AssetMetadata> AssetDatabase::GetAllAssets() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    std::vector<AssetMetadata> results;
    results.reserve(m_AssetRegistry.size());
    
    for (const auto& [guid, metadata] : m_AssetRegistry) {
        results.push_back(metadata);
    }
    
    return results;
}

const AssetMetadata* AssetDatabase::GetMetadata(const std::string& guid) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_AssetRegistry.find(guid);
    if (it != m_AssetRegistry.end()) {
        return &it->second;
    }
    
    return nullptr;
}

std::string AssetDatabase::GetGUIDFromPath(const std::string& path) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_PathToGUID.find(path);
    if (it != m_PathToGUID.end()) {
        return it->second;
    }
    
    return "";
}

void AssetDatabase::UnloadUnusedAssets() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    size_t unloadedCount = 0;
    for (auto it = m_AssetCache.begin(); it != m_AssetCache.end();) {
        if (it->second.expired()) {
            it = m_AssetCache.erase(it);
            ++unloadedCount;
        } else {
            ++it;
        }
    }
    
    if (unloadedCount > 0) {
        ENGINE_INFO("Unloaded {} unused assets", unloadedCount);
    }
}

void AssetDatabase::ClearCache() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_AssetCache.clear();
    ENGINE_INFO("Asset cache cleared");
}

size_t AssetDatabase::GetCachedAssetCount() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    size_t count = 0;
    for (const auto& [guid, weakAsset] : m_AssetCache) {
        if (!weakAsset.expired()) {
            ++count;
        }
    }
    
    return count;
}

void AssetDatabase::ScanDirectory(const std::string& directoryPath) {
    ENGINE_INFO("Scanning directory for assets: {}", directoryPath);
    
    if (!std::filesystem::exists(directoryPath)) {
        ENGINE_WARN("Directory does not exist: {}", directoryPath);
        return;
    }
    
    int assetCount = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath)) {
        if (entry.is_regular_file()) {
            std::string path = entry.path().string();
            std::string extension = entry.path().extension().string();
            std::string filename = entry.path().filename().string();
            
            // Skip meta files and hidden files
            if (extension == ".meta" || filename[0] == '.') {
                continue;
            }
            
            // Check if already registered
            if (m_PathToGUID.find(path) != m_PathToGUID.end()) {
                continue;
            }
            
            // Create metadata
            AssetMetadata metadata;
            metadata.guid = GenerateGUID();
            metadata.name = entry.path().stem().string();
            metadata.path = path;
            metadata.extension = extension;
            metadata.type = DetermineAssetType(extension);
            metadata.fileSize = entry.file_size();
            
            auto ftime = std::filesystem::last_write_time(entry);
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
            );
            metadata.modifiedTime = std::chrono::system_clock::to_time_t(sctp);
            
            RegisterAsset(path, metadata);
            ++assetCount;
        }
    }
    
    ENGINE_INFO("Found {} assets in {}", assetCount, directoryPath);
}

void AssetDatabase::Refresh() {
    ENGINE_INFO("Refreshing asset database");
    ScanDirectory("assets");
    ScanDirectory("Runtime\\Shaders");
    ScanDirectory("docs\\refer");  // Reference assets (vampire model, etc.)
    ScanDirectory("ThirdParty\\stb");  // STB image library headers
}

std::shared_ptr<Asset> AssetDatabase::LoadAssetInternal(const std::string& guid) {
    // For now, return nullptr - actual loading will be implemented with importers
    ENGINE_WARN("Asset loading not yet implemented for GUID: {}", guid);
    return nullptr;
}

void AssetDatabase::CacheAsset(const std::string& guid, std::shared_ptr<Asset> asset) {
    m_AssetCache[guid] = asset;
}

std::string AssetDatabase::GenerateGUID() const {
    UUID uuid;
    std::stringstream ss;
    ss << std::hex << uuid.Get();
    return ss.str();
}

AssetType AssetDatabase::DetermineAssetType(const std::string& extension) const {
    if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || 
        extension == ".bmp" || extension == ".tga" || extension == ".hdr") {
        return AssetType::Texture;
    }
    else if (extension == ".obj" || extension == ".fbx" || extension == ".gltf" || 
             extension == ".glb" || extension == ".dae") {
        return AssetType::Model;
    }
    else if (extension == ".vs" || extension == ".fs" || extension == ".glsl" || 
             extension == ".vert" || extension == ".frag") {
        return AssetType::Shader;
    }
    else if (extension == ".wav" || extension == ".mp3" || extension == ".ogg") {
        return AssetType::Audio;
    }
    else if (extension == ".ttf" || extension == ".otf") {
        return AssetType::Font;
    }
    
    return AssetType::Unknown;
}

} // namespace MyEngine
