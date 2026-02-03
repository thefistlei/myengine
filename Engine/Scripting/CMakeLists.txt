# Engine Scripting Module

add_library(EngineScripting STATIC
    LuaVM.cpp
    LuaBridge.cpp
    ScriptComponent.cpp
    ScriptSystem.cpp
)

# Find Lua package (only x64 compatible versions)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # We're building for 64-bit
    find_package(Lua)
    
    # Check if the found Lua library is actually 64-bit compatible
    if(LUA_FOUND AND LUA_LIBRARIES)
        # For now, assume incompatible if it's at E:/Lua (which is x86)
        # This is a workaround until proper x64 Lua is installed
        if(LUA_LIBRARIES MATCHES "E:/Lua" OR LUA_LIBRARIES MATCHES "E:\\\\Lua")
            set(LUA_ARCHITECTURE_COMPATIBLE FALSE)
            message(STATUS "Found Lua at: ${LUA_LIBRARIES}")
            message(STATUS "  This appears to be x86 Lua, skipping to avoid architecture mismatch")
        else()
            set(LUA_ARCHITECTURE_COMPATIBLE TRUE)
            message(STATUS "Found Lua: ${LUA_VERSION_STRING}")
            message(STATUS "  Include: ${LUA_INCLUDE_DIR}")
            message(STATUS "  Library: ${LUA_LIBRARIES}")
        endif()
    else()
        set(LUA_ARCHITECTURE_COMPATIBLE FALSE)
    endif()
else()
    set(LUA_FOUND FALSE)
    set(LUA_ARCHITECTURE_COMPATIBLE FALSE)
endif()

if(LUA_FOUND AND LUA_ARCHITECTURE_COMPATIBLE)
    target_link_libraries(EngineScripting PUBLIC
        EngineCore
        EngineECS
        ${LUA_LIBRARIES}
    )
    
    target_include_directories(EngineScripting PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${LUA_INCLUDE_DIR}
    )
    
    target_compile_definitions(EngineScripting PUBLIC LUA_SCRIPTING_ENABLED)
    message(STATUS "✓ Lua scripting ENABLED")
else()
    target_link_libraries(EngineScripting PUBLIC
        EngineCore
        EngineECS
    )
    
    target_include_directories(EngineScripting PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )
    
    message(STATUS "")
    message(STATUS "========================================")
    message(STATUS "⚠ Lua Scripting DISABLED")
    message(STATUS "========================================")
    if(LUA_FOUND)
        message(STATUS "Reason: Found Lua library is x86 (32-bit), but building for x64 (64-bit)")
        message(STATUS "Library: ${LUA_LIBRARIES}")
    else()
        message(STATUS "Reason: No Lua library found")
    endif()
    message(STATUS "")
    message(STATUS "To enable Lua scripting, install x64 Lua:")
    message(STATUS "  Option 1: vcpkg install lua:x64-windows")
    message(STATUS "  Option 2: Download x64 Lua from https://luabinaries.sourceforge.net/")
    message(STATUS "  Option 3: Build Lua from source for x64")
    message(STATUS "")
    message(STATUS "The engine will build successfully without Lua scripting.")
    message(STATUS "========================================")
    message(STATUS "")
endif()
