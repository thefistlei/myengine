# Editor Module - 编辑器模块

add_library(EngineEditor STATIC
    EditorCamera.cpp
    EditorLayer.cpp
    Panels/SceneHierarchyPanel.cpp
    Panels/PropertiesPanel.cpp
    Panels/ViewportPanel.cpp
    Panels/AssetBrowserPanel.cpp
)

# 包含目录
target_include_directories(EngineEditor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)

# 链接依赖
target_link_libraries(EngineEditor PUBLIC
    EngineCore
    EngineECS
    EngineRendering
)
