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
        list(FILTER CLIENT_SOURCES EXCLUDE REGEX "/rendering/metal/|/scene/scene_(ui_)?metal\.")
    elseif(MOKSHA_RENDERER STREQUAL "METAL")
        list(FILTER CLIENT_SOURCES EXCLUDE REGEX "/rendering/directx/|/scene/scene_(ui_)?dx\.")
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
    endif()

    # OS libraries.
    if(APPLE)
        find_library(APPLICATION_SERVICES_FRAMEWORK ApplicationServices)
        target_link_libraries(Moksha_Client PUBLIC ${APPLICATION_SERVICES_FRAMEWORK})
    endif()

    # Platform entry and OS glue for the current OS.
    if(WIN32)
        file(GLOB ENTRY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/platform/win/*.cpp")
    elseif(APPLE)
        file(GLOB ENTRY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/platform/mac/*.mm")
    else()
        message(FATAL_ERROR "Moksha: no windowed client entry for '${CMAKE_SYSTEM_NAME}' yet")
    endif()

    add_library(Moksha_Entry OBJECT ${ENTRY_SOURCES})
    target_link_libraries(Moksha_Entry PUBLIC Moksha_Client)
endif()

# Headless client: the same sources without the GPU, OS input, audio device and windowed app files.
set(HEADLESS_SOURCES ${SOURCES})
list(FILTER HEADLESS_SOURCES EXCLUDE REGEX
     "/rendering/(directx|metal)/|/scene/scene_(ui_)?(dx|metal)\.|/input/input_manager_(win|mac)\.|/audio/audio_impl_miniaudio\.")

add_library(Moksha_ClientHeadless STATIC ${HEADLESS_SOURCES})

target_include_directories(Moksha_ClientHeadless PUBLIC ${PROJECT_SOURCE_DIR})
target_compile_definitions(Moksha_ClientHeadless PUBLIC ME_HEADLESS)
target_link_libraries(Moksha_ClientHeadless PUBLIC Moksha_Options Moksha_Shared Moksha_Logging)

file(GLOB HEADLESS_ENTRY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/platform/headless/*.cpp")
add_library(Moksha_EntryHeadless OBJECT ${HEADLESS_ENTRY_SOURCES})
target_link_libraries(Moksha_EntryHeadless PUBLIC Moksha_ClientHeadless)
