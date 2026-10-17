# Moksha Engine. Builds the engine libraries; tests and tools only when built as the top-level project.

cmake_minimum_required(VERSION 3.31)

project(Moksha VERSION 0.1.0 LANGUAGES CXX C)

if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

option(MOKSHA_BUILD_TESTS "Build the engine unit tests" ${PROJECT_IS_TOP_LEVEL})
option(MOKSHA_BUILD_TOOLS "Build the engine tools" ${PROJECT_IS_TOP_LEVEL})

# Graphics API of the client. NONE builds the headless (windowless) client.
if(WIN32)
    set(MOKSHA_DEFAULT_RENDERER DX12)
elseif(APPLE)
    set(MOKSHA_DEFAULT_RENDERER METAL)
else()
    set(MOKSHA_DEFAULT_RENDERER NONE)
endif()
set(MOKSHA_RENDERER ${MOKSHA_DEFAULT_RENDERER} CACHE STRING "Graphics API: DX12, METAL or NONE (headless)")
set_property(CACHE MOKSHA_RENDERER PROPERTY STRINGS DX12 METAL NONE)

# MSVC adds /EHsc by default; strip it so /EHs-c- below takes effect without a D9025 override warning.
if(MSVC)
    string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

# Compile settings every engine target links PUBLIC, so code built against the engine gets the same.
add_library(Moksha_Options INTERFACE)
target_compile_features(Moksha_Options INTERFACE cxx_std_20)

# RTTI and exceptions are disabled everywhere.
if(MSVC)
    # With exceptions off MSVC only warns (C4530) on try/catch; /we4530 makes it an error, like clang.
    target_compile_options(Moksha_Options INTERFACE
        $<$<COMPILE_LANGUAGE:CXX>:/GR- /EHs-c- /we4530>
        /Wall /Qspectre- /wd5045 /wd4820)
    target_compile_definitions(Moksha_Options INTERFACE _HAS_EXCEPTIONS=0)
else()
    target_compile_options(Moksha_Options INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti -fno-exceptions>)
endif()

# Operating system.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(Moksha_Options INTERFACE ME_WIN)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(Moksha_Options INTERFACE ME_MAC)
elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_compile_definitions(Moksha_Options INTERFACE ME_IOS)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    target_compile_definitions(Moksha_Options INTERFACE ME_ANDROID)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    target_compile_definitions(Moksha_Options INTERFACE ME_WEB)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(Moksha_Options INTERFACE ME_LINUX)
else()
    message(FATAL_ERROR "Moksha: unsupported system '${CMAKE_SYSTEM_NAME}'")
endif()

# Graphics API.
if(MOKSHA_RENDERER STREQUAL "DX12")
    target_compile_definitions(Moksha_Options INTERFACE ME_DX12)
elseif(MOKSHA_RENDERER STREQUAL "METAL")
    target_compile_definitions(Moksha_Options INTERFACE ME_METAL)
elseif(MOKSHA_RENDERER STREQUAL "NONE")
    target_compile_definitions(Moksha_Options INTERFACE ME_HEADLESS)
else()
    message(FATAL_ERROR "Moksha: unknown MOKSHA_RENDERER '${MOKSHA_RENDERER}'")
endif()

add_subdirectory(moksha/logging)
add_subdirectory(moksha/shared)
add_subdirectory(moksha/test_framework)

if(MOKSHA_BUILD_TESTS)
    add_subdirectory(tests)
endif()
