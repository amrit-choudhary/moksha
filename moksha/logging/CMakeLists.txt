# Logging library.

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(Moksha_Logging STATIC ${SOURCES})

target_include_directories(Moksha_Logging PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(Moksha_Logging PUBLIC Moksha_Options)
