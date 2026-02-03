cmake_minimum_required(VERSION 3.15)

add_library(EngineAnimation STATIC
    Bone.cpp
    Bone.h
    Animation.cpp
    Animation.h
    Animator.cpp
    Animator.h
    AnimatedMesh.cpp
    AnimatedMesh.h
    AnimatedModelLoader.cpp
    AnimatedModelLoader.h
)

target_include_directories(EngineAnimation PUBLIC
    ${CMAKE_SOURCE_DIR}/Engine
)

target_link_libraries(EngineAnimation PUBLIC
    EngineMath
    EngineRendering
)

# Link Assimp if available
if(TARGET assimp::assimp)
    target_link_libraries(EngineAnimation PRIVATE assimp::assimp)
    target_compile_definitions(EngineAnimation PRIVATE USE_ASSIMP)
endif()
