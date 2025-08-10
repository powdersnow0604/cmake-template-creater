cmake_minimum_required(VERSION 3.17)
project(cmake-template-creator VERSION 1.0.0)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(ctc 
    src/main.cpp
    src/commands/init_command.cpp
    src/commands/install_command.cpp  
    src/commands/uninstall_command.cpp
    src/commands/run_command.cpp
    src/commands/list_command.cpp
    src/commands/apply_command.cpp
    src/utils/file_utils.cpp
)

# Include directories
target_include_directories(ctc PRIVATE include)

# Compiler-specific optimizations for Release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        target_compile_options(ctc PRIVATE /O2 /DNDEBUG)
    else()
        target_compile_options(ctc PRIVATE -O3 -DNDEBUG)
    endif()
endif()

# Link filesystem library if needed (some compilers require explicit linking)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(ctc stdc++fs)
endif()

# Set output directory for the binary
set_target_properties(ctc PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
