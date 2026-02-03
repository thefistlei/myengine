# Math Module - 数学库

# Math is header-only, no source files needed
add_library(EngineMath INTERFACE)

# 包含目录
target_include_directories(EngineMath INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}../
)
