# Platform Module - 平台抽象层

add_library(EnginePlatform STATIC
    WindowsWindow.cpp
    Timer.cpp
    FileSystem.cpp
    PlatformInfo.cpp
)

# 包含目录
target_include_directories(EnginePlatform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_SOURCE_DIR}/ThirdParty/GLFW/include
    ${CMAKE_SOURCE_DIR}/ThirdParty/GLFW/deps
)

# 依赖项 - GLFW (窗口管理)
if(TARGET glfw)
    target_link_libraries(EnginePlatform PUBLIC glfw)
    target_compile_definitions(EnginePlatform PUBLIC MYENGINE_USE_GLFW)
endif()
