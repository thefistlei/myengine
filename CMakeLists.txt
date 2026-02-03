cmake_minimum_required(VERSION 3.20)
project(MyEngine VERSION 0.1.0 LANGUAGES CXX)

# C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /WX-)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 全局包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Engine)

# 第三方库配置
# GLFW (窗口管理)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GLFW)
    add_subdirectory(ThirdParty/GLFW)
endif()

# Assimp (3D 模型加载)
# 使用 find_package，需要先通过 vcpkg 或系统安装 Assimp
find_package(assimp QUIET)
if(NOT assimp_FOUND)
    message(STATUS "Assimp not found, disabling FBX support")
    message(STATUS "To enable FBX support, install Assimp:")
    message(STATUS "  vcpkg install assimp:x64-windows")
    set(ASSIMP_AVAILABLE FALSE)
else()
    message(STATUS "Assimp found: ${assimp_VERSION}")
    set(ASSIMP_AVAILABLE TRUE)
endif()

# GLM (数学库 - Header Only)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glm)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glm)
endif()

# spdlog (日志库 - Header Only)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/spdlog/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/spdlog/include)
endif()

# 添加引擎模块
add_subdirectory(Engine)

# 添加 Runtime
add_subdirectory(Runtime)

# 添加测试程序
if(ASSIMP_AVAILABLE)
    add_executable(TestSkeletalAnimation Tests/TestSkeletalAnimation.cpp)
    target_link_libraries(TestSkeletalAnimation PRIVATE
        MyEngine
        EngineAnimation
        glfw
    )
    if(TARGET assimp::assimp)
        target_link_libraries(TestSkeletalAnimation PRIVATE assimp::assimp)
        target_compile_definitions(TestSkeletalAnimation PRIVATE USE_ASSIMP)
    endif()
endif()

# 打印配置信息
message(STATUS "===========================================")
message(STATUS "MyEngine Configuration")
message(STATUS "===========================================")
message(STATUS "CMake Version: ${CMAKE_VERSION}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "===========================================")
