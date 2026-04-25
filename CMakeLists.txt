cmake_minimum_required(VERSION 3.16)
project(processFKer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    "processFKer/injector.cpp"
    "processFKer/main.cpp"
)

set(HEADERS
    "processFKer/injector.h"
)

# Disable BufferSecurityCheck to match original VS project
if(MSVC)
    add_compile_options(/GS-)
    # Set Unicode
    add_definitions(-DUNICODE -D_UNICODE)
endif()

# Create executable based on current architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # x86 build
    add_executable(FKer-x86 ${SOURCES} ${HEADERS})
    set_target_properties(FKer-x86 PROPERTIES
        OUTPUT_NAME "FKer"
        WIN32_EXECUTABLE FALSE
    )
else()
    # x64 build
    add_executable(FKer-x64 ${SOURCES} ${HEADERS})
    set_target_properties(FKer-x64 PROPERTIES
        OUTPUT_NAME "FKer"
        WIN32_EXECUTABLE FALSE
    )
endif()

# Release optimizations
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Oi /Gy")
