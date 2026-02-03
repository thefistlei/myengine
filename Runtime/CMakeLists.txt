# Runtime - 主程序入口

add_executable(MyEngineRuntime
    main.cpp
)

# 链接引擎
target_link_libraries(MyEngineRuntime PRIVATE
    MyEngine
)

# 设置启动项目 (Visual Studio)
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MyEngineRuntime)

# 输出名称
set_target_properties(MyEngineRuntime PROPERTIES
    OUTPUT_NAME "MyEngine"
)
