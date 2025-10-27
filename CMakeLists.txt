# CMake configuration for ACP (Autonomous Command Protocol) Library
# Minimum version for modern CMake features
cmake_minimum_required(VERSION 3.16)

# Project definition
project(acp 
    VERSION 0.3.0 
    DESCRIPTION "Autonomous Command Protocol - Portable C99 framing library"
    LANGUAGES C
)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Build options
option(ACP_BUILD_SHARED "Build shared library" ON)
option(ACP_BUILD_STATIC "Build static library" ON)
option(ACP_BUILD_EXAMPLES "Build example programs" ON)
option(ACP_BUILD_TESTS "Build test programs" ON)
option(ACP_ENABLE_HEAP "Enable heap allocation features" OFF)

# Compile flags for no-heap enforcement (default)
if(NOT ACP_ENABLE_HEAP)
    add_compile_definitions(ACP_NO_HEAP=1)
    # Add linker flag to detect heap usage (if supported)
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
        add_compile_options(-Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=free)
    endif()
endif()

# Platform detection
if(WIN32)
    set(ACP_PLATFORM "Windows")
    set(ACP_PLATFORM_SOURCES acp_platform_windows.c)
    # Disable shared library for Windows in v0.3
    set(ACP_BUILD_SHARED OFF)
    message(STATUS "Platform: Windows - Shared library build disabled for v0.3")
elseif(APPLE)
    set(ACP_PLATFORM "macOS")
    set(ACP_PLATFORM_SOURCES acp_platform_posix.c)
elseif(UNIX)
    set(ACP_PLATFORM "Linux")
    set(ACP_PLATFORM_SOURCES acp_platform_posix.c)
else()
    set(ACP_PLATFORM "Unknown")
    set(ACP_PLATFORM_SOURCES acp_platform_posix.c)
    message(WARNING "Unknown platform, using POSIX fallbacks")
endif()

# Compiler-specific flags
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-fstack-protector-strong)
    if(WIN32)
        # MinGW-specific flags
        add_compile_definitions(_WIN32_WINNT=0x0600)
    endif()
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-fstack-protector-strong)
    if(APPLE)
        # macOS-specific flags
        add_compile_options(-mmacosx-version-min=10.9)
    endif()
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/GS)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Core source files
set(ACP_CORE_SOURCES
    acp.c
    acp_framer.c  
    acp_cobs.c
    acp_crypto.c
    acp_session.c
    acp_nvs.c
    acp_crc16.c
    acp_constants.c
    ${ACP_PLATFORM_SOURCES}
)

# Public headers
set(ACP_PUBLIC_HEADERS
    acp_protocol.h
    acp_version.h
    acp_errors.h
    acp_crc16.h
    acp_cobs.h
    acp_crypto.h
    acp_config.h
    acp_visibility.h
    acp_platform_log.h
    acp_platform_time.h
    acp_platform_mutex.h
    acp_platform_keystore.h
)

# Version definitions
add_compile_definitions(
    ACP_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    ACP_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    ACP_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

# Compiler flags
add_compile_options(-Wall -Wextra -Wpedantic)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0)
else()
    add_compile_options(-O2 -DNDEBUG)
endif()

# Static library target
if(ACP_BUILD_STATIC)
    add_library(acp_static STATIC ${ACP_CORE_SOURCES})
    set_target_properties(acp_static PROPERTIES
        OUTPUT_NAME acp
        PUBLIC_HEADER "${ACP_PUBLIC_HEADERS}"
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
    target_include_directories(acp_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    )
    
    # Platform-specific linking
    if(WIN32 AND CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(acp_static ws2_32)
    endif()
    
    add_library(acp::static ALIAS acp_static)
endif()

# Shared library target (non-Windows)
if(ACP_BUILD_SHARED AND NOT WIN32)
    add_library(acp_shared SHARED ${ACP_CORE_SOURCES})
    set_target_properties(acp_shared PROPERTIES
        OUTPUT_NAME acp
        PUBLIC_HEADER "${ACP_PUBLIC_HEADERS}"
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        POSITION_INDEPENDENT_CODE ON
    )
    target_include_directories(acp_shared PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    )
    
    # Symbol visibility for shared libraries
    target_compile_definitions(acp_shared PRIVATE ACP_BUILDING_SHARED)
    
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
        target_compile_options(acp_shared PRIVATE -fvisibility=hidden)
        
        # Add symbol versioning on Linux
        if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
            target_link_options(acp_shared PRIVATE 
                "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/acp.map")
        endif()
    endif()
    
    add_library(acp::shared ALIAS acp_shared)
endif()

# Set default target
if(TARGET acp_static)
    add_library(acp ALIAS acp_static)
elseif(TARGET acp_shared)
    add_library(acp ALIAS acp_shared)
endif()

# Examples subdirectory
if(ACP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests subdirectory  
if(ACP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
    
    # Include build system tests
    include(tests/BuildSystemTests.cmake)
endif()

# Installation
include(GNUInstallDirs)

# Install targets
if(ACP_BUILD_STATIC)
    install(TARGETS acp_static
        EXPORT acpTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

if(ACP_BUILD_SHARED AND TARGET acp_shared)
    install(TARGETS acp_shared  
        EXPORT acpTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

# Export configuration
install(EXPORT acpTargets
    FILE acpTargets.cmake
    NAMESPACE acp::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/acp
)

# Generate config files
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/acpConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/acpConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/acp
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/acpConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/acpConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/acpConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/acp
)

# Generate and install pkg-config file
set(ACP_PRIVATE_LIBS "")
if(WIN32 AND CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(ACP_PRIVATE_LIBS "-lws2_32")
endif()

if(ACP_ENABLE_HEAP)
    set(ACP_NO_HEAP "OFF")
else()
    set(ACP_NO_HEAP "ON")
endif()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/acp.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/acp.pc"
    @ONLY
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/acp.pc"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# Summary
message(STATUS "")
message(STATUS "ACP Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Platform: ${ACP_PLATFORM}")
message(STATUS "  Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build static library: ${ACP_BUILD_STATIC}")
message(STATUS "  Build shared library: ${ACP_BUILD_SHARED}")
message(STATUS "  Build examples: ${ACP_BUILD_EXAMPLES}")
message(STATUS "  Build tests: ${ACP_BUILD_TESTS}")
message(STATUS "  Heap allocation: ${ACP_ENABLE_HEAP}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  C Standard: C${CMAKE_C_STANDARD}")
message(STATUS "")