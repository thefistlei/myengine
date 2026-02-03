/******************************************************************************
 * File: AssetBrowserPanel.cpp
 * Author: AI Assistant
 * Created: 2026-02-02
 * Description: Asset browser panel implementation
 ******************************************************************************/

#include "AssetBrowserPanel.h"
#include "Core/Log.h"
#include <imgui.h>
#include <algorithm>

namespace MyEngine {

AssetBrowserPanel::AssetBrowserPanel()
    : Panel("Asset Browser", true)
    , m_CurrentFolder("assets")
    , m_HistoryIndex(0)
    , m_ViewMode(BrowserViewMode::Icon)
    , m_ThumbnailSize(100.0f)
    , m_IconSize(80.0f)
    , m_TypeFilter(AssetType::Unknown)
    , m_RefreshTimer(0.0f)
{
    m_NavigationHistory.push_back(m_CurrentFolder);
    
    // Initialize asset database
    AssetDatabase::GetInstance().Refresh();
    RefreshAssets();
}

AssetBrowserPanel::~AssetBrowserPanel() {
}

void AssetBrowserPanel::OnUpdate(float deltaTime) {
    m_RefreshTimer += deltaTime;
    if (m_RefreshTimer >= m_RefreshInterval) {
        m_RefreshTimer = 0.0f;
        RefreshAssets();
    }
}

void AssetBrowserPanel::OnUIRender() {
    if (!m_IsOpen) return;
    
    ImGui::Begin(m_Name.c_str(), &m_IsOpen);
    
    // Toolbar
    RenderToolbar();
    
    ImGui::Separator();
    
    // Search bar
    RenderSearchBar();
    
    ImGui::Separator();
    
    // Main content area
    ImGui::BeginChild("AssetContent", ImVec2(0, 0), false);
    
    if (m_ViewMode == BrowserViewMode::Icon) {
        RenderAssetGrid();
    } else {
        RenderAssetList();
    }
    
    ImGui::EndChild();
    
    // Context menu
    RenderContextMenu();
    
    ImGui::End();
}

void AssetBrowserPanel::RenderToolbar() {
    // Navigation buttons
    if (ImGui::Button("<-")) {
        NavigateBack();
    }
    ImGui::SameLine();
    
    if (ImGui::Button("^")) {
        NavigateUp();
    }
    ImGui::SameLine();
    
    // Current folder display
    ImGui::Text("Folder: %s", m_CurrentFolder.c_str());
    ImGui::SameLine();
    
    // Refresh button
    if (ImGui::Button("Refresh")) {
        AssetDatabase::GetInstance().Refresh();
        RefreshAssets();
    }
    ImGui::SameLine();
    
    // View mode toggle
    if (ImGui::Button(m_ViewMode == BrowserViewMode::Icon ? "Grid" : "List")) {
        m_ViewMode = (m_ViewMode == BrowserViewMode::Icon) ? BrowserViewMode::List : BrowserViewMode::Icon;
    }
    ImGui::SameLine();
    
    // Icon size slider (for icon view)
    if (m_ViewMode == BrowserViewMode::Icon) {
        ImGui::SetNextItemWidth(150.0f);
        ImGui::SliderFloat("Size", &m_IconSize, 50.0f, 150.0f, "%.0f");
    }
}

void AssetBrowserPanel::RenderSearchBar() {
    char searchBuffer[256];
    strcpy_s(searchBuffer, sizeof(searchBuffer), m_SearchFilter.c_str());
    
    ImGui::SetNextItemWidth(200.0f);
    if (ImGui::InputText("Search", searchBuffer, sizeof(searchBuffer))) {
        m_SearchFilter = searchBuffer;
        RefreshAssets();
    }
    
    ImGui::SameLine();
    
    // Type filter dropdown
    const char* typeNames[] = { "All", "Texture", "Model", "Mesh", "Shader", "Audio" };
    AssetType typeValues[] = { AssetType::Unknown, AssetType::Texture, AssetType::Model, 
                                AssetType::Mesh, AssetType::Shader, AssetType::Audio };
    
    int currentTypeIndex = 0;
    for (int i = 0; i < 6; ++i) {
        if (m_TypeFilter == typeValues[i]) {
            currentTypeIndex = i;
            break;
        }
    }
    
    ImGui::SetNextItemWidth(120.0f);
    if (ImGui::Combo("Type", &currentTypeIndex, typeNames, 6)) {
        m_TypeFilter = typeValues[currentTypeIndex];
        RefreshAssets();
    }
}

void AssetBrowserPanel::RenderAssetGrid() {
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    float cellSize = m_IconSize + 20.0f;
    int columnCount = std::max(1, (int)(windowSize.x / cellSize));
    
    ImGui::Columns(columnCount, nullptr, false);
    
    for (const auto& metadata : m_DisplayedAssets) {
        ImGui::PushID(metadata.guid.c_str());
        
        RenderAssetCard(metadata, m_IconSize);
        
        ImGui::NextColumn();
        ImGui::PopID();
    }
    
    ImGui::Columns(1);
}

void AssetBrowserPanel::RenderAssetList() {
    ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | 
                            ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable;
    
    if (ImGui::BeginTable("AssetTable", 4, flags)) {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        
        for (const auto& metadata : m_DisplayedAssets) {
            ImGui::TableNextRow();
            ImGui::PushID(metadata.guid.c_str());
            
            // Name column
            ImGui::TableSetColumnIndex(0);
            bool selected = std::find(m_SelectedAssets.begin(), m_SelectedAssets.end(), 
                                     metadata.guid) != m_SelectedAssets.end();
            
            if (ImGui::Selectable(metadata.name.c_str(), selected, 
                                 ImGuiSelectableFlags_SpanAllColumns)) {
                OnAssetClicked(metadata.guid);
            }
            
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                OnAssetDoubleClicked(metadata.guid);
            }
            
            // Type column
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", AssetTypeToString(metadata.type));
            
            // Size column
            ImGui::TableSetColumnIndex(2);
            if (metadata.fileSize < 1024) {
                ImGui::Text("%zu B", metadata.fileSize);
            } else if (metadata.fileSize < 1024 * 1024) {
                ImGui::Text("%.1f KB", metadata.fileSize / 1024.0f);
            } else {
                ImGui::Text("%.1f MB", metadata.fileSize / (1024.0f * 1024.0f));
            }
            
            // Path column
            ImGui::TableSetColumnIndex(3);
            ImGui::TextDisabled("%s", metadata.path.c_str());
            
            ImGui::PopID();
        }
        
        ImGui::EndTable();
    }
}

