# MyEngine - 引擎模块

add_library(MyEngine STATIC
    Engine.cpp
)

# 子模块
add_subdirectory(Platform)
add_subdirectory(Core)
add_subdirectory(Math)
add_subdirectory(Rendering)
add_subdirectory(ECS)
add_subdirectory(Scene)
add_subdirectory(Animation)
add_subdirectory(Particle)
add_subdirectory(Scripting)
add_subdirectory(Editor)

# 链接子模块
target_link_libraries(MyEngine PUBLIC
    EnginePlatform
    EngineCore
    EngineRendering
    EngineECS
    EngineAnimation
    EngineParticle
    EngineScripting
    EngineEditor
)

# 包含目录
target_include_directories(MyEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
