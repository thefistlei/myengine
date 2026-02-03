cmake_minimum_required(VERSION 3.15)

project(EngineParticle)

# Source files
set(PARTICLE_SOURCES
    Particle.cpp
    ParticleSystem.cpp
)

# Header files
set(PARTICLE_HEADERS
    Particle.h
    ParticleSystem.h
)

# Create library
add_library(EngineParticle STATIC
    ${PARTICLE_SOURCES}
    ${PARTICLE_HEADERS}
)

# Include directories
target_include_directories(EngineParticle PUBLIC
    ${CMAKE_SOURCE_DIR}/Engine
)

# Link dependencies
target_link_libraries(EngineParticle PUBLIC
    EngineMath
    EngineRendering
    EngineCore
)

# Group source files
source_group("Source Files" FILES ${PARTICLE_SOURCES})
source_group("Header Files" FILES ${PARTICLE_HEADERS})