void AssetBrowserPanel::RenderAssetCard(const AssetMetadata& metadata, float thumbnailSize) {
    ImVec2 cursorPos = ImGui::GetCursorPos();
    
    // Selection highlight
    bool selected = std::find(m_SelectedAssets.begin(), m_SelectedAssets.end(), 
                             metadata.guid) != m_SelectedAssets.end();
    
    if (selected) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.8f, 0.5f));
    }
    
    // Thumbnail button
    uint32_t thumbnailID = GetThumbnailForAsset(metadata);
    if (ImGui::ImageButton(metadata.guid.c_str(), (ImTextureID)(intptr_t)thumbnailID,
                          ImVec2(thumbnailSize, thumbnailSize))) {
        OnAssetClicked(metadata.guid);
    }
    
    if (selected) {
        ImGui::PopStyleColor();
    }
    
    // Double-click detection
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        OnAssetDoubleClicked(metadata.guid);
    }
    
    // Drag-and-drop source
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        // Set payload to carry the asset GUID and metadata
        ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", metadata.guid.c_str(), metadata.guid.size() + 1);
        
        // Display preview
        ImGui::Text("Dragging: %s", metadata.name.c_str());
        ImGui::TextDisabled("Type: %s", AssetTypeToString(metadata.type));
        
        ImGui::EndDragDropSource();
    }
    
    // Context menu
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        m_ContextMenuAsset = metadata.guid;
        ImGui::OpenPopup("AssetContextMenu");
    }
    
    // Asset name label
    ImGui::TextWrapped("%s", metadata.name.c_str());
    
    // Type label
    ImGui::TextDisabled("%s", AssetTypeToString(metadata.type));
}

