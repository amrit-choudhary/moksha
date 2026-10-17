# Code shared by clients, servers and tools: math, data structures, networking, physics, file I/O, time.

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# C sources too (cJSON).
file(GLOB_RECURSE CSOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
list(APPEND SOURCES ${CSOURCES})

add_library(Moksha_Shared STATIC ${SOURCES})

target_include_directories(Moksha_Shared PUBLIC ${PROJECT_SOURCE_DIR})

# PUBLIC, so anything linking shared can use logging directly.
target_link_libraries(Moksha_Shared PUBLIC Moksha_Options Moksha_Logging)

if(WIN32)
    # Winsock, used by the sockets. PUBLIC so every program linking shared gets it too.
    target_link_libraries(Moksha_Shared PUBLIC Ws2_32)
endif()
