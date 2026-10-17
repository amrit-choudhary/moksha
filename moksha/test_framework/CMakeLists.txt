# Minimal unit test framework, linked by test executables (the engine's and games').

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(Moksha_TestFramework STATIC ${SOURCES})

target_include_directories(Moksha_TestFramework PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(Moksha_TestFramework PUBLIC Moksha_Options)
