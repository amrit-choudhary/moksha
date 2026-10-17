# Generic UDP game server (GameServer, ServerApp). Builds on every platform, including Linux servers.

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(Moksha_Server STATIC ${SOURCES})

target_include_directories(Moksha_Server PUBLIC ${PROJECT_SOURCE_DIR})

# PUBLIC, so a game's server can use shared and logging directly.
target_link_libraries(Moksha_Server PUBLIC Moksha_Options Moksha_Shared Moksha_Logging)