void AssetBrowserPanel::RenderContextMenu() {
    if (ImGui::BeginPopup("AssetContextMenu")) {
        if (ImGui::MenuItem("Open")) {
            OnAssetDoubleClicked(m_ContextMenuAsset);
        }
        
        ImGui::Separator();
        
        if (ImGui::MenuItem("Refresh")) {
            AssetDatabase::GetInstance().RefreshAsset(m_ContextMenuAsset);
        }
        
        if (ImGui::MenuItem("Delete")) {
            DeleteAsset(m_ContextMenuAsset);
        }
        
        ImGui::Separator();
        
        if (ImGui::MenuItem("Show in Explorer")) {
            auto* metadata = AssetDatabase::GetInstance().GetMetadata(m_ContextMenuAsset);
            if (metadata) {
                ENGINE_INFO("Asset path: {}", metadata->path);
            }
        }
        
        ImGui::EndPopup();
    }
}

void AssetBrowserPanel::NavigateToFolder(const std::string& folderPath) {
    m_CurrentFolder = folderPath;
    
    // Add to history
    if (m_HistoryIndex < m_NavigationHistory.size() - 1) {
        m_NavigationHistory.erase(m_NavigationHistory.begin() + m_HistoryIndex + 1, 
                                   m_NavigationHistory.end());
    }
    m_NavigationHistory.push_back(folderPath);
    m_HistoryIndex = m_NavigationHistory.size() - 1;
    
    RefreshAssets();
}

void AssetBrowserPanel::NavigateBack() {
    if (m_HistoryIndex > 0) {
        --m_HistoryIndex;
        m_CurrentFolder = m_NavigationHistory[m_HistoryIndex];
        RefreshAssets();
    }
}

void AssetBrowserPanel::NavigateUp() {
    size_t lastSlash = m_CurrentFolder.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        NavigateToFolder(m_CurrentFolder.substr(0, lastSlash));
    }
}

void AssetBrowserPanel::OnAssetClicked(const std::string& guid) {
    // Handle selection (with Ctrl for multi-select)
    ImGuiIO& io = ImGui::GetIO();
    
    if (io.KeyCtrl) {
        // Toggle selection
        auto it = std::find(m_SelectedAssets.begin(), m_SelectedAssets.end(), guid);
        if (it != m_SelectedAssets.end()) {
            m_SelectedAssets.erase(it);
        } else {
            m_SelectedAssets.push_back(guid);
        }
    } else {
        // Single selection
        m_SelectedAssets.clear();
        m_SelectedAssets.push_back(guid);
    }
}

void AssetBrowserPanel::OnAssetDoubleClicked(const std::string& guid) {
    auto* metadata = AssetDatabase::GetInstance().GetMetadata(guid);
    if (metadata) {
        ENGINE_INFO("Double-clicked asset: {}", metadata->name);
        // TODO: Open asset in appropriate editor
    }
}

void AssetBrowserPanel::DeleteAsset(const std::string& guid) {
    ENGINE_INFO("Deleting asset: {}", guid);
    AssetDatabase::GetInstance().UnregisterAsset(guid);
    RefreshAssets();
}

void AssetBrowserPanel::RefreshAssets() {
    auto& db = AssetDatabase::GetInstance();
    
    // Get all assets
    std::vector<AssetMetadata> allAssets = db.GetAllAssets();
    
    m_DisplayedAssets.clear();
    
    for (const auto& metadata : allAssets) {
        // Filter by folder (show all if current folder is root-level)
        if (!m_CurrentFolder.empty() && 
            m_CurrentFolder != "assets" && 
            metadata.path.find(m_CurrentFolder) != 0) {
            continue;
        }
        
        // Filter by type
        if (m_TypeFilter != AssetType::Unknown && metadata.type != m_TypeFilter) {
            continue;
        }
        
        // Filter by search
        if (!m_SearchFilter.empty()) {
            std::string lowerName = metadata.name;
            std::string lowerFilter = m_SearchFilter;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
            std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), ::tolower);
            
            if (lowerName.find(lowerFilter) == std::string::npos) {
                continue;
            }
        }
        
        m_DisplayedAssets.push_back(metadata);
    }
}

uint32_t AssetBrowserPanel::GetThumbnailForAsset(const AssetMetadata& metadata) {
    // For now, return default placeholder based on type
    // TODO: Implement actual thumbnail generation
    
    auto it = m_DefaultThumbnails.find(metadata.type);
    if (it != m_DefaultThumbnails.end()) {
        return it->second;
    }
    
    // Return 0 for white texture fallback
    return 0;
}

void AssetBrowserPanel::GenerateDefaultThumbnail(AssetType type) {
    // TODO: Generate default colored thumbnails for each type
}

} // namespace MyEngine
