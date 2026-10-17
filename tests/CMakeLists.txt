# Engine unit tests. Sources are compiled into the executable so TEST() registrations are kept.

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(Moksha_Tests ${SOURCES})

target_link_libraries(Moksha_Tests PRIVATE Moksha_TestFramework Moksha_Shared)
