# Client engine: rendering, scene, UI, input, audio, animation, debug overlay and the network clients.
# Two variants from the same sources: Moksha_Client (windowed, MOKSHA_RENDERER's graphics API) and
# Moksha_ClientHeadless (ME_HEADLESS: null renderer, input and audio; no window, GPU or audio device).
# Platform entry points are separate OBJECT libraries (Moksha_Entry, Moksha_EntryHeadless), so the linker can't
# drop WinMain/main or Objective-C classes; each is linked straight into a game executable.

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX "/(platform|third_party)/")

# Windowed client, when the platform has a graphics API.
if(MOKSHA_GRAPHICS_API_DEFINE)
    if(MOKSHA_RENDERER STREQUAL "METAL")
        add_subdirectory(third_party/metal)
    endif()
    add_subdirectory(third_party/miniaudio-0.11.23)

    # Only this graphics API's renderer files.
    set(CLIENT_SOURCES ${SOURCES})
    if(MOKSHA_RENDERER STREQUAL "DX12")
        list(FILTER CLIENT_SOURCES EXCLUDE REGEX
             "/rendering/(metal|webgpu|vulkan)/|/scene/scene_(ui_)?(metal|webgpu|vulkan)\.")
    elseif(MOKSHA_RENDERER STREQUAL "METAL")
        list(FILTER CLIENT_SOURCES EXCLUDE REGEX
             "/rendering/(directx|webgpu|vulkan)/|/scene/scene_(ui_)?(dx|webgpu|vulkan)\.")
    elseif(MOKSHA_RENDERER STREQUAL "WEBGPU")
        list(FILTER CLIENT_SOURCES EXCLUDE REGEX
             "/rendering/(directx|metal|vulkan)/|/scene/scene_(ui_)?(dx|metal|vulkan)\.")
    elseif(MOKSHA_RENDERER STREQUAL "VULKAN")
        list(FILTER CLIENT_SOURCES EXCLUDE REGEX
             "/rendering/(directx|metal|webgpu)/|/scene/scene_(ui_)?(dx|metal|webgpu)\.")
    endif()

    add_library(Moksha_Client STATIC ${CLIENT_SOURCES})

    target_include_directories(Moksha_Client PUBLIC ${PROJECT_SOURCE_DIR})
    target_compile_definitions(Moksha_Client PUBLIC ${MOKSHA_GRAPHICS_API_DEFINE})

    # PUBLIC, so a game can use shared and logging directly.
    target_link_libraries(Moksha_Client PUBLIC Moksha_Options Moksha_Shared Moksha_Logging miniaudio)

    # Graphics API libraries.
    if(MOKSHA_RENDERER STREQUAL "DX12")
        target_link_libraries(Moksha_Client PUBLIC d3d12 dxgi d3dcompiler)
    elseif(MOKSHA_RENDERER STREQUAL "METAL")
        target_link_libraries(Moksha_Client PUBLIC METAL_LIB)
    elseif(MOKSHA_RENDERER STREQUAL "WEBGPU")
        # Both compiling and linking need the port.
        target_compile_options(Moksha_Client PUBLIC --use-port=emdawnwebgpu)
        target_link_options(Moksha_Client PUBLIC --use-port=emdawnwebgpu)
    elseif(MOKSHA_RENDERER STREQUAL "VULKAN" AND CMAKE_SYSTEM_NAME STREQUAL "Android")
        # Headers are in the NDK's sysroot; libvulkan is the system loader.
        target_link_libraries(Moksha_Client PUBLIC vulkan)
    elseif(MOKSHA_RENDERER STREQUAL "VULKAN")
        # Headers and the loader from the Vulkan SDK (VULKAN_SDK).
        find_package(Vulkan REQUIRED)
        target_link_libraries(Moksha_Client PUBLIC Vulkan::Vulkan)
    endif()

    # OS libraries.
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        find_library(APPLICATION_SERVICES_FRAMEWORK ApplicationServices)
        target_link_libraries(Moksha_Client PUBLIC ${APPLICATION_SERVICES_FRAMEWORK})
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
        # NativeActivity and window (android); logcat comes through Moksha_Logging.
        target_link_libraries(Moksha_Client PUBLIC android)
    endif()

    # Platform entry and OS glue for the current OS.
    if(WIN32)
        file(GLOB ENTRY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/platform/win/*.cpp")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        file(GLOB ENTRY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/platform/ios/*.mm")
    elseif(APPLE)
        file(GLOB ENTRY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/platform/mac/*.mm")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
        file(GLOB ENTRY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/platform/web/*.cpp")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
        # The NDK's native_app_glue exports ANativeActivity_onCreate and runs android_main on its own thread.
        # Compiled into this OBJECT library, so the linker keeps it without -u ANativeActivity_onCreate.
        set(ANDROID_GLUE_DIR "${ANDROID_NDK}/sources/android/native_app_glue")
        file(GLOB ENTRY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/platform/android/*.cpp")
        list(APPEND ENTRY_SOURCES "${ANDROID_GLUE_DIR}/android_native_app_glue.c")
    else()
        message(FATAL_ERROR "Moksha: no windowed client entry for '${CMAKE_SYSTEM_NAME}' yet")
    endif()

    if(NOT ENTRY_SOURCES)
        message(FATAL_ERROR "Moksha: no windowed client entry sources for '${CMAKE_SYSTEM_NAME}' yet")
    endif()
    add_library(Moksha_Entry OBJECT ${ENTRY_SOURCES})
    target_link_libraries(Moksha_Entry PUBLIC Moksha_Client)
    # The iOS driver uses ARC (the macOS one predates it).
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        target_compile_options(Moksha_Entry PRIVATE -fobjc-arc)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
        target_include_directories(Moksha_Entry PRIVATE "${ANDROID_GLUE_DIR}")
    endif()
endif()

# Headless client: the same sources without the GPU, OS input, audio device and windowed app files.
set(HEADLESS_SOURCES ${SOURCES})
list(FILTER HEADLESS_SOURCES EXCLUDE REGEX
     "/rendering/(directx|metal|webgpu|vulkan)/|/scene/scene_(ui_)?(dx|metal|webgpu|vulkan)\.|/input/input_manager_(win|mac|ios|web|android)\.|/audio/audio_impl_miniaudio\.")

add_library(Moksha_ClientHeadless STATIC ${HEADLESS_SOURCES})

target_include_directories(Moksha_ClientHeadless PUBLIC ${PROJECT_SOURCE_DIR})
target_compile_definitions(Moksha_ClientHeadless PUBLIC ME_HEADLESS)
target_link_libraries(Moksha_ClientHeadless PUBLIC Moksha_Options Moksha_Shared Moksha_Logging)

file(GLOB HEADLESS_ENTRY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/platform/headless/*.cpp")
add_library(Moksha_EntryHeadless OBJECT ${HEADLESS_ENTRY_SOURCES})
target_link_libraries(Moksha_EntryHeadless PUBLIC Moksha_ClientHeadless)
