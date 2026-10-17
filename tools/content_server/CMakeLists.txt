# Content server: serves the files listed in dlc/manifest.json (next to the executable) to clients over TCP.
# The dlc folder comes from the game; test_dlc/ is a minimal one for trying the server out.

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(Moksha_ContentServer ${SOURCES})

target_link_libraries(Moksha_ContentServer PRIVATE Moksha_Shared)
