# Rendering Module - 渲染系统

add_library(EngineRendering STATIC
    Rendering.cpp
    Renderer.cpp
    RenderBackend.cpp
    OpenGL/OpenGLRenderBackend.cpp
    OpenGL/OpenGLContext.cpp
    OpenGL/OpenGLBuffer.cpp
    OpenGL/OpenGLVertexArray.cpp
    OpenGL/OpenGLShader.cpp
    OpenGL/OpenGLTexture.cpp
    ../Resource/Mesh.cpp
    ../Resource/Asset.cpp
    ../Resource/AssetDatabase.cpp
    Camera.cpp
    Pass/RenderPass.cpp
    Pass/GeometryPass.cpp
    Pass/SkyPass.cpp
    Pass/WaterPass.cpp
    Pass/TerrainPass.cpp
    Pass/GrassPass.cpp
    Pass/PostProcessPass.cpp
    Pass/SkeletalAnimationPass.cpp
    Pass/ParticlePass.cpp
)

# 条件编译 Assimp 相关文件
if(ASSIMP_AVAILABLE)
    target_sources(EngineRendering PRIVATE ../Resource/AssimpMeshLoader.cpp)
    target_compile_definitions(EngineRendering PUBLIC MYENGINE_ASSIMP_ENABLED)
endif()

# 包含目录
target_include_directories(EngineRendering PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_SOURCE_DIR}/ThirdParty/GLFW/include
    ${CMAKE_SOURCE_DIR}/ThirdParty/GLFW/deps
    ${CMAKE_SOURCE_DIR}/ThirdParty/stb
)

# 链接 Assimp
if(ASSIMP_AVAILABLE)
    target_link_libraries(EngineRendering PUBLIC assimp::assimp)
    target_link_libraries(EngineRendering PUBLIC EngineAnimation)
endif()

# 依赖 GLM (数学库)
if(EXISTS ${CMAKE_SOURCE_DIR}/ThirdParty/glm)
    target_include_directories(EngineRendering PUBLIC
        ${CMAKE_SOURCE_DIR}/ThirdParty/glm
    )
endif()
