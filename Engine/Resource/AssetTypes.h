/******************************************************************************
 * File: AssetTypes.h
 * Author: AI Assistant
 * Created: 2026-02-02
 * Description: Asset type definitions and metadata structures
 *              Based on asset_browser_architecture.md
 ******************************************************************************/

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <ctime>

namespace MyEngine {

// Forward declarations
class Asset;

/**
 * @brief Asset type enumeration
 */
enum class AssetType {
    Unknown = 0,
    Texture,
    Material,
    Mesh,
    Model,
    Prefab,
    Scene,
    Script,
    Shader,
    Audio,
    Animation,
    Font,
    Cubemap,
    RenderTexture,
    ComputeShader,
    PhysicsMaterial,
    TerrainData,
    Custom
};

/**
 * @brief Convert asset type to string
 */
inline const char* AssetTypeToString(AssetType type) {
    switch (type) {
        case AssetType::Texture: return "Texture";
        case AssetType::Material: return "Material";
        case AssetType::Mesh: return "Mesh";
        case AssetType::Model: return "Model";
        case AssetType::Prefab: return "Prefab";
        case AssetType::Scene: return "Scene";
        case AssetType::Script: return "Script";
        case AssetType::Shader: return "Shader";
        case AssetType::Audio: return "Audio";
        case AssetType::Animation: return "Animation";
        case AssetType::Font: return "Font";
        case AssetType::Cubemap: return "Cubemap";
        case AssetType::RenderTexture: return "RenderTexture";
        case AssetType::ComputeShader: return "ComputeShader";
        case AssetType::PhysicsMaterial: return "PhysicsMaterial";
        case AssetType::TerrainData: return "TerrainData";
        case AssetType::Custom: return "Custom";
        default: return "Unknown";
    }
}

/**
 * @brief Asset metadata structure
 */
struct AssetMetadata {
    std::string guid;                                           // Global unique identifier
    std::string name;                                           // Asset name
    std::string path;                                           // File path (relative to project)
    std::string extension;                                      // File extension
    AssetType type = AssetType::Unknown;                       // Asset type
    size_t fileSize = 0;                                       // File size in bytes
    time_t createdTime = 0;                                    // Creation time
    time_t modifiedTime = 0;                                   // Last modification time
    std::string importerType;                                  // Importer type name
    std::unordered_map<std::string, std::string> customProperties;  // Custom properties
    
    AssetMetadata() = default;
    AssetMetadata(const std::string& guid, const std::string& name, const std::string& path)
        : guid(guid), name(name), path(path) {}
};

/**
 * @brief Asset thumbnail data
 */
struct AssetThumbnail {
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<uint8_t> pixelData;  // RGBA format
    uint32_t textureID = 0;          // OpenGL texture ID (0 if not uploaded)
    bool isDirty = true;             // Whether thumbnail needs regeneration
};

/**
 * @brief Asset reference (weak reference to asset)
 */
struct AssetReference {
    std::string guid;
    std::weak_ptr<Asset> cachedAsset;
    
    bool IsValid() const;
    std::shared_ptr<Asset> Resolve() const;
};

/**
 * @brief Asset load state
 */
enum class AssetLoadState {
    Unloaded,   // Not loaded yet
    Loading,    // Currently loading
    Loaded,     // Successfully loaded
    Failed      // Load failed
};

} // namespace MyEngine
