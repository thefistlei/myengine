# ECS Module - 实体组件系统

add_library(EngineECS STATIC
    Registry.cpp
)

# 包含目录
target_include_directories(EngineECS PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)
