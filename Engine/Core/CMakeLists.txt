# Core Module - 核心系统

add_library(EngineCore STATIC
    Core.cpp
    UUID.cpp
    Log.cpp
    LinearAllocator.cpp
    StackAllocator.cpp
    PoolAllocator.cpp
    FrameAllocator.cpp
    MemoryTracker.cpp
    Application.cpp
    Module.cpp
    Config.cpp
    TaskSystem.cpp
    LayerStack.cpp
    ImGuiLayer.cpp
)

# 包含目录
target_include_directories(EngineCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)

# Dependency: spdlog (logging library)
if(EXISTS ${CMAKE_SOURCE_DIR}/ThirdParty/spdlog/include)
    target_include_directories(EngineCore PUBLIC
        ${CMAKE_SOURCE_DIR}/ThirdParty/spdlog/include
    )
endif()

# Dependency: ImGui (UI library)
find_package(imgui CONFIG REQUIRED)
target_link_libraries(EngineCore PUBLIC imgui::imgui)

# Dependency: EnginePlatform (for OpenGL headers)
target_link_libraries(EngineCore PUBLIC EnginePlatform)
