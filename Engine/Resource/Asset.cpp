/******************************************************************************
 * File: Asset.cpp
 * Author: AI Assistant
 * Created: 2026-02-02
 * Description: Asset base class implementation
 ******************************************************************************/

#include "Asset.h"
#include "Core/Log.h"

namespace MyEngine {

bool Asset::Reload() {
    ENGINE_INFO("Reloading asset: {}", m_Metadata.name);
    
    if (!Unload()) {
        ENGINE_ERROR("Failed to unload asset for reload: {}", m_Metadata.name);
        return false;
    }
    
    return Load();
}

// AssetReference implementation
bool AssetReference::IsValid() const {
    return !cachedAsset.expired();
}

std::shared_ptr<Asset> AssetReference::Resolve() const {
    return cachedAsset.lock();
}

} // namespace MyEngine
