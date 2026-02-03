/******************************************************************************
 * File: AssetBrowserPanel.h
 * Author: AI Assistant
 * Created: 2026-02-02
 * Description: Asset browser panel for viewing and managing assets
 *              Based on asset_browser_architecture.md
 ******************************************************************************/

#pragma once

#include "Editor/Panel.h"
#include "Resource/AssetTypes.h"
#include "Resource/AssetDatabase.h"
#include <vector>
#include <string>
#include <unordered_map>

namespace MyEngine {

/**
 * @brief Asset browser view mode
 */
enum class BrowserViewMode {
    Icon,   // Icon/grid view
    List    // List view
};

/**
 * @brief Asset browser panel for viewing and managing project assets
 */
class AssetBrowserPanel : public Panel {
public:
    AssetBrowserPanel();
    virtual ~AssetBrowserPanel();
    
    virtual void OnUIRender() override;
    virtual void OnUpdate(float deltaTime) override;
    
    // Navigation
    void NavigateToFolder(const std::string& folderPath);
    void NavigateBack();
    void NavigateUp();
    std::string GetCurrentFolder() const { return m_CurrentFolder; }
    
    // View control
    void SetViewMode(BrowserViewMode mode) { m_ViewMode = mode; }
    BrowserViewMode GetViewMode() const { return m_ViewMode; }
    
    // Selection
    const std::vector<std::string>& GetSelectedAssets() const { return m_SelectedAssets; }
    
private:
    // UI rendering methods
    void RenderToolbar();
    void RenderFolderTree();
    void RenderAssetGrid();
    void RenderAssetList();
    void RenderSearchBar();
    void RenderContextMenu();
    void RenderAssetCard(const AssetMetadata& metadata, float thumbnailSize);
    
    // Asset operations
    void OnAssetClicked(const std::string& guid);
    void OnAssetDoubleClicked(const std::string& guid);
    void DeleteAsset(const std::string& guid);
    void RefreshAssets();
    
    // Thumbnail management
    uint32_t GetThumbnailForAsset(const AssetMetadata& metadata);
    void GenerateDefaultThumbnail(AssetType type);
    
private:
    // Current state
    std::string m_CurrentFolder;
    std::vector<std::string> m_NavigationHistory;
    size_t m_HistoryIndex;
    
    // View settings
    BrowserViewMode m_ViewMode;
    float m_ThumbnailSize;
    float m_IconSize;
    
    // Asset filtering
    std::string m_SearchFilter;
    AssetType m_TypeFilter;
    
    // Selection
    std::vector<std::string> m_SelectedAssets;
    std::string m_ContextMenuAsset;
    
    // Cached data
    std::vector<AssetMetadata> m_DisplayedAssets;
    std::unordered_map<AssetType, uint32_t> m_DefaultThumbnails;
    
    // Timing
    float m_RefreshTimer;
    const float m_RefreshInterval = 2.0f;  // Refresh every 2 seconds
};

} // namespace MyEngine
