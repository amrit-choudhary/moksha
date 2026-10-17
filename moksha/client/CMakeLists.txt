# Client engine: rendering, scene, UI, input, audio, animation, debug overlay and the network clients.
# Platform entry points (platform/<os>/) are a separate OBJECT library, so the linker can't drop WinMain/main or
# Objective-C classes; it is linked straight into each game executable.

if(APPLE)
    add_subdirectory(third_party/metal)
endif()
add_subdirectory(third_party/miniaudio-0.11.23)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX "/(platform|third_party)/")

add_library(Moksha_Client STATIC ${SOURCES})

target_include_directories(Moksha_Client PUBLIC ${PROJECT_SOURCE_DIR})

# PUBLIC, so a game can use shared and logging directly.
target_link_libraries(Moksha_Client PUBLIC Moksha_Options Moksha_Shared Moksha_Logging miniaudio)

if(WIN32)
    target_link_libraries(Moksha_Client PUBLIC d3d12 dxgi d3dcompiler)
elseif(APPLE)
    find_library(APPLICATION_SERVICES_FRAMEWORK ApplicationServices)
    target_link_libraries(Moksha_Client PUBLIC ${APPLICATION_SERVICES_FRAMEWORK} METAL_LIB)
endif()

# Platform entry and OS glue for the current OS.
if(WIN32)
    file(GLOB ENTRY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/platform/win/*.cpp")
elseif(APPLE)
    file(GLOB ENTRY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/platform/mac/*.mm")
else()
    message(FATAL_ERROR "Moksha: no client platform entry for '${CMAKE_SYSTEM_NAME}' yet")
endif()

add_library(Moksha_Entry OBJECT ${ENTRY_SOURCES})
target_link_libraries(Moksha_Entry PUBLIC Moksha_Client)
